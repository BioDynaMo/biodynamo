#include "gtest/gtest.h"
#include "json/json.h"

using std::string;

TEST (Json, objectOrder) {
  Json::Reader reader;
  Json::Value j1;
  Json::Value j2;

  reader.parse("{  \"cellList\": {    \"1000002\": {          \"ID\": 2        },    \"1000001\": {        \"ID\": 1      }    }}", j1);
  reader.parse("{  \"cellList\": {    \"1000001\": {          \"ID\": 1        },    \"1000002\": {        \"ID\": 2      }    }}", j2);
  EXPECT_EQ(j1, j2);
}

TEST (Json, arrayElementOrder) {
  Json::Reader reader;
  Json::Value j1;
  Json::Value j2;

  reader.parse("{  \"cellList\": [0, 1, 2] }", j1);
  reader.parse("{  \"cellList\": [1, 2, 3] }", j2);

  EXPECT_NE(j1, j2);
}

TEST (Json, floatsEqual) {
  Json::Value j1;
  Json::Value j2;

  Json::Reader reader;
  reader.parse("{key: 2.3012345678899999e+02}", j1);
  reader.parse("{key: 230.123456789}", j2);
  EXPECT_EQ(j1, j2);
}

TEST (Json, floatsNotEqual) {
  Json::Reader reader;
  Json::Value j1;
  Json::Value j2;

  reader.parse("{ \"key\": 230.878191}", j1);
  reader.parse("{ \"key\": 230.8781907032909}", j2);
  EXPECT_NE(j1, j2);
}