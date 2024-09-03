export module system;

import std;
import formatting;

constexpr std::string_view advantage = "advantage";
constexpr std::string_view disadvantage = "disadvantage";

template <typename Gen>
auto roll_die(int die, Gen&& gen) {
	std::uniform_int_distribution<> distrib(1, die);
	return distrib(std::forward<Gen>(gen));
}

template <typename T>
static std::optional<T> parse(std::string_view v) {
	T value = 0;
	if (std::from_chars(v.data(), v.data() + v.length(), value).ec == std::errc{})
		return value;
	else
		return std::nullopt;
}

export namespace sys {
	enum class special_roll {
		none,
		advantage,
		disadvantage
	};

	struct parts {
		int die_type = 20;
		int die_count = 1;
		int modifier = 0;
		bool is_multimodifier = false;
		special_roll special = special_roll::none;

		void graduate_die() {
			switch (die_type) {
			case 4: die_type = 6; break;
			case 6: die_type = 8; break;
			case 8: die_type = 10; break;
			case 10: die_type = 12; break;
			case 12: die_type = 20; break;
			}
		}
	};

	struct system_t {
		system_t(system_t&&) = delete;
		system_t(const system_t&) = delete;
		system_t& operator=(system_t&&) = delete;
		system_t& operator=(const system_t&) = delete;

		system_t() = default;
		virtual ~system_t() = default;

		virtual std::optional<parts> parse_input(std::string_view) = 0;

		virtual std::string perform_roll(parts p) = 0;
	};

	class dnd_system : public system_t {
		std::mt19937& gen_;

		std::optional<parts> parse_special(std::string_view v, std::string_view special_type, sys::special_roll type) {
			int modifier{};
			char plus{};

			if (v.length() > special_type.length()) {
				std::ispanstream s{ std::span{ v.data() + special_type.length(), v.length() } };
				s >> plus >> modifier;

				if (modifier == 0 || (plus != '+' && plus != '-')) {
					return std::nullopt;
				}

				if (plus == '-') {
					modifier *= -1;
				}
			}

			return{ sys::parts{.modifier = modifier, .special = type } };
		}

		std::optional<parts> parse_regular(std::string_view v) {
			std::ispanstream s{ std::span{ v.data(), v.length() } };

			int die_type{}, modifier{}, die_count{};
			bool is_multimodifier{};
			char d{}, plus{}, all{};
			s >> die_count >> d >> die_type;

			if (std::tolower(d) != 'd') {
				return std::nullopt;
			}

			if (!s.eof()) {
				s >> plus >> modifier;
				if (plus != '+' && plus != '-') {
					return std::nullopt;
				}
				if (plus == '-') {
					modifier *= -1;
				}
			}

			if (!s.eof()) {
				s >> all;
				if (all != 'a') {
					return std::nullopt;
				}
				is_multimodifier = true;
			}

			return std::optional<parts>{ parts{ die_type, die_count, modifier, is_multimodifier } };
		}

		auto roll_not_special(int die_count, int die_type, int modifier, bool is_multimodifier) {
			std::string result;
			auto total = 0;

			for (auto i = 0; i < die_count; ++i) {
				auto die = roll_die(die_type, gen_);
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

			result += std::format("\nFor a total roll value of: {}", total);
			return result;
		}

		auto roll_advantage_or_disadvantage(int modifier, bool is_advantage) {
			auto die1 = roll_die(20, gen_);
			auto die2 = roll_die(20, gen_);

			if (is_advantage) {
				return std::format("Advantage result: {}", format::format_die(20, std::max(die1, die2), modifier));
			}

			return std::format("Disadvantage result: {}", format::format_die(20, std::min(die1, die2), modifier));
		}

	public:
		dnd_system(std::mt19937& gen) : gen_{ gen } {}

		std::optional<parts> parse_input(std::string_view v) override {
			if (v.starts_with(advantage)) {
				return parse_special(v, advantage, special_roll::advantage);
			}
			if (v.starts_with(disadvantage)) {
				return parse_special(v, disadvantage, special_roll::disadvantage);
			}
			if (v == "coin") {
				return std::optional<parts>{ parts{ .die_type = 2 } };
			}
			if (std::tolower(v[0]) == 'd') {
				return parse<int>(v.substr(1))
					.transform([](auto i) { return parts{ i }; });
			}

			return parse_regular(v);
		}

		std::string perform_roll(parts p) override {
			switch (p.special) {
			case special_roll::none:
				return roll_not_special(p.die_count, p.die_type, p.modifier, p.is_multimodifier);
			case special_roll::advantage:
				return roll_advantage_or_disadvantage(p.modifier, true);
			case special_roll::disadvantage:
				return roll_advantage_or_disadvantage(p.modifier, false);
			}

			throw std::format("An invalid special rolling state was parsed: {}", std::to_underlying(p.special));
		}
	};

	class explosion_system : public system_t {
		std::mt19937& gen_;
	public:
		explosion_system(std::mt19937& gen) : gen_{ gen } {}

		std::optional<parts> parse_input(std::string_view v) override {
			return parse<int>(v)
				.transform([](auto i) { return parts{ i }; });
		}

		std::string perform_roll(parts p) override {
			std::string result;
			auto total = 0;

			for (auto keep_rolling = true; keep_rolling;) {
				auto roll_result = roll_die(p.die_type, gen_);
				total += roll_result;
				result += format::format_die(p.die_type, roll_result);
				if (roll_result != p.die_type) {
					keep_rolling = false;
				} else {
					result += " EXPLOSION, ";
					p.graduate_die();
				}
			}

			result += std::format("\nFor a total roll value of: {}", total);
			return result;
		}
	};

	using system = std::unique_ptr<system_t>;
}
