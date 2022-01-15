#include <string_view>
#include <json.hpp>

namespace HTMLParsing
{
    /**
     * @brief Finds the <table> tag from the HTML. It then parses all 
     * <th> + <td> tuples into a list which is returned as a json, i.e. :
     * {
     *   {"Key": "value"},
     *   {"Key" :"value"},
     *   ( ... )
     * }
     */
    nlohmann::json parse_table(const std::string_view html);
}