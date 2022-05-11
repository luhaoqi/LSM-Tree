#pragma once

#include "kvstore_api.h"
#include "SkipList.h"
#include <string>
#include "index.h"
#include "buffer.h"
#include <fstream>
#include "utils.h"
#include "SSTable.h"
#include <queue>

//TODO: LSMTree 还没有模板化
class KVStore : public KVStoreAPI {
    // You can add your implementation here
public:
    std::string datadir;
    SkipList<uint64_t, std::string> memTable;
    Index index;
    Buffer buffer;
    vector<int> levelFilesNum;  //number of SSTable of each Level;
    uint64_t maxTimeStamp;

    int memTableSize();

    void saveToDisk();

    std::string getSSTablePath(int level, int id);

    std::string generateLevel(int level);

    std::string getLevelPath(int level);

    std::string findInSSTable(uint64_t key);

public:
    /*
     * @param s pointer to SSTable
     * @param offset  offset to read
     * @return the value
     */
    string readValueByOffset(SSTable *s,uint32_t offset);

    explicit KVStore(const std::string &dir);

    ~KVStore();

    void put(uint64_t key, const std::string &s) override;

    std::string get(uint64_t key) override;

    bool del(uint64_t key) override;

    void reset() override;

    void scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list) override;
};
