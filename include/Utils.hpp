#pragma once

#include <vector>
#include <string>
#include <random>

namespace utils
{
    using namespace std;

    template <class T>
    inline void move_append(vector<T>&& source, vector<T>& destination)
    {
        if (destination.empty())
            destination = move(source);
        else
            destination.insert(end(destination), make_move_iterator(begin(source)),
                               make_move_iterator(end(source)));
    }

    template <class T>
    inline void move_append(vector<T>& source, vector<T>& destination)
    {
        if (destination.empty())
            destination = move(source);
        else
            destination.insert(end(destination), make_move_iterator(begin(source)),
                               make_move_iterator(end(source)));
    }

    inline size_t get_random_number(size_t from, size_t to)
    {
        static random_device device;
        static mt19937 rng(device());
        uniform_int_distribution<mt19937::result_type> distribution(from, to);
        return distribution(rng);
    }

    inline string to_string(vector<string> const& v)
    {
        string s;
        for (auto const& a : v)
            s += a + '\n';
        s.erase(s.cend() - 1);
        return s;
    }

    
}