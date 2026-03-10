#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#define P 10
#define M (1 << P)

typedef struct {
    uint8_t registers[M];
    double alpha_m;
} HyperLogLog;

uint64_t hash_function(const char *str) {
    uint64_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

uint8_t get_rho(uint64_t w, uint8_t max_bits) {
    uint8_t count = 1;
    if (w == 0) return max_bits + 1;
    while (!(w & 1) && count <= max_bits) {
        w >>= 1;
        count++;
    }
    return count;
}

void hll_init(HyperLogLog *hll) {
    memset(hll->registers, 0, M);
    hll->alpha_m = 0.7213 / (1.0 + 1.079 / M);
}

void hll_add(HyperLogLog *hll, const char *item) {
    uint64_t x = hash_function(item);
    uint32_t idx = (uint32_t)(x >> (64 - P));
    uint64_t w = x & (((uint64_t)1 << (64 - P)) - 1);
    uint8_t rho = get_rho(w, 64 - P);
    
    if (rho > hll->registers[idx]) {
        hll->registers[idx] = rho;
    }
}

double hll_count(HyperLogLog *hll) {
    double sum = 0;
    double estimate;
    int i; // Deðiþkeni dýþarýda tanýmladýk
    
    for (i = 0; i < M; i++) {
        sum += pow(2.0, -hll->registers[i]);
    }
    
    estimate = hll->alpha_m * M * M / sum;
    
    if (estimate <= 2.5 * M) {
        int empty_registers = 0;
        for (i = 0; i < M; i++) {
            if (hll->registers[i] == 0) empty_registers++;
        }
        if (empty_registers != 0) {
            estimate = M * log((double)M / empty_registers);
        }
    }
    return estimate;
}

void hll_merge(HyperLogLog *dest, HyperLogLog *src) {
    int i;
    for (i = 0; i < M; i++) {
        if (src->registers[i] > dest->registers[i]) {
            dest->registers[i] = src->registers[i];
        }
    }
}

int main() {
    HyperLogLog my_hll;
    char buffer[32];
    int i;
    double result;

    hll_init(&my_hll);

    printf("--- HyperLogLog Projesi ---\n");
    for (i = 0; i < 10000; i++) {
        sprintf(buffer, "item_%d", i);
        hll_add(&my_hll, buffer);
    }

    result = hll_count(&my_hll);
    printf("Gercek: 10000, Tahmin: %.2f\n", result);

    return 0;
}
