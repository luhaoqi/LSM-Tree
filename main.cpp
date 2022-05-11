//
// Created by 66lhq on 2022/5/7.
//
#include <string>
#include <list>
#include <utility>
#include <iostream>
#include <fstream>
#include "kvstore.h"
#include <assert.h>

using namespace std;
struct player_data {
    std::string name;
    int score;
};

void f(const int &x) {
    cout << x << endl;
}

void test() {
    f(1);
    std::ofstream savefile("./data/test.txt", std::ios_base::binary);
    if (savefile.good()) {
        player_data Player1;
        Player1.name = "John Doell";
        std::cout << "Player1 name: " << Player1.name << std::endl << Player1.name.size() << std::endl;
        Player1.score = 55;
        savefile.write(Player1.name.c_str(), Player1.name.size()); // write string to binary file
        //savefile.write("\0",sizeof(char)); // null end string for easier reading
        savefile.write(reinterpret_cast<char *>(&Player1.score), sizeof(Player1.score)); // write int to binary file
        savefile.close();
    }

    std::ifstream loadfile("./data/test.txt", std::ios_base::binary);
    if (loadfile.good()) {
        player_data Player1;
        //std::getline(loadfile,Player1.name,'\0'); // get player name (remember we null ternimated in binary)
        char *c = new char[10];
        loadfile.read(c, 10);
        Player1.name = string(c, 10);
        delete[] c;
        c = new char[20];
        loadfile.read((char *) &Player1.score, sizeof(Player1.score)); // read int bytes
        std::cout << "Player1 name: " << Player1.name << std::endl << Player1.name.size() << std::endl;
        std::cout << "Player1 score: " << Player1.score << std::endl;
    }
}

void test_skiplist() {
    std::list<std::pair<uint64_t, std::string>> list;
    SkipList<uint64_t, std::string> SL(0, -1, 0.25);
    SL.Insert(1, string("123"));
    SL.Insert(1, string("1"));
    SL.Insert(2, string("22"));
    SL.Insert(3, string("333"));
    bool flag = false;
    cout << SL.Search(1, flag) << endl;
    SL.Search(1, 3, list);
    for (auto x: list)
        cout << x.second << " ";
    cout << endl;
    SL.Delete(2);
    SL.Search(1, 3, list);
    for (auto x: list)
        cout << x.second << " ";
    cout << endl;
    SL.get_all_elm(list);
    for (auto x: list)
        cout << x.second << " ";
    cout << endl;
    cout << SL.size() << " " << SL.dataSize() << endl;
}

//void test_bloom_filter() {
//    Bloom_Fliter B(10240 * 8);
//    for (int i = 0; i < 10; i++) {
//        B.insert(i);
//        B.insert(i * i);
//    }
//    for (int i = 0; i < 20; i++) {
//        cout << B.find(i) << endl;
//    }
//
//
//}

void test_kv() {
    KVStore kv("./data");
    kv.put(1, "1");
    kv.put(2, "22");
    kv.put(3, "333");
    kv.put(4, "4444");
    cout << kv.get(1) << endl;
    cout << kv.get(2) << endl;
    cout << kv.get(3) << endl;
    cout << kv.get(5) << endl;
    kv.del(2);
    cout << kv.get(2) << endl;
    std::list<std::pair<uint64_t, string>> list;
    kv.scan(1, 4, list);
    for (auto &x: list)
        cout << "key:" << x.first << " value:" << x.second << endl;
}

void test_kv2(uint64_t max) {
    KVStore kv("./data");
    kv.reset();
    uint64_t i;
    for (i = 0; i < max; ++i) {
        kv.put(i, std::string(i + 1, 's'));
        assert(std::string(i + 1, 's') == kv.get(i));
        //EXPECT(std::string(i+1, 's'), kv.get(i));
    }
    puts("1 pass!");
    // Test after all insertions
    for (i = 0; i < max; ++i) {
        assert(std::string(i + 1, 's') == kv.get(i));
//        if (i == 3441) {
//            uint32_t offset;
//            auto S = kv.index.search(3441, offset);
//            cout << S->getLevel() << " " << S->getId() << " " << offset << S->getTimeStamp() << endl;
//            fstream in(kv.getSSTablePath(S->getLevel(), S->getId()), ios::binary | ios::in);
//            string res;
//            in.seekg(offset, ios::beg);
//            std::getline(in,res,'\0');
//            cout<<res;
//            cout << "!" << endl;
//            in.close();
//            cout << kv.get(i) << endl;
//        }
    }

    //EXPECT(std::string(i + 1, 's'), kv.get(i));
    puts("2 pass!");
    // Test deletions
    for (i = 0; i < max; i += 2) {
        assert(true == kv.del(i));
    }
    puts("del pass!");
    string not_found;
    // Prepare data for Test Mode
    for (i = 0; i < max; ++i) {
//        cout << i << endl;
//        if (i == 3441) {
//            uint32_t offset;
//            auto S = kv.index.search(3441, offset);
//            cout << S->getLevel() << " " << S->getId() << " " << offset << S->getTimeStamp() << endl;
//            fstream in(kv.getSSTablePath(S->getLevel(), S->getId()), ios::binary | ios::in);
//            char res[200];
//            in.seekg(offset, ios::beg);
//            in.read(res, 100);
//            for (int j = 0; j < 100; j++) cout << ((res[j] == '\0') ? '0' : (char) res[j]);
//            cout << "!" << endl;
//            in.close();
//            cout << kv.get(i) << endl;
//
//            break;
//        }
        switch (i & 3) {
            case 0:
                assert(not_found == kv.get(i));
                kv.put(i, std::string(i + 1, 't'));
                break;
            case 1:
                assert(std::string(i + 1, 's') == kv.get(i));
                kv.put(i, std::string(i + 1, 't'));
                break;
            case 2:
                assert(not_found == kv.get(i));
                break;
            case 3:
                assert(std::string(i + 1, 's') == kv.get(i));
                break;
//                assert(0);
        }
    }
    puts("3 pass!");
//    cout << kv.get(1) << endl;
//    kv.put(1, "SE");
//    cout << kv.get(1) << endl;
//    cout << kv.del(1) << endl;
//    cout << kv.get(1) << endl;
//    cout << kv.del(1) << endl;
}

int main() {
    //test();
    //test_skiplist();
    //test_bloom_filter();
    //test_kv();
    //test_kv2(1024 * 32);
    KVStore kv("./data");
    cout<<kv.get(1)<<endl;
    return 0;
}
