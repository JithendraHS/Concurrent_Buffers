struct stack_node{
    int element;
    struct stack_node * next;
};

typedef struct stack_node stack_node;

struct queue_node{
    int element;
    struct queue_node * right;
    struct queue_node * left;
};
typedef struct queue_node queue_node;

class stack{
    public:
    stack_node * top;
    stack_node * bottom;
    stack();
    void push(int element);
    int pop();
};

class queue{
    public:
    queue_node * head;
    queue_node * tail;
    queue();
    void insert(int element);
    int remove();
};