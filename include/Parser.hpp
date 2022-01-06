#pragma once

#include "AuctionPersister.hpp"

#include <curlpp/Options.hpp>
#include <string>
#include <inttypes.h>
#include <memory>

/**
 * This class fetches info from auctions hosted at subastas.boe.es
 * and stores them according the auctionPersister
 * 
 * When creating a Parser object, a session will be created
 * When deleting a Parser object the session will be closed
 */
class Parser
{
    private:
        //We will use the same session token for all requests
        static inline std::string SESSION_TOKEN;

        std::unique_ptr<AuctionPersister> persister;

        //Updates the static session_token_variable
        static void open_boe_session();

        //Makes a request to sign out
        static void close_boe_session();

        static inline curlpp::Options::Cookie get_session_cookie();

        /**
         * @brief Called once for each header in the response
         * @return size_t Calculation necessary for libcurl
         */
        static inline size_t header_callback_get_session_token(char* ptr, size_t size, size_t nmemb);

        static const inline std::string BOE_BASE_URI            =   "https://subastas.boe.es"   ;
        static const inline std::string CREDENTIALS_FILEPATH    =   "../resources/login.txt"    ;
    public:
        Parser(AuctionPersister* persister);
        ~Parser();
        
        //Will parse all the data from the first no_auctions available AND persist them
        void parse(uint32_t no_auctions);
};