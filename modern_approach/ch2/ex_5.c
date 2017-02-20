#include <time.h>
#include <stdio.h>
#include <stdlib.h>

static double gettime() {
    struct timespec tv = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &tv);
    return (double) (tv.tv_sec) + 0.000000001 * tv.tv_nsec;
}

static int calc1(int x) {
    return 3 * x * x * x * x * x + 2 * x * x * x * x - 5 * x * x * x - x * x + 7 * x - 6;
}

static int calc2(int x) {
    return ((((3 * x + 2) * x - 5) * x - 1) * x + 7) * x - 6;
}

static void benchmark(char *name, int x, int (*fun)(int)) {
    double start, cost;
    int i, n = 10000000;
    int r;

    start = gettime();
    for (i = 0; i < n; i++) {
        r = fun(x);
    }
    cost = gettime() - start;
    printf("%s: r=%d, cost=%.4f secs\n", name, r, cost);
}

int main() {
    int x;

    printf("Input x: ");
    if (scanf("%d", &x)) {
        benchmark("calc1", x, calc1);
        benchmark("calc2", x, calc2);
    } else {
        printf("Invalid x, it must an integer.\n");
        exit(1);
    }

    return 0;
}

