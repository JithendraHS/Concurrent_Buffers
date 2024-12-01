#include <atomic> // Include atomic for potential atomic operations (not used directly here)

// Memory order definitions for atomic operations
#define SEQCST (memory_order_seq_cst)        // Sequentially consistent
#define RELAXED (memory_order_relaxed)      // Relaxed memory order
#define ACQ (memory_order_acquire)          // Acquire memory order
#define REL (memory_order_release)           // Release memory order
#define ACQ_REL (memory_order_acq_rel)      // Acquire-release memory order

#define STACK  (1) // Define a macro for stack buffer type
#define QUEUE  (2) // Define a macro for queue buffer type

using namespace std; // Use the standard namespace to avoid prefixing standard library components

// Define a node structure for the stack
template <typename T>
struct node {
    T element;              // Value stored in the stack node
    struct node *next;  // Pointer to the next node in the stack
    node(T element, node* next = nullptr)
        : element(element), next(next) {}
};
typedef node<int> stack_node; // Typedef for convenience
typedef node<int> queue_node; // Typedef for convenience

// Stack class for implementing a stack (Last In, First Out - LIFO)
class stack {
    public:
        stack_node *top;    // Pointer to the top node of the stack
        stack();            // Constructor to initialize an empty stack
        void push(int element); // Push an element onto the stack
        bool pop(int &element);              // Pop an element from the stack and return its value
};

// Queue class for implementing a queue (First In, First Out - FIFO)
class queue {
    public:
        queue_node *head; // Pointer to the head (front) of the queue
        queue_node *tail; // Pointer to the tail (rear) of the queue
        queue();          // Constructor to initialize an empty queue
        void insert(int element); // Insert an element at the tail of the queue
        bool remove(int &element);             // Remove an element from the head of the queue and return its value
};

class treiber_stack{
    public:
        atomic<stack_node *> top;
        treiber_stack();
        void push(int element);
        bool pop(int &element);
};

class mns_queue{
    public:
        atomic<queue_node *>head;
        atomic<queue_node *>tail;
        mns_queue();
        void insert(int element);
        bool remove(int &element);
};