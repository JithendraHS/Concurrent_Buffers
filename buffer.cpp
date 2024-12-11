#include "buffer.hpp"
#include <iostream>
#include <random>
#include <chrono>
#include <thread>

// Compare and Swap (CAS) helper function for atomic operations
template <typename T>
bool cas(atomic<T> &status, T expected, T desired, memory_order mem_order) {
    T expected_ref = expected;
    return status.compare_exchange_strong(expected_ref, desired, mem_order);
}



// Constructor for the stack
stack::stack() {
    top = nullptr; // Initialize the stack to be empty
    lock.store(false);
}

// Push an element onto the stack
void stack::push(int element) {
    stack_node *temp = new stack_node(element, nullptr);
    while(!cas(lock, false, true, ACQ_REL));
    temp->next = top;  // Link the new node to the current top
    top = temp;        // Update the top pointer to the new node
    lock.store(false, REL);
}

// Pop an element from the stack and return its value
bool stack::pop(int &element) {
    while(!cas(lock, false, true, ACQ_REL));
    stack_node *temp = top;
    if(!temp) {
        lock.store(false, REL);
        return false;  // Return false if the stack is empty
    }
    element = temp->element;  // Get the value from the top node
    top = top->next;  // Update the top pointer to the next node
    delete temp;      // Free the memory of the popped node
    lock.store(false, REL);
    return true;
}

// Constructor for the queue
queue::queue() {
    head = nullptr;
    tail = nullptr;
    lock.store(false);
}

// Insert an element at the end of the queue
void queue::insert(int element) {
    queue_node *temp = new queue_node(element, nullptr);
    while(!cas(lock, false, true, ACQ_REL));
    if (!head) {
        head = tail = temp;  // If the queue is empty, the new node becomes the head and tail
    } else {
        tail->next = temp;  // Add the new node to the end of the queue
        tail = temp;        // Update the tail pointer to the new node
    }
    lock.store(false, REL);
}

// Remove an element from the front of the queue and return its value
bool queue::remove(int &element) {
    while(!cas(lock, false, true, ACQ_REL));
    if (!head) {  // If the queue is empty
        tail = nullptr;  // Reset the tail pointer
        lock.store(false, REL);
        return false;    // Return false
    }
    queue_node *temp = head;
    element = temp->element;  // Get the value from the head node
    head = head->next;        // Update the head pointer to the next node
    delete temp;              // Free the memory of the removed node
    lock.store(false, REL);
    return true;
}

// Constructor for Treiber's stack (non-blocking stack)
treiber_stack::treiber_stack() {
    top.store(nullptr, RELAXED);  // Initialize the top pointer atomically to null
}

// Push an element onto the Treiber stack (non-blocking)
void treiber_stack::push(int element) {
    stack_node *temp = new stack_node(element, nullptr);
    temp->next = top.load(ACQ);  // Set the next pointer to the current top
    while (!cas(top, temp->next, temp, ACQ_REL)) {  // Attempt to update the top pointer atomically
        temp->next = top.load(ACQ);  // Reload the top if CAS fails
    }
}

// Pop an element from the Treiber stack (non-blocking)
bool treiber_stack::pop(int &element) {
    stack_node *temp = top.load(ACQ);  // Load the current top node atomically
    /*while(true){
        if(!temp){
            return false;
        }
        if(cas(top, temp, temp->next, ACQ_REL)){
            break;
        }
         temp = top.load(ACQ);
    }*/
    if(!temp){
        return false;
    }
    while (!cas(top, temp, temp->next, ACQ_REL)) {  // Attempt to pop the top node atomically
        temp = top.load(ACQ);  // Reload the top if CAS fails
        if(!temp){
            return false;
        }
    }
    element = temp->element;  // Get the value of the popped node
    delete temp;  // Free the memory of the popped node
    return true;
}

// Constructor for M&S queue (multi-threaded, lock-free queue)
mns_queue::mns_queue() {
    head = new queue_node(0, nullptr);  // Create a dummy head node
    tail.store(head, RELAXED);  // Initialize the tail pointer atomically to the dummy node
}

// Insert an element at the end of the M&S queue
void mns_queue::insert(int element) {
    queue_node *temp = new queue_node(element, nullptr);  // Create a new node
    queue_node *last = tail.load(ACQ);  // Load the current tail node atomically
    last->next = temp;  // Link the new node to the current tail
    while (!cas(tail, last, temp, ACQ_REL)) {  // Attempt to update the tail pointer atomically
        last = tail.load(ACQ);  // Reload the tail if CAS fails
        last->next = temp;  // Re-link the new node to the current tail
    }
}

