#include <atomic> // Include atomic for potential atomic operations (not used directly here)
#include <vector> // Include vector for dynamic arrays (used for elimination arrays)

// Memory order definitions for atomic operations
#define SEQCST (memory_order_seq_cst)    // Sequentially consistent
#define RELAXED (memory_order_relaxed)  // Relaxed memory order
#define ACQ (memory_order_acquire)      // Acquire memory order
#define REL (memory_order_release)      // Release memory order
#define ACQ_REL (memory_order_acq_rel)  // Acquire-release memory order

#define STACK  (1)  // Define a macro for stack buffer type
#define QUEUE  (2)  // Define a macro for queue buffer type

using namespace std;  // Use the standard namespace

// Performs a compare-and-swap operation on an atomic variable.
// Compares the variable to an expected value; if they are equal, sets it to a desired value.
template <typename T>
bool cas(atomic<T> &status, T expected, T desired, memory_order mem_order);


// Define a node structure for the stack or queue
template <typename T>
struct node {
    T element;               // Value stored in the stack/queue node
    struct node *next;       // Pointer to the next node
    
    node(T element, node* next = nullptr) : element(element), next(next) {}
};

typedef node<int> stack_node;  // Type alias for a stack node holding integer values
typedef node<int> queue_node;  // Type alias for a queue node holding integer values

// Stack class to implement a basic stack (LIFO: Last In, First Out)
class stack {
    public:
        atomic<bool> lock;           // Atomic lock to prevent race
        stack_node *top;    // Pointer to the top of the stack
        
        stack();            // Constructor to initialize an empty stack
        void push(int element);     // Push an element onto the stack
        bool pop(int &element);     // Pop an element from the stack and return its value
};

// Queue class to implement a basic queue (FIFO: First In, First Out)
class queue {
    public:
        atomic<bool> lock;           // Atomic lock to prevent race
        queue_node *head;  // Pointer to the head (front) of the queue
        queue_node *tail;  // Pointer to the tail (rear) of the queue
        
        queue();           // Constructor to initialize an empty queue
        void insert(int element);  // Insert an element at the tail of the queue
        bool remove(int &element); // Remove an element from the head of the queue and return its value
};

// Treiber Stack (Lock-Free Stack)
class treiber_stack {
    public:
        atomic<stack_node *> top;  // Atomic pointer to the top of the stack

        treiber_stack();           // Constructor to initialize the Treiber stack
        void push(int element);    // Push an element onto the Treiber stack
        bool pop(int &element);    // Pop an element from the Treiber stack and return its value
};

// MNS Queue (Multi-Node Stack) with atomic operations
class mns_queue {
    public:
        atomic<queue_node *> head; // Atomic pointer to the head (front) of the queue
        atomic<queue_node *> tail; // Atomic pointer to the tail (rear) of the queue
        
        mns_queue();            // Constructor to initialize the MNS queue
        void insert(int element);  // Insert an element at the tail of the MNS queue
        bool remove(int &element); // Remove an element from the head of the MNS queue and return its value
};

// Enumeration to represent different states in the elimination array
enum states {
    EMPTY,  // No operation in progress
    PUSH,   // Push operation in progress
    POP     // Pop operation in progress
};

// Struct for an elimination array entry
struct elimination_array {
    atomic<int> status;  // Status of the operation (EMPTY, PUSH, POP)
    int element;         // The element to be pushed or popped
    
    elimination_array() : status(EMPTY), element(0) {}  // Constructor initializes status as EMPTY
};

typedef struct elimination_array elimination_array;  // Typedef for convenience

// Treiber Stack with Elimination
class treiber_stack_elim {
    public:
        atomic<stack_node *> top;     // Atomic pointer to the top of the Treiber stack
        vector<elimination_array> eli_arr; // Vector of elimination arrays (one for each thread)
        
        treiber_stack_elim(int num) : top(nullptr), eli_arr(num) {}  // Constructor initializes top and elimination array
        
        void push(int element);        // Push an element onto the Treiber stack with elimination
        bool pop(int &element);        // Pop an element from the Treiber stack with elimination
};

// Stack with Elimination (Lock-Free Stack with Elimination)
class stack_elim {
    public:
        atomic<bool> lock;           // Atomic lock to prevent race conditions in push/pop operations
        atomic<stack_node *> top;    // Atomic pointer to the top of the stack
        vector<elimination_array> eli_arr; // Vector of elimination arrays (one for each thread)
        
        stack_elim(int num) : lock(false), top(nullptr), eli_arr(num) {} // Constructor initializes lock, top, and elimination array
        
        void push(int element);       // Push an element onto the stack with elimination
        bool pop(int &element);       // Pop an element from the stack with elimination
};

// Stack with Flat Locking
class stack_flat {
    public:
        atomic<bool> lock;           // Atomic lock to prevent race conditions in push/pop operations
        atomic<stack_node *> top;    // Atomic pointer to the top of the stack
        vector<elimination_array> eli_arr; // Vector of elimination arrays (one for each thread)
        
        stack_flat(int num) : lock(false), top(nullptr), eli_arr(num) {}  // Constructor initializes lock, top, and elimination array
        
        void push(int element);       // Push an element onto the stack with flat locking
        bool pop(int &element);       // Pop an element from the stack with flat locking
};
