#pragma once

#include <stdint.h>
#include <iostream>
#include <list>
#include <optional>
#include <unordered_map>

namespace hvs {

// Attention: LRU list isn't designed to manage memory, it only provide the
// infomation. PLEASE keep key simple, like long or pointer NOT THREAD SAFE.

template <class T>
struct LRUObject {
  bool pinned;
  T data;
  LRUObject(T _data) : data(_data), pinned(false) {}
};

template <class T>
class LRU {
 private:
  typedef std::list<LRUObject<T>> LruList;
  typedef typename LruList::iterator LruIter;
  typedef typename LruList::reverse_iterator rLruIter;
 public:
  int expire() {}

  bool empty() { return lruindex.empty(); }
  bool remove(T key) {
    auto indexIter = lruindex.find(key);
    if (indexIter == lruindex.end()) {
      return false;
    } else {
      lrulist.erase(indexIter->second);
      lruindex.erase(indexIter);
      return true;
    }
  }

  bool touch(T key) {
    auto indexIter = lruindex.find(key);
    // not exists
    if (indexIter == lruindex.end()) {
      // I want to use emplace_front but unluckily, emplace_front not return
      // iterator
      auto lruit = lrulist.emplace(lrulist.begin(), key);
      lruindex.insert_or_assign(key, lruit);
      return true;
    } else {
      lrulist.erase(indexIter->second);
      auto lruit = lrulist.emplace(lrulist.begin(), key);
      indexIter->second = lruit;
    }
  }

  void observe() {
    std::cout << "<<<< observing lru status." << std::endl;
    std::cout << "lru index contains: [";
    for (auto it : lruindex) {
      std::cout << it.second->data << ", ";
    }
    std::cout << " ]" << std::endl;
    std::cout << "lru list contains: [";
    for (auto it : lrulist) {
      std::cout << it.data << ", ";
    }
    std::cout << " ]" << std::endl;

    std::cout << ">>>> done observing lru status." << std::endl;
  }

  // iterate from old to new
  rLruIter begin() { return lrulist.rbegin(); }
  rLruIter end() { return lrulist.rend(); }

 private:
  LruList lrulist;
  std::unordered_map<T, LruIter> lruindex;
};

}  // namespace hvs