#include "buffer.hpp"
#include <iostream>

template <typename T>
bool cas(atomic<T> &status, T expected, T desired, memory_order mem_order) {
    T expected_ref = expected; // Create a reference to the expected value
    // Attempt to compare and swap 'status' atomically
    return status.compare_exchange_strong(expected_ref, desired, mem_order);
}


// Constructor for stack
stack::stack() {
    top = nullptr;    // Initialize the top pointer to null (empty stack)
}

// Push an element onto the stack
void stack::push(int element) {
    // Create a new node for the stack
    stack_node *temp = new stack_node(element, nullptr);
    temp->element = element; // Assign the value to the new node
    temp->next = top;        // Point the new node to the current top
    top = temp;              // Update the top pointer to the new node
}

// Pop an element from the stack and return its value
bool stack::pop(int &element) {
    if(!top){
        return false;
    }
    stack_node *temp = top;  // Save the current top node
    element = temp->element; // Retrieve the value of the top node
    top = top->next;         // Move the top pointer to the next node
    delete temp;             // Free the memory of the removed node
    return true;          // Return the value of the removed node
}

// Constructor for queue
queue::queue() {
    head = nullptr; // Initialize the head pointer to null (empty queue)
    tail = nullptr; // Initialize the tail pointer to null (empty queue)
}

// Insert an element at the end of the queue
void queue::insert(int element) {
    // Create a new node for the queue
    queue_node *temp = new queue_node(element, nullptr);
    if (!head) {
        head = tail = temp;         // If the queue is empty, the new node becomes the head
    }else{
        tail->next = temp;
        tail = temp;
    }
}

// Remove an element from the front of the queue and return its value
bool queue::remove(int &element) {
    if (!head) {                 // If the queue becomes empty, reset the tail
        tail = nullptr;
        return false;
    }
    queue_node *temp = head; // Save the current head node
    element = temp->element; // Retrieve the value of the head node
    head = head->next;       // Move the head pointer to the next node
    delete temp;             // Free the memory of the removed node
    return true;          // Return the value of the removed node
}


treiber_stack::treiber_stack(){
     top.store(nullptr, RELAXED);
}

void treiber_stack::push(int element){
    stack_node * temp = new stack_node(element, nullptr);
    temp->next = top.load(ACQ);
    while(!cas(top, temp->next, temp, ACQ_REL)){
         temp->next = top.load(ACQ);
    }
}

bool treiber_stack::pop(int &element){
    stack_node * temp = top.load(ACQ);
    while(!cas(top, temp, temp->next, ACQ_REL)){
         temp = top.load(ACQ);
    }
    element = temp->element;
    delete temp;
    return true;
}

mns_queue::mns_queue(){
    head = new queue_node(0, nullptr);
    tail.store(head,RELAXED);
}

void mns_queue::insert(int element){
    queue_node *temp = new queue_node(element, nullptr);
    queue_node *last = tail.load(ACQ);
    last->next = temp;
    while(!cas(tail,last, temp, ACQ_REL)){
        last = tail.load(ACQ);
        last->next = temp;
    }
}

bool mns_queue::remove(int &element){
    queue_node * temp = head.load(ACQ);
    if(!temp->next){
        return false;
    }
    while(!cas(head, temp, temp->next, ACQ_REL)){
        temp = head.load(ACQ);
    }
    element = temp->next->element;
    delete temp;
    return true;
}