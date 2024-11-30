#include "buffer.hpp"

stack::stack(){
    top = nullptr;
    bottom = nullptr;
}

void stack::push(int element){
    stack_node * temp = new stack_node();
    temp->element = element;
    temp->next = top;
    top = temp;
}

int stack::pop(){
    stack_node * temp = top;
    int element = temp->element;
    top = top->next;
    delete temp;
    return element;
}

queue::queue(){
    head = nullptr;
    tail = nullptr;
}

void queue::insert(int element){
    queue_node * temp = new queue_node();
    temp->element = element;
    temp->right = tail;
    if(tail){
        tail->left = temp;
    }
    temp->left = nullptr;
    if(tail == nullptr && head == nullptr){
        head = temp;
    }
    tail = temp;
}

int queue::remove(){
    queue_node * temp = head;
    int element = temp->element;
    head = head->left;
    if (head) {
        head->right = nullptr;
    } else {
        tail = nullptr;
    }
    delete temp;
    return element;
}