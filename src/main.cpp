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
using work_t = double;
constexpr auto epsilon = std::numeric_limits<work_t>::epsilon();
constexpr size_t work_count = 1'000'000;
constexpr work_t rand_min = -1'000'000;
constexpr work_t rand_max = 1'000'000;

// [0][1][2][3][4][5][6][7][8]
//  1  0  0  0  0  0  0  0  0
template<typename T>
string comma_separate(T d) {
	string s = std::to_string((size_t)d);
	string s1 = s;
	for (int i = 1; i < s.size() ; i++) {
		if (i % 3 == 0) {
			s1.insert(i, 1, (char)i);
		}
	}
	return s1;
}

void work(size_t i, vector<work_t> &A, vector<work_t> &B, vector<work_t> &res) {
	res[i] = A[i] * B[i];
}

bool fequal(work_t a, work_t b) {
	return abs(a - b) < epsilon;
}

bool check_work(vector<work_t> &A, vector<work_t> &B, vector<work_t> &res) {
	size_t incorrect = 0;
	for (size_t i = 0; i < work_count; i++) {
		if (!fequal(A[i]*B[i], res[i])) {
			++incorrect;
		}
	}
	printf("%lu/%lu successful\n", work_count - incorrect, work_count);
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

int main() {
	CLOCK clock;

	auto A = new vector<work_t>(work_count, 0 );
	auto B = new vector<work_t>(work_count, 0 );
	auto res = new vector<work_t>(work_count, 0 );

	randomize_vec(*A);
	randomize_vec(*B);
	randomize_vec(*res);
	clock.start();
	for (size_t i = 0; i < work_count; i++) {
		work(i, *A, *B, *res);
	}
	clock.stop();
	check_work(*A, *B, *res);

	double s = clock.get_s_elapsed();
	double ms = clock.get_ms_elapsed();
	double us = clock.get_us_elapsed();
	size_t ns = clock.get_ns_elapsed();
	printf("did %s FP%lu units\n", comma_separate(work_count).c_str(), bitsof(work_t));
	printf("did %lu FP%lu units\n", work_count, bitsof(work_t));
	printf("took   %gus\n", us);
	printf("took   %luns\n", ns);
	printf("took   %gms\n", ms);
	printf("took   %gs\n", s);

	printf("ns per op: %gns/op\n", calc_ns_per_op(ns, work_count));
	printf("FLOPS: %0.2lf\n", calc_flops(ns, work_count));
	printf("GFLOPS: %0.2lf\n", calc_gflops(ns, work_count));
	printf("TFLOPS: %0.2lf\n", calc_tflops(ns, work_count));
}


