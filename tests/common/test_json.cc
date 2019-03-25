#include <iostream>
#include "common/JsonSerializer.h"
#include "datastore/datastore.h"
#include "context.h"
#include "common/json.h"
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

  bool operator==(const Pet& othr) const {
    if (name == othr.name && children_name == othr.children_name &&
        children_age == othr.children_age) {
      return true;
    }
    return false;
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

  bool operator==(const User& othr) const {
    if (age == othr.age && name == othr.name && height == othr.height &&
        married == othr.married && pet == othr.pet)
      return true;
    return false;
  }

 public:
  User(std::string n, int a, float h, bool m, std::string pet_name)
      : age(a), name(n), height(h), married(m), pet(pet_name) {}
  User() = default;
};

User generate_user() {
  User user("bob", 10, 173.4, false, "jojo");
  user.pets_name.emplace_back("jojo1");
  user.pets_name.emplace_back("jojo2");
  user.pets_name.emplace_back("jojo3");
  user.pets_age["jojo1"] = 1;
  user.pets_age["jojo2"] = 2;
  user.pets_age["jojo3"] = 3;
  user.pet.children_name.emplace_back("jojo2");
  user.pet.children_age["jojo2"] = 0;
  return user;
}

TEST(HVSJsonTest, Simple) {
  User u1 = generate_user();
  User u2;
  std::string u1_json = u1.serialize();
  std::cout << u1_json << std::endl;
  u2.deserialize(u1_json);
  std::cout << u2.serialize() << std::endl;
  EXPECT_EQ(u1, u2);
}

TEST(HVSJsonTest, DencVector) {
  std::vector<int> vec = {1, 2, 3, 4, 5};
  std::string json_str = hvs::json_encode(vec);
  std::cout << json_str << std::endl;
  std::vector<int> vec2 = {2, 3, 4};
  hvs::json_decode(json_str, vec2);
  EXPECT_EQ(vec, vec2);
}

TEST(HVSJsonTest, DencUser) {
  User u1 = generate_user();
  // decode sub object
  std::string json_str = hvs::json_encode(u1.pet);
  User u2;
  hvs::json_decode(json_str, u2.pet);
  EXPECT_EQ(u1.pet, u2.pet);

  // decode normal type
  json_str = hvs::json_encode(u1.height);
  hvs::json_decode(json_str, u2.height);
  EXPECT_EQ(u1.height, u2.height);

  // decode map type
  json_str = hvs::json_encode(u1.pets_age);
  hvs::json_decode(json_str, u2.pets_age);
  EXPECT_EQ(u1.pets_age, u2.pets_age);

  // decode all
  json_str = hvs::json_encode(u1);
  hvs::json_decode(json_str, u2);
  EXPECT_EQ(u1, u2);
}