#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdalign.h> // 用于 alignas

#define NUMCPUS 12
#define CACHE_LINE_SIZE 64 // 现代 CPU 常见的缓存行大小

// 定义分片结构，确保每个分片独占一个缓存行
typedef struct {
    alignas(CACHE_LINE_SIZE) pthread_mutex_t llock;
    alignas(CACHE_LINE_SIZE) int local;
} shard_t;

typedef struct __counter_t {
    // 全局部分也进行对齐，防止与局部变量产生干扰
    alignas(CACHE_LINE_SIZE) pthread_mutex_t glock;
    alignas(CACHE_LINE_SIZE) int global;
    
    int threshold;
    
    // 局部计数器数组：每个元素现在都保证在不同的缓存行上
    shard_t shards[NUMCPUS];
} counter_t;

// 初始化：为每个分片分配资源
void init(counter_t *c, int threshold) {
    c->threshold = threshold;
    c->global = 0;
    pthread_mutex_init(&c->glock, NULL);
    for (int i = 0; i < NUMCPUS; i++) {
        c->shards[i].local = 0;
        pthread_mutex_init(&c->shards[i].llock, NULL);
    }
}

// 核心更新函数
void update(counter_t *c, int threadID, int amt) {
    int cpu = threadID % NUMCPUS;
    
    // 锁定局部锁，此时不会因为伪共享干扰其他 CPU
    pthread_mutex_lock(&c->shards[cpu].llock);
    c->shards[cpu].local += amt;
    
    if (c->shards[cpu].local >= c->threshold) {
        // 只有超过阈值时才去竞争昂贵的全局锁
        pthread_mutex_lock(&c->glock);
        c->global += c->shards[cpu].local;
        pthread_mutex_unlock(&c->glock);
        c->shards[cpu].local = 0;
    }
    pthread_mutex_unlock(&c->shards[cpu].llock);
}

// 获取当前的近似总数
int get(counter_t *c) {
    pthread_mutex_lock(&c->glock);
    int val = c->global;
    pthread_mutex_unlock(&c->glock);
    return val;
}

// --- 以下为压测逻辑，保持不变 ---

typedef struct {
    counter_t *c;
    int threadID;
    int loop;
} args_t;

void *worker(void *arg) {
    args_t *args = (args_t *) arg;
    for (int i = 0; i < args->loop; i++) {
        update(args->c, args->threadID, 1);
    }
    return NULL;
}

double get_time() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + (double)t.tv_usec / 1e6;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "usage: %s <threads> <loops> <threshold>\n", argv[0]);
        exit(1);
    }

    int num_threads = atoi(argv[1]);
    int num_loops = atoi(argv[2]);
    int threshold = atoi(argv[3]);

    counter_t c;
    init(&c, threshold);

    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    args_t *args = malloc(num_threads * sizeof(args_t));

    double t1 = get_time();
    for (int i = 0; i < num_threads; i++) {
        args[i].c = &c;
        args[i].threadID = i;
        args[i].loop = num_loops;
        pthread_create(&threads[i], NULL, worker, &args[i]);
    }
    for (int i = 0; i < num_threads; i++) pthread_join(threads[i], NULL);
    double t2 = get_time();

    printf("Final value: %d\n", get(&c));
    printf("Time (seconds): %f\n", t2 - t1);

    free(threads);
    free(args);
    return 0;
}