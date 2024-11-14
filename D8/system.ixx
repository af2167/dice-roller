export module system;

import std;
import formatting;

import dependencies_without_module_support;

constexpr auto is_advantage_string = ctre::match<"^advantage\\s*([\\+\\-]?\\d+)?$">;
constexpr auto is_disadvantage_string = ctre::match<"^disadvantage\\s*([\\+\\-]?\\d+)?$">;
constexpr auto is_single_die = ctre::match<"^[dD]?(\\d+)$">;
constexpr auto is_full_dnd_expression = ctre::match<"^(\\d+)[dD]?(\\d+)([\\+\\-]\\d+[aA]?)?$">;

template <typename Gen>
auto roll_die(int die, Gen& gen) {
  std::uniform_int_distribution<> distrib(1, die);

  return distrib(gen);
}

template <typename T>
std::optional<T> parse_from_chars(std::string_view v) {
  T value = 0;

  if (std::from_chars(v.data(), v.data() + v.length(), value).ec == std::errc{}) {
    return value;
  }

  return std::nullopt;
}

template <typename T>
std::optional<T> parse_modifier(std::string_view v) {
  if (v[0] == '+') {
    v = v.substr(1);
  }

  return parse_from_chars<T>(v);
}

enum class special_roll { none, advantage, disadvantage };

export namespace sys {
// clang-format off
template <typename T>
concept dice_roller = requires(T t, std::string_view v, std::mt19937 gen) {
  { t.parse(v) } -> std::same_as<std::optional<std::string>>;
  { t.perform_roll(gen) } -> std::same_as<std::string>;
};
// clang-format on

class dungeons_and_dragons_roller {
  special_roll special = special_roll::none;
  int die_type = 20;
  int die_count = 1;
  int modifier = 0;
  bool is_multimodifier = false;
  bool display_running_total = true;

  bool parse_coin(std::string_view v) {
    if (v == "coin") {
      die_type = 2;
      display_running_total = false;
      return true;
    }

    return false;
  }

  std::expected<bool, std::string> parse_advantage(std::string_view v) {
    if (auto m = is_advantage_string(v)) {
      special = special_roll::advantage;
      auto matched_modifier = m.get<1>().to_view();
      if (matched_modifier.length() > 0) {
        auto maybe_mod = parse_modifier<int>(matched_modifier);
        if (maybe_mod) {
          modifier = *maybe_mod;
        } else {
          return std::unexpected{ std::format("invalid modifier: {}", matched_modifier) };
        }
      }
      return true;
    }

    return false;
  }

  std::expected<bool, std::string> parse_disadvantage(std::string_view v) {
    if (auto m = is_disadvantage_string(v)) {
      special = special_roll::disadvantage;
      auto matched_modifier = m.get<1>().to_view();
      if (matched_modifier.length() > 0) {
        auto maybe_mod = parse_modifier<int>(matched_modifier);
        if (maybe_mod) {
          modifier = *maybe_mod;
        } else {
          return std::unexpected{ std::format("invalid modifier: {}", matched_modifier) };
        }
      }
      return true;
    }

    return false;
  }

  std::expected<bool, std::string> parse_single_die(std::string_view v) {
    if (auto m = is_single_die(v)) {
      auto matched_die_type = m.get<1>().to_view();
      auto maybe_die_type = parse_from_chars<int>(matched_die_type);
      if (maybe_die_type) {
        die_type = *maybe_die_type;
      } else {
        return std::unexpected{ std::format("invalid die type: {}", matched_die_type) };
      }
      return true;
    }

    return false;
  }

  std::expected<bool, std::string> parse_otherwise(std::string_view v) {
    if (auto m = is_full_dnd_expression(v)) {
      auto matched_die_count = m.get<1>().to_view();
      auto maybe_die_count = parse_from_chars<int>(matched_die_count);
      if (maybe_die_count) {
        die_count = *maybe_die_count;
      } else {
        return std::unexpected{ std::format("invalid die count: {}", matched_die_count) };
      }
      auto matched_die_type = m.get<2>().to_view();
      auto maybe_die_type = parse_from_chars<int>(matched_die_type);
      if (maybe_die_type) {
        die_type = *maybe_die_type;
      } else {
        return std::unexpected{ std::format("invalid die type: {}", matched_die_type) };
      }
      auto matched_modifier = m.get<3>().to_view();
      if (matched_modifier.length() > 0) {
        if (std::tolower(matched_modifier[matched_modifier.length() - 1]) == 'a') {
          is_multimodifier = true;
          matched_modifier = matched_modifier.substr(0, matched_modifier.length() - 1);
        }

        auto maybe_modifier = parse_modifier<int>(matched_modifier);
        if (maybe_modifier) {
          modifier = *maybe_modifier;
        } else {
          return std::unexpected{ std::format("invalid modifier: {}", matched_modifier) };
        }
      }
      return true;
    }

    return false;
  }

