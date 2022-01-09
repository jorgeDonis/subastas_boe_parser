#pragma once

#include <curlpp/Info.hpp>
#include <string>
#include <sstream>

namespace curlutils
{
    using namespace std;
    static inline string getResponseBody(curlpp::Easy const& handler)
    {
        stringstream ss;
        ss << handler;
        return ss.str();
    }
}