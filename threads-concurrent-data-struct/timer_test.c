#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>
#include <unistd.h>

// 用于 x86 架构读取 CPU 周期数 (rdtsc)
static inline uint64_t rdtsc()
{
    unsigned int lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

int main()
{
    struct timeval start, end, current, last;
    long seconds, useconds;
    double duration;
    long min_diff = 1000000; // 初始化为一个很大的数
    int i;
    int iterations = 1000000;

    printf("=== Testing gettimeofday() ===\n");

    // 1. 测量 gettimeofday() 的调用开销
    // 我们运行很多次，取平均值
    gettimeofday(&start, NULL);
    for (i = 0; i < iterations; i++)
    {
        gettimeofday(&current, NULL);
    }
    gettimeofday(&end, NULL);

    seconds = end.tv_sec - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    duration = seconds + useconds / 1000000.0;

    printf("Total time for %d calls: %f seconds\n", iterations, duration);
    printf("Average overhead per call: %f microseconds (us)\n", (duration * 1000000.0) / iterations);

    // 2. 测量最小分辨率 (Smallest Interval)
    // 连续调用，直到时间发生变化，记录变化的最小步长
    gettimeofday(&last, NULL);
    for (i = 0; i < iterations; i++)
    {
        gettimeofday(&current, NULL);

        long diff = (current.tv_sec - last.tv_sec) * 1000000 + (current.tv_usec - last.tv_usec);

        if (diff > 0)
        {
            if (diff < min_diff)
            {
                min_diff = diff;
            }
            last = current; // 更新 last
        }
    }

    printf("Smallest measurable interval (resolution): %ld microseconds\n", min_diff);

    // 3. 对比：使用 CPU 周期计数器 (RDTSC)
    // 这是硬件级别的计数器，通常是纳秒级甚至更高精度
    printf("\n=== Testing RDTSC (CPU Cycles) ===\n");
    uint64_t start_cycles, end_cycles;

    start_cycles = rdtsc();
    // 做一些非常轻微的操作，比如赋值
    int dummy;
    for (int i = 0; i < 1000; i++)
    {
        dummy += 1;
    }
    // 平均耗时 = (end - start) / 1000
    end_cycles = rdtsc();

    printf("Cycle count for a simple operation: %lu cycles\n", end_cycles - start_cycles);

    return 0;
}
