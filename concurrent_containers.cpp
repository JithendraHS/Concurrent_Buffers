/*******************************************************************************
 * Copyright (C) 2024 by Jithendra H S
 *
 * Redistribution, modification, or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. Users are
 * permitted to modify this and use it to learn about the field of embedded
 * software. Jithendra H S and the University of Colorado are not liable for
 * any misuse of this material.
 ******************************************************************************/

#include <limits>
#include <algorithm>
#include <thread>
#include <cstring>
#include "command_handling.hpp"
#include "buffer.hpp"
#include "parallelized_code.hpp"

using namespace std;

// Global vector to store dynamically allocated thread objects
vector<thread*> threads;

// Main function
int main(int argc, char *argv[]) {
    // Create a command_param object to handle command-line arguments
    command_param *ch = new command_param();

    // Parse command-line arguments and populate the command_param object
    int success = command_handle(argc, argv, ch);
    if (success == EXIT_FAILURE) {
        return 0; // Exit if argument handling fails
    }

    // Open the input file for reading
    ifstream fptr_src(ch->source_file);
    if (!fptr_src) {
        // Display an error message if the input file cannot be opened
        cout << "Failed to open " << ch->source_file << endl;
        return EXIT_FAILURE; // Exit with failure status
    }

    // Open the output file for writing
    ofstream fptr_out(ch->out_file);
    if (!fptr_out) {
        // Display an error message if the output file cannot be opened
        cout << "Failed to open " << ch->out_file << endl;
        return EXIT_FAILURE; // Exit with failure status
    }

    // Resize the threads vector to hold NUM_THREADS thread pointers
    threads.resize(NUM_THREADS);

    // Determine the buffer type based on the user-provided argument
    unsigned buffer_type;
    if (ch->stack) {
        buffer_type = STACK; // Use stack if specified
    } else if (ch->queue) {
        buffer_type = QUEUE; // Use queue if specified
    } else {
        buffer_type = STACK; // Default to stack
    }

    // Check if single global lock (sgl) is selected for stack or queue
    if ((ch->stack && strcmp(ch->stack, "sgl") == 0) || 
        (ch->queue && strcmp(ch->queue, "sgl") == 0)) {

        // Create threads for inserting and deleting from the buffer
        for (unsigned i = 0; i < NUM_THREADS; i++) {
            if (!(i % 2)) {
                // Odd threads perform insertion
                threads[i] = new thread(insert_sgl, ref(fptr_src), i, buffer_type);
            } else {
                // Even threads perform deletion
                threads[i] = new thread(delete_sgl, ref(fptr_out), i, buffer_type);
            }
        }
        // Join all threads to ensure the main thread waits for them to complete
    for (unsigned i = 0; i < NUM_THREADS; i++) {
        threads[i]->join();  // Wait for the thread to finish
        delete threads[i];   // Clean up the dynamically allocated thread
    }
    }else if((ch->stack && strcmp(ch->stack, "treiber") == 0)){
         // Create threads for inserting and deleting from the buffer
        for (unsigned i = 0; i < NUM_THREADS; i++) {
            if (!(i % 2)) {
                // Odd threads perform insertion
                threads[i] = new thread(insert_treiber, ref(fptr_src), i, buffer_type);
            } else {
                // Even threads perform deletion
                threads[i] = new thread(delete_treiber, ref(fptr_out), i, buffer_type);
            }
        }
       // Join all threads to ensure the main thread waits for them to complete
    for (unsigned i = 0; i < NUM_THREADS; i++) {
        threads[i]->join();  // Wait for the thread to finish
        delete threads[i];   // Clean up the dynamically allocated thread
    }
    }else if((ch->queue && strcmp(ch->queue, "mns") == 0)){
         // Create threads for inserting and deleting from the buffer
        for (unsigned i = 0; i < NUM_THREADS; i++) {
            if (!(i % 2)) {
                // Odd threads perform insertion
                threads[i] = new thread(insert_mns, ref(fptr_src), i, buffer_type);
            } else {
                // Even threads perform deletion
                threads[i] = new thread(delete_mns, ref(fptr_out), i, buffer_type);
            }
        }
       // Join all threads to ensure the main thread waits for them to complete
    for (unsigned i = 0; i < NUM_THREADS; i++) {
        threads[i]->join();  // Wait for the thread to finish
        delete threads[i];   // Clean up the dynamically allocated thread
    }
    }else if((ch->stack && strcmp(ch->stack, "treiber_elim") == 0)){
         // Create threads for inserting and deleting from the buffer
        for (unsigned i = 0; i < NUM_THREADS; i++) {
            if (!(i % 2)) {
                // Odd threads perform insertion
                threads[i] = new thread(insert_treiber_elim, ref(fptr_src), i, buffer_type);
            } else {
                // Even threads perform deletion
                threads[i] = new thread(delete_treiber_elim, ref(fptr_out), i, buffer_type);
            }
        }
       // Join all threads to ensure the main thread waits for them to complete
       
    for (unsigned i = 0; i < NUM_THREADS; i++) {
        //if (!(i % 2)) {
            threads[i]->join();  // Wait for the thread to finish
            delete threads[i];   // Clean up the dynamically allocated thread
        //}
    }
    }else if ((ch->stack && strcmp(ch->stack, "sgl_elim") == 0)) {
        cout<< "I'm here!!!"<<endl;
        // Create threads for inserting and deleting from the buffer
        for (unsigned i = 0; i < NUM_THREADS; i++) {
            if (!(i % 2)) {
                // Odd threads perform insertion
                threads[i] = new thread(insert_sgl_elim, ref(fptr_src), i, buffer_type);
            } else {
                // Even threads perform deletion
                threads[i] = new thread(delete_sgl_elim, ref(fptr_out), i, buffer_type);
            }
        }
        // Join all threads to ensure the main thread waits for them to complete
    for (unsigned i = 0; i < NUM_THREADS; i++) {
        threads[i]->join();  // Wait for the thread to finish
        delete threads[i];   // Clean up the dynamically allocated thread
    }
    }

    // Close input and output files
    fptr_src.close(); // Close the input file
    fptr_out.close(); // Close the output file

    // Indicate that the program has completed successfully
    cout << "Done!!!" << endl;

    return 0; // Return success
}
