export module utility;

import std;

export namespace utility {
template <typename... T>
struct overload : public T... {
  using T::operator()...;
};

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
struct check {
  std::variant<std::monostate, T, std::string> value;

  ~check() = default;
  check() = default;
  check(check&&) = default;
  check(const check&) = default;
  check& operator=(check&&) = default;
  check& operator=(const check&) = default;

  template <typename U>
  check(U&& u) : value{ std::forward<U>(u) } {
  }

  template <typename F>
  check or_else(F&& f) & {
    if (value.index() == 0) {
      return std::forward<F>(f)();
    }

    return *this;
  }

  template <typename F>
  check or_else(F&& f) && {
    if (value.index() == 0) {
      return std::forward<F>(f)();
    }

    return std::move(*this);
  }
};

} // namespace utility
