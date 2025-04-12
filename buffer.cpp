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
    queue_node *dummy = new queue_node(0, nullptr);
    head.store(dummy, RELAXED);;  // Create a dummy head node
    tail.store(dummy, RELAXED);  // Initialize the tail pointer atomically to the dummy node
}

void mns_queue::insert(int element) {
    queue_node *temp = new queue_node(element, nullptr);  // Create a new node
    while (true) {
        queue_node *last = tail.load(ACQ);               // Load the current tail atomically
        queue_node *next = last->next;                  // Check the next pointer of the tail
        
        if (next == nullptr) {                          // If the tail's next is null, link the new node
                last->next = temp;
                // If successful, update the tail to point to the new node
                cas(tail, last, temp, ACQ_REL);
                //cout << "I am here 1: "<< element << endl;
                return;
        } else {                                        // Tail is already being updated; advance the tail
            cas(tail, last, next, ACQ_REL);
        }
    }
}

bool mns_queue::remove(int &element) {
    while (true) {
        queue_node *temp = head.load(ACQ);  // Load the current head atomically
        if (!temp || !temp->next) {        // If the queue is empty
            return false;
        }
        queue_node *next_node = temp->next;  // Get the next node
        if (cas(head, temp, next_node, ACQ_REL)) {  // Attempt to update the head atomically
            element = next_node->element;    // Retrieve the value from the next node
            //delete temp;                     // Free the old head node
            //cout << "I am here 2" << endl;
            return true;
        }
        // CAS failed, retry
    }
}


void treiber_stack_elim::push(int element) {
    stack_node *temp = new stack_node(element, nullptr);
    temp->next = top.load(ACQ);  // Set the next pointer to the current top

    while (true) {
        if (cas(top, temp->next, temp, ACQ_REL)) {
            // Stack push successful
            break;
        }

        // Elimination logic: try to place the element in the elimination array
        random_device rd;
        static thread_local mt19937 gen(rd());
        uniform_int_distribution<> distrib(0, eli_arr.size() - 1);
        int index = distrib(gen);

        // Attempt to reserve the slot for elimination
        if (cas(eli_arr[index].status, (int)EMPTY, (int)PUSH, ACQ_REL)) {
            eli_arr[index].element = element;  // Store the element
            this_thread::sleep_for(chrono::nanoseconds(10));  // Allow time for a matching pop
            if (cas(eli_arr[index].status, (int)POP, (int)EMPTY, ACQ_REL)) {
                // Element was consumed, cleanup and exit
                delete temp;
                return;
            } else {
                // Elimination failed, retry stack push
                eli_arr[index].status.store(EMPTY, REL);  // Reset the slot
            }
        }

        // Update temp->next in case the stack top changed during elimination
        temp->next = top.load(ACQ);
    }
}



bool treiber_stack_elim::pop(int &element) {
    stack_node* temp = top.load(ACQ);

    while (true) {
        if (temp == nullptr) {
            // Stack is empty, check elimination array
            random_device rd;    // Random number generator seed
            static thread_local mt19937 gen(rd());
            uniform_int_distribution<> distrib(0, eli_arr.size() - 1);
            int index = distrib(gen);

            // Attempt to match a PUSH operation in the elimination array
            if (cas(eli_arr[index].status, (int)PUSH, (int)POP, ACQ_REL)) {
                // Successfully matched a push; retrieve the element
                element = eli_arr[index].element;
                eli_arr[index].status.store(EMPTY, REL);  // Reset the slot
                return true;
            }
            return false;  // No stack elements or elimination match
        }

        // Attempt to pop from the stack
        if (cas(top, temp, temp->next, ACQ_REL)) {
            element = temp->element;
            delete temp;  // Free the memory
            return true;
        }

        // Retry stack pop or switch to elimination logic
        temp = top.load(ACQ);  // Reload the top pointer for retry
    }
}


// Push an element onto the stack with elimination
void stack_elim::push(int element) {
    // Create a new stack node with the provided element
    stack_node* temp = new stack_node(element, nullptr);

    static thread_local mt19937 gen(std::random_device{}()); // Thread-local random generator
    uniform_int_distribution<> distrib(0, eli_arr.size() - 1); // Uniform distribution for index

    while (true) {
        // Attempt to acquire the lock for the stack operation
        if (cas(lock, false, true, ACQ_REL)) {
            // Lock acquired, perform the stack push
            temp->next = top.load(ACQ);  // Atomically read the current top of the stack
            top.store(temp, REL);        // Update the top of the stack to point to the new node
            lock.store(false, REL);      // Release the lock after successful push operation
            return;
        }

        // Lock acquisition failed, attempt elimination
        int index = distrib(gen); // Generate a random index for the elimination array

        // Try placing the element into the elimination array
        if (cas(eli_arr[index].status, (int)EMPTY, (int)PUSH, ACQ_REL)) {
            eli_arr[index].element = element;  // Set the element in the chosen slot
            std::this_thread::sleep_for(std::chrono::nanoseconds(10)); // Short delay to simulate work

            // Check if the element was consumed by a corresponding pop operation
            if (cas(eli_arr[index].status, (int)POP, (int)EMPTY, ACQ_REL)) {
                delete temp; // Element successfully eliminated; clean up node
                return;
            }

            // Reset the elimination slot if not consumed
            eli_arr[index].status.store(EMPTY, REL);
        }
    }
}


