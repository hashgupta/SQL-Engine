#include <iostream>
#include <string>
#include <unordered_map>
#include <sql-engine/engine.h>
#include <sql-engine/parser.h>

auto main(int argc, char** argv) -> int {
  Engine engine;
  std::cout << engine.execute("select name, (age + weight) * weight from test where (weight * age > 1900) and (weight > 39);") << std::endl ;
}
