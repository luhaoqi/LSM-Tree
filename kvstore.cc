#include "kvstore.h"

KVStore::KVStore(const std::string &dir) : KVStoreAPI(dir), datadir(dir), memTable(0, -1, 0.5), maxTimeStamp(0) {
    std::string dirPath = getLevelPath(0);
    for (int level = 0; utils::dirExists(dirPath); level++, dirPath = getLevelPath(level)) {
        levelFilesNum.push_back(0);
        int id = 0;
        while (true) {
            std::string filePath = getSSTablePath(level, id);
            fstream in = fstream(filePath.c_str(), ios::binary | ios::in);
            if (!in.is_open()) {
                in.close();
                break;
            }
            levelFilesNum[level]++;
            SSTable *tmp = index.readFile(level, id, &in);
            if (tmp->getTimeStamp() >= maxTimeStamp) {
                maxTimeStamp = tmp->getTimeStamp() + 1; //计算所有SSTable的maxtimeStamp 并且新的timeStamp需要+1
            }
            id++;
            in.close();
        }
    }
}

KVStore::~KVStore() {
    saveToDisk();
}

/**
 * return the size of memTable.
 */
int KVStore::memTableSize() {
    int infoSize = 32 + 10240;                    //size of Header + BloomFilter (Byte)
    int indexSize = memTable.size() * (8 + 4); // size of index area（key: 8Byte, offset: 4Byte）
    int dataSize = memTable.dataSize();        // size of data area
    return infoSize + indexSize + dataSize;
}

/**
 * generate a LevelPath
 * @return the Path of the level
 */
std::string KVStore::getLevelPath(int level) {
    std::string path = datadir + "/level-" + to_string(level) + "/";
    return path;
}

/**
 * check if LevelPathDir is exist.
 * if not, create it.
 * @return the Path of the level
 */
std::string KVStore::generateLevel(int level) {
    std::string path = getLevelPath(level);
    if (!utils::dirExists(path)) {
        utils::mkdir(path.c_str());
        levelFilesNum.push_back(0); //新的一层内初始有0个文件
    }
    return path;
}

/**
 * generate a SSTablePath
 * @param level the level of SSTable
 * @param id the idth SSTable
 * @return the Path of the SSTable
 */
std::string KVStore::getSSTablePath(int level, int id) {
    std::string path = getLevelPath(level);
    path += "/SSTable" + to_string(id) + ".sst";
    return path;
}

/**
 * save memTable to SSTable on disk
 */
