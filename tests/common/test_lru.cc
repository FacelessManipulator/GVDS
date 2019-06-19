#include "common/LRU.h"
#include "gtest/gtest.h"
#include <iostream>

using namespace hvs;
using namespace std;

TEST(LRU_TEST, Simple) {
    LRU<int, int> lru(10);
    for(int i = 0; i < 11; i++) {
        lru.touch(i);
    }
        cout << "loop-range: ";
    for(auto it : lru) {
        cout << it.key << ", ";
    }
    cout << ". "<< endl;
    lru.observe();lru.touch(5);lru.touch(5);lru.touch(5);lru.touch(5);lru.touch(5);
    lru.observe();lru.remove(5);
    lru.observe();
    EXPECT_EQ(lru.size(), 9);
    lru.touch(5);
    lru.hit(0);
    lru.hit(10);
    EXPECT_EQ(lru.size(), 10);
    cout << "expire: [";
    for(int i = 0; i < 15; i++) {
        auto v = lru.expire();
        if (v.has_value())
            cout << v->key << ", ";
        else
            cout << -1 << ", ";
    }
    cout << "]" << endl;
    EXPECT_EQ(lru.size(), 0);
    // cout << endl;
}