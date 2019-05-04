#include "gtest/gtest.h"
#include "msg/pack.h"
#include "msg/stat_demo.h"
#include <iostream>

using namespace hvs;
using namespace std;

class ComplexStructure {
 public:
  // data part
  string data;
  size_t size;
  unsigned type;
  ComplexStructure() {
    cout << "call constructor, data = {" << data << "}" << endl;
  }
  ComplexStructure(string& _d) {
    type = 1;
    data = _d;
    size = data.size();
    cout << "call constructor, data = {" << data << "}" << endl;
  }
  ~ComplexStructure() {
    cout << "call destructor." << endl;
  }
  MSGPACK_DEFINE_ARRAY(data, size, type);
};

TEST(PACKTEST, simple) {
  stat_buffer sb1 = get_stat("/tmp/hvs/tests/data/example.cfg");
  auto buffer = pack(sb1);
  stat_buffer sb2;
  sb2 = unpack<stat_buffer>(buffer);
  EXPECT_EQ(sb1.st_ino, sb2.st_ino);
}

TEST(PACKTEST, data) {
  string buf = "some random words!";
  ComplexStructure* cs = new ComplexStructure(buf);
  auto buffer = pack(*cs);
  delete cs;
  ComplexStructure cs2 = unpack<ComplexStructure>(buffer);
  cout << cs2.data << endl;
  EXPECT_STREQ(cs2.data.c_str(), buf.c_str());
}