// main.cpp
// This program rolls any die that the user would like
// It terminates after the user no longer needs it as defined by the user
// This program is optimized for D&D
// Die rolls are denoted by (R) actual values (including bonuses) are denoted by
// [V] By: Alex Frazier

import std;

import parser;

auto to_vector = [](int argc, const char **argv) {
  std::vector<std::string_view> args{};
  args.reserve(argc - 1);

  for (int i = 1; i < argc; ++i) {
    args.push_back(argv[i]);
  }

  return args;
};

int main(int argc, const char **argv) {
  std::random_device rd;
  std::mt19937 gen{ rd() };

  if (argc > 1) {
    parsing::from_argument_list(to_vector(argc, argv), gen);
    return 0;
  }

  std::println("Welcome to the dice rolling application.");
  std::println("You may enter the command 'help' at any time to get a list of options.");

  parsing::from_command_line(gen);
}
