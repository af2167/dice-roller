export module dnd;

import std;
import utility;
import formatting;

import dependencies_without_module_support;

constexpr auto is_advantage_string = ctre::match<"^advantage\\s*([\\+\\-]?\\d+)?$">;
constexpr auto is_disadvantage_string = ctre::match<"^disadvantage\\s*([\\+\\-]?\\d+)?$">;
constexpr auto is_single_die = ctre::match<"^[dD]?(\\d+)$">;
constexpr auto is_full_dnd_expression = ctre::match<"^(\\d+)[dD]?(\\d+)([\\+\\-]\\d+[aA]?)?$">;

template <typename T>
std::optional<T> parse_modifier(std::string_view v) {
  if (v[0] == '+') {
    v = v.substr(1);
  }

  return utility::parse_from_chars<T>(v);
}

export namespace parse_dnd {
enum class special_roll { none, advantage, disadvantage };

struct result {
  special_roll special = special_roll::none;
  int die_type = 20;
  int die_count = 1;
  int modifier = 0;
  bool is_multimodifier = false;
  bool display_running_total = true;

  template <typename Gen>
  auto roll_not_special(Gen& gen) const {
    std::string result;
    auto total = 0;

    for (auto i = 0; i < die_count; ++i) {
      auto die = utility::roll_die(die_type, gen);
      total += die;

      if (i > 0) {
        result += ", ";
      }

      if (is_multimodifier || i == 0) {
        result += format::format_die(die_type, die, modifier);
        total += modifier;
      } else {
        result += format::format_die(die_type, die);
      }
    }

    if (display_running_total) {
      result += std::format("\nFor a total roll value of: {}", total);
    }

    return result;
  }

  template <typename Gen>
  auto roll_advantage_or_disadvantage(Gen& gen) const {
    auto die1 = utility::roll_die(20, gen);
    auto die2 = utility::roll_die(20, gen);

    if (special == special_roll::advantage) {
      return std::format("Advantage result: {}",
                         format::format_die(20, std::max(die1, die2), modifier));
    }

    return std::format("Disadvantage result: {}",
                       format::format_die(20, std::min(die1, die2), modifier));
  }

  template <typename Gen>
  std::string perform_roll(Gen& gen) const {
    switch (special) {
    case special_roll::none:
      return roll_not_special(gen);
    case special_roll::advantage:
      [[fallthrough]];
    case special_roll::disadvantage:
      return roll_advantage_or_disadvantage(gen);
    }

    throw std::format("An invalid special rolling state was parsed: {}",
                      std::to_underlying(special));
  }
};

utility::check<result> parse_coin(std::string_view v) {
  if (v == "coin") {
    return result{ .die_type = 2, .display_running_total = false };
  }

  return std::monostate{};
}

utility::check<result> parse_advantage(std::string_view v) {
  auto m = is_advantage_string(v);

  if (!m) {
    return std::monostate{};
  }

  auto matched_modifier = m.get<1>().to_view();
  auto mod = 0;
  if (matched_modifier.length() > 0) {
    auto maybe_mod = parse_modifier<int>(matched_modifier);
    if (!maybe_mod) {
      return std::format("invalid modifier: {}", matched_modifier);
    }

    mod = *maybe_mod;
  }

  return result{ .special = special_roll::advantage, .modifier = mod };
}

utility::check<result> parse_disadvantage(std::string_view v) {
  auto m = is_disadvantage_string(v);

  if (!m) {
    return std::monostate{};
  }

  auto matched_modifier = m.get<1>().to_view();
  auto mod = 0;
  if (matched_modifier.length() > 0) {
    auto maybe_mod = parse_modifier<int>(matched_modifier);
    if (!maybe_mod) {
      return std::format("invalid modifier: {}", matched_modifier);
    }

    mod = *maybe_mod;
  }

  return result{ .special = special_roll::disadvantage, .modifier = mod };
}

utility::check<result> parse_single_die(std::string_view v) {
  auto m = is_single_die(v);

  if (!m) {
    return std::monostate{};
  }

  auto matched_die_type = m.get<1>().to_view();
  auto maybe_die_type = utility::parse_from_chars<int>(matched_die_type);
  if (!maybe_die_type) {
    return std::format("invalid die type: {}", matched_die_type);
  }

  return result{ .die_type = *maybe_die_type };
}

utility::check<result> parse_otherwise(std::string_view v) {
  auto m = is_full_dnd_expression(v);

  if (!m) {
    return std::monostate{};
  }

  auto matched_die_count = m.get<1>().to_view();
  auto maybe_die_count = utility::parse_from_chars<int>(matched_die_count);
  if (!maybe_die_count) {
    return std::format("invalid die count: {}", matched_die_count);
  }

  auto matched_die_type = m.get<2>().to_view();
  auto maybe_die_type = utility::parse_from_chars<int>(matched_die_type);
  if (!maybe_die_type) {
    return std::format("invalid die type: {}", matched_die_type);
  }

  auto matched_modifier = m.get<3>().to_view();
  auto is_multimodifier = false;
  auto modifier = 0;
  if (matched_modifier.length() > 0) {
    if (std::tolower(matched_modifier[matched_modifier.length() - 1]) == 'a') {
      is_multimodifier = true;
      matched_modifier = matched_modifier.substr(0, matched_modifier.length() - 1);
    }

    auto maybe_modifier = parse_modifier<int>(matched_modifier);
    if (!maybe_modifier) {
      return std::format("invalid modifier: {}", matched_modifier);
    }
    modifier = *maybe_modifier;
  }

  return result{ .die_type = *maybe_die_type,
                 .die_count = *maybe_die_count,
                 .modifier = modifier,
                 .is_multimodifier = is_multimodifier };
}

} // namespace parse_dnd