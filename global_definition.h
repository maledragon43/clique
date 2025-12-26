#ifndef GLOBAL_DEFINITION_H
#define GLOBAL_DEFINITION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <bitset>
#include <vector>

#define MAX_LINE_LENGTH 64
// #define MAX_LINE_LENGTH 1048576

// Structure to hold graph information
typedef struct {
    uint64_t num_vertices;
    uint64_t num_edges;
    std::vector<std::vector<std::bitset<64>>> adjacency_matrix;
} Graph;

#endif // GLOBAL_DEFINITION_H