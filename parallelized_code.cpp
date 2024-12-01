#include "parallelized_code.hpp"
#include "buffer.hpp"
#include <mutex>
#include <iostream>

// Mutex to enforce mutual exclusion for shared resources
mutex sg_lock;

// Global stack and queue buffers for storing data
stack stack_buffer;
queue queue_buffer;

treiber_stack trieber_stack_buffer;
mns_queue mns_queue_buffer;

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
            if (stack_buffer.top) {
                int element;
                bool state = stack_buffer.pop(element);
                // Pop an element from the stack and write it to the output file
                if (state) fptr_out << element << "\n";
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
                int element;
                bool state = queue_buffer.remove(element);
                // Remove an element from the queue and write it to the output file
                if(state) fptr_out << element << "\n";
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


void insert_treiber(ifstream &fptr_src, int thread_id, int buffer_type) {
    string line;
    while (true) {        
        // Debugging/logging: Print the thread ID
        cout << thread_id << endl;
        // Attempt to read a line from the input file
        if (getline(fptr_src, line)) {
            trieber_stack_buffer.push(atoi(line.c_str())); // Convert string to integer and push to stack
        } else {
            // If the file reading is complete, update the atomic flag and exit the loop
            read_complete.store(true, REL);
            break;
        }
    }
}


void delete_treiber(ofstream &fptr_out, int thread_id, int buffer_type) {
    while (true) {
        // Debugging/logging: Print the thread ID
        cout << thread_id << endl;
        // Check if the stack is not empty
        if (trieber_stack_buffer.top) {
            int element;
            bool state = trieber_stack_buffer.pop(element);
            // Pop an element from the stack and write it to the output file
            if(state) fptr_out << element << "\n";
        } else {
          // If the stack is empty but reading is not complete, continue waiting
            if (!read_complete.load(ACQ)) {
                continue;
            }
                // Exit the loop if reading is complete and the stack is empty
            break;
        }
    }
}

void insert_mns(ifstream &fptr_src, int thread_id, int buffer_type) {
    string line;
    while (true) {        
        // Attempt to read a line from the input file
        if (getline(fptr_src, line)) {
            mns_queue_buffer.insert(atoi(line.c_str())); // Convert string to integer and push to stack
            // Debugging/logging: Print the thread ID
            cout << thread_id << ">" << line.c_str() << endl;
        } else {
            // If the file reading is complete, update the atomic flag and exit the loop
            read_complete.store(true, REL);
            break;
        }
    }
}

void delete_mns(ofstream &fptr_out, int thread_id, int buffer_type) {
    while (true) {
        // Check if the stack is not empty
        if (mns_queue_buffer.head.load(ACQ) != mns_queue_buffer.tail.load(ACQ)) {
            int element;
            bool state = mns_queue_buffer.remove(element);
            // Pop an element from the stack and write it to the output file 
            cout << thread_id << "<" << element << endl;
            if(state) fptr_out << element << "\n";
        } else {
          // If the stack is empty but reading is not complete, continue waiting
            if (!read_complete.load(ACQ)) {
                continue;
            }
                // Exit the loop if reading is complete and the stack is empty
            break;
        }
    }
}