export module parser;

import std;
import system;
import streaming;

import dependencies_without_module_support;

constexpr std::string_view help_text =
    R"(When prompted for a dice rolling system you have two options:
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

constexpr std::string_view help_arg = "-h";
constexpr std::string_view dnd_arg = "-d";
constexpr std::string_view kob_arg = "-k";

constexpr std::string_view help = "help";
constexpr std::string_view exit_prompted = "exit";
constexpr std::string_view switch_prompted = "switch";

constexpr auto is_dnd_string = ctre::match<"^[dD]([nN][dD])?$">;
constexpr auto is_kob_string = ctre::match<"^[kK]([oO][bB])?$">;

enum class unexpected_prompt { exit, change };

std::expected<std::string, unexpected_prompt> get_response(std::format_string<> prompt,
                                                           std::istream& in = std::cin) {
  while (true) {
    std::print(prompt);
    auto result = stream::getline(in);
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

std::optional<sys::system> choose_system(std::string_view choice) {
  if (choice.length() < 1) {
    return std::nullopt;
  }

  if (is_dnd_string(choice)) {
    return std::optional<sys::system>{ std::in_place, sys::dungeons_and_dragons_roller{} };
  } else if (is_kob_string(choice)) {
    return std::optional<sys::system>{ std::in_place, sys::kids_on_bikes_roller{} };
  }

  return std::nullopt;
}

std::expected<sys::system, unexpected_prompt> prompt_for_system() {
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

template <typename Gen>
std::string perform_single_roll(sys::dice_roller auto&& roller, std::string_view v, Gen& gen) {
  auto parse_error = roller.parse(v);
  if (parse_error) {
    return *parse_error;
  } else {
    return roller.perform_roll(gen);
  }
}

template <typename Gen>
unexpected_prompt prompt_for_roll(sys::system& system, Gen& gen) {
  while (true) {
    auto input = get_response("Please describe the roll: ");

    if (!input) {
      return input.error();
    }

    std::println("{}", perform_single_roll(system, *input, gen));
  }
}

export namespace parsing {
const std::string TOO_FEW_ARGUMENTS = "Too few arguments entered. Enter '-h' for help";
const std::string TOO_MANY_ARGUMENTS = "Too many arguments entered. Enter '-h' for help";
const std::string MISSING_ROLL_INFO = "You must enter the roll information. Enter '-h' for help";
const std::string UNKNOWN_COMMAND = "Unrecognized command. Enter '-h' for help";
const std::string HELP_TEXT_ARG = R"(-h => help
-d => roll using the dungeons and dragons system
-k => roll using the kids on bikes system)";

template <typename Gen>
void from_command_line(Gen& gen) {
  auto system = prompt_for_system();

  while (system) {
    auto escape = prompt_for_roll(system.value(), gen);

    switch (escape) {
    case unexpected_prompt::change:
      system = prompt_for_system();
      break;
    case unexpected_prompt::exit:
      return;
    }
  }
}

template <typename Gen>
std::string from_argument_list(std::span<const std::string_view> strings, Gen& gen) {
  if (strings.size() < 1) {
    return TOO_FEW_ARGUMENTS;
  }

  if (strings.size() > 2) {
    return TOO_MANY_ARGUMENTS;
  }

  if (strings[0] == help_arg) {
    return HELP_TEXT_ARG;
  }

  if (strings[0] == kob_arg) {
    if (strings.size() == 2) {
      return perform_single_roll(sys::kids_on_bikes_roller{}, strings[1], gen);
    }

    return MISSING_ROLL_INFO;
  }

  if (strings[0] == dnd_arg) {
    if (strings.size() == 2) {
      return perform_single_roll(sys::dungeons_and_dragons_roller{}, strings[1], gen);
    }

    return MISSING_ROLL_INFO;
  }

  return UNKNOWN_COMMAND;
}
} // namespace parsing