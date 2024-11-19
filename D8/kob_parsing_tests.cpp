import utility;
import kob;

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Kids on Bikes parsing", "[parse]") {
  using namespace std::literals::string_view_literals;

  auto result = parse_kob::result{};

  auto should_be_result = parse_kob::parse("4"sv);
  auto should_be_error = parse_kob::parse("d20"sv);

  REQUIRE(should_be_result.has_value());
  REQUIRE(*should_be_result == result);
  REQUIRE(!should_be_error.has_value());
  REQUIRE(should_be_error.error() == "That is not a valid roll type"sv);
}

TEST_CASE("Kids on Bikes graduate die", "[graduate_die]") {
  using namespace std::literals::string_view_literals;

  auto result = parse_kob::result{};

  result.graduate_die();
  REQUIRE(result.die_type == 6);

  result.graduate_die();
  REQUIRE(result.die_type == 8);

  result.graduate_die();
  REQUIRE(result.die_type == 10);

  result.graduate_die();
  REQUIRE(result.die_type == 12);

  result.graduate_die();
  REQUIRE(result.die_type == 20);

  result.graduate_die();
  REQUIRE(result.die_type == 20);
}
