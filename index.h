#pragma once
#ifndef LSM_KV_INDEX_H
#define LSM_KV_INDEX_H

#include <vector>
#include "SSTable.h"
#include <fstream>
#include "MurmurHash3.h"

/*
 * index只存有一个二维vector,内容为所有SSTable的指针
 */
class Index {

public:
    std::vector<std::vector<SSTable *>> index; //每一层的每一个SSTable;

    Index() = default;

    ~Index() {
        clear();
    }

    //读取level层第id个SSTable, 文件流in作为参数传入
    //@return 返回生成的SSTable*
    SSTable *readFile(const int &level, const int &id, std::fstream *in);

    //@param key 需要查找的key
    //@return 对应的SSTable的指针并且设置offset到参数
    SSTable *search(uint64_t key, uint32_t &offset);

    //删除文件，并且在索引里删除
    void deleteFileIndex(int level, int id);

    ////返回level层键值与minKey到maxKey有交集的所有SSTable的id，并在index里删除这些索引，重排id
    std::vector<int> findIntersectionId(int level, uint64_t minKey,
                                        uint64_t maxKey);

    void clear();

    //更新level层的oldId的文件id为id
    void changeIndex(int level, int oldId, int id);
};

#endif //LSM_KV_INDEX_H
