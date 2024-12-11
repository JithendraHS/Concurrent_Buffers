#include "parallelized_code.hpp"
#include "buffer.hpp"
#include <mutex>
#include <iostream>


template <typename T>
T fai(atomic<T> &status, T amount, memory_order mem_order) {
    // Atomically add 'amount' to 'status' and return the previous value
    return status.fetch_add(amount, mem_order);
}

// Global stack and queue buffers for storing data
stack stack_buffer;
queue queue_buffer;

treiber_stack trieber_stack_buffer;
mns_queue mns_queue_buffer;

treiber_stack_elim treiber_stack_elim_buffer(8);

stack_elim stack_elim_buffer(8);

stack_flat stack_flat_buffer(8);

atomic<int> input_index = 0;
atomic<int> output_index = 0;

// Atomic flag to indicate whether the file reading is complete
atomic<bool> read_complete = false;

void insert_remove_sgl_stack(vector<int>& input_data, 
                             vector<int>& output_data, 
                             int thread_id, 
                             int buffer_type)  {
    int push_index = 0;
    int pop_index = 0;

    while (true) {
#ifdef DEBUG
        // Debugging/logging: Print the thread ID
        cout << "Thread ID: " << thread_id << endl;
#endif

        // Fetch the next index for insertion and check bounds
        push_index = fai(input_index, 1, ACQ_REL);
        if (push_index < (int)input_data.size()) {
            stack_buffer.push(input_data[push_index]); // Push the element to the stack
        }

        // Try to pop an element from the stack
        int element;
        if (stack_buffer.pop(element)) {
            // Fetch the next index for output and store the popped element
            pop_index = fai(output_index, 1, ACQ_REL);
            output_data[pop_index] = element;
        }

        // Boundary condition: Check if all input is processed and stack is empty
        if (push_index >= (int)input_data.size() && stack_buffer.top == nullptr) {
            break; // Exit the loop
        }
    }
}


void insert_remove_sgl_queue(vector<int>& input_data, 
                             vector<int>& output_data, 
                             int thread_id, 
                             int buffer_type) {
    int push_index = 0;
    int pop_index = 0;

    while (true) {
#ifdef DEBUG
        // Debugging/logging: Print the thread ID
        cout << "Thread ID: " << thread_id << endl;
#endif
        // Fetch the next index for insertion and check bounds
        push_index = fai(input_index, 1, ACQ_REL);
        if (push_index < (int)input_data.size()) {
            queue_buffer.insert(input_data[push_index]);
        }
        // Try to pop an element from the stack
        int element;
        if (queue_buffer.remove(element)) {
            // Fetch the next index for output and store the popped element
            pop_index = fai(output_index, 1, ACQ_REL);
            output_data[pop_index] = element;
        }
        // Check for the boundary condition: file end and queue empty
        if (push_index >= (int)input_data.size() && queue_buffer.head == nullptr) {
            break;  // Exit the loop when file is done and the queue is empty
        }
    }
}

void insert_remove_treiber(vector<int>& input_data, 
                             vector<int>& output_data, 
                             int thread_id, 
                             int buffer_type)  {
    int push_index = 0;
    int pop_index = 0;

    while (true) {
#ifdef DEBUG
        // Debugging/logging: Print the thread ID
        cout << "Thread ID: " << thread_id << endl;
#endif
        // Fetch the next index for insertion and check bounds
        push_index = fai(input_index, 1, ACQ_REL);
        if (push_index < (int)input_data.size()) {
            trieber_stack_buffer.push(input_data[push_index]);
        }
        // Try to pop an element from the stack
        int element;
        if (trieber_stack_buffer.pop(element)) {
            // Fetch the next index for output and store the popped element
            pop_index = fai(output_index, 1, ACQ_REL);
            output_data[pop_index] = element;
        }
        
        // Check for the boundary condition: file end and stack empty
        if (push_index >= (int)input_data.size() && trieber_stack_buffer.top == nullptr) {
            break;  // Exit the loop when file is done and the stack is empty
        }
    }
}

/**
 * Function to insert elements into a lock-free MNS queue in a thread-safe manner.
 * 
 * @param fptr_src - Input file stream containing elements to insert
 * @param thread_id - ID of the thread performing the operation (for debugging/logging)
 * @param buffer_type - Specifies the type of buffer: QUEUE
 */