  template <typename Gen>
  auto roll_not_special(Gen& gen) const {
    std::string result;
    auto total = 0;

    for (auto i = 0; i < die_count; ++i) {
      auto die = roll_die(die_type, gen);
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
  auto roll_advantage_or_disadvantage(Gen& gen, bool is_advantage) const {
    auto die1 = roll_die(20, gen);
    auto die2 = roll_die(20, gen);

    if (is_advantage) {
      return std::format("Advantage result: {}",
                         format::format_die(20, std::max(die1, die2), modifier));
    }

    return std::format("Disadvantage result: {}",
                       format::format_die(20, std::min(die1, die2), modifier));
  }

public:
  std::optional<std::string> parse(std::string_view v) {
    special = special_roll::none;
    die_type = 20;
    die_count = 1;
    modifier = 0;
    is_multimodifier = false;
    display_running_total = true;

    if (parse_coin(v)) {
      return std::nullopt;
    }

    auto a = parse_advantage(v);
    if (a && *a) {
      return std::nullopt;
    } else if (!a) {
      return a.error();
    }

    auto d = parse_disadvantage(v);
    if (d && *d) {
      return std::nullopt;
    } else if (!d) {
      return d.error();
    }

    auto s = parse_single_die(v);
    if (s && *s) {
      return std::nullopt;
    } else if (!s) {
      return s.error();
    }

    auto o = parse_otherwise(v);
    if (o && *o) {
      return std::nullopt;
    } else if (!o) {
      return o.error();
    }

    return std::string{ "That is not a valid roll type" };
  }

  template <typename Gen>
  std::string perform_roll(Gen& gen) const {
    switch (special) {
    case special_roll::none:
      return roll_not_special(gen);
    case special_roll::advantage:
      return roll_advantage_or_disadvantage(gen, true);
    case special_roll::disadvantage:
      return roll_advantage_or_disadvantage(gen, false);
    }

    throw std::format("An invalid special rolling state was parsed: {}",
                      std::to_underlying(special));
  }
};

class kids_on_bikes_roller {
  int die_type = 4;

  void graduate_die() {
    switch (die_type) {
    case 4:
      die_type = 6;
      break;
    case 6:
      die_type = 8;
      break;
    case 8:
      die_type = 10;
      break;
    case 10:
      die_type = 12;
      break;
    case 12:
      die_type = 20;
      break;
    }
  }

public:
  std::optional<std::string> parse(std::string_view v) {
    auto maybe_die_type = parse_from_chars<int>(v);
    if (maybe_die_type) {
      die_type = *maybe_die_type;
      return std::nullopt;
    }

    return std::string{ "That is not a valid roll type" };
  }

  template <typename Gen>
  std::string perform_roll(Gen& gen) {
    std::string result;
    auto total = 0;

    for (auto keep_rolling = true; keep_rolling;) {
      auto roll_result = roll_die(die_type, gen);
      total += roll_result;
      result += format::format_die(die_type, roll_result);
      if (roll_result != die_type) {
        keep_rolling = false;
      } else {
        result += " EXPLOSION, ";
        graduate_die();
      }
    }

    result += std::format("\nFor a total roll value of: {}", total);
    return result;
  }
};

enum class rolling_type { dungeons_and_dragons, kids_on_bikes };

struct system {
  std::variant<dungeons_and_dragons_roller, kids_on_bikes_roller> roller{};

  std::optional<std::string> parse(std::string_view v) {
    return std::visit([v](auto& r) { return r.parse(v); }, roller);
  }

  template <typename Gen>
  std::string perform_roll(Gen& gen) {
    return std::visit([&gen](auto& r) { return r.perform_roll(gen); }, roller);
  }
};
} // namespace sys
