import formatting;

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Formatted die with no set bonus", "[format_die]") {
  using namespace std::string_literals;

  REQUIRE(format::format_die(20, 1) == "(1)"s);
}
