// Source.cpp (In project D8)
// This program rolls any die that the user would like
// It terminates after the user no longer needs it as defined by the user
// This program is optimized for D&D
// Die rolls are denoted by (R) actual values (including bonuses) are denoted by [V]
// By: Alex Frazier

import std;
import streaming;
import system;

constexpr std::string_view help_text = R"(When prompted for a dice rolling system you have two options:
    1) DnD (or 'd' for short)
    2) explosion (or 'e' for short)
When rolling a die in the 'explosion' system, simply type the die number (e.g. 4)
When rolling in the DnD system, you have a handful of options:
    1) d20
        a) This expample rolls a single d20
        b) You may substitute the number for whichever die you prefer to roll
    2) 4d4+7
        a) This expample rolls four d4 and adds 7 to the total
        b) The modifier can be positive, negative, or omitted entirely
        c) You may subsitiute any of the numbers as needed
    3) 4d4+7a
        a) This expample is almost identical to example '2' but applies the bonus to each die
    4) advantage +3
        a) This expample makes a single roll with advantage and adds 3 to the final result
        b) The modifier can be positive, negative, or omitted entirely
    5) disadvantage +3
        a) This expample makes a single roll with disadvantage and adds 3 to the final result
        b) The modifier can be positive, negative, or omitted entirely
    6) coin
        a) flip a coin
You may enter the command 'exit' at any time to end the application.
)";

constexpr std::string_view help = "help";
constexpr std::string_view exit_prompted = "exit";
constexpr std::string_view switch_prompted = "switch";

enum class unexpected_prompt {
	exit,
	change
};

static std::expected<std::string, unexpected_prompt> get_response(std::format_string<> prompt) {
	while (true) {
		std::print(prompt);
		auto result = stream::getline();
		if (result == help) {
			using namespace std::chrono_literals;

			std::println(help_text);
			std::this_thread::sleep_for(200ms);
			std::print(prompt);
		} else if (result == exit_prompted) {
			return std::unexpected(unexpected_prompt::exit);
		} else if (result == switch_prompted) {
			return std::unexpected(unexpected_prompt::change);
		} else {
			return result;
		}
	}
}

static std::optional<sys::system> choose_system(std::string_view choice) {
	if (choice.length() < 1) {
		return std::nullopt;
	}

	if (choice == "DnD" || std::tolower(choice[0]) == 'd') {
		return{ sys::rolling_type::dungeons_and_dragons };
	} else if (choice == "explosion" || std::tolower(choice[0]) == 'e') {
		return{ sys::rolling_type::kids_on_bikes };
	}

	return std::nullopt;
}

static std::expected<sys::system, unexpected_prompt> prompt_for_system() {
	while (true) {
		auto response = get_response("What system would you like to use? ");
		if (!response) {
			return std::unexpected(response.error());
		}
		auto result = choose_system(*response);
		if (result.has_value()) {
			return std::move(*result);
		}
		std::println("That system cannot be used with this tool.");
	}
}

static unexpected_prompt prompt_for_roll(sys::system& system, std::mt19937& gen) {
	while (true) {
		auto input = get_response("Please describe the roll: ");

		if (!input) {
			return input.error();
		}

		auto parse_error = system.parse(*input);
		if (parse_error) {
			std::println("{}", *parse_error);
		} else {
			std::println("{}", system.perform_roll(gen));
		}
	}
}

int main() {
	std::random_device rd;
	std::mt19937 gen{ rd() };

	std::println("Welcome to the dice rolling application.");
	std::println("You may enter the command 'help' at any time to get a list of options.");

	auto system = prompt_for_system();

	while (system) {
		auto escape = prompt_for_roll(system.value(), gen);

		switch (escape) {
		case unexpected_prompt::change: system = prompt_for_system(); break;
		case unexpected_prompt::exit: return 0;
		}
	}
}