//
// Created by 66lhq on 2022/5/7.
//
#include "SkipList.h"
#include <string>
#include <list>
#include <utility>
#include <iostream>

using namespace std;

int main() {
    std::list <std::pair<uint64_t, std::string>> list;
    SkipList SL(0.25);
    SL.Insert(1, string("123"));
    SL.Insert(1, string("111"));
    SL.Insert(2, string("222"));
    SL.Insert(3, string("333"));
    cout << SL.Search(1) << endl;
    SL.Search(1,3,list);
    for (auto x:list)
        cout<<x.second<<" ";
    cout<<endl;
    SL.Delete(2);
    SL.Search(1,3,list);
    for (auto x:list)
        cout<<x.second<<" ";
    cout<<endl;
    return 0;
}
