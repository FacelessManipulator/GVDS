#pragma once

#include <stdint.h>
#include <iostream>
#include <list>
#include <optional>
#include <unordered_map>

namespace hvs {

// Attention: LRU list isn't designed to manage memory, it only provide the
// infomation. PLEASE keep key simple, like long or pointer NOT THREAD SAFE.

template <class K, class V>
struct LRUObject {
  bool pinned;
  K key;
  V data;
  LRUObject(const K& _key) : key(_key), pinned(false) {}
  LRUObject(const K& _key, const V& _v) : key(_key), pinned(false), data(_v) {}
  LRUObject() : pinned(false) {}
  void swap(LRUObject& oths) {
    LRUObject tmp = oths;
    oths = *this;
    *this = tmp;
  }
};

template <class K, class V>
class LRU {
 private:
  typedef std::list<LRUObject<K, V>> LruList;
  typedef typename LruList::iterator LruIter;
  typedef typename LruList::reverse_iterator rLruIter;

 public:
  LRU(unsigned long max = 0) : max_n(max) {}
  std::optional<LRUObject<K, V>> expire() {
    LRUObject<K, V> tmp;
    auto list_it = lrulist.end();
    --list_it;
    if (list_it == lrulist.end()) return std::nullopt;
    tmp.swap(*list_it);
    lrulist.erase(list_it);
    lruindex.erase(tmp.key);
    return tmp;
  }

  size_t size() { return lrulist.size(); }

  LruIter find(const K& key) {
    auto index_it = lruindex.find(key);
    if (index_it == lruindex.end()) return lrulist.end();
    // LRUObject may destroyed by lru, so we copy rather than referrence
    return index_it->second;
  }

  bool empty() { return lruindex.empty(); }
  bool remove(const K& key) {
    auto indexIter = lruindex.find(key);
    if (indexIter == lruindex.end()) {
      return false;
    } else {
      lrulist.erase(indexIter->second);
      lruindex.erase(indexIter);
      return true;
    }
  }

  LRUObject<K, V>& touch(K key) {
    auto indexIter = lruindex.find(key);
    // not exists
    if (indexIter == lruindex.end()) {
      if (max_n != 0 && size() >= max_n) {
        expire();
      }
      // I want to use emplace_front but unluckily, emplace_front not return
      // iterator
      auto lruit = lrulist.emplace(lrulist.begin(), key);
      lruindex.insert_or_assign(key, lruit);
      return *lruit;
    } else {
      LRUObject<K, V> tmp = *(indexIter->second);
      lrulist.erase(indexIter->second);
      auto lruit = lrulist.emplace(lrulist.begin(), tmp);
      indexIter->second = lruit;
      return *lruit;
    }
  }

    void touch(K key, V value) {
      auto indexIter = lruindex.find(key);
      // not exists
      if (indexIter == lruindex.end()) {
        if (max_n != 0 && size() >= max_n) {
          expire();
        }
        // I want to use emplace_front but unluckily, emplace_front not return
        // iterator
        auto lruit = lrulist.emplace(lrulist.begin(), key, value);
        lruindex.insert_or_assign(key, lruit);
        return ;
      } else {
        LRUObject<K, V> tmp = *(indexIter->second);
        lrulist.erase(indexIter->second);
        auto lruit = lrulist.emplace(lrulist.begin(), tmp);
        indexIter->second = lruit;
        return ;
      }
    }

  bool hit(K key) {
    auto indexIter = lruindex.find(key);
    // not exists
    if (indexIter == lruindex.end()) {
      return false;
    } else {
      LRUObject<K, V> tmp = *(indexIter->second);
      lrulist.erase(indexIter->second);
      auto lruit = lrulist.emplace(lrulist.begin(), tmp);
      indexIter->second = lruit;
      return true;
    }
  }

  void observe() {
    std::cout << "<<<< observing lru status." << std::endl;
    std::cout << "lru index contains: [";
    for (auto it : lruindex) {
      std::cout << it.second->key << ", ";
    }
    std::cout << " ]" << std::endl;
    std::cout << "lru list contains: [";
    for (auto it : lrulist) {
      std::cout << it.key << ", ";
    }
    std::cout << " ]" << std::endl;

    std::cout << ">>>> done observing lru status." << std::endl;
  }

  // iterate from new to old
  LruIter begin() { return lrulist.begin(); }
  LruIter end() { return lrulist.end(); }
  // iterate from old to new
  rLruIter rbegin() { return lrulist.rbegin(); }
  rLruIter rend() { return lrulist.rend(); }

 private:
  LruList lrulist;
  std::unordered_map<K, LruIter> lruindex;
  // 0 means infinite
  unsigned int max_n;
};

}  // namespace hvs