// Remove an element from the front of the M&S queue
bool mns_queue::remove(int &element) {
    queue_node *temp = head.load(ACQ);  // Load the current head node atomically
    if (!temp || !temp->next) {  // If the queue is empty
        return false;
    }
    while (!cas(head, temp, temp->next, ACQ_REL)) {  // Attempt to update the head pointer atomically
        temp = head.load(ACQ);  // Reload the head if CAS fails
        if (!temp || !temp->next) {  // If the queue is empty
            return false;
        }
    }
    element = temp->next->element;  // Get the value from the removed node
    delete temp;  // Free the memory of the removed node
    return true;
}

// Push an element onto the Treiber stack with elimination (elimination-based stack)
void treiber_stack_elim::push(int element) {
    stack_node *temp = new stack_node(element, nullptr);
    temp->next = top.load(ACQ);  // Set the next pointer to the current top
    while (!cas(top, temp->next, temp, ACQ_REL)) {  // Attempt to update the top pointer atomically
        // Elimination logic: try to place the element in the elimination array
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> distrib(0, eli_arr.size() - 1);
        int index = distrib(gen);  // Randomly choose an index in the elimination array
        if (cas(eli_arr[index].status, (int)EMPTY, (int)PUSH, ACQ_REL)) {  // Attempt to push the element
            eli_arr[index].element = element;  // Store the element in the chosen array slot
            this_thread::sleep_for(chrono::nanoseconds(2));  // Short delay for synchronization
            if (cas(eli_arr[index].status, (int)POP, (int)EMPTY, ACQ_REL)) {  // Check if the element was consumed
                delete temp;  // Delete the temporary node if consumed
                break;
            } else {
                temp->next = top.load(ACQ);  // Reset the temporary node if elimination failed
                eli_arr[index].status.store(EMPTY, REL);  // Reset the array slot status
            }
        }
    }
}


// Pop an element from the Treiber stack with elimination
bool treiber_stack_elim::pop(int &element) {
    // Try to pop from the stack by accessing the top element atomically
    stack_node* temp = top.load(ACQ);
    
    while (temp && !cas(top, temp, temp->next, ACQ_REL)) {
        // If the stack is not empty, but the CAS (Compare-And-Swap) fails, attempt elimination
        random_device rd;    // Random number generator seed
        mt19937 gen(rd());   // Mersenne Twister random number generator
        uniform_int_distribution<> distrib(0, eli_arr.size() - 1);  // Uniform distribution for index
        int index = distrib(gen);  // Generate a random index for the elimination array

        // Try placing the element into the elimination array (simulates the push operation)
        element = eli_arr[index].element;
        if (cas(eli_arr[index].status, (int)PUSH, (int)POP, ACQ_REL)) {
            // If the element is successfully matched with a pop request, return true
            return true;
        }
    }

    // If the stack pop was successful (element found), retrieve and delete it
    element = temp->element;
    delete temp;
    return true;
}

// Push an element onto the stack with elimination
void stack_elim::push(int element) {
    // Create a new stack node with the provided element
    stack_node* temp = new stack_node(element, nullptr);

    while (true) {
        // Attempt to acquire the lock for the stack operation
        if (cas(lock, false, true, ACQ_REL)) {
            temp->next = top.load(ACQ);  // Atomically read the current top of the stack
            top.store(temp, REL);        // Update the top of the stack to point to the new node
            lock.store(false, REL);      // Release the lock after successful push operation
            return;
        }

        // If the lock acquisition fails, attempt to use the elimination array
        random_device rd;    // Random number generator seed
        mt19937 gen(rd());   // Mersenne Twister random number generator
        uniform_int_distribution<> distrib(0, eli_arr.size() - 1);  // Uniform distribution for index
        int index = distrib(gen);  // Generate a random index for the elimination array

        // Try placing the element into the elimination array for elimination-based pushing
        if (cas(eli_arr[index].status, (int)EMPTY, (int)PUSH, ACQ_REL)) {
            eli_arr[index].element = element;  // Set the element in the chosen slot
            std::this_thread::sleep_for(std::chrono::nanoseconds(2)); // Short delay to simulate work

            // Check if the element was consumed by a corresponding pop operation
            if (cas(eli_arr[index].status, (int)POP, (int)EMPTY, ACQ_REL)) {
                delete temp; // If eliminated successfully, delete the temporary node
                return;
            }

            // If elimination failed, reset the slot status
            eli_arr[index].status.store(EMPTY, REL);
        }
    }
}

