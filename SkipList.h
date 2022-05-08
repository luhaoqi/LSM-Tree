#pragma once

#include <vector>
#include <climits>
#include <time.h>
#include <string>
#include <list>
#include <utility>
#include <iostream>
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
    SKNode(const uint64_t &_key, const SKNodeType &_type)
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

double SkipList::my_rand() {
//    s = (16807 * s) % 2147483647ULL;
//    return (s + 0.0) / 2147483647ULL;
    return (rand() + 0.0) / (RAND_MAX + 1);
}

int SkipList::randomLevel() {
    int result = 1;
    while (result < MAX_LEVEL && my_rand() < P) {
        ++result;
    }
    cur_maxlevel = max(cur_maxlevel, result);
    return result;
}

void SkipList::Insert(uint64_t key, std::string value) {
    std::vector < SKNode * > update;
    for (int i = 0; i < MAX_LEVEL; i++)
        update.push_back(nullptr);
    SKNode *p = head;
    for (int i = MAX_LEVEL - 1; i >= 0; i--) {
        while (p->forwards[i] != SkipList::NIL && p->forwards[i]->key < key) {
            p = p->forwards[i];
        }
        update[i] = p;
    }
    p = p->forwards[0];
    if (p->key == key) p->val = value;
    else {
        int v = randomLevel();
        SKNode *x = new SKNode(key, value, NORMAL);
        for (int i = 0; i < v; i++) {
            x->forwards[i] = update[i]->forwards[i];
            update[i]->forwards[i] = x;
        }
    }
}

string SkipList::Search(uint64_t key) {
    SKNode *p = head;
    for (int i = MAX_LEVEL - 1; i >= 0; i--) {
//        if (p == head) std::cout << i + 1 << "," << "h" << " ";
//        else std::cout << i + 1 << "," << p->key << " ";
        while (p->forwards[i] != SkipList::NIL && p->forwards[i]->key < key) {
//            std::cout << i + 1 << "," << p->forwards[i]->key << " ";
            p = p->forwards[i];
        }
    }
    p = p->forwards[0];
    return (p->key == key)?p->val:string("");
//    if (p == SkipList::NIL) std::cout << 1 << "," << "N" << " ";
//    else std::cout << 1 << "," << p->key << " ";
//    if (p->key != key) std::cout << "Not Found" << std::endl;
//    else std::cout << p->val << std::endl;
}

void SkipList::Search(uint64_t a, uint64_t b, std::list <std::pair<uint64_t, std::string>> &list) {
    list.clear();
    SKNode *p = head;
    for (int i = MAX_LEVEL - 1; i >= 0; i--) {
//        if (p == head) std::cout << i+1 << "," << "h" << " ";
//        else std::cout << i+1 << "," << p->key << " ";
        while (p->forwards[i] != SkipList::NIL && p->forwards[i]->key < a) {
//            std::cout << i+1 << "," << p->forwards[i]->key << " ";
            p = p->forwards[i];
        }
    }
    p = p->forwards[0];
    while (p->key <= b) {
        list.push_back(make_pair(p->key, p->val));
        p = p->forwards[0];
    }
//    if (p == SkipList::NIL) std::cout << 1 << "," << "N" << " ";
//    else std::cout << 1 << "," << p->key << " ";
//    if (p->key != key) std::cout << "Not Found" << std::endl;
//    else std::cout << p->val << std::endl;
}

bool SkipList::Delete(uint64_t key) {
    std::vector < SKNode * > update;
    for (int i = 0; i < MAX_LEVEL; i++)
        update.push_back(nullptr);
    SKNode *p = head;
    for (int i = MAX_LEVEL - 1; i >= 0; i--) {
        while (p->forwards[i] != SkipList::NIL && p->forwards[i]->key < key) {
            p = p->forwards[i];
        }
        update[i] = p;
    }
    p = p->forwards[0];
    if (p->key != key) return false;
    else {
        for (int i = 0; i < MAX_LEVEL; i++) {
            if (update[i]->forwards[i] != p) break;
            update[i]->forwards[i] = p->forwards[i];
        }
        delete p;
        return true;
    }
}

void SkipList::Display() {
    for (int i = MAX_LEVEL - 1; i >= 0; --i) {
        std::cout << "Level " << i + 1 << ":h";
        SKNode *node = head->forwards[i];
        while (node->type != SKNodeType::NIL) {
            std::cout << "-->(" << node->key << "," << node->val << ")";
            node = node->forwards[i];
        }

        std::cout << "-->N" << std::endl;
    }
}
