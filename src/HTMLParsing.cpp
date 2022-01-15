#include "HTMLParsing.hpp"

#include <tuple>

using namespace std;
using namespace nlohmann;

template <typename FunctionType>
void for_each_line(const string_view str, FunctionType f)
{
    size_t line_begin = 0;
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] == '\n')
        {
            f(str.substr(line_begin, i - line_begin));
            line_begin = i + 1;
        }
    }
}

auto find_table_element(const string_view html)
{
    const size_t table_begin = html.find("<table>");

    // Find out how many spaces precede <table> (i.e. , its indentation level)
    size_t base_leading_whitespaces = 0;
    while (html[(table_begin - 1) - ++base_leading_whitespaces] != '\n');

    const size_t table_end = html.find("</table>", table_begin);
    const string_view table_element = html.substr(table_begin, table_end - table_begin);
    return tuple { table_element, base_leading_whitespaces };
}

uint16_t no_leading_whitespaces(const string_view line)
{
    uint16_t i = 0;
    while (line[i] == ' ') { ++i; }
    return (i - 1);
}

json HTMLParsing::parse_table(const string_view html)
{
    auto const [table_element, base_leading_whitespaces] = find_table_element(html);
    for_each_line(table_element, [](const string_view line) {
        uint16_t leading_whitespace = no_leading_whitespaces(line);
        int a = 2;
    });
}