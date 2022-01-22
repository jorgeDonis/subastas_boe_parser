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
    for_each_line(table_element, [base_leading_whitespaces, &parsed_table, &key](const string_view line)
    {
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


template
<typename FunctionType>
void for_each_a_tag(const string_view html, FunctionType f)
{
    const static string_view links_title = R"(<ul class="enlaces">)";
    size_t left{}, right{};
    left = html.find(links_title);
    size_t box_end = html.find("</ul>", left);
    while (left != string::npos && left < box_end && right != string::npos)
    {
        left = html.find("<a href=", left);
        if (left != string::npos && left < box_end)
        {
            const size_t right = html.find("</a>", left);
            if (right != string::npos)
            {
                f(html.substr(left + 3, right - left + 1));
                left = right + 4;
            }
        }
    }
}

/**
 * @param str The original string
 * @param left_del Left delimiter string
 * @param right_del Right delimiter string
 * @return The substring, EXCLUDING the delimiters
 * @throws RuntimeException if the substring cannot be found
 */
string_view substr(const string_view str, const string_view left_del, const string_view right_del)
{
    size_t left{}, right{};
    left = str.find(left_del);
    if (left != string::npos)
    {
        left += left_del.size();
        right = str.find(right_del, left);
        if (right != string::npos)
            return str.substr(left, right - left);
    }
    throw runtime_error("Could not find substring. Original string:\n" + string(str) + 
    "\n Left delimiter: \n" + string(left_del) + "\n Right delimiter: \n" + string(right_del));
}

// Returns a version of 'str' where every occurrence of
// 'find' is substituted by 'replace'.
string replace_all(const string_view str, const string_view find, const string_view replace)
{
    string result;
    size_t find_len = find.size();
    size_t pos, from = 0;
    while ((pos = str.find(find, from)) != string::npos)
    {
        result.append(str, from, pos - from);
        result.append(replace);
        from = pos + find_len;
    }
    result.append(str, from, string::npos);
    return result;
}

vector<pair<string, string>> HTMLParsing::get_attachment_links(const string_view html)
{
    vector<pair<string, string>> attachments;
    for_each_a_tag(html, [&attachments](const string_view a_tag)
    {
        // a_tag is "href="link" target="_blank">documentName</a>
        string document_name = string(substr(a_tag, R"(target="_blank">)", "</a>"));
        charset::encode_unicode_chars(document_name);
        string_view link = substr(a_tag, R"(href=")", R"(" target="_blank">)");
        string formatted_link = replace_all(link, "&amp;", "&");
        formatted_link = "/reg/" + formatted_link;
        attachments.emplace_back(pair{document_name, formatted_link});
    });
    return attachments;
}
