#pragma once

#include <string>
#include <list>
#include <utility>

template<class K, class V>
class Container {
public:
    Container() {}

    virtual void Insert(const K &key, const V &value) = 0;

    virtual V Search(const K &key, bool &found) = 0;

    virtual bool Delete(const K &key) = 0;

    virtual void Display() = 0;

    virtual void Search(const K &a, const K &b, std::list <std::pair<K, V>> &list) = 0;
};