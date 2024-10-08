#pragma once

#include <unordered_map>
#include <string>
using std::unordered_map;
using std::string;

template <typename T>
class DSPParameters
{
public:
    unordered_map<string, T> parameters;

    T operator[] (string key) {
        if (parameters.find(key) != parameters.end()) {
            return parameters[key];
        }
        else {
            return 1.0f;
        }
    }

    void set(string key, T value) {
        parameters[key] = value;
    }

};


