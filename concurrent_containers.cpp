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

vector<thread*> threads;
struct timespec startTime, endTime;

int main(int argc, char *argv[]) {
    command_param *ch = new command_param();
    int success = command_handle(argc, argv, ch);
    if (success == EXIT_FAILURE) {
        return 0;
    }

    ifstream fptr_src(ch->source_file);
    if (!fptr_src) {
        cout << "Failed to open " << ch->source_file << endl;
        return EXIT_FAILURE;
    }

    ofstream fptr_out(ch->out_file);
    if (!fptr_out) {
        cout << "Failed to open " << ch->out_file << endl;
        return EXIT_FAILURE;
    }

    threads.resize(NUM_THREADS);
    // Read data from the input file into a vector
    vector<int> input_data;
    vector<int> output_data;
    string line;
    while (getline(fptr_src, line)) {
        input_data.push_back(atoi(line.c_str())); // Convert line to integer and add to the vector
    }
    output_data.resize(input_data.size() + 10); // adding extra size of 10 to see the abnormalities of stack
    fill(output_data.begin(), output_data.end(), 0); // Fill all elements with 0

    unsigned buffer_type;
    // Set buffer type based on command-line arguments (stack or queue)
    if (ch->stack) {
        buffer_type = STACK;
    } else if (ch->queue) {
        buffer_type = QUEUE;
    } else {
        buffer_type = STACK;
    }

    clock_gettime(CLOCK_MONOTONIC, &startTime);
    for (unsigned i = 0; i < NUM_THREADS; i++) {
    // Handle different buffer configurations based on the command-line input
    if ((ch->stack && strcmp(ch->stack, "sgl") == 0)) {
          threads[i] = new thread(insert_remove_sgl_stack, ref(input_data), ref(output_data), i, buffer_type);
    }else if ((ch->queue && strcmp(ch->queue, "sgl") == 0)) {
          threads[i] = new thread(insert_remove_sgl_queue, ref(input_data), ref(output_data), i, buffer_type);
    } else if((ch->stack && strcmp(ch->stack, "treiber") == 0)){
          threads[i] = new thread(insert_remove_treiber, ref(input_data), ref(output_data), i, buffer_type);
    } else if((ch->queue && strcmp(ch->queue, "mns") == 0)){
          threads[i] = new thread(insert_remove_mns, ref(input_data), ref(output_data), i, buffer_type);
    } else if((ch->stack && strcmp(ch->stack, "treiber_elim") == 0)){
          //cout << "I am here"<< endl;
          threads[i] = new thread(insert_remove_treiber_elim, ref(input_data), ref(output_data), i, buffer_type);
    } else if ((ch->stack && strcmp(ch->stack, "sgl_elim") == 0)) {
         threads[i] = new thread(insert_remove_sgl_elim, ref(input_data), ref(output_data), i, buffer_type);
    } else if ((ch->stack && strcmp(ch->stack, "stack_flat") == 0)) {
        threads[i] = new thread(insert_remove_stack_flat, ref(input_data), ref(output_data), i, buffer_type);
            
    }
    }

    // Wait for all threads to finish
    for (unsigned i = 0; i < NUM_THREADS; i++) {
        threads[i]->join();
        delete threads[i];
    }
    clock_gettime(CLOCK_MONOTONIC, &endTime);
    unsigned long long elapsed_ns;
    elapsed_ns = (endTime.tv_sec - startTime.tv_sec) * 1000000000 +      (endTime.tv_nsec - startTime.tv_nsec);
    printf("Elapsed (ns): %llu\n", elapsed_ns);
    double elapsed_s = ((double)elapsed_ns) / 1000000000.0;
    printf("Elapsed (s): %lf\n", elapsed_s);
    // Write the sorted data to the output file
    for (auto i = output_data.begin(); i < output_data.end(); ++i) {
        fptr_out << *i << "\n";
    }
    fptr_src.close();
    fptr_out.close();

    cout << "Done!!!" << endl;

    return 0;
}

