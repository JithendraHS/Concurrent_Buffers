#include "parallelized_code.hpp"
#include "buffer.hpp"
#include <mutex>
#include <iostream>

// Mutex to enforce mutual exclusion for shared resources
mutex sg_lock;

// Global stack and queue buffers for storing data
stack stack_buffer;
queue queue_buffer;

// Atomic flag to indicate whether the file reading is complete
atomic<bool> read_complete = false;

/**
 * Function to insert elements into a stack or queue in a thread-safe manner.
 * 
 * @param fptr_src - Input file stream containing elements to insert
 * @param thread_id - ID of the thread performing the operation (for debugging/logging)
 * @param buffer_type - Specifies the type of buffer: STACK or QUEUE
 */
void insert_sgl(ifstream &fptr_src, int thread_id, int buffer_type) {
    string line;
    while (true) {
        // Lock the mutex to ensure exclusive access to shared resources
        lock_guard<mutex> lock(sg_lock);
        
        // Debugging/logging: Print the thread ID
        cout << thread_id << endl;

        // Attempt to read a line from the input file
        if (getline(fptr_src, line)) {
            // Insert the element into the specified buffer
            if (buffer_type == STACK) {
                stack_buffer.push(atoi(line.c_str())); // Convert string to integer and push to stack
            } else if (buffer_type == QUEUE) {
                queue_buffer.insert(atoi(line.c_str())); // Convert string to integer and insert to queue
            } else {
                // Handle invalid buffer type
                cout << "Incorrect buffer type" << endl;
                return;
            }
        } else {
            // If the file reading is complete, update the atomic flag and exit the loop
            read_complete.store(true);
            break;
        }
    }
}

/**
 * Function to remove elements from a stack or queue in a thread-safe manner.
 * 
 * @param fptr_out - Output file stream to write removed elements
 * @param thread_id - ID of the thread performing the operation (for debugging/logging)
 * @param buffer_type - Specifies the type of buffer: STACK or QUEUE
 */
void delete_sgl(ofstream &fptr_out, int thread_id, int buffer_type) {
    while (true) {
        // Lock the mutex to ensure exclusive access to shared resources
        lock_guard<mutex> lock(sg_lock);

        // Debugging/logging: Print the thread ID
        cout << thread_id << endl;

        // Handle stack buffer operations
        if (buffer_type == STACK) {
            // Check if the stack is not empty
            if (stack_buffer.top != stack_buffer.bottom) {
                // Pop an element from the stack and write it to the output file
                fptr_out << stack_buffer.pop() << "\n";
            } else {
                // If the stack is empty but reading is not complete, continue waiting
                if (!read_complete.load()) {
                    continue;
                }
                // Exit the loop if reading is complete and the stack is empty
                break;
            }
        }
        // Handle queue buffer operations
        else if (buffer_type == QUEUE) {
            // Check if the queue is not empty
            if (queue_buffer.head) {
                // Remove an element from the queue and write it to the output file
                fptr_out << queue_buffer.remove() << "\n";
            } else {
                // If the queue is empty but reading is not complete, continue waiting
                if (!read_complete.load()) {
                    continue;
                }
                // Exit the loop if reading is complete and the queue is empty
                break;
            }
        }
        // Handle invalid buffer type
        else {
            cout << "Incorrect buffer type" << endl;
            return;
        }
    }
}
