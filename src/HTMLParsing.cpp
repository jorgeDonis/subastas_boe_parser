#include "HTMLParsing.hpp"
#include "Charset.hpp"

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
    return i;
}

// Removes anything enclosed in '<>'
string remove_html_tags(const string_view str)
{
    string res;
    res.reserve(str.size());
    bool skipping = false;
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (skipping)
        {
            if (str[i] == '>')
                skipping = false;
        }
        else 
        {
            if (str[i] == '<')
                skipping = true;
            else
                res += str[i];
        }
    }
    return res;
}


json HTMLParsing::parse_table(const string_view html)
{
    json parsed_table;
    //The attribute's name (a <th> element)
    string key;
    auto const [table_element, base_leading_whitespaces] = find_table_element(html);
    for_each_line(table_element, [base_leading_whitespaces, &parsed_table, &key](const string_view line) {
        uint16_t leading_whitespaces = no_leading_whitespaces(line);
        // all <tr> and <th> elements are 4 spaces more indented than the parent <table>
        if (leading_whitespaces >= (base_leading_whitespaces + 4))
        {
            string processed_line = remove_html_tags(line.substr(leading_whitespaces));
            if (!processed_line.empty())
            {
                charset::encode_unicode_chars(processed_line);
                if (key.empty())
                    key = processed_line;
                else
                {
                    parsed_table[key] = processed_line;
                    key.clear();
                }
            }
        }
    });
    return parsed_table;
}


vector<pair<string, string>> HTMLParsing::get_attachment_links(const string_view html)
{
    vector<pair<string, string>> attachments;
    constexpr inline string_view info_box_title = "<h4>Informaci&#xF3;n complementaria de la subasta</h4>";


    return attachments;
}