void KVStore::saveToDisk() {
//    cout << "save to disk:" << memTable.size() << endl;
    if (memTable.size() == 0) return;
    std::string foldPath, SSTablePath;
    foldPath = generateLevel(0);
    SSTablePath = getSSTablePath(0, levelFilesNum[0]);
    levelFilesNum[0]++;
    fstream out(SSTablePath.c_str(), ios::binary | ios::out);
    //写入时间戳
    out.write(reinterpret_cast<const char *>(&maxTimeStamp), sizeof(maxTimeStamp));
    maxTimeStamp++;

    //写入键值对数量
    uint64_t size = memTable.size();
    out.write(reinterpret_cast<const char *>(&size), sizeof(size));

    //写入键最小值和最大值
    uint64_t minKey = memTable.getMinKey();
    uint64_t maxKey = memTable.getMaxKey();
    out.write(reinterpret_cast<const char *>(&minKey), sizeof(minKey));
    out.write(reinterpret_cast<const char *>(&maxKey), sizeof(maxKey));

    //写入BloomFilter
    vector<bool> BF = memTable.genBFVector(BloomFilterSize);
    for (size_t i = 0; i < BF.size(); i += 8) {
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
    std::list<std::pair<uint64_t, string>> list;
    memTable.get_all_elm(list);
    for (auto &x: list) {
        out.write(reinterpret_cast<const char *>(&(x.first)), sizeof(x.first));
        out.write(reinterpret_cast<const char *>(&offset), sizeof(offset));
        offset += x.second.size() + 1;
    }

    //写入数据区
    for (auto &x: list) {
        out.write(x.second.c_str(), (int) x.second.size() + 1);
    }

    //关闭文件
    out.close();

    //读取到index
    fstream in(SSTablePath.c_str(), ios::binary | ios::in);
    index.readFile(0, levelFilesNum[0] - 1, &in);
    in.close();
    memTable.clear(); //写入完成后，应清空memTable
}

std::string KVStore::findInSSTable(uint64_t key) {
    std::string result;
    uint32_t offset = 0;
//    cout << "index size: " << index.index.size() << endl;
    SSTable *SSTp = index.search(key, offset);
//    cout << (SSTp) << endl;
    if (!SSTp) return "";
    fstream in(getSSTablePath(SSTp->getLevel(), SSTp->getId()), ios::binary | ios::in);
    in.seekg(offset, ios::beg);
    std::getline(in, result, '\0');
    in.close();

    if (result == "~DELETED~") return "";
    return result;
}

/**
 * Insert/Update the key-value pair.
 * No return values for simplicity.
 */
void KVStore::put(uint64_t key, const std::string &s) {
    cout << "In put function:\n key: " << key << " |value:" << s << endl;
    if (memTableSize() + 8 + 4 + s.size() > 2 * 1024 * 1024) {
        cout << "put: mem if full! savetodisk!" << endl;
        saveToDisk();
        //TODO:compact
//        if (levelFilesNum[0] >= 3)
//            compact(0);
    }
    cout << "put: Insert" << endl;
    memTable.Insert(key, s);
    cout << "put: Over!" << endl;
}

/**
 * Returns the (string) value of the given key.
 * An empty string indicates not found.
 */
std::string KVStore::get(uint64_t key) {
    std::string result;
    //在memTable中找
    bool flag = false;
    result = memTable.Search(key, flag);
//    cout << "In get funtion\nflag:" << flag << " key:" << key << endl;
    if (flag && result == "~DELETED~") return "";
    if (flag && !result.empty()) return result;
    //没找到,在SSTable中找
    result = findInSSTable(key);
    return result;
}

/**
 * Delete the given key-value pair if it exists.
 * Returns false iff the key is not found.
 */
bool KVStore::del(uint64_t key) {
    bool found = false;
    std::string value = memTable.Search(key, found);

    if (found) {
        //如果在memTable找到 且是删除标记,则一定没有 因为里面只会有一个Key
        if (value == "~DELETED~") return false;
        //否则 存在未被删除 则打上删除标记
        memTable.Delete(key);
        return true;
    }
    //如果没有在memTable找到,在SSTable接着找
    value = findInSSTable(key);
    if (value == "~DELETED~" || value.empty()) return false;
    put(key, "~DELETED~");
    return true;
}

/**
 * This resets the kvstore. All key-value pairs should be removed,
 * including memtable and all sstables files.
 */
void KVStore::reset() {
    memTable.clear();
    index.clear();
    //buffer.clear();

    int level = 0;
    std::string dirPath = getLevelPath(level);
    for (; utils::dirExists(dirPath); level++, dirPath = getLevelPath(level)) {
        std::vector<string> files;
        utils::scanDir(dirPath, files);
        for (auto &file: files) {
            std::string filePath = dirPath + file;
            utils::rmfile(filePath.c_str());
        }
        utils::rmdir(dirPath.c_str());
    }
}

struct pqnode {
    SSTable *s;
    uint64_t key, timestamp;
    std::vector<IndexNode>::iterator it;

    pqnode(SSTable *_s, uint64_t _k, uint64_t _t, std::vector<IndexNode>::iterator _it) :
            s(_s), key(_k), timestamp(_t), it(_it) {}

    //小根堆 key越小越好 key相同timestamp越大越好  重载要反着写 默认大根堆
    bool operator<(const pqnode &x) const { return key != x.key ? key > x.key : timestamp < x.timestamp; }
};


/**
 * Return a list including all the key-value pair between key1 and key2.
 * keys in the list should be in an ascending order.
 * An empty string indicates not found.
 */
void KVStore::scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list) {
    list.clear();
    memTable.Search(key1, key2, list);
    vector<SSTable *> v;
    for (auto &i: index.index)
        for (auto &j: i) {
            //如果有交集;
            if (j->getMinKey() <= key2 || j->getMaxKey() >= key1)
                v.push_back(j);
        }
    uint64_t curMaxKey = 0;
    bool flag = false;
    priority_queue<pqnode> PQ;
    for (auto &it: v) {
        auto pos = it->searchKey(key1);
        if (pos != it->index.end() && pos->key <= key2) {
            PQ.push(pqnode(it, pos->key, it->getTimeStamp(), pos));
//            printf("PQ PUSH: key %llu\n",pos->key);
        }
    }
    while (!PQ.empty()) {
        pqnode t = PQ.top();
        PQ.pop();
        //确保这个key第一次出来,第一次出来的一定是timestamp最大的;
        if (!flag || t.key > curMaxKey) {
            list.emplace_back(t.key, readValueByOffset(t.s, t.it->offset));
            flag = true;
            curMaxKey = t.key;
            auto it = t.it + 1;
            if (it != t.s->index.end())
//                printf("PQ PUSH: key %llu\n",it->key);
                if (it != t.s->index.end() && it->key <= key2)
                    PQ.push(pqnode(t.s, it->key, t.timestamp, it)); //下一个节点 *SSTable一样，key为下一个的key,timestamp一样,迭代器+1
        }
    }
}

string KVStore::readValueByOffset(SSTable *s, uint32_t offset) {
    string filePath = getSSTablePath(s->getLevel(), s->getId());
    fstream in = fstream(filePath.c_str(), ios::binary | ios::in);
    string ret;
    in.seekg(offset, std::ios::beg);
    std::getline(in, ret, '\0');
    return ret;
}