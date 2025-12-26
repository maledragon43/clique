#include "file_management.h"
#include <stdio.h>
#include <chrono>

int main(int argc, char *argv[]) {
    // Start timing
    auto start_time = std::chrono::high_resolution_clock::now();
    
    Graph *graph = read_mtx_file("datasets/frb30-15-1.mtx");
    
    if (graph == NULL) {
        return 1;
    }
    
    // Print graph information
    print_graph_info(graph);
    
    // Print adjacency matrix (limit to 20x20 for readability)
    print_adjacency_matrix(graph, 20);
    
    // Free memory
    free_graph(graph);
    
    // End timing
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    // Print execution time
    printf("\n=== Execution Time ===\n");
    printf("Total execution time: %lu microseconds (%.3f milliseconds, %.6f seconds)\n", 
           duration.count(), 
           duration.count() / 1000.0, 
           duration.count() / 1000000.0);
    
    return 0;
}