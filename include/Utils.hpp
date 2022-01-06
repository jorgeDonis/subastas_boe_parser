#pragma once

#include <vector>

namespace utils
{
    using namespace std;
    template <typename T> inline void move_append(vector<T> source, vector<T>& destination)
    {
        if (destination.empty())
            destination = move(source);
        else
            destination.insert(end(destination), make_move_iterator(begin(source)),
                               make_move_iterator(end(source)));
    }
}