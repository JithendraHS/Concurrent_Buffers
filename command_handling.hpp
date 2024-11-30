#include <iostream>
struct command_param{
    char* source_file;
    char* out_file;
    char* stack;
    char* queue;
    unsigned pop_count;
};
typedef struct command_param command_param;

extern unsigned NUM_THREADS;

int command_handle(int argc, char *argv[], command_param * ch);