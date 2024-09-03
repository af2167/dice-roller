export module utility;

export constexpr bool is_one_of(const auto&) { return false; }
export constexpr bool is_one_of(const auto& test_value, const auto& first_test, const auto& ... other_values) {
	return test_value == first_test || is_one_of(test_value, other_values...);
}