void insert_remove_mns(vector<int>& input_data, 
                             vector<int>& output_data, 
                             int thread_id, 
                             int buffer_type) {
    int push_index = 0;
    int pop_index = 0;

    while (true) {
#ifdef DEBUG
        // Debugging/logging: Print the thread ID
        cout << "Thread ID: " << thread_id << endl;
#endif   
        // Fetch the next index for insertion and check bounds
        push_index = fai(input_index, 1, ACQ_REL);
        if (push_index < (int)input_data.size()) {
            mns_queue_buffer.insert(input_data[push_index]);
        }
        // Try to pop an element from the stack
        int element;
        if (mns_queue_buffer.remove(element)) {
            // Fetch the next index for output and store the popped element
            pop_index = fai(output_index, 1, ACQ_REL);
            output_data[pop_index] = element;
        }
        // Check for the boundary condition: file end and stack empty
        if (push_index >= (int)input_data.size()){
            if(mns_queue_buffer.head.load(ACQ) != mns_queue_buffer.tail.load(ACQ)) {
                break;
            }   // Exit the loop when file is done and the stack is empty
        }
    }
}

/**
 * Function to insert elements into the Treiber stack (elimination) in a thread-safe manner.
 * 
 * @param fptr_src - Input file stream containing elements to insert
 * @param thread_id - ID of the thread performing the operation (for debugging/logging)
 * @param buffer_type - Specifies the type of buffer: STACK
 */
void insert_remove_treiber_elim(vector<int>& input_data, 
                             vector<int>& output_data, 
                             int thread_id, 
                             int buffer_type) {
    int push_index = 0;
    int pop_index = 0;

    while (true) {
#ifdef DEBUG
        // Debugging/logging: Print the thread ID
        cout << "Thread ID: " << thread_id << endl;
#endif   
        // Fetch the next index for insertion and check bounds
        push_index = fai(input_index, 1, ACQ_REL);
        if (push_index < (int)input_data.size()) {
            treiber_stack_elim_buffer.push(input_data[push_index]);
        }
        // Try to pop an element from the stack
        int element;
        if (treiber_stack_elim_buffer.pop(element)) {
            // Fetch the next index for output and store the popped element
            pop_index = fai(output_index, 1, ACQ_REL);
            output_data[pop_index] = element;
        }  
        // Check for the boundary condition: file end and stack empty
        if (push_index >= (int)input_data.size() && treiber_stack_elim_buffer.top == nullptr) {
            break;  // Exit the loop when file is done and the stack is empty
        }
    }
}

/**
 * Function to insert elements into a stack with elimination in a thread-safe manner.
 * 
 * @param fptr_src - Input file stream containing elements to insert
 * @param thread_id - ID of the thread performing the operation (for debugging/logging)
 * @param buffer_type - Specifies the type of buffer: STACK
 */
void insert_remove_sgl_elim(vector<int>& input_data, 
                             vector<int>& output_data, 
                             int thread_id, 
                             int buffer_type) {
    int push_index = 0;
    int pop_index = 0;

    while (true) {
#ifdef DEBUG
        // Debugging/logging: Print the thread ID
        cout << "Thread ID: " << thread_id << endl;
#endif   
        // Fetch the next index for insertion and check bounds
        push_index = fai(input_index, 1, ACQ_REL);
        if (push_index < (int)input_data.size()) {
            stack_elim_buffer.push(input_data[push_index]);
        }
        // Try to pop an element from the stack
        int element;
        if (stack_elim_buffer.pop(element)) {
            // Fetch the next index for output and store the popped element
            pop_index = fai(output_index, 1, ACQ_REL);
            output_data[pop_index] = element;
        }  
        // Check for the boundary condition: file end and stack empty
        if (push_index >= (int)input_data.size() && stack_elim_buffer.top == nullptr) {
            break;  // Exit the loop when file is done and the stack is empty
        }
    }
}

/**
 * Function to insert elements into a flat stack in a thread-safe manner.
 * 
 * @param fptr_src - Input file stream containing elements to insert
 * @param thread_id - ID of the thread performing the operation (for debugging/logging)
 * @param buffer_type - Specifies the type of buffer: STACK
 */
void insert_remove_stack_flat(vector<int>& input_data, 
                             vector<int>& output_data, 
                             int thread_id, 
                             int buffer_type) {
    int push_index = 0;
    int pop_index = 0;

    while (true) {
#ifdef DEBUG
        // Debugging/logging: Print the thread ID
        cout << "Thread ID: " << thread_id << endl;
#endif   
        // Fetch the next index for insertion and check bounds
        push_index = fai(input_index, 1, ACQ_REL);
        if (push_index < (int)input_data.size()) {
            stack_flat_buffer.push(input_data[push_index]);
        }
        // Try to pop an element from the stack
        int element;
        if (stack_flat_buffer.pop(element)) {
            // Fetch the next index for output and store the popped element
            pop_index = fai(output_index, 1, ACQ_REL);
            output_data[pop_index] = element;
        }  
        // Check for the boundary condition: file end and stack empty
        if (push_index >= (int)input_data.size() && stack_flat_buffer.top == nullptr) {
            break;  // Exit the loop when file is done and the stack is empty
        }
    }
}
