#include <cstdio>
#include <chrono>
#include <random>
#include <unistd.h>
#include <vector>
#include <cmath>

#include "SDMTL.hpp"
#include "CHRONO_WRAPPER.hpp"
#define bitsof(T)	(sizeof(T)*8)
using std::vector;
using std::string;
using work_t = float;
constexpr auto epsilon = std::numeric_limits<work_t>::epsilon();
constexpr work_t rand_min = -1'000'000;
constexpr work_t rand_max = 1'000'000;

template<typename T>
string comma_separate(T d) {
	string s = std::to_string((size_t)d);
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

bool fequal(work_t a, work_t b) {
	return abs(a - b) < epsilon;
}

bool check_work(vector<work_t> &A, vector<work_t> &B, vector<work_t> &res) {
	size_t incorrect = 0;
	for (size_t i = 0; i < A.size(); i++) {
		if (!fequal(A[i]*B[i], res[i])) {
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
	return 5.3;
}

double ns_to_do_work(size_t work_count) {
	CLOCK clock;

	auto A = new vector<work_t>(work_count, 0 );
	auto B = new vector<work_t>(work_count, 0 );
	auto res = new vector<work_t>(work_count, 0 );

	randomize_vec(*A);
	randomize_vec(*B);
	randomize_vec(*res);
	//warmup
	for (size_t i = 0; i < work_count; i++) {
		work(i, *A, *B, *res);
	}

	randomize_vec(*A);
	randomize_vec(*B);
	randomize_vec(*res);

	clock.start();
	for (size_t i = 0; i < work_count; i++) {
		work(i, *A, *B, *res);
	}
	clock.stop();
	check_work(*A, *B, *res);

	size_t ns = clock.get_ns_between();

	delete A;
	delete B;
	delete res;

	return ns;
}
double percent_of_theoretical_flops(size_t ns, size_t work_count) {
	return calc_tflops(ns, work_count) / calc_theoretical_tflops() * 100.0;
}
using std::to_string;

template <typename T>
static inline const char *to_cstr(T val) {
	string* s = new string(std::to_string(val));
	return s->c_str();
}
struct Worker {

	size_t N;
	size_t ns;
	size_t ns_outer;
	int col_width = 15;
	vector<vector<string>> results;

	void do_work(size_t N) {
		this->N = N;
		CLOCK outer_clock;
		outer_clock.start();
		ns = ns_to_do_work(N);
		outer_clock.stop();
		ns_outer = outer_clock.get_ns_between();

		size_t flops = calc_flops(ns, N);
		results.push_back({
			comma_separate(N),
			comma_separate(ns / 1'000'000) + "ms",
			comma_separate(ns_outer / 1'000'000) + "ms",
			comma_separate(flops),
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
	for (size_t N = 100; N < 10000'000'000; N *= 2) {
		Worker w;
		w.do_work(N);
		workers.push_back(w);
	}

	Worker w = workers[0];
	printf("%-*.*s ", w.col_width, w.col_width, "N");
	printf("%-*.*s ", w.col_width, w.col_width, "MS CALC");
	printf("%-*.*s ", w.col_width, w.col_width, "MS OVERALL");
	printf("%-*.*s ", w.col_width, w.col_width, "FLOPS");
	printf("%-*.*s ", w.col_width, w.col_width, "% POSSIBLE FLOPS");
	printf("\n");
	for (auto w : workers)
		w.print_results();
	printf("----------\ntook %0.2lfs\n", prog_clock.s_since_start());

}


