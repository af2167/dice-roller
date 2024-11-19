export module system;

import std;
import dnd;
import kob;

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
  parse_dnd::result r_;

public:
  std::optional<std::string> parse(std::string_view v) {
    auto r = parse_dnd::parse_coin(v)
                 .or_else([&] { return parse_dnd::parse_advantage(v); })
                 .or_else([&] { return parse_dnd::parse_disadvantage(v); })
                 .or_else([&] { return parse_dnd::parse_single_die(v); })
                 .or_else([&] { return parse_dnd::parse_otherwise(v); });

    return std::visit(
        utility::overload{
            [](std::monostate) {
              return std::optional<std::string>{ "That is not a valid roll type" };
            },
            [this](parse_dnd::result& r) -> std::optional<std::string> {
              r_ = std::move(r);
              return std::nullopt;
            },
            [](std::string& s) -> std::optional<std::string> { return std::move(s); } },
        r.value);
  }

  template <typename Gen>
  std::string perform_roll(Gen& gen) const {
    return r_.perform_roll(gen);
  }
};

class kids_on_bikes_roller {
  parse_kob::result r_;

public:
  std::optional<std::string> parse(std::string_view v) {
    auto r = parse_kob::parse(v);

    if (r) {
      r_ = *r;
      return std::nullopt;
    }

    return std::move(r.error());
  }

  template <typename Gen>
  std::string perform_roll(Gen& gen) {
    return r_.perform_roll(gen);
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
