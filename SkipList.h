#pragma once

#include <vector>
#include <climits>
#include <ctime>
#include <string>
#include <list>
#include <utility>
#include <iostream>
#include "MurmurHash3.h"

using namespace std;

#include "Container.h"

enum SKNodeType {
    HEAD = 1,
    NORMAL,
    NIL
};

template<class K, class V, int MAX_LEVEL = 12>
struct SKNode {
    K key;
    V val;
    SKNodeType type;
    std::vector<SKNode *> forwards;

    SKNode(const K &_key, const SKNodeType &_type)
            : key(_key), type(_type) {
        for (int i = 0; i < MAX_LEVEL; ++i) {
            forwards.push_back(nullptr);
        }
    }

    SKNode(const K &_key, const V &_val, const SKNodeType &_type)
            : key(_key), val(_val), type(_type) {
        for (int i = 0; i < MAX_LEVEL; ++i) {
            forwards.push_back(nullptr);
        }
    }

    ~SKNode() = default;
};

template<class K, class V, int MAX_LEVEL = 12>
class SkipList : public Container<K, V> {
private:
//    int cur_maxlevel = 0;
    SKNode<K, V, MAX_LEVEL> *head;
    SKNode<K, V, MAX_LEVEL> *nil;
    unsigned long long s = 1;
    K minKey, maxKey;
//TODO:rand更换
    double my_rand() { return (rand() + 0.0) / ((double) RAND_MAX + 1); }

    int randomLevel() {
        int result = 1;
        while (result < MAX_LEVEL && my_rand() < P) {
            ++result;
        }
//        cur_maxlevel = max(cur_maxlevel, result);
        return result;
    }

    const double P;

public:
    SkipList(const K &_minKey, const K &_maxKey, const double &P_ = 0.5) : minKey(_minKey), maxKey(_maxKey), P(P_) {
        head = new SKNode<K, V, MAX_LEVEL>(_minKey, SKNodeType::HEAD);
        nil = new SKNode<K, V, MAX_LEVEL>(_maxKey, SKNodeType::NIL);
        for (int i = 0; i < MAX_LEVEL; ++i) {
            head->forwards[i] = nil;
        }
    }

    void clear() {
        SKNode<K, V, MAX_LEVEL> *n1 = head->forwards[0];
        SKNode<K, V, MAX_LEVEL> *n2;
        while (n1 != nil) {
            n2 = n1->forwards[0];
            delete n1;
            n1 = n2;
        }
        for (int i = 0; i < MAX_LEVEL; ++i) {
            head->forwards[i] = nil;
        }
    }

    ~SkipList() {
        clear();
        delete head;
        delete nil;
    }

    void Insert(const K &key, const V &value) {
        std::vector<SKNode<K, V, MAX_LEVEL> *> update;
        for (int i = 0; i < MAX_LEVEL; i++)
            update.push_back(nullptr);
        SKNode<K, V, MAX_LEVEL> *p = head;
        for (int i = MAX_LEVEL - 1; i >= 0; i--) {
            while (p->forwards[i] != nil && p->forwards[i]->key < key) {
                p = p->forwards[i];
            }
            update[i] = p;
        }
        p = p->forwards[0];
        if (p->key == key) p->val = value;
        else {
            int v = randomLevel();
            auto *x = new SKNode<K, V, MAX_LEVEL>(key, value, NORMAL);
            for (int i = 0; i < v; i++) {
                x->forwards[i] = update[i]->forwards[i];
                update[i]->forwards[i] = x;
            }
        }
    }

    V Search(const K &key, bool &found) {
        found = false;
        SKNode<K, V, MAX_LEVEL> *p = head;
        for (int i = MAX_LEVEL - 1; i >= 0; i--) {
            while (p->forwards[i] != nil && p->forwards[i]->key < key) {
                p = p->forwards[i];
            }
        }
        p = p->forwards[0];
        if (p->key == key) {
            found = true;
            return p->val;
        }
        return V();
    }

    void get_all_elm(std::list<std::pair<K, V>> &list) {
        list.clear();
        SKNode<K, V, MAX_LEVEL> *p = head;
        p = p->forwards[0];
        while (p != nil) {
            list.push_back(make_pair(p->key, p->val));
            p = p->forwards[0];
        }
    }


    void Search(const K &a, const K &b, std::list<std::pair<K, V>> &list) {
        list.clear();
        SKNode<K, V, MAX_LEVEL> *p = head;
        for (int i = MAX_LEVEL - 1; i >= 0; i--) {
            while (p->forwards[i] != nil && p->forwards[i]->key < a) {
                p = p->forwards[i];
            }
        }
        p = p->forwards[0];
        while (p->key <= b) {
            list.push_back(make_pair(p->key, p->val));
            p = p->forwards[0];
        }
    }

    bool Delete(const K &key) {
        std::vector<SKNode<K, V, MAX_LEVEL> *> update;
        for (int i = 0; i < MAX_LEVEL; i++)
            update.push_back(nullptr);
        SKNode<K, V, MAX_LEVEL> *p = head;
        for (int i = MAX_LEVEL - 1; i >= 0; i--) {
            while (p->forwards[i] != nil && p->forwards[i]->key < key) {
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

    void Display() {
        for (int i = MAX_LEVEL - 1; i >= 0; --i) {
            std::cout << "Level " << i + 1 << ":h";
            SKNode<K, V, MAX_LEVEL> *node = head->forwards[i];
            while (node->type != SKNodeType::NIL) {
                std::cout << "-->(" << node->key << "," << node->val << ")";
                node = node->forwards[i];
            }

            std::cout << "-->N" << std::endl;
        }
    }

    vector<bool> genBFVector(const int &BFSIZE) {
        vector<bool> result(BFSIZE, 0);
        SKNode<K, V, MAX_LEVEL> *p = head;
        p = p->forwards[0];
        unsigned int hash[4] = {0};
        while (p != nil) {
            MurmurHash3_x64_128(&(p->key), sizeof(p->key), 1, hash);
            for (size_t i = 0; i < 4; i++) {
                result[hash[i] % BFSIZE] = 1;
            }
            p = p->forwards[0];
        }
        return result;
    }

    /* return the num of Keys */
    int size() {
        int t = 0;
        SKNode<K, V, MAX_LEVEL> *p = head;
        p = p->forwards[0];
        while (p != nil) {
            t++;
            p = p->forwards[0];
        }
        return t;
    }

    /* return the size of Values */
    //TODO: 优化到过程中计算;
    int dataSize() {
        int t = 0;
        SKNode<K, V, MAX_LEVEL> *p = head;
        p = p->forwards[0];
        while (p != nil) {
            t += p->val.size() + 1;
            p = p->forwards[0];
        }
        return t;
    }

    K getMaxKey() {
        K t = minKey;
        SKNode<K, V, MAX_LEVEL> *p = head;
        p = p->forwards[0];
        while (p != nil) {
            t = max(t, p->key);
            p = p->forwards[0];
        }
        return t;
    }

    K getMinKey() {
        return (head->forwards[0] != nil) ? head->forwards[0]->key : maxKey;
    }

    /* 返回最底层的head */
    SKNode<K, V, MAX_LEVEL> *getLowestHead() {
        return head;
    }

};
