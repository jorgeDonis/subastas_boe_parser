#pragma once

#include "AuctionPersister.hpp"

#include <curlpp/Options.hpp>
#include <inttypes.h>
#include <memory>
#include <stdlib.h>
#include <string_view>


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
        static const inline std::string BOE_BASE_URI            =   "https://subastas.boe.es"   ;
        static const inline std::string CREDENTIALS_FILEPATH    =   "../resources/login.txt"    ;

        static constexpr inline uint32_t MAX_AUCTIONS_PER_QUERY = 2000;
        static_assert(Parser::MAX_AUCTIONS_PER_QUERY >= 50);

        // Threads working on parsing specific auctions (one thread per auction) simultaneously
        static constexpr inline uint16_t MAX_WORKERS = 5;

        static const inline std::array<std::pair<const std::string, const std::string>, 42>
        FETCH_AUCTION_IDS_QUERY_PARAMETERS
        {{
            {"campo[0]"		    ,   "SUBASTA.ORIGEN"						},
            {"dato[0]"		    ,   ""										},
            {"campo[1]"		    ,   "SUBASTA.ESTADO"						},
            {"dato[1]"		    ,   ""										},
            {"campo[2]"		    ,   "BIEN.TIPO"								},
            {"dato[2]"		    ,   ""										},
            {"dato[3]"		    ,   ""										},
            {"campo[4]"		    ,   "BIEN.DIRECCION"						},
            {"dato[4]"		    ,   ""										},
            {"campo[5]"		    ,   "BIEN.CODPOSTAL"						},
            {"dato[5]"		    ,   ""										},
            {"campo[6]"		    ,   "BIEN.LOCALIDAD"						},
            {"dato[6]"		    ,   ""										},
            {"campo[7]"		    ,   "BIEN.COD_PROVINCIA"					},
            {"dato[7]"		    ,   ""										},
            {"campo[8]"		    ,   "SUBASTA.POSTURA_MINIMA_MINIMA_LOTES"	},
            {"dato[8]"		    ,   ""										},
            {"campo[9]"		    ,   "SUBASTA.NUM_CUENTA_EXPEDIENTE_1"		},
            {"dato[9]"		    ,   ""										},
            {"campo[10]"	    ,   "SUBASTA.NUM_CUENTA_EXPEDIENTE_2"		},
            {"dato[10]"		    ,   ""										},
            {"campo[11]"	    ,   "SUBASTA.NUM_CUENTA_EXPEDIENTE_3"		},
            {"dato[11]"		    ,   ""										},
            {"campo[12]"	    ,   "SUBASTA.NUM_CUENTA_EXPEDIENTE_4"		},
            {"dato[12]"		    ,   ""										},
            {"campo[13]"	    ,   "SUBASTA.NUM_CUENTA_EXPEDIENTE_5"		},
            {"dato[13]"		    ,   ""										},
            {"campo[14]"	    ,   "SUBASTA.ID_SUBASTA_BUSCAR"				},
            {"dato[14]"		    ,   ""										},
            {"campo[15]"	    ,   "SUBASTA.FECHA_FIN_YMD"					},
            {"dato[15][0]"	    ,   ""										},
            {"dato[15][1]"	    ,   ""										},
            {"campo[16]"	    ,   "SUBASTA.FECHA_INICIO_YMD"				},
            {"dato[16][0]"	    ,   ""										},
            {"dato[16][1]"	    ,   ""										},
            {"sort_field[0]"    ,   "SUBASTA.FECHA_FIN_YMD"					},
            {"sort_order[0]"    ,   "desc"									},
            {"sort_field[1]"    ,   "SUBASTA.FECHA_FIN_YMD"					},
            {"sort_order[1]"    ,   "asc"									},
            {"sort_field[2]"    ,   "SUBASTA.HORA_FIN"						},
            {"sort_order[2]"    ,   "asc"									},
            {"accion"		    ,   "Buscar"                                }
        }};

        static inline const std::string BASE_LIST_AUCTIONS_QUERY = []() -> auto
        {
            std::string query;
            for (auto const& pair : FETCH_AUCTION_IDS_QUERY_PARAMETERS)
                query += pair.first + '=' + pair.second + '&';
            query.pop_back(); //remove last '&'
            return query;
        }
        ();

        static inline const std::string get_list_auctions_query(uint32_t no_auctions);

        //We will use the same session token for all requests
        static inline std::string SESSION_TOKEN;
        std::unique_ptr<AuctionPersister> persister;

        //Updates the static session_token_variable
        static void open_boe_session();

        //Makes a request to sign out
        static void close_boe_session();

        /**
         * @brief Calls https://subastas.boe.es/endpoint with the previously fetched session cookie.
         * 
         * @param endpoint Must begin with '/'
         * @return std::string the repsonse body
         */
        static inline std::string http_get_boe(std::string const& endpoint);

        static inline std::vector<std::string> get_auction_ids(uint32_t no_auctions);

        static inline curlpp::Options::Cookie get_session_cookie();

        /**
         * @brief Called once for each header in the response
         * @return size_t Calculation necessary for libcurl
         */
        static inline size_t header_callback_get_session_token(char* ptr, size_t size, size_t nmemb);
    public:
        Parser(std::unique_ptr<AuctionPersister>&& persister);
        ~Parser();
        
        //Will parse all the data from the first no_auctions available AND persist them
        void parse(uint32_t no_auctions);
};