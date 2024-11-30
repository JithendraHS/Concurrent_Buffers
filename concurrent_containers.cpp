/*******************************************************************************
 * Copyright (C) 2024 by Jithendra H S
 *
 * Redistribution, modification, or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. Users are
 * permitted to modify this and use it to learn about the field of embedded
 * software. Jithendra H S and the University of Colorado are not liable for
 * any misuse of this material.
 ******************************************************************************/

#include <fstream>
#include <limits>
#include <algorithm>
#include <thread>
#include "command_handling.hpp"
#include "buffer.hpp"


using namespace std;

vector<thread*> threads;

// Main function
int main(int argc, char *argv[]) {
    command_param * ch = new command_param();
    stack stack_buffer;
    queue queue_buffer;
    int success = command_handle(argc, argv, ch);
    if(success == EXIT_FAILURE){
        return 0;
    }
    // Open the input file for reading
    ifstream fptr_src(ch->source_file);
    if (!fptr_src) {
        cout << "Failed to open " << ch->source_file << endl; // Error message if file can't be opened
        return EXIT_FAILURE; // Exit with failure status
    }

    // Open the output file for writing
    ofstream fptr_out(ch->out_file);
    if (!fptr_out) {
        cout << "Failed to open " << ch->out_file << endl; // Error message if file can't be opened
        return EXIT_FAILURE; // Exit with failure status
    }
    string line;
    while (getline(fptr_src, line)) {
        stack_buffer.push(atoi(line.c_str())); // Convert line to integer
        queue_buffer.insert(atoi(line.c_str()));
    }
    while(stack_buffer.top != stack_buffer.bottom){
        fptr_out << stack_buffer.pop() << "\n";
    }
    while(queue_buffer.head){
        cout << queue_buffer.remove() << "\n";
    }
    // Close input and output files
    fptr_src.close(); // Close the input file
    fptr_out.close(); // Close the output file
    cout << "Done!!!" << endl;
    return 0; // Return success
}