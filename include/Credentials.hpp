#pragma once

#include <string>
#include <fstream>
#include <stdexcept>

//Credentials used to access additional info through subastas.boe.es
struct Credentials
{
    std::string user;
    std::string password;
    
    Credentials(std::string const& cred_filepath)
    {
        std::ifstream ifstream(cred_filepath);
        if (ifstream.is_open())
        {
            ifstream >> user >> user >> password >> password;
            ifstream.close();
        }
        else
        {
            throw std::invalid_argument("Could not open " + cred_filepath);
        }
    }
};