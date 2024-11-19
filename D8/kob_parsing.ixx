export module kob;

import std;
import formatting;
import utility;

export namespace parse_kob {
struct result {
  int die_type = 4;

  auto operator<=>(const result&) const = default;

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

  template <typename Gen>
  std::string perform_roll(Gen& gen) {
    std::string result;
    auto total = 0;

    for (auto keep_rolling = true; keep_rolling;) {
      auto roll_result = utility::roll_die(die_type, gen);
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

std::expected<result, std::string> parse(std::string_view v) {
  auto maybe_die_type = utility::parse_from_chars<int>(v);
  if (maybe_die_type) {
    return result{ *maybe_die_type };
  }

  return std::unexpected(std::string{ "That is not a valid roll type" });
}
} // namespace parse_kob