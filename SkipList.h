#pragma once

#include <vector>
#include <climits>
#include <time.h>
#include <string>
#include <list>
#include <utility>
using namespace std;
#include "Container.h"

#define MAX_LEVEL 12

enum SKNodeType
{
    HEAD = 1,
    NORMAL,
    NIL
};

struct SKNode
{
    uint64_t key;
    string val;
    SKNodeType type;
    std::vector<SKNode *> forwards;
    SKNode(uint64_t _key, SKNodeType _type)
            : key(_key), type(_type)
    {
        for (int i = 0; i < MAX_LEVEL; ++i)
        {
            forwards.push_back(nullptr);
        }
    }
    SKNode(uint64_t _key, string _val, SKNodeType _type)
        : key(_key), val(_val), type(_type)
    {
        for (int i = 0; i < MAX_LEVEL; ++i)
        {
            forwards.push_back(nullptr);
        }
    }
};

class SkipList : public Container
{
private:
    int cur_maxlevel=0;
    SKNode *head;
    SKNode *NIL;
    unsigned long long s = 1;
    double my_rand();
    int randomLevel();
    const double P;

public:
    SkipList(double P_=0.5):P(P_)
    {
        head = new SKNode(0, SKNodeType::HEAD);
        NIL = new SKNode(INT_MAX, SKNodeType::NIL);
        for (int i = 0; i < MAX_LEVEL; ++i)
        {
            head->forwards[i] = NIL;
        }
    }
    void Insert(uint64_t key, std::string value);
    string Search(uint64_t key);
    void Search(uint64_t a, uint64_t b, std::list <std::pair<uint64_t, std::string>> &list);
    bool Delete(uint64_t key);
    void Display();
    ~SkipList()
    {
        SKNode *n1 = head;
        SKNode *n2;
        while (n1)
        {
            n2 = n1->forwards[0];
            delete n1;
            n1 = n2;
        }
    }
};
