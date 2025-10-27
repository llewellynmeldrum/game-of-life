
// *INDENT-OFF*
#include <cstddef>
#include <string>
#include <sstream>
#include <iostream>
#include <numeric>
#include <type_traits>
#include <iomanip>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <cmath>
#include "strmanip.hpp"
using std::string;

string minify_unit(double num, size_t target_minify_lvl, size_t precision) {
	// target len includes suffix (K,M,B,T,Q)
	size_t minify_lvl = 0;
	while (minify_lvl < target_minify_lvl) {
		num = (double) num / 1000.0;
		minify_lvl++;
	}
	string s = float_to_str(num, precision);
	char prefix = get_unit_prefix(minify_lvl);
	if (prefix == ' ') return s;
	return s + prefix;
}

string minify_number(double num, size_t target_len) {
	size_t minify_lvl = 0;
	while (float_to_str_truncate_zeroes(num, 2).size() > target_len) {
		num = (double) num / 1000.0;
		minify_lvl++;
	}
	string s = float_to_str_truncate_zeroes(num, 2);
	char prefix = get_num_prefix(minify_lvl);
	if (prefix == ' ') return s;
	return s + prefix;
}
char get_unit_prefix(int minify_lvl) {
	switch(minify_lvl) {
	case 1: return 'K'; break;
	case 2: return 'M'; break;
	case 3: return 'G'; break;
	case 5: return 'T'; break;
	case 6: return 'P'; break;
	}
	return ' ';
}
char get_num_prefix(int minify_lvl) {
	switch(minify_lvl) {
	case 1: return 'k'; break; // as in thousand
	case 2: return 'm'; break; // as in million 
	case 3: return 'b'; break; // as in billion
	case 4: return 't'; break; // as in trillion
	case 5: return 'q'; break; // as in quintillion
	}
	return ' ';
}


string float_to_str(float f, size_t precision){

        std::ostringstream ss;
        ss << std::fixed << std::setprecision(precision) << f;
	string s = ss.str();
	return s;
}

string float_to_str_truncate_zeroes(float f, size_t precision){
	string s = float_to_str(f,precision);
//	cout << "ftostr" << s << endl;
	if (s.size()<=3) return s;
	char last_ch = s.at(s.size()-1);
	while (s.size()>3){
		last_ch = s.at(s.size()-1);
		if (last_ch=='0')
			s.erase(s.size()-1);
		else 
			break;
	}
	if (s.at(s.size()-1)=='.'){
		s.erase(s.size()-1);
	}
	return s;
}
