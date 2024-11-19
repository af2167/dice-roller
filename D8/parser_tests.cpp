import parser;
import system;

#include <catch2/catch_test_macros.hpp>

template <std::size_t Result = 0, std::size_t MAX = 20>
struct fake_gen {
  std::size_t current_state = Result;

  using result_type = std::size_t;
  static std::size_t min() {
    return 0;
  }
  static std::size_t max() {
    return MAX;
  }

  std::size_t operator()() {
    return current_state++;
  }
};

TEST_CASE("Command line formatting failures", "[from_argument_list]") {
  using namespace std::literals::string_view_literals;

  auto fake = fake_gen{};

  REQUIRE(parsing::from_argument_list(std::array<std::string_view, 0>{}, fake) ==
          parsing::TOO_FEW_ARGUMENTS);
  REQUIRE(parsing::from_argument_list(std::array<std::string_view, 3>{}, fake) ==
          parsing::TOO_MANY_ARGUMENTS);
  REQUIRE(parsing::from_argument_list(std::array{ "-h"sv }, fake) == parsing::HELP_TEXT_ARG);
  REQUIRE(parsing::from_argument_list(std::array{ "-h"sv, "anything else"sv }, fake) ==
          parsing::HELP_TEXT_ARG);
  REQUIRE(parsing::from_argument_list(std::array{ "h"sv }, fake) == parsing::UNKNOWN_COMMAND);
  REQUIRE(parsing::from_argument_list(std::array{ "-d"sv }, fake) == parsing::MISSING_ROLL_INFO);
  REQUIRE(parsing::from_argument_list(std::array{ "-k"sv }, fake) == parsing::MISSING_ROLL_INFO);
}

TEST_CASE("DnD formatted result", "[from_argument_list]") {
  using namespace std::literals::string_view_literals;

  auto fake = fake_gen{};

  fake.current_state = 0;
  REQUIRE(fake.current_state == 0);

  SECTION("coin flip") {
    REQUIRE(parsing::from_argument_list(std::array{ "-d"sv, "coin"sv }, fake) == "(H)"sv);
    REQUIRE(parsing::from_argument_list(std::array{ "-d"sv, "coin"sv }, fake) == "(T)"sv);
  }

  SECTION("advantage") {
    REQUIRE(parsing::from_argument_list(std::array{ "-d"sv, "advantage"sv }, fake) ==
            "Advantage result: (16)"sv);
    REQUIRE(parsing::from_argument_list(std::array{ "-d"sv, "advantage 1"sv }, fake) ==
            "Advantage result: (10) + 1 = [11]"sv);
    REQUIRE(parsing::from_argument_list(std::array{ "-d"sv, "advantage +1"sv }, fake) ==
            "Advantage result: (18) + 1 = [19]"sv);
  }

  SECTION("disadvantage") {
    REQUIRE(parsing::from_argument_list(std::array{ "-d"sv, "disadvantage"sv }, fake) ==
            "Disadvantage result: (2)"sv);
  }

  SECTION("Single die") {
    REQUIRE(parsing::from_argument_list(std::array{ "-d"sv, "d20"sv }, fake) ==
            "(2)\nFor a total roll value of: 2"sv);
  }

  SECTION("Multiple dice no bonues") {
    REQUIRE(parsing::from_argument_list(std::array{ "-d"sv, "2d20"sv }, fake) ==
            "(2), (16)\nFor a total roll value of: 18"sv);
  }

  SECTION("Mulitple dice with bonus") {
    REQUIRE(parsing::from_argument_list(std::array{ "-d"sv, "2d20+1"sv }, fake) ==
            "(2) + 1 = [3], (16)\nFor a total roll value of: 19"sv);
    REQUIRE(parsing::from_argument_list(std::array{ "-d"sv, "2d20-1"sv }, fake) ==
            "(10) + -1 = [9], (4)\nFor a total roll value of: 13"sv);
    REQUIRE(parsing::from_argument_list(std::array{ "-d"sv, "2d20+4a"sv }, fake) ==
            "(18) + 4 = [22], (12) + 4 = [16]\nFor a total roll value of: 38"sv);
  }
}

TEST_CASE("DnD formatted result parsing failures", "[from_argument_list]") {
  using namespace std::literals::string_view_literals;

  auto fake = fake_gen{};

  REQUIRE(parsing::from_argument_list(std::array{ "-d"sv, "adv"sv }, fake) ==
          "That is not a valid roll type"sv);
  REQUIRE(parsing::from_argument_list(std::array{ "-d"sv, "advantage 100000000000000000000000"sv },
                                      fake) == "invalid modifier: 100000000000000000000000"sv);
  REQUIRE(
      parsing::from_argument_list(std::array{ "-d"sv, "disadvantage 100000000000000000000000"sv },
                                  fake) == "invalid modifier: 100000000000000000000000"sv);
  REQUIRE(parsing::from_argument_list(std::array{ "-d"sv, "d100000000000000000000000"sv }, fake) ==
          "invalid die type: 100000000000000000000000"sv);
  REQUIRE(parsing::from_argument_list(
              std::array{ "-d"sv, "100000000000000000000000d100000000000000000000000"sv }, fake) ==
          "invalid die count: 100000000000000000000000"sv);
  REQUIRE(parsing::from_argument_list(std::array{ "-d"sv, "3d100000000000000000000000"sv }, fake) ==
          "invalid die type: 100000000000000000000000"sv);
  REQUIRE(parsing::from_argument_list(std::array{ "-d"sv, "3d20+100000000000000000000000"sv },
                                      fake) == "invalid modifier: +100000000000000000000000"sv);
}

TEST_CASE("Kids on Bikes formatted result", "[from_argument_list]") {
  using namespace std::literals::string_view_literals;

  auto fake = fake_gen{ 3 };

  REQUIRE(parsing::from_argument_list(std::array{ "-k"sv, "4"sv }, fake) ==
          "(4) EXPLOSION, (5)\nFor a total roll value of: 9"sv);
  REQUIRE(parsing::from_argument_list(std::array{ "-k"sv, "20"sv }, fake) ==
          "(7)\nFor a total roll value of: 7"sv);
}

TEST_CASE("Kids on Bikes formatted result parsing failures", "[from_argument_list]") {
  using namespace std::literals::string_view_literals;

  auto fake = fake_gen{};

  REQUIRE(parsing::from_argument_list(std::array{ "-k"sv, "d20"sv }, fake) ==
          "That is not a valid roll type"sv);
}
