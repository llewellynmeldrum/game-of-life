#include <cstdio>
#include <chrono>
#include <iomanip>
#include <random>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <cmath>

#include "SDMTL.hpp"
#include "CHRONO_WRAPPER.hpp"
#define bitsof(T)	(sizeof(T)*8)
using std::vector;
using std::string;
using std::to_string;
using work_t = float;
constexpr auto epsilon = std::numeric_limits<work_t>::epsilon();
constexpr work_t rand_min = -1'000'000;
constexpr work_t rand_max = 1'000'000;
using std::cout;
using std::endl;

template<typename T>
string comma_separate(T d) {
	string s;
	if constexpr(std::is_same<T, string>::value) {
		s = d;
	} else {
		s = to_string(static_cast<size_t>(d));
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

void work(size_t i, vector<work_t> &A, vector<work_t> &B, vector<work_t> &res) {
	res[i] = A[i] * B[i];
}


// *INDENT-OFF*
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

class MCPU_INFO{
// TODO: 
// - before anything, reorganize this file, it is a mess
// - create methods to fill fields like cpu freq, cores, etc
// - use systemctl 

};

string minify_unit(double num, size_t target_minify_lvl, size_t precision) {
	// target len includes suffix (K,M,B,T,Q)
	size_t minify_lvl = 0;
	while (minify_lvl<target_minify_lvl){
		num = (double) num/1000.0;
		minify_lvl++;
	}
	string s = float_to_str(num, precision);
	char prefix = get_unit_prefix(minify_lvl);
	if (prefix==' ') return s;
	return s + prefix;
}

string minify_number(double num, size_t target_len) {
	size_t minify_lvl = 0;
	while (float_to_str_truncate_zeroes(num,2).size()>target_len){
		num = (double) num/1000.0;
		minify_lvl++;
	}
	string s = float_to_str_truncate_zeroes(num, 2);
	char prefix = get_num_prefix(minify_lvl);
	if (prefix==' ') return s;
	return s + prefix;
}

bool float_equals(work_t a, work_t b) {
	return abs(a - b) < epsilon;
}

bool check_work(vector<work_t> &A, vector<work_t> &B, vector<work_t> &res) {
	size_t incorrect = 0;
	for (size_t i = 0; i < A.size(); i++) {
		if (!float_equals(A[i]*B[i], res[i])) {
			++incorrect;
		}
	}
	return (incorrect == 0);

}

void randomize_vec(vector<work_t> &v) {
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);

	// 2. Define a distribution for floating-point numbers
	// This creates a uniform distribution between 0.0 and 1.0 (inclusive).
	std::uniform_real_distribution<work_t> distribution(rand_min, rand_max);

	// 3. Create a vector to store the random floats
	int vector_size = 10;
	std::vector<work_t> random_floats(vector_size);

	// 4. Fill the vector with random floats
	for (int i = 0; i < vector_size; ++i) {
		v[i] = distribution(generator);
	}
}

double calc_ns_per_op(size_t ns_elapsed, size_t ops_completed) {
	return ns_elapsed / (work_t)ops_completed;
}
double calc_flops(size_t ns_elapsed, size_t ops_completed) {
	return (work_t)ops_completed / (ns_elapsed / 1'000'000'000.0);
}
double calc_gflops(size_t ns_elapsed, size_t ops_completed) {
	return ((work_t)ops_completed / (ns_elapsed / 1'000'000'000.0)) / 1'000'000'000;
}
double calc_tflops(size_t ns_elapsed, size_t ops_completed) {
	return calc_gflops(ns_elapsed, ops_completed) / 1'000;
}
// flosp = ops/(ns/1'000'000'000)
// tflops = flops/1'000'000'000'000
//

double calc_theoretical_tflops() {
	// lookup table based on device id or something is the only other way i can think to do this
	// i use an m1 pro so this is a starting point
	// also this is FP32
	return 5.3;
}

constexpr size_t MAX_STACK_ALLOWANCE = 2'000'000;
double ns_to_do_work_heap(size_t work_count) {
	CLOCK clock;

	auto A = new vector<work_t>(work_count, 0 );
	auto B = new vector<work_t>(work_count, 0 );
	auto res = new vector<work_t>(work_count, 0 );

	randomize_vec(*A);
	randomize_vec(*B);
	//warmup
	for (size_t i = 0; i < work_count; i++) {
		work(i, *A, *B, *res);
	}

	randomize_vec(*A);
	randomize_vec(*B);

	clock.start();
	for (size_t i = 0; i < work_count; i++) {
		work(i, *A, *B, *res);
	}
	clock.stop();
//	check_work(*A, *B, *res);

	size_t ns = clock.get_ns_between();

	delete A;
	delete B;
	delete res;

	return ns;
}
double ns_to_do_work_stack(size_t work_count) {
	CLOCK clock;

	auto A = vector<work_t>(work_count, 0 );
	auto B = vector<work_t>(work_count, 0 );
	auto res = vector<work_t>(work_count, 0 );

	randomize_vec(A);
	randomize_vec(B);
	//warmup
	for (size_t i = 0; i < work_count; i++) {
		work(i, A, B, res);
	}

	randomize_vec(A);
	randomize_vec(B);

	clock.start();
	for (size_t i = 0; i < work_count; i++) {
		work(i, A, B, res);
	}
	clock.stop();
//	check_work(*A, *B, *res);

	size_t ns = clock.get_ns_between();


	return ns;
}
double ns_to_do_work(size_t work_count) {
	if (work_count*sizeof(work_t) > MAX_STACK_ALLOWANCE){
		return 	ns_to_do_work_heap(work_count);
	}
	return ns_to_do_work_stack(work_count);
}

double percent_of_theoretical_flops(size_t ns, size_t work_count) {
	return calc_tflops(ns, work_count) / calc_theoretical_tflops() * 100.0;
}

template <typename T>
static inline const char *to_cstr(T val) {
	string* s = new string(std::to_string(val));
	return s->c_str();
}

struct Worker {

	size_t N;
	size_t ns;
	size_t ns_outer;
	int col_width = 12;
	vector<vector<string>> results;

	void do_work(size_t N) {
		this->N = N;
		CLOCK outer_clock;
		outer_clock.start();
		ns = ns_to_do_work(N);
		outer_clock.stop();
		ns_outer = outer_clock.get_ns_between();
	}

	void calc_results(){
		size_t flops = calc_flops(ns, N);
		results.push_back({
			minify_number(N, 5),
			minify_unit(N*sizeof(work_t), 1, 2) + "B",
			comma_separate(ns / 1'000'000) + "ms",
			comma_separate(ns_outer / 1'000'000) + "ms",
			minify_unit(flops, 2, 2),
			to_string(percent_of_theoretical_flops(ns, N)) + "%"
		});
	}
	void print_results() {
		for (auto res : results) {
			for (auto col: res) {
				printf("%-*.*s ", col_width, col_width, col.c_str());
			}
			printf("\n");
		}
	}
};
int main() {

	CLOCK prog_clock;
	prog_clock.start();
	vector<Worker> workers;
	for (size_t N = 100; N < 10'000'000; N = N*1.5) {
		Worker w;
		w.do_work(N);
		workers.push_back(w);
	}

	Worker w = workers[0];
	printf("%-*.*s ", w.col_width, w.col_width, "N");
	printf("%-*.*s ", w.col_width, w.col_width, "INPUT SZ");
	printf("%-*.*s ", w.col_width, w.col_width, "MS CALC");
	printf("%-*.*s ", w.col_width, w.col_width, "MS OVERALL");
	printf("%-*.*s ", w.col_width, w.col_width, "FLOPS");
	printf("%-*.*s ", w.col_width, w.col_width, "% POSSIBLE FLOPS");
	printf("\n");
	for (auto w : workers){
		w.calc_results();
		w.print_results();
	}
	printf("----------\ntook %0.2lfs\n", prog_clock.s_since_start());

}


