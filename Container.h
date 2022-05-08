#pragma once

#include <string>
#include <list>
#include <utility>

class Container {
public:
    Container() {}

    virtual void Insert(uint64_t key, std::string value) = 0;

    virtual std::string Search(uint64_t key) = 0;

    virtual bool Delete(uint64_t key) = 0;

    virtual void Display() = 0;

    virtual void Search(uint64_t a, uint64_t b, std::list <std::pair<uint64_t, std::string>> &list) = 0;
};