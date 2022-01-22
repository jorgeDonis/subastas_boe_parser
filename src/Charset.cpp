#include "Charset.hpp"

#include <string_view>
#include <stdexcept>
#include <array>
#include <tuple>
#include <inttypes.h>

using namespace std;

bool constexpr is_decimal(const char character) { return (character >= '0' && character <= '9'); }

bool constexpr is_uppercase_hex(const char character) { return (character >= 'A' && character <= 'F'); }

bool constexpr is_lowecase_hex(const char character) { return (character >= 'a' && character <= 'f'); }

constexpr array<uint8_t, 128> HEX_CHAR_TO_BINARY = []() {
    array<uint8_t, 128> a{};
    for (uint8_t i = 0; i < 128; ++i)
    {
        if (is_decimal(i))
            a[i] = i - '0';
        else if (is_uppercase_hex(i))
            a[i] = i - 'A' + 10;
        else if (is_lowecase_hex(i))
            a[i] = i - 'a' + 10;
        else
            a[i] = -1;
    }
    return a;
}();

uint32_t codepoint_to_utf8(uint32_t codepoint)
{
    if (codepoint <= 0x7F)
    {
        return codepoint;
    }
    else if (codepoint <= 0x7FF)
    {
        uint32_t res = codepoint & 0b00111111;
        res |= 0b10000000;
        res &= 0xFF;
        res |= 0b1100000000000000;
        res |= (codepoint << 2) & 0b00000000000000000001111100000000;
        return res;
    }
    else if (codepoint <= 0xFFFF)
    {
        uint32_t res = codepoint & 0b00111111;
        res |= 0b10000000;
        res &= 0xFF;
        res |= 0b1000000000000000;
        res |= (codepoint << 2) & 0b00000000000000000011111100000000;
        res &= 0xFFFF;
        res |= 0b111000000000000000000000;
        res |= (codepoint << 4) & 0b00000000000011110000000000000000;
        return res;
    }
    else if (codepoint <= 0x10FFFF)
    {
        uint32_t res = codepoint & 0b00111111;
        res |= 0b10000000;
        res &= 0xFF;
        res |= 0b1000000000000000;
        res |= (codepoint << 2) & 0b00000000000000000011111100000000;
        res &= 0xFFFF;
        res |= 0b100000000000000000000000;
        res |= (codepoint << 4) & 0b00000000001111110000000000000000;
        res &= 0xFFFFFF;
        res |= 0b11110000000000000000000000000000;
        res |= (codepoint << 6) & 0b00000111000000000000000000000000;
        return res;
    }
    throw std::runtime_error("UTF-8 codepoint out of range (0x10FFFF)");
}

uint32_t decode_hex(string_view str)
{
    uint32_t four_byte_codepoint{};
    if ((str.length() * 4) > 32)
        throw runtime_error("Cannot have UTF-8 code point larger than 4 bytes");
    for (const char c : str)
        four_byte_codepoint = (four_byte_codepoint << 4) | HEX_CHAR_TO_BINARY[c];
    return four_byte_codepoint;
}

// Takes 4 bytes and returns a string with length [0, 4].
// If some byte from data is null, then the effective size of string
// will be lower than 32 bits. e.g. :
// 
//               32 bits                     24 bits (first byte is skipped)
//   0    0    A    1    7    F   2    D
// 0000 0000 1010 0001 0111 1111 0010 1101    =>     [-95, 127, 45]
string binary_to_str(uint32_t data)
{
    array<char, 4> str_data;
    str_data[0] = data >> 24;
    str_data[1] = data >> 16;
    str_data[2] = data >> 8;
    str_data[3] = data;
    uint8_t first_non_null_byte = 0;
    while (str_data[first_non_null_byte] == 0) { ++first_non_null_byte; }
    return string(str_data.begin() + first_non_null_byte, str_data.end());
}

constexpr inline string_view HEX_PREFIX = "&#x";

template
<typename FunctionType>
void for_each_unicode_codepoint(string& str, FunctionType f)
{
    size_t left{}, right{};
    while (left != string::npos && right != string::npos)
    {
        left = str.find(HEX_PREFIX, right);
        if (left != string::npos)
        {
            left += HEX_PREFIX.size();
            right = str.find(';', left);
            if (right != string::npos)
            {
                f(str.substr(left, right - left), left, right);

                // After replacing the HTML unicode, the string may be up to 8 bytes shorter
                int signed_right = right - 8;
                signed_right = max(1, signed_right);
                right = signed_right;
            }
        }
    }
}

void charset::encode_unicode_chars(string& str)
{
    for_each_unicode_codepoint(str, [&str](const string_view hex_string, size_t left, size_t right)
    {
        const uint32_t binary_codepoint = decode_hex(hex_string);
        const uint32_t binary_utf8 = codepoint_to_utf8(binary_codepoint);
        const string_view utf8_str = binary_to_str(binary_utf8);
        left -= HEX_PREFIX.size();
        str.replace(left, right - left + 1, utf8_str);
    });
}