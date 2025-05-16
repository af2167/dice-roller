export module system;

import std;
import dnd;
import kob;

enum class special_roll { none, advantage, disadvantage };

struct roller_base {
  virtual ~roller_base() = default;
  roller_base() = default;

  roller_base(roller_base&&) = delete;
  roller_base& operator=(roller_base&&) = delete;
  roller_base(const roller_base&) = delete;
  roller_base& operator=(const roller_base&) = delete;

  virtual std::optional<std::string> parse(std::string_view v) = 0;

  virtual std::string perform_roll(std::mt19937& gen) = 0;
};

template <typename T>
class roller_impl final : public roller_base {
  T t;

public:
  template <typename... Args>
  roller_impl(Args&&... args) : t{ std::forward<Args>(args)... } {
  }

  std::optional<std::string> parse(std::string_view v) override {
    return t.parse(v);
  }

  std::string perform_roll(std::mt19937& gen) {
    return t.perform_roll(gen);
  }
};

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

class system {
  std::unique_ptr<roller_base> base_;

public:
  template <dice_roller T>
  system(T&& t) : base_{ std::make_unique<roller_impl<T>>(std::forward<T>(t)) } {
  }

  std::optional<std::string> parse(std::string_view v) {
    return base_->parse(v);
  }

  std::string perform_roll(std::mt19937& gen) {
    return base_->perform_roll(gen);
  }
};
} // namespace sys
