#include "command_handling.hpp"
#include <cstring>
#include <getopt.h>

using namespace std;
unsigned NUM_THREADS = 4;

int command_handle(int argc, char *argv[], command_param * ch){
    int opt = 0; // Variable to hold option and a flag for name
    ch->source_file = nullptr; // To store the input file name
    ch->out_file = strdup("stack_queue_output.txt"); // To store the output file name
    ch->stack = nullptr;
    ch->queue = nullptr;
    ch->pop_count = 0;

    // Structure to define long options for command line arguments
    static struct option long_options[] = {
        {"stack", required_argument, 0, 0},
        {"queue", required_argument, 0, 0},
        {"pop", required_argument, 0, 0},
        {0, 0, 0, 0} // End of options
    };
    int option_index = 0; // Index for long options

    // Parse command line arguments
    while ((opt = getopt_long(argc, argv, "i:o:t:h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 0: // Handle long options
                cout << "option --> " << long_options[option_index].name << ": ";
                if (strcmp(long_options[option_index].name, "stack") == 0) {
                    cout << optarg << endl; // Display specified barrier
                    ch->stack = optarg; // Store barrier type
                }
                if (strcmp(long_options[option_index].name, "queue") == 0) {
                    cout << optarg << endl; // Display specified lock
                    ch->queue = optarg; // Store lock type
                }
                if (strcmp(long_options[option_index].name, "pop") == 0) {
                    cout << optarg << endl;
                    ch->pop_count = atoi(optarg); // Store pop count
                }
                break;
            case 'i': // Handle input file option
                cout << "option --> " << static_cast<char>(opt) << ":";
                if (optarg) {
                    cout <<  optarg; // Display input file name
                    ch->source_file = optarg; // Store input file name
                }
                cout << endl;
                break;
            case 'o': // Handle output file option
                cout << "option --> " << static_cast<char>(opt) << ":";
                if (optarg) {
                    cout << optarg; // Display output file name
                    ch->out_file = optarg; // Store output file name
                }
                cout << endl;
                break;
            case 't': // Handle number of threads option
                cout << "option --> " << static_cast<char>(opt) << ":";
                if (optarg) {
                    cout << optarg; // Display number of threads
                    NUM_THREADS = atoi(optarg); // Convert string to integer and store
                }
                cout << endl;
                break;
            case 'h':
                cout << "Usage: " << argv[0]
                     << " container [-i source.txt] [-o out.txt] [-t NUMTHREADS] [--stack=<sgl,treiber>] [--queue=<sgl,m&s>] [--pop=<pop_count>]"
                     << endl;
                cout << "--name : print the developer name" << endl;
                cout << "-i : file contain elements to insert to stack or queue" << endl;
                cout << "-o : file to store remaining elements in stack or queue" << endl;
                cout << "-t : Number of threads for parallelism" << endl;
                cout << "--stack : stack type" << endl;
                cout << "--queue : queue type" << endl;
                cout << "--pop : # of elements to pop from the stack or queue" << endl;
                return EXIT_FAILURE;
                break;
            default:
                // Display usage instructions if arguments are incorrect
                cout << "Usage: " << argv[0]
                     << " container [-i source.txt] [-o out.txt] [-t NUMTHREADS] [--stack=<sgl,treiber>] [--queue=<sgl,m&s>] [--pop=<pop_count>]"
                     << endl;
                return EXIT_FAILURE; // Exit with failure status
        }
    }
    // Check if both input and output files were specified and number of threads is positive
    if (!ch->source_file || (!ch->stack && !ch->queue)) {
        cout << "All parameters not specified correctly, please check and try again!!!" << endl;
        cout << "Usage: " << argv[0]
             << " container [-i source.txt] [-o out.txt] [-t NUMTHREADS] [--stack=<sgl,treiber>] [--queue=<sgl,m&s>] [--pop=<pop_count>]" << endl;
        return EXIT_FAILURE; // Exit with failure status
    }
    return EXIT_SUCCESS;
}