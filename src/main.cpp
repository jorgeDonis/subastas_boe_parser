#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>

#include <iostream>


// Callback must be declared static, otherwise it won't link...
size_t WriteMemoryCallback(char *ptr, size_t size, size_t nmemb)
{
    std::cout << std::string(ptr);

    // return the real size of the buffer...
    return size * nmemb;
}

int main()
{
    curlpp::initialize(CURL_GLOBAL_SSL);
    curlpp::Easy request;
    request.setOpt(curlpp::Options::Url("https://subastas.boe.es/id/login.php"));
    request.setOpt(curlpp::Options::HttpPost
    (
        curlpp::Forms
        {
            new curlpp::FormParts::Content("usuario", "jorgito.donis.del.alamo7@gmail.com"),
            new curlpp::FormParts::Content("password", "no se sabe"),
            new curlpp::FormParts::Content("conectar", "Conectar")
        }
    ));
    request.setOpt(curlpp::Options::HeaderFunction(curlpp::Types::WriteFunctionFunctor(WriteMemoryCallback)));
    request.perform();
    curlpp::terminate();
    return 0;
}