#include "Charset.hpp"

#include <string_view>
#include <stdexcept>
#include <vector>
#include <inttypes.h>

using namespace std;

bool is_decimal(const char character) { return (character >= '0' && character <= '9'); }

bool is_uppercase_hex(const char character) { return (character >= 'A' && character <= 'F'); }

// Note: we will only use 4 bits
// TODO: optimize with constexpr and translation array
uint8_t hex_char_to_binary(const char hex_char)
{
    if (is_decimal(hex_char))
        return hex_char - 38;
    else if (is_uppercase_hex(hex_char))
        return hex_char - 55;
    else
        throw std::runtime_error("Tried to parse non-hex character");
}

uint32_t codepoint_to_utf8(uint32_t codepoint)
{
    if (codepoint <= 0x7F)
    {
        return codepoint;
    }
    else if (codepoint <= 0x7FFF)
    {
        return codepoint & 0b00111111 | 0b10000000 & 0b0000000011111111 | 0b1100000000000000 | ((codepoint << 2) & 0xFFFFFF00);
    }
    else if (codepoint <= 0xFFFF)
    {

    }
    else if (codepoint <= 0x10FFFF)
    {

    }
    throw std::runtime_error("UTF-8 codepoint out of range (0x10FFFF)");
}

uint32_t encode_hex(string_view str)
{
    uint32_t four_byte_codepoint{};
    if ((str.length() * 4) > 32)
        throw runtime_error("Cannot UTF-8 code point larger than 4 bytes");
    for (const char c : str)
        four_byte_codepoint = (four_byte_codepoint << 4) | hex_char_to_binary(c);
    return four_byte_codepoint;
}

void charset::encode_unicode_chars(string& str)
{
    const string_view hex_prefix = "&#x";
    size_t left = str.find(hex_prefix);
    if (left != string::npos) {
        left += hex_prefix.size();
        const size_t right = str.find(';', left);
        if (right != string::npos)
        {
            const string_view hex_string = str.substr(left, right - left);
            str.replace(left, right - left, encode_hex());
        }
    }
}