#include "Parser.hpp"
#include "Credentials.hpp"
#include "CurlUtils.hpp"

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
        static const string header_begin = "Set-Cookie: SESSID=";
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

Parser::Parser(AuctionPersister* persister) : persister(persister)
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

void Parser::parse(uint32_t no_auctions)
{
    cout << "parsed!" << '\n';
}
