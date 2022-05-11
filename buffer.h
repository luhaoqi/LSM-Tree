#pragma once
#ifndef LSM_KV_BUFFER_H
#define LSM_KV_BUFFER_H

#include "SSTable.h"
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include <list>
#include <fstream>

struct dataNode {
    uint64_t key;
    std::string value;

    dataNode(uint64_t k, std::string v) : key(k), value(std::move(v)) {}
};

struct dataList {
    //归并排序时为了方便从头部取出而设置成list而不是vector
    std::list<dataNode> data;
    uint64_t timeStamp;

    dataList(std::list<dataNode> d, uint64_t t) : data(std::move(d)), timeStamp(t) {}
};

//用于合并SSTable
class Buffer {
private:
    //要被合并的SSTable的数据暂存于此
    std::vector<dataList> datas;
    //归并排序后的输出数据
    std::vector<dataNode> output;
    //合并后新SSTable的时间戳（就是datas里最大的时间戳）
    uint64_t timeStamp;

    //从output中获取生成的SSTable不超过2M的最大数据量的数据，并在参数中填入生成的BloomFilter，并从output中删除这部分数据
    std::vector<dataNode> get2MVector(std::vector<bool> &BF);

public:
    Buffer() : timeStamp(0) {}

    ~Buffer() = default;

    //把要合并的SSTable读入datas
    void readFile(std::fstream *in);

    //flag为true表示下一层为空，合并时需要删除~DELETED~
    void compact(bool flag);

    void clear() {
        datas.clear();
        output.clear();
        timeStamp = 0;
    }

    //以SSTable形式输出2M数据
    void write(std::fstream *out);

    uint64_t getMinKey();

    uint64_t getMaxKey();

    bool isOutputEmpty() { return output.empty(); }
};

#endif //LSM_KV_BUFFER_H
