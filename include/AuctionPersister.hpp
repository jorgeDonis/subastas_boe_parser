#pragma once

#include <string>

class AuctionPersister
{
    public:
        virtual void persist(std::string const& json_data) = 0;
        virtual ~AuctionPersister() = 0;
};