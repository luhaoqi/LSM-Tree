#include "buffer.h"

std::vector<dataNode> Buffer::get2MVector(std::vector<bool> &BF) {
//    std::cout << "output:" << output.size() << std::endl;
    BF.assign(BloomFilterSize, false);
    const int MAXSIZE = 2 * 1024 * 1024; //2M
    int tmpSize = 32 + 10240;
    std::vector<dataNode> result;
    auto it = output.begin();
    while (it != output.end()) {
        tmpSize += (8 + 4);
        tmpSize += (int) it->value.size() + 1;
        if (tmpSize > MAXSIZE) break;
        //没超过就加入result
        result.push_back(*it);
        //BloomFilter
        unsigned int hash[4] = {0};
        MurmurHash3_x64_128(&(it->key), sizeof(it->key), 1, hash);
        for (unsigned int i: hash) {
            BF[i % BloomFilterSize] = true;
        }
        it++;
    }
    //TODO:output换个list
    //一次性删除
    output.erase(output.begin(), it);
    return result;
}

void Buffer::write(std::fstream &out) {
    if (output.empty()) return;

    //SSTable Header: 32Byte
    //timestamp(8byte) + number of keys(8byte) + MinKey(8byte) + MaxKey(8byte)
    std::vector<bool> BF;
    std::vector<dataNode> data = get2MVector(BF);
    //写入时间戳
    out.write(reinterpret_cast<const char *>(&timeStamp), sizeof(timeStamp));

    //写入键值对数量
    uint64_t size = data.size();
    out.write(reinterpret_cast<const char *>(&size), sizeof(size));

    //写入键最小值和最大值
    uint64_t minKey = data.front().key;
    uint64_t maxKey = data.back().key;
    out.write(reinterpret_cast<const char *>(&minKey), sizeof(minKey));
    out.write(reinterpret_cast<const char *>(&maxKey), sizeof(maxKey));

    //写入BloomFilter
    for (size_t i = 0; i < BloomFilterSize; i += 8) {
        unsigned char b = 0;
        for (int j = 0; j < 8; ++j) {
            if (BF[i + j]) {
                b |= (1 << (7 - j));
            }
        }
        out.write(reinterpret_cast<const char *>(&b), sizeof(b));
    }

    //写入索引区
    uint32_t offset = 32 + 10240 + size * (8 + 4);
    for (size_t i = 0; i < size; i++) {
        out.write(reinterpret_cast<const char *>(&(data[i].key)), sizeof(data[i].key));
        out.write(reinterpret_cast<const char *>(&offset), sizeof(offset));
        offset += data[i].value.size() + 1;
    }

    //写入数据区
    for (size_t i = 0; i < size; i++) {
        out.write(data[i].value.c_str(), data[i].value.size() + 1);
    }
}

uint64_t Buffer::getMinKey() {
    uint64_t minKey = UINT64_MAX;
    int size = (int) datas.size();
    for (int i = 0; i < size; i++)
        minKey = std::min(minKey, datas[i].data.front().key);
    return minKey;
}

uint64_t Buffer::getMaxKey() {
    uint64_t maxKey = 0;
    int size = (int) datas.size();
    for (int i = 0; i < size; i++)
        maxKey = std::max(maxKey, datas[i].data.back().key);
    return maxKey;
}

void Buffer::readFile(std::fstream &in) {
    //SSTable Header: 32Byte
    //timestamp(8byte) + number of keys(8byte) + MinKey(8byte) + MaxKey(8byte)

    //读取时间戳
    uint64_t time;
    in.read((char *) &time, sizeof(time));

    //读取键值对数量
    uint64_t size = 0;
    in.read((char *) &size, sizeof(size));

    //读取索引区
    in.seekg(32 + 10240, std::ios::beg);
    std::vector<IndexNode> ind;
    uint64_t key;
    uint32_t offset;
    for (size_t i = 0; i < size; i++) {
        in.read((char *) &key, sizeof(key));
        in.read((char *) &offset, sizeof(offset));
        ind.emplace_back(key, offset);
    }

    //记录到dataList
    std::list<dataNode> tmpDatas;
    std::string value;
    for (size_t i = 0; i < size; i++) {
        in.seekg(ind[i].offset, std::ios::beg);
        std::getline(in, value, '\0');
        tmpDatas.emplace_back(ind[i].key, value);
    }
    datas.emplace_back(tmpDatas, time); //同时存入时间戳
    timeStamp = std::max(timeStamp, time); //Buffer的时间戳是所有合并的SSTable的时间戳的最大值
}

void Buffer::compact(bool flag) {
    output.clear();
    uint64_t minKey = UINT64_MAX; //归并排序时，每一轮的最小值
    int target = 0;               //归并排序时每一轮被取值的目标文件（在datas中的索引）
    uint64_t maxTime = 0;         //每一轮归并排序时，取值的目标文件的时间戳
    timeStamp = 0;
    for (auto &data: datas)
        timeStamp = std::max(timeStamp, data.timeStamp);

    //由已知特性，key可能存在相同，但每个list里面key各不相同
    //TODO：可以用堆来优化合并过程
    while (!datas.empty()) {
        minKey = UINT64_MAX;
        target = 0;
        maxTime = 0;

        int fileNum = (int) datas.size();
        for (int i = 0; i < fileNum; i++) {
            dataNode data = datas[i].data.front();
            if (data.key < minKey) {
                minKey = data.key;
                target = i;
                maxTime = datas[i].timeStamp;
            } else if (data.key == minKey) {
                if (datas[i].timeStamp > maxTime) {
                    maxTime = datas[i].timeStamp;
                    datas[target].data.pop_front(); //键相同选择时间戳大的，所以原先文件里的该键值对被抛弃，可以直接删掉
                    target = i;
                } else {
                    datas[i].data.pop_front(); //否则该文件时间戳小，该键值对被抛弃，可以删掉
                }
            }
        }
        dataNode tmp = datas[target].data.front();
        if (!(flag && tmp.value == "~DELETED~")) //只有在最后一层(flag=true)且value为删除标记的时候才不加入output
            output.push_back(tmp);
        datas[target].data.pop_front();
        fileNum = (int) datas.size();
        for (int i = 0; i < fileNum; i++) //检查已经被读完的文件
        {
            if (datas[i].data.empty()) {
                datas.erase(datas.begin() + i);
                i -= 1; //由于是用下标访问，每次循环i++, 不减一的话会跳过下一个元素
                fileNum -= 1;
            }
        }
    }
}