// Pop an element from the stack (using both lock and elimination)
bool stack_elim::pop(int &element) {
    while (true) {
        // Attempt to acquire the lock for the stack pop operation
        if (cas(lock, false, true, ACQ_REL)) {
            stack_node* temp = top.load(ACQ);  // Load the top element atomically
            if (!temp) {
                lock.store(false, REL);  // Release the lock if stack is empty
                return false;  // Stack is empty, nothing to pop
            }

            // Retrieve the element and update the top pointer
            element = temp->element;
            top.store(temp->next, REL);  // Update the top to the next element
            lock.store(false, REL);      // Release the lock after pop
            delete temp;                 // Delete the node to free memory
            return true;
        }

        // If lock acquisition fails, attempt elimination first
        random_device rd;    // Random number generator seed
        mt19937 gen(rd());   // Mersenne Twister random number generator
        uniform_int_distribution<> distrib(0, eli_arr.size() - 1);  // Uniform distribution for index
        int index = distrib(gen);  // Generate a random index for the elimination array

        // Try to match with a pop operation in the elimination array
        if (cas(eli_arr[index].status, (int)EMPTY, (int)POP, ACQ_REL)) {
            while (true) {
                if (eli_arr[index].status.load(ACQ) == EMPTY) {
                    // If the slot is empty, the element was successfully popped
                    element = eli_arr[index].element;
                    return true;
                }
                std::this_thread::yield();  // Yield the thread to allow others to proceed
            }
        }
    }
}

// Push an element onto the stack using a flat stack with elimination
void stack_flat::push(int element) {
    stack_node* temp = new stack_node(element, nullptr);

    while (true) {
        // Attempt to acquire the lock for stack push operation
        if (cas(lock, false, true, ACQ_REL)) {
            temp->next = top.load(ACQ);  // Atomically set the next pointer to the current top
            top.store(temp, REL);        // Update the top of the stack

            // Handle events in the elimination array to match PUSH with POP
            for (int i = 0; i < (int)eli_arr.size(); i++) {
                if (eli_arr[i].status.load(ACQ) == PUSH) {
                    bool matched = false;
                    for (int j = i + 1; j < (int)eli_arr.size(); j++) {
                        // Try matching a PUSH with a POP in the elimination array
                        if (eli_arr[j].status.load(ACQ) == POP) {
                            eli_arr[j].element = eli_arr[i].element;  // Resolve the PUSH-POP pair
                            eli_arr[j].status.store(EMPTY, REL);  // Mark the slot as empty
                            eli_arr[i].status.store(EMPTY, REL);  // Mark the slot as empty
                            matched = true;
                            break; // Break to the next PUSH slot
                        }
                    }
                    // If no match found, push the element into the stack
                    if (!matched) {
                        stack_node* new_node = new stack_node(eli_arr[i].element, nullptr);
                        new_node->next = top.load(ACQ);  // Point the new node to the current top
                        top.store(new_node, REL);        // Update the top of the stack
                        eli_arr[i].status.store(EMPTY, REL);  // Reset the slot
                    }
                } else if (eli_arr[i].status.load(ACQ) == POP) {
                    bool matched = false;
                    for (int j = i + 1; j < (int)eli_arr.size(); j++) {
                        // Try matching a POP with a PUSH in the elimination array
                        if (eli_arr[j].status.load(ACQ) == PUSH) {
                            eli_arr[i].element = eli_arr[j].element;  // Resolve the POP-PUSH pair
                            eli_arr[i].status.store(EMPTY, REL);  // Mark the slot as empty
                            eli_arr[j].status.store(EMPTY, REL);  // Mark the slot as empty
                            matched = true;
                            break; // Break to the next POP slot
                        }
                    }
                    // If no match found, pop from the stack
                    if (!matched) {
                        stack_node* stack_top = top.load(ACQ);
                        if (stack_top) {
                            eli_arr[i].element = stack_top->element;  // Assign top element to elimination slot
                            top.store(stack_top->next, REL);  // Update top
                            delete stack_top;  // Delete the old top node
                        }
                        eli_arr[i].status.store(EMPTY, REL);  // Mark the slot as empty
                    }
                }
            }

            lock.store(false, REL);  // Release the lock
            return;
        }

        // Attempt elimination if lock acquisition fails
        random_device rd;    // Random number generator seed
        mt19937 gen(rd());   // Mersenne Twister random number generator
        uniform_int_distribution<> distrib(0, eli_arr.size() - 1);  // Uniform distribution for index
        int index = distrib(gen);  // Generate a random index for the elimination array

        // Try placing the element into the elimination array
        if (cas(eli_arr[index].status, (int)EMPTY, (int)PUSH, ACQ_REL)) {
            eli_arr[index].element = element;  // Set the element in the chosen slot
            std::this_thread::sleep_for(std::chrono::nanoseconds(2)); // Brief wait

            // Check if the element was successfully consumed
            if (eli_arr[index].status.load(ACQ) == EMPTY) {
                delete temp;  // Element was successfully eliminated
                return;
            }
        }
    }
}

