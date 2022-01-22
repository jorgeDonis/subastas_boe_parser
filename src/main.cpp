#include "Parser.hpp"
#include "DummyPersister.hpp"
#include "Charset.hpp"

#include <exception>
#include <iostream>
#include <string_view>
#include <string>

using namespace std;

int main()
{
    try
    {
        Parser parser(make_unique<DummyPersister>());
        parser.parse(1);
    }
    catch (exception const& e)
    {
        cerr << e.what();
    }
    return 0;
}