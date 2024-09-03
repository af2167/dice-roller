export module roll_description;

import std;
import formatting;
import streaming;
import utility;

export enum class special_roll {
	none,
	advantage,
	disadvantage
};

export struct roll_result {
	int number_on_die{};
	int bonus{};
	std::string formatted_die{};
};

// Function written to eliminate redundant code
export roll_result roll_die(int die, int bonus) {
	auto roll_result = std::rand() % die + 1;
	auto format = format_die(die, roll_result, bonus);
	return{ roll_result, bonus, format };
}

// Function written to eliminate redundant code
export template <typename Callable>
roll_result roll_die(int die, int bonus, Callable&& rng) {
	auto roll_result = std::forward<Callable>(rng)() % die + 1;
	auto format = format_die(die, roll_result, bonus);
	return{ roll_result, bonus, format };
}

export struct roll_description {
	int die_type{};
	int die_count{};
	int bonus{};
	bool bonus_applies_to_all = false;
	bool keep_running_total = false;
	special_roll is_special = special_roll::none;

	// Function used to roll multiple dice
	// Takes into account if the roll bonus applies to all dice or just one
	std::string perform_roll() const {
		roll_result result, previous_result;
		auto total_score = 0;
		std::string format;

		for (auto i = 0; i < die_count; ++i) {
			previous_result = result;
			if (bonus_applies_to_all || i == 0) {
				result = roll_die(die_type, bonus);
			} else {
				result = roll_die(die_type, 0);
			}
			total_score += result.number_on_die + result.bonus;
			format += result.formatted_die;
		}

		if (keep_running_total && total_score > 0) {
			format += std::format("For a total roll value of: {}\n", total_score);
		}

		if (is_special == special_roll::advantage) {
			format += std::format("Advantage result: {}", format_die(die_type, std::max(result.number_on_die, previous_result.number_on_die), bonus));
		}

		if (is_special == special_roll::disadvantage) {
			format += std::format("Disadvantage result: {}", format_die(die_type, std::min(result.number_on_die, previous_result.number_on_die), bonus));
		}

		return format;
	}

	void get_bonus(std::ostream& out = std::cout, std::istream& in = std::cin) {
		std::print(out, "Does the bonus apply for all die rolls? (Y or N)? ");
		if (bonus > 0 && stream::user_answered_yes(in)) {
			bonus_applies_to_all = true;
		}
	}
	void determine_if_running_total(std::ostream& out = std::cout, std::istream& in = std::cin) {
		std::print(out, "Would you like a total value printed? (Y or N)? ");
		if (die_count > 1 && stream::user_answered_yes(in)) {
			keep_running_total = true;
		}
	}
	void handle_special_cases(std::ostream& out = std::cout, std::istream& in = std::cin) {
		if (die_type == 20 && die_count == 2) {
			std::print(out, "Would you like a total value printed? (Y or N)? ");
			if (yes_or_no_message("Is this a roll with advantage? (Y or N)? ", out, in)) {
				is_special = special_roll::advantage;
			} else if (yes_or_no_message("Is this a roll with disadvantage? (Y or N)? ", out, in)) {
				is_special = special_roll::disadvantage;
			}
		}
	}
};

export roll_description handle_special_cases(special_roll type, std::ostream& out = std::cout, std::istream& in = std::cin) {
	if (is_one_of(type, special_roll::advantage, special_roll::disadvantage)) {
		auto bonus = 0;
		if (yes_or_no_message("Is there a bonus? (Y or N) ", out, in)) {
			bonus = get<int>("What is it? ", out, in);
		}
		return{ .die_type = 20, .die_count = 2, .bonus = bonus, .bonus_applies_to_all = true, .is_special = type };
	}

	throw "Invalid format\n";
}

export roll_description parse_roll(std::string v, std::ostream& out = std::cout, std::istream& in = std::cin) {
	if (v == "advantage") {
		return handle_special_cases(special_roll::advantage, out, in);
	}
	if (v == "disadvantage") {
		return handle_special_cases(special_roll::disadvantage, out, in);
	}

	std::istringstream s{ std::move(v) };

	int die_type{}, die_count{}, bonus{};
	char d{}, plus{};
	s >> die_count >> d >> die_type;
	if (!s.eof()) {
		s >> plus >> bonus;
	}

	if (s.fail() || !s.eof()) throw "Invalid format\n";
	if (std::tolower(d) != 'd') throw "The formatted string did not indicate the die type.\n";
	if (bonus != 0 && plus != '+') throw "The formatted string had values after the die type that failed to indicate the bonus.\n";
	if (!is_one_of(die_type, 2, 4, 6, 8, 10, 12, 20, 100)) throw "Invalid die type\n";

	roll_description roll_parsed{ die_type, die_count, bonus };

	roll_parsed.get_bonus(out, in);
	roll_parsed.determine_if_running_total(out, in);
	roll_parsed.handle_special_cases(out, in);

	return roll_parsed;
}