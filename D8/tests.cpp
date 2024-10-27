import formatting;
import system;

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Formatted die with no bonus", "[format_die]") {
  using namespace std::literals::string_view_literals;

  REQUIRE(format::format_die(20, 1) == "(1)"sv);
  REQUIRE(format::format_die(12, 12) == "(12)"sv);
  REQUIRE(format::format_die(10, 5) == "(5)"sv);
  REQUIRE(format::format_die(8, 5) == "(5)"sv);
  REQUIRE(format::format_die(6, 2) == "(2)"sv);
  REQUIRE(format::format_die(4, 3) == "(3)"sv);
  REQUIRE(format::format_die(2, 1) == "(H)"sv);
  REQUIRE(format::format_die(2, 2) == "(T)"sv);
}

TEST_CASE("Formatted die with bonus of zero", "[format_die]") {
  using namespace std::literals::string_view_literals;

  REQUIRE(format::format_die(20, 1, 0) == "(1)"sv);
  REQUIRE(format::format_die(12, 12, 0) == "(12)"sv);
  REQUIRE(format::format_die(10, 5, 0) == "(5)"sv);
  REQUIRE(format::format_die(8, 5, 0) == "(5)"sv);
  REQUIRE(format::format_die(6, 2, 0) == "(2)"sv);
  REQUIRE(format::format_die(4, 3, 0) == "(3)"sv);
  REQUIRE(format::format_die(2, 1, 0) == "(H)"sv);
  REQUIRE(format::format_die(2, 2, 0) == "(T)"sv);
}

TEST_CASE("Formatted die with non-zero bonus", "[format_die]") {
  using namespace std::literals::string_view_literals;

  REQUIRE(format::format_die(20, 1, 1) == "(1) + 1 = [2]"sv);
  REQUIRE(format::format_die(12, 12, 5) == "(12) + 5 = [17]"sv);
  REQUIRE(format::format_die(10, 5, 4) == "(5) + 4 = [9]"sv);
  REQUIRE(format::format_die(8, 5, 3) == "(5) + 3 = [8]"sv);
  REQUIRE(format::format_die(6, 2, 2) == "(2) + 2 = [4]"sv);
  REQUIRE(format::format_die(4, 3, 6) == "(3) + 6 = [9]"sv);
}
