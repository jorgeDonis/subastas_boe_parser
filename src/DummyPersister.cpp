#include "DummyPersister.hpp"

#include <iostream>

void DummyPersister::persist(std::string const &json_data)
{
    std::cout << json_data << '\n';
}