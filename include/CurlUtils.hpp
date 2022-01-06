#pragma once

#include <curlpp/Info.hpp>
#include <string>
#include <string_view>
#include <sstream>

namespace curlutils
{
    /**
     * @brief Tries to return the HTTP status code from the response. If there's none, an
     * exception is thrown
     */
    static inline uint16_t getResponseCode(curlpp::Easy const& handler)
    {
        long status_code;
        curlpp::InfoGetter::get(handler, CURLINFO_RESPONSE_CODE, status_code);
        if (!status_code)
            throw std::runtime_error("Could not get HTTP status code: " + status_code);
        return status_code;
    }

    static inline const std::string_view getResponseBody(curlpp::Easy const& handler)
    {
        std::stringstream ss;
        ss << handler;
        return ss.str();
    }

    /**
     * @brief Tries to get the effective (finally used) URL. If there's none, an
     * exception is thrown
     */
    static inline std::string getEffectiveURL(curlpp::Easy const& handler)
    {
        char* effective_url;
        curlpp::InfoGetter::get(handler, CURLINFO_EFFECTIVE_URL, effective_url);
        if (effective_url == nullptr)
            throw std::runtime_error("Could not get effective URL");
        return effective_url;
    }

    static inline void add_cookie(curlpp::Easy& handle, std::string const& name, std::string const& value)
    {
    }
}