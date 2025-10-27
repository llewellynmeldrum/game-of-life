#include <cstdio>
#include <chrono>
#include <iomanip>
#include <random>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <cmath>

#include "log.h"
#include "SDMTL.hpp"
#include "CHRONO_WRAPPER.hpp"
#include <thread>
#include <pthread.h>
#include "strmanip.hpp"
#include <pthread/qos.h>
#include <typeindex>
#include <any>
#define bitsof(T)	(sizeof(T)*8)
constexpr size_t MIN_N = 1'000;
constexpr size_t MAX_N = 100'000'000;
const size_t MIN_THREADS = 1;
const size_t MAX_THREADS = 256;
struct Settings {

	size_t thread_count;
	Settings(): thread_count(MIN_THREADS) {};

	Settings(size_t tc) : thread_count(tc) {
	};
	template<typename T>
	void print_as(string name, T val) {
		printf("%-*.*s:", Settings::col_w, Settings::col_w, name.c_str());
		cout << val << endl;
	}
	void print() {
//		printf("SETTINGS:\n");
//		print_as("WORKER THREAD COUNT:", thread_count);
		cout << thread_count << endl;
	}

	static const int col_w = 20;
};
Settings settings;

constexpr auto EPSILON = std::numeric_limits<work_t>::epsilon();

constexpr size_t MAX_STACK_ALLOWANCE = 2'000'000; /* unused */
constexpr work_t WORK_RAND_MIN = -1'000'000;
constexpr work_t WORK_RAND_MAX = 1'000'000;
constexpr bool CHECK_WORK = true;

struct thread_cfg {
	pthread_t thread;
	size_t start;
	size_t end;
	vector<work_t> *A;
	vector<work_t> *B;
	vector<work_t> *res;
};




class MCPU_INFO {
// TODO:
// - before anything, reorganize this file, it is a mess
// - create methods to fill fields like cpu freq, cores, etc
// - use systemctl
  public:
	void query_sysctl();
};


bool float_equals(work_t a, work_t b) {
	return abs(a - b) < EPSILON;
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

void randomize_vec(vector<work_t> *v) {
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);

	// 2. Define a distribution for floating-point numbers
	// This creates a uniform distribution between 0.0 and 1.0 (inclusive).
	std::uniform_real_distribution<work_t> distribution(WORK_RAND_MIN, WORK_RAND_MAX);

	// 3. Create a vector to store the random floats
	int vector_size = 10;
	std::vector<work_t> random_floats(vector_size);

	// 4. Fill the vector with random floats
	for (int i = 0; i < vector_size; ++i) {
		v->at(i) = distribution(generator);
	}
}

