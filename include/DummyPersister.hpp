#pragma once

#include <AuctionPersister.hpp>

class DummyPersister : public AuctionPersister
{
    public:
        void persist(std::string const &json_data);
};