export module formatting;

import std;

static std::string formatted_result(auto result) {
	return std::format("({})", result);
}

std::string formatted_coin(bool value) {
	return formatted_result(value ? 'H' : 'T');
}

static std::string formatted_bonus(std::integral auto result, std::integral auto bonus) {
	using namespace std::string_literals;
	return bonus != 0 ? std::format(" + {} = [{}]", bonus, result + bonus) : ""s;
}

static std::string format_roll_output(std::integral auto die_type, std::integral auto result) {
	return die_type == 2 ? formatted_coin(result == 1) : formatted_result(result);
}

namespace format {
	export inline std::string format_die(std::integral auto die_type, std::integral auto result, std::integral auto bonus) {
		return std::format("{}{}", format_roll_output(die_type, result), formatted_bonus(result, bonus));
	}

	export inline std::string format_die(std::integral auto die_type, std::integral auto result) {
		return std::format("{}", format_roll_output(die_type, result));
	}
}
