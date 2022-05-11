#pragma once
#ifndef LSM_KV_SSTABLE_H
#define LSM_KV_SSTABLE_H

#include <cstdint>
#include <utility>
#include <vector>
#include "MurmurHash3.h"

const int BloomFilterSize = 81920;

// 索引区节点
struct IndexNode {
    uint64_t key;
    uint32_t offset;

    explicit IndexNode(uint64_t k = 0, uint32_t o = 0) : key(k), offset(o) {}

    IndexNode() : key(0), offset(0) {}
};

class SSTable {
private:
    int level;  //层数
    int id; //在该层的编号
    uint64_t timeStamp;  //最后修改的时间戳  时间戳越大表示越晚更新
    std::vector<bool> bloomFilter;


public:
    SSTable(int l, int i, uint64_t time, std::vector<bool> BF, std::vector<IndexNode> ind) :
            level(l), id(i), timeStamp(time), bloomFilter(std::move(BF)), index(std::move(ind)) {}

    ~SSTable() = default;

    std::vector<IndexNode> index;  //索引节点

    //二分查找key，返回index中第一个>=key的IndexNode(key,offset)的迭代器，没找到就返回end()
    std::vector<IndexNode>::iterator searchKey(uint64_t key) {
        int left = 0, right = (int) index.size() - 1, mid;
        auto res = index.end();
        while (left <= right) {
            mid = (left + right) / 2;
            if (index[mid].key >= key) {
                res = index.begin() + mid;
                right = mid - 1;
            } else
                left = mid + 1;
        }
        return res;
    }

    //判断该SSTable中是否存在key，若存在返回offset，不存在返回0
    uint32_t hasKey(uint64_t key) {
        //用BloomFilter判断是否存在key
        unsigned int hash[4] = {0};
        MurmurHash3_x64_128(&key, sizeof(key), 1, hash);
        for (size_t i = 0; i < 4; i++) {
            if (!bloomFilter[hash[i] % BloomFilterSize]) {
                return 0;
            }
        }
        auto x = searchKey(key);
        return x == index.end() ? 0 : x->offset;
    }

    uint64_t getTimeStamp() const { return timeStamp; }

    int getLevel() const { return level; }

    int getId() const { return id; }

    uint64_t getMinKey() { return index.front().key; }

    uint64_t getMaxKey() { return index.back().key; }

    void changeId(int newId) { id = newId; }
};

#endif