// Pop an element from the stack (using both lock and elimination)
bool stack_elim::pop(int &element) {
    static thread_local mt19937 gen(std::random_device{}()); // Thread-local random generator
    uniform_int_distribution<> distrib(0, eli_arr.size() - 1); // Uniform distribution for index

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

        // Lock acquisition failed, attempt elimination
        int index = distrib(gen);  // Generate a random index for the elimination array

        // Attempt to match with a push operation in the elimination array
        if (cas(eli_arr[index].status, (int)EMPTY, (int)POP, ACQ_REL)) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(2)); // Short delay to allow matching

            // Check if an element was provided by a push operation
            if (eli_arr[index].status.load(ACQ) == EMPTY) {
                element = eli_arr[index].element; // Retrieve the matched element
                return true;
            }

            // Reset the slot to EMPTY if no match occurred
            eli_arr[index].status.store(EMPTY, REL);
        }
    }
}


void stack_flat::push(int element) {
    stack_node* temp = new stack_node(element, nullptr);
    static thread_local mt19937 gen(std::random_device{}()); // Thread-local random generator
    uniform_int_distribution<> distrib(0, eli_arr.size() - 1);

    while (true) {
        // Attempt to acquire the lock for stack push operation
        if (cas(lock, false, true, ACQ_REL)) {
            // Lock acquired - process all entries in the elimination array
            for (int i = 0; i < (int)eli_arr.size(); i++) {
                // Process elimination array slots in a lock-holder exclusive manner
                if (eli_arr[i].status.load(ACQ) == PUSH) {
                    bool matched = false;
                    // Attempt to match this PUSH with a POP in the array
                    for (int j = i + 1; j < (int)eli_arr.size(); j++) {
                        if (eli_arr[j].status.load(ACQ) == POP) {
                            // Match found - resolve the PUSH-POP pair
                            eli_arr[j].element = eli_arr[i].element;  // Transfer the element
                            eli_arr[j].status.store(EMPTY, REL);      // Reset the POP slot
                            eli_arr[i].status.store(EMPTY, REL);      // Reset the PUSH slot
                            matched = true;
                            break;
                        }
                    }
                    // If no match found, push the element into the stack
                    if (!matched) {
                        stack_node* new_node = new stack_node(eli_arr[i].element, nullptr);
                        new_node->next = top.load(ACQ);  // Point the new node to the current top
                        top.store(new_node, REL);        // Update the top of the stack
                        eli_arr[i].status.store(EMPTY, REL);  // Reset the slot after push
                    }
                }
                else if (eli_arr[i].status.load(ACQ) == POP) {
                    bool matched = false;
                    // Attempt to match this POP with a PUSH in the array
                    for (int j = i + 1; j < (int)eli_arr.size(); j++) {
                        if (eli_arr[j].status.load(ACQ) == PUSH) {
                            // Match found - resolve the POP-PUSH pair
                            eli_arr[i].element = eli_arr[j].element;  // Transfer the element
                            eli_arr[i].status.store(EMPTY, REL);      // Reset the POP slot
                            eli_arr[j].status.store(EMPTY, REL);      // Reset the PUSH slot
                            matched = true;
                            break;
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
                        eli_arr[i].status.store(EMPTY, REL);  // Reset the slot after pop
                    }
                }
            }

            // Proceed with the regular push operation
            temp->next = top.load(ACQ);  // Atomically set the next pointer to the current top
            top.store(temp, REL);        // Update the top of the stack

            lock.store(false, REL);  // Release the lock
            return;
        }

        // Elimination handling if the lock acquisition fails
        int index = distrib(gen);  // Generate a random index
        if (cas(eli_arr[index].status, (int)EMPTY, (int)PUSH, ACQ_REL)) {
            eli_arr[index].element = element;
            std::this_thread::sleep_for(std::chrono::nanoseconds(2));

            // If the element was consumed, exit the loop
            if (eli_arr[index].status.load(ACQ) == EMPTY) {
                delete temp;  // Element was successfully eliminated
                return;
            }
        }
    }
}


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
            }
        }
    }
}