// Pop an element from the stack using flat stack with elimination
bool stack_flat::pop(int &element) {
    while (true) {
        // Attempt to acquire the lock for stack pop operation
        if (cas(lock, false, true, ACQ_REL)) {
            stack_node* temp = top.load(ACQ);  // Load the top element atomically
            if (!temp) {
                lock.store(false, REL);  // Release the lock if stack is empty
                return false;  // Stack is empty
            }

            element = temp->element;  // Retrieve the element
            top.store(temp->next, REL);  // Update the top pointer

            // Traverse the elimination array to handle events
            for (int i = 0; i < (int)eli_arr.size(); i++) {
                if (eli_arr[i].status.load(ACQ) == PUSH) {
                    bool matched = false;
                    for (int j = i + 1; j < (int)eli_arr.size(); j++) {
                        // Try matching a PUSH with a POP
                        if (eli_arr[j].status.load(ACQ) == POP) {
                            eli_arr[j].element = eli_arr[i].element;  // Resolve the PUSH-POP pair
                            eli_arr[j].status.store(EMPTY, REL);  // Mark the slot as empty
                            eli_arr[i].status.store(EMPTY, REL);  // Mark the slot as empty
                            matched = true;
                            break; // Continue with the next PUSH slot
                        }
                    }
                    // If no match found, push the element into the stack
                    if (!matched) {
                        stack_node* new_node = new stack_node(eli_arr[i].element, nullptr);
                        new_node->next = top.load(ACQ);  // Point the new node to the current top
                        top.store(new_node, REL);        // Update the top of the stack
                        eli_arr[i].status.store(EMPTY, REL);  // Reset the slot
                    }
                } else if (eli_arr[i].status.load(ACQ) == POP) {
                    bool matched = false;
                    for (int j = i + 1; j < (int)eli_arr.size(); j++) {
                        // Try matching a POP with a PUSH
                        if (eli_arr[j].status.load(ACQ) == PUSH) {
                            eli_arr[i].element = eli_arr[j].element;  // Resolve the POP-PUSH pair
                            eli_arr[i].status.store(EMPTY, REL);  // Mark the slot as empty
                            eli_arr[j].status.store(EMPTY, REL);  // Mark the slot as empty
                            matched = true;
                            break; // Continue with the next POP slot
                        }
                    }
                    // If no match found, pop from the stack
                    if (!matched) {
                        stack_node* stack_top = top.load(ACQ);
                        if (stack_top) {
                            eli_arr[i].element = stack_top->element;  // Assign top element to the elimination slot
                            top.store(stack_top->next, REL);  // Update top pointer
                            delete stack_top;  // Delete the old top node
                        }
                        eli_arr[i].status.store(EMPTY, REL);  // Mark the slot as empty
                    }
                }
            }

            lock.store(false, REL);  // Release the lock
            return true;  // Return true indicating a successful pop operation
        }

        // Attempt elimination if lock acquisition fails
        random_device rd;    // Random number generator seed
        mt19937 gen(rd());   // Mersenne Twister random number generator
        uniform_int_distribution<> distrib(0, eli_arr.size() - 1);  // Uniform distribution for index
        int index = distrib(gen);  // Generate a random index for the elimination array

        // Try to eliminate the pop operation
        if (cas(eli_arr[index].status, (int)EMPTY, (int)POP, ACQ_REL)) {
            while (true) {
                if (eli_arr[index].status.load(ACQ) == EMPTY) {
                    // If the slot is empty, element has been successfully popped
                    element = eli_arr[index].element;
                    return true;
                }
                std::this_thread::yield();  // Yield the thread to allow others to proceed
            }
        }
    }
}


