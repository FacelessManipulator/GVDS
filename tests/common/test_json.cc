#include <iostream>
#include "common/JsonSerializer.h"
#include "datastore/datastore.h"
#include "gtest/gtest.h"

class Pet : public hvs::JsonSerializer {
 public:
  std::string name;
  std::vector<std::string> children_name;
  std::map<std::string, int> children_age;

 public:
  void serialize_impl() override {
    put("name", name);
    put("children_name", children_name);
    put("children_age", children_age);
  }
  void deserialize_impl() override {
    get("name", name);
    get("children_name", children_name);
    get("children_age", children_age);
  }

 public:
  Pet(std::string n) : name(n) {}
  Pet() = default;
};
class User : public hvs::JsonSerializer {
 public:
  int age;
  std::string name;
  float height;
  bool married;
  std::vector<std::string> pets_name;
  std::map<std::string, int> pets_age;
  Pet pet;

 public:
  void serialize_impl() override {
    put("name", name);
    put("age", age);
    put("height", height);
    put("married", married);
    put("pets", pets_name);
    put("pets_age", pets_age);
    put("pet", pet);
  }

  void deserialize_impl() override {
    get("name", name);
    get("age", age);
    get("height", height);
    get("married", married);
    get("pets", pets_name);
    get("pets_age", pets_age);
    get("pet", pet);
  }

 public:
  User(std::string n, int a, float h, bool m, std::string pet_name)
      : age(a), name(n), height(h), married(m), pet(pet_name) {}
  User() = default;

 public:
  void assign() {
    pets_name.emplace_back("jojo1");
    pets_name.emplace_back("jojo2");
    pets_name.emplace_back("jojo3");
    pets_age["jojo1"] = 1;
    pets_age["jojo2"] = 2;
    pets_age["jojo3"] = 3;
    pet.children_name.emplace_back("jojo2");
    pet.children_age["jojo2"] = 0;
  }
};

TEST(HVSJsonTest, Simple) {
  User u1("bob", 10, 173.4, false, "jojo");
  User u2;
  u1.assign();
  std::string u1_json = u1.serialize();
  std::cout << u1_json << std::endl;
  u2.deserialize(u1_json);
  std::cout << u2.serialize() << std::endl;

  // EXPECT_TRUE("the file in /tmp/hvs.logtest should be correct");
}

TEST(HVSJsonTest, Datastore) {
  User u1("bob", 10, 173.4, false, "jojo");
  u1.assign();
  std::string u1_value = u1.serialize();
  std::string u1_key = u1.name;
  hvs::DatastorePtr dbPtr = hvs::DatastoreFactory::create_datastore(
      "test", hvs::DatastoreType::couchbase);
  dbPtr->init();
  EXPECT_EQ(0, dbPtr->set(u1_key, u1_value));
  EXPECT_EQ(u1_value, *(dbPtr->get(u1_key)));
  //    EXPECT_EQ(0, dbPtr->remove(u1_key));
  //    EXPECT_EQ("", *(dbPtr->get(u1_key)));
}