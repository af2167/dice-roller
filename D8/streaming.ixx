export module streaming;

import std;

namespace stream {
export auto getline(std::istream& in = std::cin) {
  std::string input;
  std::getline(in, input);
  return input;
}
} // namespace stream
