#include "index.h"

SSTable *Index::readFile(const int &level, const int &id, std::fstream *in) {
    //SSTable Header: 32Byte
    //timestamp(8byte) + number of keys(8byte) + MinKey(8byte) + MaxKey(8byte)

    //读取时间戳
    uint64_t timeStamp;
    in->read((char *) &timeStamp, sizeof(timeStamp));

    //读取键值对数量
    uint64_t size = 0;
    in->read((char *) &size, sizeof(size));

    //读取BloomFilter
    in->seekg(32, std::ios::beg); //移动读写位置到离开始位置32字节处
    std::vector<bool> BF(BloomFilterSize, false);
    char b;
    for (size_t i = 0; i < BloomFilterSize; i += 8) {
        in->read(&b, sizeof(b));
        for (size_t j = 0; j < 8; j++) {
            if (b & (1 << (7 - j))) {
                BF[i + j] = true;
            }
        }
    }

    //读取索引区
    std::vector<IndexNode> ind;
    uint64_t key;
    uint32_t offset;
    for (size_t i = 0; i < size; i++) {
        in->read((char *) &key, sizeof(key));
        in->read((char *) &offset, sizeof(offset));
        ind.emplace_back(key, offset);
    }

    //记录到index
    auto *tmp = new SSTable(level, id, timeStamp, BF, ind);
    while (level > (int) (index.size() - 1))  //有第level层,index得有level+1层
        index.emplace_back();

    auto it = index[level].begin();
    for (; it != index[level].end(); it++) {
        if ((*it)->getId() > id) {
            break;
        }
    }
    index[level].insert(it, tmp);
    return tmp;
}


SSTable *Index::search(uint64_t key, uint32_t &offset) {
    SSTable *result = nullptr;
    uint32_t tmpOffset;
    bool flag = false;
    for (auto &i: index) {
        for (auto &j: i) {
            if ((tmpOffset = j->hasKey(key)) > 0) {
                if ((!result) || j->getTimeStamp() > result->getTimeStamp()) //选时间戳最大的,其实也就在第0层会有多个key
                {
                    result = j;
                    offset = tmpOffset;
                    flag = true;
                }
            }
        }
        if (flag)
            break;
    }
    return result;
}


void Index::deleteFileIndex(int level, int id) {
    for (auto it = index[level].begin(); it != index[level].end(); it++) {
        if ((*it)->getId() == id) {
            index[level].erase(it);
            break;
        }
    }
}

void Index::clear() {
    for (auto &i: index) {
        for (auto &j: i) {
            delete j;
        }
        i.clear();
    }
    index.clear();
}


std::vector<int> Index::findIntersectionId(int level, uint64_t minKey, uint64_t maxKey) {
    std::vector<int> result;
    for (auto it = index[level].begin(); it != index[level].end();) {
        if (!((*it)->getMaxKey() < minKey || (*it)->getMinKey() > maxKey)) {
            result.push_back((*it)->getId());
            it = index[level].erase(it);
            continue;
        }
        it++;
    }
    int size = (int) index[level].size();
    for (int i = 0; i < size; ++i) {
        index[level][i]->changeId(i);
    }
    return result;
}


void Index::changeIndex(int level, int oldId, int id) {
    for (auto &it: index[level]) {
        if (it->getId() == oldId) {
            it->changeId(id);
            break;
        }
    }
}


