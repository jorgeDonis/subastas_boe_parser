#pragma once

#include <string_view>
#include <json.hpp>
#include <string>
#include <vector>
#include <utility>

namespace HTMLParsing
{
    /**
     * Finds the <table> tag from the HTML. It then parses all 
     * <th> + <td> tuples into a list which is returned as a json, i.e. :
     * {
     *   {"Key": "value"},
     *   {"Key" :"value"},
     *   ( ... )
     * }
     */
    nlohmann::json parse_table(const std::string_view html);

    /**
     * Reads the html and returns a list like:
     *
     * [
     *    {'document_name',  'link'},
     *    {'document_name',  'link'}
     *    ( ... )
     * ]
     */
    std::vector<std::pair<std::string, std::string>> get_attachment_links(const std::string_view html);
}