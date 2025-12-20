#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include <unistd.h>

// 获取高精度时间（纳秒）
static inline long long get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_pages> <num_trials>\n", argv[0]);
        return 1;
    }
    
    int num_pages = atoi(argv[1]);
    int num_trials = atoi(argv[2]);
    
    if (num_pages <= 0 || num_trials <= 0) {
        fprintf(stderr, "Both arguments must be positive integers\n");
        return 1;
    }
    
    // 获取系统页大小（通常为4KB）
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size < 0) {
        perror("sysconf");
        return 1;
    }
    
    printf("Page size: %ld bytes\n", page_size);
    printf("Number of pages: %d\n", num_pages);
    printf("Number of trials: %d\n", num_trials);
    
    // 分配足够大的数组（num_pages页）
    size_t array_size = num_pages * page_size;
    int *array = mmap(NULL, array_size, 
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if (array == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    
    // 确保每页都被实际分配（写一下每页的第一个字）
    int jump = page_size / sizeof(int);
    for (int i = 0; i < num_pages * jump; i += jump) {
        array[i] = 0;
    }
    
    // 预热：先运行几次确保CPU缓存等状态稳定
    for (int warmup = 0; warmup < 3; warmup++) {
        for (int i = 0; i < num_pages * jump; i += jump) {
            array[i] += 1;
        }
    }
    
    // 主测量循环
    long long start_time = get_time_ns();
    
    for (int trial = 0; trial < num_trials; trial++) {
        // 遍历每页的第一个整数
        for (int i = 0; i < num_pages * jump; i += jump) {
            array[i] += 1;
        }
    }
    
    long long end_time = get_time_ns();
    long long total_time = end_time - start_time;
    
    // 计算结果
    long long total_accesses = (long long)num_pages * num_trials;
    double avg_time_per_access = (double)total_time / total_accesses;
    double avg_time_per_page_in_loop = (double)total_time / num_trials / num_pages;
    
    printf("\nResults:\n");
    printf("Total time: %lld ns\n", total_time);
    printf("Total page accesses: %lld\n", total_accesses);
    printf("Average time per page access: %.2f ns\n", avg_time_per_access);
    printf("Average time per page in one loop: %.2f ns\n", avg_time_per_page_in_loop);
    
    // 清理
    munmap(array, array_size);
    return 0;
}
