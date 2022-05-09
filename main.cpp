//
// Created by 66lhq on 2022/5/7.
//
#include "SkipList.h"
#include "Bloom_Fliter.h"
#include <string>
#include <list>
#include <utility>
#include <iostream>
#include <fstream>

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
        delete c;
        c = new char[20];
        loadfile.read((char *) &Player1.score, sizeof(Player1.score)); // read int bytes
        std::cout << "Player1 name: " << Player1.name << std::endl << Player1.name.size() << std::endl;
        std::cout << "Player1 score: " << Player1.score << std::endl;
    }
}

void test_skiplist() {
    std::list <std::pair<uint64_t, std::string>> list;
    SkipList<uint64_t, std::string> SL(0, -1, 0.25);
    SL.Insert(1, string("123"));
    SL.Insert(1, string("111"));
    SL.Insert(2, string("222"));
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
}

void test_bloom_filter() {
    Bloom_Fliter B(10240 * 8);
    for (int i = 0; i < 10; i++) {
        B.insert(i);
        B.insert(i * i);
    }
    for (int i = 0; i < 20; i++) {
        cout << B.find(i) << endl;
    }


}


int main() {
    //test();
    //test_skiplist();
    test_bloom_filter();
    return 0;
}
