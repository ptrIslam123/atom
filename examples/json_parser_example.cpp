#include "include/parser/json/parser.h"

#include <iostream>

using namespace atom;

int main() {
    std::string jsonSample = R"(
    {
      "users": [
        {
          "id": 1,
          "name": "John Doe",
          "email": "john.doe@example.com",
          "age": 28,
          "address": {
            "city": "New York",
            "street": "5th Avenue"
          }
        }
      ]
    }
    )";
    parser::json::Parser parser;
    parser.parse(std::move(jsonSample));
    return 0;
}
