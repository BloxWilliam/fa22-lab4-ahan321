#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "cache.h"
#include "jbod.h"

static cache_entry_t *cache = NULL;
static int cache_size = 0;
static int num_queries = 0;
static int num_hits = 0;

int cache_create(int num_entries) {
    return -1;
}

int cache_destroy(void) {
    return -1;
}

int cache_lookup(int disk_num, int block_num, uint8_t *buf) {
    if (cache == NULL || buf == NULL) {
        return -1;
    }
    num_queries++;
    for (int i = 0; i < cache_size; i++) {
        if (cache[i].valid && cache[i].disk_num == disk_num && cache[i].block_num == block_num) {
            memcpy(buf, cache[i].block, JBOD_BLOCK_SIZE);
            num_hits++;
            cache[i].num_accesses++;
            return 1;
        }
    }
    return 0;
}

void cache_update(int disk_num, int block_num, const uint8_t *buf) {
    if (cache == NULL || buf == NULL) {
        return;
    }
    for (int i = 0; i < cache_size; i++) {
        if (cache[i].valid && cache[i].disk_num == disk_num && cache[i].block_num == block_num) {
            memcpy(cache[i].block, buf, JBOD_BLOCK_SIZE);
            cache[i].num_accesses++;
            return;
        }
    }
}

int cache_insert(int disk_num, int block_num, const uint8_t *buf) {
    if (cache == NULL || buf == NULL) {
        return -1;
    }
    int lfu_index = -1;
    int min_accesses = INT_MAX;
    for (int i = 0; i < cache_size; i++) {
        if (!cache[i].valid) {
            lfu_index = i;
            break;
        } else if (cache[i].num_accesses < min_accesses) {
            min_accesses = cache[i].num_accesses;
            lfu_index = i;
        }
    }
    if (lfu_index == -1) {
        return -1;
    }
    cache[lfu_index].valid = true;
    cache[lfu_index].disk_num = disk_num;
    cache[lfu_index].block_num = block_num;
    memcpy(cache[lfu_index].block, buf, JBOD_BLOCK_SIZE);
    cache[lfu_index].num_accesses = 1;
    return 0;
}


bool cache_enabled(void) {
	return cache != NULL && cache_size > 0;
}

void cache_print_hit_rate(void) {
	fprintf(stderr, "num_hits: %d, num_queries: %d\n", num_hits, num_queries);
	fprintf(stderr, "Hit rate: %5.1f%%\n", 100 * (float) num_hits / num_queries);
}
