#pragma once

#include <string>

namespace charset
{
    // This method compresses hex strings into their unicode (UTF-8) binary
    // representations, e.g. :
    // dep&#xF3;sito => depósito
    void encode_unicode_chars(std::string& str);
}