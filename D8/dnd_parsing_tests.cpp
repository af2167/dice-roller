import utility;
import dnd;

#include <catch2/catch_test_macros.hpp>

void should_be_nothing(const utility::check<parse_dnd::result>& r) {
  REQUIRE(r.value.index() == 0);
}

void should_be_error(const utility::check<parse_dnd::result>& r, std::string_view error_text) {
  REQUIRE(r.value.index() == 2);
  REQUIRE(std::get<2>(r.value) == error_text);
}

void should_be(const utility::check<parse_dnd::result>& r, const parse_dnd::result& result) {
  REQUIRE(r.value.index() == 1);
  REQUIRE(std::get<1>(r.value) == result);
}

TEST_CASE("Coin", "[parse_coin]") {
  using namespace std::literals::string_view_literals;

  auto coin_result = parse_dnd::result{ .die_type = 2, .display_running_total = false };

  should_be(parse_dnd::parse_coin("coin"sv), coin_result);
  should_be_nothing(parse_dnd::parse_coin("scoin"sv));
}

TEST_CASE("Advantage", "[parse_advantage]") {
  using namespace std::literals::string_view_literals;

  auto advantage_result = parse_dnd::result{ parse_dnd::special_roll::advantage };
  auto advantage_with_modifier_result =
      parse_dnd::result{ .special = parse_dnd::special_roll::advantage, .modifier = 5 };

  should_be(parse_dnd::parse_advantage("advantage"sv), advantage_result);
  should_be(parse_dnd::parse_advantage("advantage +5"sv), advantage_with_modifier_result);
  should_be_error(parse_dnd::parse_advantage("advantage 1000000000000000000000000"sv),
                  "invalid modifier: 1000000000000000000000000"sv);
  should_be_nothing(parse_dnd::parse_advantage("d20"sv));
}

TEST_CASE("Disadvantage", "[parse_disadvantage]") {
  using namespace std::literals::string_view_literals;

  auto disadvantage_result = parse_dnd::result{ parse_dnd::special_roll::disadvantage };
  auto disadvantage_with_modifier_result =
      parse_dnd::result{ .special = parse_dnd::special_roll::disadvantage, .modifier = 5 };

  should_be(parse_dnd::parse_disadvantage("disadvantage"sv), disadvantage_result);
  should_be(parse_dnd::parse_disadvantage("disadvantage +5"sv), disadvantage_with_modifier_result);
  should_be_error(parse_dnd::parse_disadvantage("disadvantage 1000000000000000000000000"sv),
                  "invalid modifier: 1000000000000000000000000"sv);
  should_be_nothing(parse_dnd::parse_disadvantage("d20"sv));
}

TEST_CASE("Single Die", "[parse_single_die]") {
  using namespace std::literals::string_view_literals;

  auto die_result = parse_dnd::result{};

  should_be(parse_dnd::parse_single_die("d20"sv), die_result);
  should_be_error(parse_dnd::parse_single_die("d1000000000000000000000000"sv),
                  "invalid die type: 1000000000000000000000000"sv);
  should_be_nothing(parse_dnd::parse_single_die("3d20"sv));
}

TEST_CASE("Full Dice Options", "[parse_otherwise]") {
  using namespace std::literals::string_view_literals;

  auto no_modifier_result = parse_dnd::result{ .die_type = 20, .die_count = 3 };
  auto modifier_result = parse_dnd::result{ .die_type = 20, .die_count = 3, .modifier = 5 };
  auto multimodifier_result =
      parse_dnd::result{ .die_type = 20, .die_count = 3, .modifier = 5, .is_multimodifier = true };

  should_be(parse_dnd::parse_otherwise("3d20"sv), no_modifier_result);
  should_be(parse_dnd::parse_otherwise("3d20+5"sv), modifier_result);
  should_be(parse_dnd::parse_otherwise("3d20+5a"sv), multimodifier_result);
  should_be_error(parse_dnd::parse_otherwise("1000000000000000000000000d20"sv),
                  "invalid die count: 1000000000000000000000000"sv);
  should_be_error(parse_dnd::parse_otherwise("2d1000000000000000000000000"sv),
                  "invalid die type: 1000000000000000000000000"sv);
  should_be_error(parse_dnd::parse_otherwise("2d20+1000000000000000000000000"sv),
                  "invalid modifier: +1000000000000000000000000"sv);
  should_be_nothing(parse_dnd::parse_otherwise("abc"sv));
}