static inline double calc_ns_per_op(size_t ns_elapsed, size_t ops_completed) {
	return ns_elapsed / (work_t)ops_completed;
}
static inline double calc_flops(size_t ns_elapsed, size_t ops_completed) {
	return (work_t)ops_completed / (ns_elapsed / 1'000'000'000.0);
}
static inline double calc_mflops(size_t ns_elapsed, size_t ops_completed) {
	return ((work_t)ops_completed / (ns_elapsed / 1'000'000'000.0)) / 1'000'000;
}
static inline double calc_gflops(size_t ns_elapsed, size_t ops_completed) {
	return ((work_t)ops_completed / (ns_elapsed / 1'000'000'000.0)) / 1'000'000'000;
}
static inline double calc_tflops(size_t ns_elapsed, size_t ops_completed) {
	return calc_gflops(ns_elapsed, ops_completed) / 1'000;
}

static inline double calc_theoretical_tflops() {
	// lookup table based on device id or something is the only other way i can think to do this
	// i use an m1 pro so this is a starting point
	// also this is FP32
	return 5.3;
}

static inline work_t work(work_t a, work_t b) {
	return a * b;
}

void *work_n_times(void* v_args) {
	pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
	thread_cfg* args = (thread_cfg*)v_args;
	for (int i = args->start; i < args->end; i++) {
		args->res->at(i) = work(args->A->at(i), args->B->at(i));
	}
	return NULL;
}


size_t ns_to_do_work(size_t work_count) {
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	CLOCK clock;

	// threads should be setup to handle an equal split of the work
	// with N work_count, each thread should do N/work_count work
	size_t thread_count = settings.thread_count;
	size_t per_thread_work_count = work_count / thread_count;

	// all of the threads share a few heap allocated objects,
	// the only thing they dont share are the start and end pointers
	thread_cfg base_cfg = thread_cfg();
	base_cfg.A = new vector<work_t>(work_count, 0 );
	base_cfg.B = new vector<work_t>(work_count, 0 );
	base_cfg.res = new vector<work_t>(work_count, 0 );
	randomize_vec(base_cfg.A);
	randomize_vec(base_cfg.B);
	vector<thread_cfg *> cfg_list = vector<thread_cfg *>(thread_count);


	// warmup
	clock.start();
	for (size_t t = 0; t < thread_count; t++) {
		// copy assign with base config (contains the heap allocated pointers threads share)
		cfg_list[t] = new thread_cfg(base_cfg);
		cfg_list[t]->start = t * (per_thread_work_count);
		cfg_list[t]->end = (t + 1) * (per_thread_work_count);
		if (t + 1 == thread_count) {
			cfg_list[t]->end = work_count - 1;
		}

		pthread_create(&cfg_list[t]->thread, &attr, work_n_times, (void*)cfg_list[t]);
	}

	for (size_t i = 0; i < thread_count; i++) {
		int err = pthread_join(cfg_list[i]->thread, NULL);
		if(err) {
			string s = "";
			switch (err) {
			case EINVAL:
				s = " The implementation has detected that the value specified by thread does not refer to a joinable thread.";
				_logfatal("%s\n", s.c_str());
				break;
			case ESRCH:
				s = "No thread could be found corresponding to that specified case by the given thread ID, thread.";
				_logfatal("%s\n", s.c_str());
				break;
			case EDEADLK:
				s = "A deadlock was detected or the value of thread specifies the calling thread.";
				_logfatal("%s\n", s.c_str());
				_logfatal("main thread_id: %p\n", pthread_self());
				break;
			}
			_logfatalerrno_exit("Failed to join t%lu, err=%d\n", i, err);
		}
	}
	clock.stop();
	if (CHECK_WORK)
		check_work(*base_cfg.A, *base_cfg.B, *base_cfg.res);

	for (size_t t = 0; t < thread_count; t++) {
		delete cfg_list[t];
	}




// do actual work

	delete base_cfg.A;
	delete base_cfg.B;
	delete base_cfg.res;

	return clock.get_ns_between();
}

double percent_of_theoretical_flops(size_t ns, size_t work_count) {
	return calc_tflops(ns, work_count) / calc_theoretical_tflops() * 100.0;
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

	void calc_results() {
		size_t flops = calc_flops(ns, N);
		results.push_back({
			minify_number(N, 5),
			minify_unit(N*sizeof(work_t), 1, 2) + "B",
			comma_separate(ns / 1'000'000) + "ms",
			comma_separate(ns_outer / 1'000'000) + "ms",
			minify_unit(flops, 3, 2),
			float_to_str(percent_of_theoretical_flops(ns, N), 2) + "%"
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


CLOCK prog_clock;

void test_n_threads(size_t N) {
	assert(N > 1 || "Must have at least 1 thread");
	settings = {N};
	settings.print();
	double total_ns_elapsed = 0;
	size_t total_work_completed = 0;
	double max_gflops = 0;
	vector<Worker> workers;
	for (size_t N = MIN_N; N < MAX_N; N = N * 1.3) {
		Worker w;
		w.do_work(N);
		workers.push_back(w);
		max_gflops = fmax(max_gflops, calc_gflops(w.ns, w.N));
	}

	Worker w = workers[0];

	//printf("%-*.*s ", w.col_width, w.col_width, "N");
	//printf("%-*.*s ", w.col_width, w.col_width, "INPUT SZ");
	//printf("%-*.*s ", w.col_width, w.col_width, "MS CALC");
	//printf("%-*.*s ", w.col_width, w.col_width, "MS OVERALL");
	//printf("%-*.*s ", w.col_width, w.col_width, "FLOPS");
	//printf("%-*.*s ", w.col_width, w.col_width, "%FLOPS");

	//printf("\n");
	for (auto w : workers) {
		total_ns_elapsed += w.ns;
		total_work_completed += w.N;
		//	w.calc_results();
		//	w.print_results();
	}
	//printf("----------\ntook %0.2lfs so far, this run:\n", prog_clock.s_since_start());
	printf("%0.3lf\t%0.3lf\t", calc_gflops(total_ns_elapsed, total_work_completed), max_gflops);
}

int main() {
	prog_clock.start();

	printf("avg flops\tmax flops\ttcount\n");
	for (size_t n = MIN_THREADS; n <= MAX_THREADS; n++)
		test_n_threads(n);
	printf("----------\ntook %0.2lfs so far, this run:\n", prog_clock.s_since_start());

}


