#include "Parser.hpp"
#include "DummyPersister.hpp"

#include <exception>
#include <iostream>

int main()
{
    try
    {
        Parser parser(std::make_unique<DummyPersister>());
        parser.parse(20);
    }
    catch (std::exception const& e)
    {
        std::cerr << e.what();
    }
    return 0;
}