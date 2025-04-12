#include "command_handling.hpp"  // Include header file for command handling functionality
#include <cstring>  // For string manipulation (e.g., strcmp)
#include <getopt.h>  // For parsing command line options

using namespace std;

// Default number of threads for parallelism
unsigned NUM_THREADS = 4;

// Function to handle command line arguments and populate the command_param structure
int command_handle(int argc, char *argv[], command_param * ch) {
    int opt = 0;  // Variable to hold option character
    ch->source_file = nullptr;  // Initialize source file pointer to nullptr
    ch->out_file = strdup("stack_queue_output.txt");  // Default output file name
    ch->stack = nullptr;  // Initialize stack to nullptr
    ch->queue = nullptr;  // Initialize queue to nullptr
    ch->pop_count = 0;  // Default pop count is set to 0

    // Structure to define long options for command line arguments
    static struct option long_options[] = {
        {"stack", required_argument, 0, 0},  // Stack option, requires an argument
        {"queue", required_argument, 0, 0},  // Queue option, requires an argument
        {"pop", required_argument, 0, 0},    // Pop count option, requires an argument
        {0, 0, 0, 0}  // End of long options
    };
    int option_index = 0;  // Index for long options

    // Parse command line arguments using getopt_long
    while ((opt = getopt_long(argc, argv, "i:o:t:h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 0:  // Handle long options
                cout << "option --> " << long_options[option_index].name << ": ";
                if (strcmp(long_options[option_index].name, "stack") == 0) {
                    cout << optarg << endl;  // Display the value for the stack option
                    ch->stack = optarg;  // Store the stack type
                }
                if (strcmp(long_options[option_index].name, "queue") == 0) {
                    cout << optarg << endl;  // Display the value for the queue option
                    ch->queue = optarg;  // Store the queue type
                }
                if (strcmp(long_options[option_index].name, "pop") == 0) {
                    cout << optarg << endl;  // Display the value for the pop option
                    ch->pop_count = atoi(optarg);  // Convert string to integer and store the pop count
                }
                break;
            case 'i':  // Handle input file option
                cout << "option --> " << static_cast<char>(opt) << ":";
                if (optarg) {
                    cout << optarg;  // Display the input file name
                    ch->source_file = optarg;  // Store the input file name
                }
                cout << endl;
                break;
            case 'o':  // Handle output file option
                cout << "option --> " << static_cast<char>(opt) << ":";
                if (optarg) {
                    cout << optarg;  // Display the output file name
                    ch->out_file = optarg;  // Store the output file name
                }
                cout << endl;
                break;
            case 't':  // Handle number of threads option
                cout << "option --> " << static_cast<char>(opt) << ":";
                if (optarg) {
                    cout << optarg;  // Display the number of threads
                    NUM_THREADS = atoi(optarg);  // Convert string to integer and store the number of threads
                }
                cout << endl;
                break;
            case 'h':  // Display usage information
                cout << "Usage: ./container [-i source.txt] [-o out.txt] [-t NUMTHREADS] [--stack=<sgl,treiber,sgl_elim,treiber_elim,stack_flat>] [--queue=<sgl,mns>]"
                     << endl; // not enough time to implement [--pop=<pop_count>]
                cout << "-i : file containing elements to insert into stack or queue" << endl;
                cout << "-o : file to store remaining elements in stack or queue" << endl;
                cout << "-t : Number of threads for parallelism" << endl;
                cout << "--stack : stack type (e.g., sgl, treiber)" << endl;
                cout << "--queue : queue type (e.g., sgl, m&s)" << endl;
                cout << "--pop : # of elements to pop from the stack or queue" << endl;
                return EXIT_FAILURE;  // Exit the program with failure status
                break;
            default:  // If an unknown option is passed, display usage information
                cout << "Usage: ./container [-i source.txt] [-o out.txt] [-t NUMTHREADS] [--stack=<sgl,treiber,sgl_elim,treiber_elim,stack_flat>] [--queue=<sgl,mns>]"
                     << endl; // not enough time to implement [--pop=<pop_count>]
                return EXIT_FAILURE;  // Exit with failure status
        }
    }

    // Validate that the required parameters are specified
    if (!ch->source_file || (!ch->stack && !ch->queue)) {
        cout << "All parameters not specified correctly, please check and try again!!!" << endl;
        cout << "Usage: ./container [-i source.txt] [-o out.txt] [-t NUMTHREADS] [--stack=<sgl,treiber,sgl_elim,treiber_elim,stack_flat>] [--queue=<sgl,mns>]"
                     << endl; // not enough time to implement [--pop=<pop_count>]
        return EXIT_FAILURE;  // Exit with failure status
    }

    return EXIT_SUCCESS;  // Successfully parsed the command line arguments
}
