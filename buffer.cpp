#include "buffer.hpp"

template <typename T>
bool cas(atomic<T> &status, T expected, T desired, memory_order mem_order) {
    T expected_ref = expected; // Create a reference to the expected value
    // Attempt to compare and swap 'status' atomically
    return status.compare_exchange_strong(expected_ref, desired, mem_order);
}


// Constructor for stack
stack::stack() {
    top = nullptr;    // Initialize the top pointer to null (empty stack)
    bottom = nullptr; // Initialize the bottom pointer to null (empty stack)
}

// Push an element onto the stack
void stack::push(int element) {
    // Create a new node for the stack
    stack_node *temp = new stack_node();
    temp->element = element; // Assign the value to the new node
    temp->next = top;        // Point the new node to the current top
    top = temp;              // Update the top pointer to the new node
}

// Pop an element from the stack and return its value
int stack::pop() {
    stack_node *temp = top;  // Save the current top node
    int element = temp->element; // Retrieve the value of the top node
    top = top->next;         // Move the top pointer to the next node
    delete temp;             // Free the memory of the removed node
    return element;          // Return the value of the removed node
}

// Constructor for queue
queue::queue() {
    head = nullptr; // Initialize the head pointer to null (empty queue)
    tail = nullptr; // Initialize the tail pointer to null (empty queue)
}

// Insert an element at the end of the queue
void queue::insert(int element) {
    // Create a new node for the queue
    queue_node *temp = new queue_node();
    temp->element = element; // Assign the value to the new node
    temp->right = tail;      // Link the new node to the current tail
    if (tail) {
        tail->left = temp;   // Update the left pointer of the current tail
    }
    temp->left = nullptr;    // The new node's left pointer is null
    if (tail == nullptr && head == nullptr) {
        head = temp;         // If the queue is empty, the new node becomes the head
    }
    tail = temp;             // Update the tail pointer to the new node
}

// Remove an element from the front of the queue and return its value
int queue::remove() {
    queue_node *temp = head; // Save the current head node
    int element = temp->element; // Retrieve the value of the head node
    head = head->left;       // Move the head pointer to the next node
    if (head) {
        head->right = nullptr; // Disconnect the removed node from the queue
    } else {
        tail = nullptr;       // If the queue becomes empty, reset the tail pointer
    }
    delete temp;             // Free the memory of the removed node
    return element;          // Return the value of the removed node
}


treiber_stack::treiber_stack(){
     top.store(nullptr, RELAXED);
     bottom.store(nullptr, RELAXED);
}

void treiber_stack::push(int element){
    stack_node * temp = new stack_node();
    temp->element = element;
    temp->next = top.load(ACQ);
    while(!cas(top, temp->next, temp, ACQ_REL)){
         temp->next = top.load(ACQ);
    }
}

int treiber_stack::pop(){
    stack_node * temp = top.load(ACQ);
    while(!cas(top, temp, temp->next, ACQ_REL)){
         temp = top.load(ACQ);
    }
    int element = temp->element;
    delete temp;
    return element;
}