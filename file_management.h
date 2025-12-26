#ifndef FILE_MANAGEMENT_H
#define FILE_MANAGEMENT_H

#include "global_definition.h"

// Function to read MTX file and build graph
Graph* read_mtx_file(const char* filename);

// Function to print graph information
void print_graph_info(Graph *graph);

// Function to print adjacency matrix (for small graphs)
void print_adjacency_matrix(Graph *graph, uint64_t max_size);

// Function to free graph memory
void free_graph(Graph *graph);

#endif // FILE_MANAGEMENT_H