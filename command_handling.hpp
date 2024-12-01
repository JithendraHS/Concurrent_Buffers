#include <iostream> // Standard library for input and output operations

// Structure to hold command-line parameters
struct command_param {
    char* source_file; // Pointer to a string holding the source file name
    char* out_file;    // Pointer to a string holding the output file name
    char* stack;       // Pointer to a string specifying stack operations
    char* queue;       // Pointer to a string specifying queue operations
    unsigned pop_count;// Unsigned integer specifying the number of elements to pop
};
typedef struct command_param command_param; // Typedef for ease of use

// Global variable declaration for the number of threads
extern unsigned NUM_THREADS; // Declared elsewhere; shared across files in the project

// Function prototype for handling command-line arguments
int command_handle(int argc, char *argv[], command_param *ch);
/*
 * Parameters:
 * - `argc`: Number of command-line arguments passed to the program
 * - `argv`: Array of character pointers representing the command-line arguments
 * - `ch`: Pointer to a `command_param` structure to store parsed parameters
 *
 * Return Value:
 * - Returns an integer indicating success or failure (likely `EXIT_SUCCESS` or `EXIT_FAILURE`).
 */
