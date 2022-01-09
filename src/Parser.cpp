#include "Parser.hpp"
#include "Credentials.hpp"
#include "CurlUtils.hpp"
#include "Utils.hpp"
#include "ThreadPool.hpp"

#include <curlpp/Easy.hpp>
#include <curlpp/Info.hpp>
#include <curlpp/Infos.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>
#include <thread>
#include <mutex>
#include <chrono>

using namespace std;

size_t Parser::header_callback_get_session_token(char* ptr, size_t size, size_t nmemb)
{
    if (SESSION_TOKEN.empty())
    {
        auto const& header = string(ptr);
        static const string_view header_begin = "Set-Cookie: SESSID=";
        size_t left_del = header.find(header_begin);
        if (left_del != string::npos) {
            left_del = left_del + header_begin.size();
            size_t right_del = header.find(';', left_del);
            SESSION_TOKEN = header.substr(left_del, right_del - left_del);
        }
    }
    return size * nmemb;
}

void Parser::open_boe_session()
{
    curlpp::Easy handle;
    handle.setOpt(curlpp::Options::Url(BOE_BASE_URI + "/id/login.php"));
    Credentials creds(CREDENTIALS_FILEPATH);
    handle.setOpt(curlpp::Options::HttpPost(curlpp::Forms { new curlpp::FormParts::Content("usuario", creds.user),
        new curlpp::FormParts::Content("password", creds.password),
        new curlpp::FormParts::Content("conectar", "Conectar") }));
    handle.setOpt(curlpp::Options::HeaderFunction(Parser::header_callback_get_session_token));
    cout << "Fetching session ... (calling "  << curlpp::Infos::EffectiveUrl::get(handle) << ")\n";
    handle.perform();
    cout << "Got session " << SESSION_TOKEN << '\n';
}

Parser::Parser(unique_ptr<AuctionPersister>&& persister) : persister(move(persister))
{
    curlpp::initialize(CURL_GLOBAL_SSL);
    open_boe_session();
}

curlpp::Options::Cookie Parser::get_session_cookie()
{
    return curlpp::options::Cookie("SESSID=" + SESSION_TOKEN);
}

void Parser::close_boe_session()
{
    curlpp::Easy handle;
    handle.setOpt(curlpp::Options::Url(BOE_BASE_URI + "/reg/desconectar.php"));
    handle.setOpt(get_session_cookie());
    cout << "Closing session " << SESSION_TOKEN << '\n';
    handle.perform();
    const uint16_t status_code = curlpp::Infos::ResponseCode::get(handle);
    if (status_code != 302)
    {
        throw runtime_error("Error while closing sesion for " + SESSION_TOKEN + 
                            " . Got status code: " + to_string(status_code));
    }
    cout << "Succesfully closed session for " << SESSION_TOKEN << '\n';
}

Parser::~Parser()
{
    close_boe_session();
    curlpp::terminate();
}

const string Parser::get_list_auctions_query(uint32_t no_auctions)
{
    return BASE_LIST_AUCTIONS_QUERY + "&page_hits=" + to_string(no_auctions);
}

vector<string> parse_auction_ids(string_view const html_body, const uint32_t no_auctions)
{
    vector<string> auction_ids;
    size_t offset{}, left_del{}, right_del{};
    static const string_view left_sequence("idSub=");
    static const string_view right_sequence("&amp;");
    while (auction_ids.size() < no_auctions)
    {
        left_del = html_body.find(left_sequence, offset);
        if (left_del == string::npos)
            break;
        right_del = html_body.find(right_sequence, left_del + left_sequence.size());
        left_del = left_del + left_sequence.size();
        auction_ids.emplace_back(html_body.substr(left_del, right_del - left_del));
        left_del = html_body.find(left_sequence, right_del + right_sequence.size());
        offset = left_del + left_sequence.size();
    }
    return auction_ids;
}

string get_query_id(const string_view html_body)
{
    const string_view left_del = "idBus=";
    const string_view right_del = "\" ";
    size_t left_in{0}, right_in{0};
    left_in = html_body.find(left_del) + left_del.size();
    right_in = html_body.find(right_del, left_in);
    return string(html_body.substr(left_in, right_in - left_in));
}

// Recieves the HTTP response and does nothing
size_t emptyMemoryCallBack(char* data, size_t size, size_t nmemb)
{
    // Mandatory value for libcurl
    return size * nmemb;
}

string Parser::http_get_boe(std::string const& endpoint)
{
    curlpp::Easy handle;
    auto const& url = BOE_BASE_URI + endpoint;
    handle.setOpt(curlpp::Options::Url(url));
    handle.setOpt(curlpp::Options::Cookie(get_session_cookie()));
    handle.setOpt(curlpp::Options::WriteFunction(emptyMemoryCallBack));
    handle.perform();
    return curlutils::getResponseBody(handle);
}

vector<string> Parser::get_auction_ids(uint32_t const no_auctions)
{
    vector<string> auction_ids;
    auction_ids.reserve(no_auctions);

    //First call (first page)
    uint32_t queried_actions = min(no_auctions, MAX_AUCTIONS_PER_QUERY);
    string const& endpoint = "/reg/subastas_ava.php?" + get_list_auctions_query(queried_actions);
    string const& response_body = http_get_boe(endpoint);
    utils::move_append(parse_auction_ids(response_body, no_auctions), auction_ids);
    cout << "Fetched auction id's from 0 to " << queried_actions << '\n';
    string const& query_id = get_query_id(response_body);

    //Additional calls (from page number 2 onwards)
    vector<thread> thread_pool;
    mutex results_vector_mutex;
    for (uint32_t from_auction = MAX_AUCTIONS_PER_QUERY; from_auction < no_auctions; from_auction += MAX_AUCTIONS_PER_QUERY)
    {
        uint32_t to_auction = min(no_auctions - from_auction, MAX_AUCTIONS_PER_QUERY);
        thread_pool.emplace_back([query_id, from_auction, to_auction,
                                  &results_vector_mutex, no_auctions, &auction_ids]
        {
            string const& endpoint = "/reg/subastas_ava.php?accion=Mas&id_busqueda=" + query_id 
                                    + "-" + to_string(from_auction) + "-" + to_string(to_auction);
            size_t ms_to_sleep = utils::get_random_number(2 * to_auction, 8 * to_auction);
            this_thread::sleep_for(chrono::duration<size_t, milli>(ms_to_sleep));
            string const& response_body = http_get_boe(endpoint);
            lock_guard<mutex> lock(results_vector_mutex);
            utils::move_append(parse_auction_ids(response_body, to_auction), auction_ids);
            cout << "Fetched auction id's from " << from_auction << " to " << from_auction + to_auction << '\n';
        });
    }
    for (auto& t : thread_pool) { t.join(); }

    return auction_ids;
}

thread build_thread(function<void(void)> f)
{
    return thread(f);
}

void Parser::parse(uint32_t no_auctions)
{
    cout << "Fetcing auction ids ..." << '\n';
    auto const& auction_ids = get_auction_ids(no_auctions);
    cout << "Got " << auction_ids.size() << " auction ids\n";
    mutex stdout_mutex;
    ThreadPool pool(MAX_WORKERS);
    for (const string_view auction_id : auction_ids)
    {
        pool.addWork([&stdout_mutex, auction_id]
        {
            stdout_mutex.lock();
            cout << "Parseando " << auction_id << " ... \n";
            stdout_mutex.unlock();
        });
    }
    pool.execute();
}
