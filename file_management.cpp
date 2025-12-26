#include "file_management.h"
#include <stdio.h>
#include <chrono>

// Function to read MTX file and build graph
// Function to read MTX file and build graph
Graph* read_mtx_file(const char* filename) {

    // Start timing for file reading
    auto read_start = std::chrono::high_resolution_clock::now();
    
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return NULL;
    }

    char line[MAX_LINE_LENGTH];
    uint64_t rows = 0, cols = 0, num_entries = 0;
    int dimensions_found = 0;

    // Step 1: Read header and dimensions to get number of vertices and edges
    while (fgets(line, sizeof(line), file) != NULL) {
        // Skip comment lines (starting with %)
        if (line[0] == '%') {
            continue;
        }
        
        // First non-comment line contains: rows cols num_entries
        if (!dimensions_found && sscanf(line, "%lu %lu %lu", &rows, &cols, &num_entries) == 3) {
            dimensions_found = 1;
            break;
        }
    }

    if (!dimensions_found) {
        fprintf(stderr, "Error: Could not read dimensions from file\n");
        fclose(file);
        return NULL;
    }

    // Step 2: Determine number of vertices (use max of rows and cols)
    uint64_t num_vertices = rows > cols ? rows : cols;
    
    // Step 3: Allocate graph structure
    Graph *graph = (Graph*)malloc(sizeof(Graph));
    graph->num_vertices = num_vertices;
    graph->num_edges = 0;  // Will be counted as we read edges
    
    // Step 4: Calculate number of bitset chunks needed per row
    uint64_t chunks_per_row = (num_vertices + 63) >> 6;
    
    // Step 5: Initialize adjacency matrix using bitset chunks (all ones initially = unconnected)
    graph->adjacency_matrix.resize(num_vertices);
    for (uint64_t i = 0; i < num_vertices; i++) {
        graph->adjacency_matrix[i].resize(chunks_per_row);
        // Initialize all bits to 1 (unconnected by default)
        for (uint64_t chunk = 0; chunk < chunks_per_row; chunk++) {
            graph->adjacency_matrix[i][chunk].set();  // Set all bits to 1
            // For the last chunk, only set bits up to num_vertices
            if (chunk == chunks_per_row - 1) {
                uint64_t last_chunk_start = chunk << 6;
                for (uint64_t bit = 0; bit < 64; bit++) {
                    uint64_t vertex_idx = last_chunk_start + bit;
                    if (vertex_idx >= num_vertices) {
                        graph->adjacency_matrix[i][chunk].reset(bit);  // Reset unused bits to 0
                    }
                }
            }
        }
    }
    
    // Set diagonal elements to 0 (each vertex is connected to itself)
    for (uint64_t i = 0; i < num_vertices; i++) {
        uint64_t chunk = i >> 6;
        uint64_t bit = i & 0b111111;
        graph->adjacency_matrix[i][chunk].reset(bit);  // Set [i][i] = 0 (connected)
    }

    // Step 6: Read edges and build adjacency matrix
    // MTX files are 1-indexed, so we convert to 0-indexed
    uint64_t edges_read = 0;
    uint64_t row, col;
    
    while (fgets(line, sizeof(line), file) != NULL) {
        // Skip empty lines
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0') {
            continue;
        }
        
        // Parse edge: format is "row col" or "row col value" (we ignore value for pattern type)
        if (sscanf(line, "%lu %lu", &row, &col) >= 2) {
            // Convert from 1-indexed to 0-indexed
            row--;
            col--;
            
            // Validate indices
            if (row < num_vertices && col < num_vertices) {
                // Calculate which chunk and bit position for (row, col)
                uint64_t col_chunk = col >> 6;
                uint64_t col_bit = col & 0b111111;
                
                // Check if edge already processed (avoid duplicates)
                // In inverted matrix: 0 = connected, 1 = unconnected
                if (graph->adjacency_matrix[row][col_chunk].test(col_bit)) {
                    // Reset the bit to 0 for edge (row, col) - means connected
                    graph->adjacency_matrix[row][col_chunk].reset(col_bit);
                    graph->num_edges++;
                    
                    // For symmetric/undirected graphs, also reset the reverse edge (col, row)
                    // unless it's a self-loop
                    if (row != col) {
                        uint64_t row_chunk = row >> 6;
                        uint64_t row_bit = row & 0b111111;
                        if (graph->adjacency_matrix[col][row_chunk].test(row_bit)) {
                            graph->adjacency_matrix[col][row_chunk].reset(row_bit);
                        }
                    }
                }
                edges_read++;
            } else {
                fprintf(stderr, "Warning: Edge (%lu, %lu) out of bounds, skipping\n", row+1, col+1);
            }
        }
    }

    // End timing for file reading
    auto read_end = std::chrono::high_resolution_clock::now();
    auto read_duration = std::chrono::duration_cast<std::chrono::microseconds>(read_end - read_start);
    
    fclose(file);

    // Print summary
    printf("File: %s\n", filename);
    printf("Vertices: %lu\n", graph->num_vertices);
    printf("Edges in file: %lu (expected %lu)\n", edges_read, num_entries);
    printf("Unique edges in graph: %lu\n", graph->num_edges);
    printf("Adjacency matrix created: %lux%lu (using %lu bitset chunks per row)\n", 
           num_vertices, num_vertices, chunks_per_row);
    printf("File reading time: %lu microseconds (%.3f milliseconds)\n", 
           read_duration.count(), read_duration.count() / 1000.0);
    
    return graph;
}

// Function to print graph information
void print_graph_info(Graph *graph) {
    if (graph == NULL) {
        printf("Graph is NULL\n");
        return;
    }
    
    printf("\n=== Graph Information ===\n");
    printf("Number of vertices: %lu\n", graph->num_vertices);
    printf("Number of edges: %lu\n", graph->num_edges);
    printf("\n");
}

// Function to print adjacency matrix (for small graphs)
void print_adjacency_matrix(Graph *graph, uint64_t max_size) {
    if (graph == NULL) {
        return;
    }
    
    printf("=== Adjacency Matrix ===\n");
    
    uint64_t print_size = graph->num_vertices;
    if (print_size > max_size) {
        print_size = max_size;
        printf("(Showing first %lux%lu submatrix)\n\n", max_size, max_size);
    }
    
    // Print column indices header
    printf("    ");
    for (uint64_t j = 0; j < print_size; j++) {
        printf("%3lu ", j);
    }
    printf("\n");
    
    // Print matrix
    for (uint64_t i = 0; i < print_size; i++) {
        printf("%3lu ", i);
        for (uint64_t j = 0; j < print_size; j++) {
            uint64_t col_chunk = j >> 6;
            uint64_t col_bit = j & 0b111111;
            // Inverted matrix: 0 = connected, 1 = unconnected
            int value = graph->adjacency_matrix[i][col_chunk].test(col_bit) ? 1 : 0;
            printf("%3d ", value);
        }
        printf("\n");
    }
    printf("\n");
}

// Function to free graph memory
void free_graph(Graph *graph) {
    if (graph == NULL) {
        return;
    }
    
    // Vector destructors will handle cleanup automatically
    free(graph);
}