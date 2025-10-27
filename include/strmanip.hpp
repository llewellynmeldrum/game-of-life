#pragma once
#include <string>
char get_unit_prefix(int minify_lvl) ;
char get_num_prefix(int minify_lvl) ;
std::string float_to_str(float f, size_t precision);
std::string float_to_str_truncate_zeroes(float f, size_t precision);
std::string minify_unit(double num, size_t target_minify_lvl, size_t precision);
std::string minify_number(double num, size_t target_len);

template <typename T>
static inline const char *to_cstr(T val) {
	std::string* s = new std::string(std::to_string(val));
	return s->c_str();
}

template<typename T>
std::string comma_separate(T d) {
	std::string s;
	if constexpr(std::is_same<T, std::string>::value) {
		s = d;
	} else {
		s = std::to_string(static_cast<size_t>(d));
	}
	int digits_seen = 0;
	for (int i = s.size() - 1; i > 0 ; i--) {
		if (isdigit(s[i]))
			digits_seen++;
		if (digits_seen == 3) {
			s.insert(i, 1, ',');
			digits_seen = 0;
		}
	}
	return s;
}
