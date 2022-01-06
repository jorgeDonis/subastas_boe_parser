#include "Parser.hpp"
#include "Credentials.hpp"
#include "CurlUtils.hpp"
#include "Utils.hpp"

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Info.hpp>

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
    cout << "Fetching session ... (calling "  << curlutils::getEffectiveURL(handle) << ")\n";
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
    const uint16_t status_code = curlutils::getResponseCode(handle);
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

const vector<string_view> parse_auction_ids(string_view const html_body)
{
    vector<string_view> auction_ids;
    size_t offset{}, left_del{}, right_del{};
    static const string_view left_sequence("idSub=");
    static const string_view right_sequence("&amp;");
    while (true)
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

//Recieves the HTTP response and does nothing
size_t emptyMemoryCallBack(char* data, size_t size, size_t nmemb)
{
    //Mandatory value for libcurl
    return size * nmemb;
}

vector<string_view> Parser::get_auction_ids(uint32_t const no_auctions)
{
    vector<string_view> auction_ids;
    uint32_t remaining_auctions = no_auctions;
    while (remaining_auctions != 0)
    {
        uint32_t auctions_to_query = min(remaining_auctions, MAX_AUCTIONS_PER_QUERY);
        curlpp::Easy handle;
        auto const& url = BOE_BASE_URI + "/reg/subastas_ava.php?" + get_list_auctions_query(auctions_to_query);
        handle.setOpt(curlpp::Options::Url(url));
        handle.setOpt(get_session_cookie());
        handle.setOpt(curlpp::Options::WriteFunction(emptyMemoryCallBack));
        handle.perform();
        auto const& response_body = curlutils::getResponseBody(handle);
        utils::move_append(parse_auction_ids(response_body), auction_ids);
        remaining_auctions -= auctions_to_query;
    }
    return auction_ids;
}

void Parser::parse(uint32_t no_auctions)
{
    auto const& auction_ids = get_auction_ids(no_auctions);
}
