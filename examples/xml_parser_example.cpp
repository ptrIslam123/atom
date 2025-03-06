#include <iostream>

#include "include/parser/xml/parser.h"

using namespace atom;

int main() {
    std::string xmlSample = R"(
    <root>
      <users>
        <user>
          <id>1</id>
          <name attr="first">John Doe</name>
          <name attr="second">Doe</name>
          <age>28</age>
          <img src="image.jpg" alt="Description" width="300" height="200"/>
        </user>
      </users>
    </root>
    )";

    parser::xml::Parser parser;
    parser.parse(std::move(xmlSample));
    return 0;
}
