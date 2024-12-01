#include <fstream> // For input and output file stream operations
using namespace std;

// Function declarations for inserting and deleting elements from the stack or queue
// These functions will be defined elsewhere, but the declarations tell the compiler about their signatures.

void insert_sgl(ifstream &fptr_src, int thread_id, int buffer_type); // Insert elements from the input file into the buffer (stack or queue)
void delete_sgl(ofstream &fptr_out, int thread_id, int buffer_type); // Delete elements from the buffer (stack or queue) and write to the output file
