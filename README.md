Got sidetracked looking into benchmarking FP32 on the cpu, and then the gpu to learn about cache and whatnot
- Read somewhere that the M1 is optimized for FP16 and FP32, so i'll be using those in the future.
> [!NOTE]
> All benchmarks are performed on my own 2021, 14in M1 Pro, (16GB ram, 16 core GPU)

**Problem: you have 3 contiguous arrays, A, B and res. Fill res with the product of A and B at every index.**

Problem is obviously O(N), and I dont see anything changing that. What I would like to do is see how far I can optimize this problem from a baseline.

This is my baseline:

```c++
void work(size_t index, vector<work_t> &A, vector<work_t> &B, vector<work_t> &res) {
	res[index] = A[index] * B[index];
}
```
```
// compiled with -O0. Zero vectorization, 1 thread, all on cpu.
N               MS CALC         MS OVERALL      FLOPS           % POSSIBLE FLOP
100             0ms             0ms             184,842,883     0.003488%
...
204K            1ms             10ms            196,576,421     0.003709%
409K            2ms             24ms            175,505,198     0.003311%
819K            4ms             43ms            195,668,820     0.003692%
1.6M            7ms             80ms            212,137,546     0.004003%
3.3M            15ms            157ms           217,587,812     0.004105%
...
210M            967ms           10,218ms        216,734,610     0.004089%
419M            2,095ms         20,865ms        200,118,842     0.003776%
839M            5,943ms         46,326ms        141,145,144     0.002663%
1.7B            11,984ms        93,479ms        139,989,123     0.002641%
3.4B            24,089ms        188,668ms       139,290,570     0.002628%
6.7B            46,373ms        368,849ms       144,713,973     0.002730%
```
As we can see, the FLOPS we are actually performing are far lower than they could theoretically go (5.6TFLOPS on my machine).
The thing limiting us right now is data fetch and store wait times.
To lower this, we can leverage a few things:
- concurrency/threading
- simd
- a compute shader, basically does both but requires more overhead and syncing
CPU will probably beat gpu at low enough N.

**Results from before in a graph:**

<img width="600" alt="Screenshot 2025-10-26 at 8 21 22 PM" src="https://github.com/user-attachments/assets/542e8cd7-bc36-4e1c-9039-2a2492c57469" />

Some interesting takeaways here:
- relatively stable until around 400KB of memory is used
- severe dropoff at 2MB, rises and stablizes after
- slight dropoff at 2GB
- Extreme dropoff after 2GB

**To better understand these, quick overview of the memory/cache layout of the M1:**
```
M1 (pro) memory stats:
Cache line size:       128B
Page size:             16KB
SLC (L3):              24MB
Efficiency cores:
  L1 I/D (per core):   128KB/64KB
  L2 (shared):         4MB
  
Performance cores:
  L1 I/D (per core):   192KB/128KB
  L2 (shared):         24MB
```
- Sadly, theres no super observable cache boundaries in our graph.
- Im not sure exactly why this would occur, but my best guess is that context switches, thread migration, and other things I failed to control for contributed to this noise.
- Ill have to do more runs, especially on the lower end, in order to get some averages which I can have a look at

# 1st optimisation: multithreading

<img width="700" alt="image" src="https://github.com/user-attachments/assets/b80e9d35-06aa-48d9-999b-8442b334349f" />

# References:

[https://metaltutorial.com/Setup/](https://metaltutorial.com/Setup/)
[https://www.cutting.io/posts/game-of-life-on-metal/](https://www.cutting.io/posts/game-of-life-on-metal/)

