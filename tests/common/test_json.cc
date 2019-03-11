#include <iostream>
#include "common/JsonSerializer.h"
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
};

TEST(HVSJsonTest, Simple) {
    User u1("bob", 10, 173.4, false, "jojo");
    User u2;
  u1.pets_name.emplace_back("jojo1");
  u1.pets_name.emplace_back("jojo2");
  u1.pets_name.emplace_back("jojo3");
  u1.pets_age["jojo1"] = 1;
  u1.pets_age["jojo2"] = 2;
  u1.pets_age["jojo3"] = 3;
  u1.pet.children_name.emplace_back("jojo2");
  u1.pet.children_age["jojo2"] = 0;
  std::string u1_json = u1.serialize();
  std::cout << u1_json << std::endl;
  u2.deserialize(u1_json);
  std::cout << u2.serialize() << std::endl;

  // EXPECT_TRUE("the file in /tmp/hvs.logtest should be correct");
}
