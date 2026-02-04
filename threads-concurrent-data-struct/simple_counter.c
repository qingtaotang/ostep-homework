#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

// --- 简单的并发计数器结构 ---
typedef struct __counter_t {
    int             value;
    pthread_mutex_t lock;
} counter_t;

void init(counter_t *c) {
    c->value = 0;
    pthread_mutex_init(&c->lock, NULL);
}

void increment(counter_t *c) {
    for(int j=0; j<100; j++);
    pthread_mutex_lock(&c->lock);
    c->value++;
    pthread_mutex_unlock(&c->lock);
}

int get(counter_t *c) {
    pthread_mutex_lock(&c->lock);
    int rc = c->value;
    pthread_mutex_unlock(&c->lock);
    return rc;
}

// --- 线程工作相关 ---

// 定义传入线程的参数结构
typedef struct {
    counter_t *c;
    int loop;
} args_t;

void *worker(void *arg) {
    args_t *args = (args_t *) arg;
    for (int i = 0; i < args->loop; i++) {
        increment(args->c);
    }
    return NULL;
}

// --- 计时辅助函数 ---
double get_time() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + (double)t.tv_usec / 1e6;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <threads> <loops>\n", argv[0]);
        exit(1);
    }

    int num_threads = atoi(argv[1]);
    int num_loops   = atoi(argv[2]);
    
    counter_t c;
    init(&c);

    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    args_t args;
    args.c = &c;
    args.loop = num_loops;

    printf("Threads: %d, Loops per thread: %d\n", num_threads, num_loops);
    
    double t1 = get_time();

    // 创建所有线程
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, worker, &args);
    }

    // 等待所有线程结束
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    double t2 = get_time();

    printf("Final value: %d\n", get(&c));
    printf("Time (seconds): %f\n", t2 - t1);

    free(threads);
    return 0;
}
