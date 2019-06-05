#include "common/LRU.h"
#include "gtest/gtest.h"
#include <iostream>

using namespace hvs;
using namespace std;

TEST(LRU_TEST, Simple) {
    LRU<int> lru;
    for(int i = 0; i < 10; i++) {
        lru.touch(i);
    }
        cout << "loop-range: ";
    for(auto it : lru) {lru.remove(5);
        cout << it.data << ", ";
    }
    cout << ". "<< endl;
    lru.observe();lru.touch(5);lru.touch(5);lru.touch(5);lru.touch(5);lru.touch(5);
    lru.observe();lru.remove(5);
    lru.observe();

    for(int i = 0; i < 10; i++) {
        lru.remove(i);
    }
    lru.observe();


    // cout << endl;
}