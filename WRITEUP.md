# Final project: Concurrent Containers
## Objective
The main goal of this project is to implement and analyze various concurrent stack and queue algorithms. This includes Single Global Lock Stack and Queue, Treiber’s Lock-Free Stack, Michael and Scott (M&S) Lock-Free Queue. Additionally, the project explores the performance differences between lock-based and lock-free buffer implementations. Advanced techniques like Elimination and Flat Combining have also been implemented for both stacks and queues to optimize concurrency and reduce contention.

Key tools and debugging methods such as Address Sanitizer and GDB were utilized to ensure the correctness and stability of the implementations. Memory leaks are not accounted for in the analysis.

The implementations include:
- Single global lock stack
- Single global lock Queue
- Treiber stack
- Single global lock stack with elimination method
- Treiber stack with elimination method
- Micheal and scott queue
- Single global lock stack with flat combining

## Implementation details
The stacks and queues in this project are implemented using a linked list structure. This design facilitates efficient insertion and removal operations for both stacks and queues.

Each concurrent method (e.g., SGL, Treiber, M&S, Elimination, Flat Combining) is encapsulated within a dedicated class. These classes include:
- Push and Pop methods for stack implementations.
- Insertion and Removal methods for queue implementations

### SGL stack: 
#### Push Operation:
- Creates a new node to store the given element.
- Links the new node to the current top of the stack.
- Updates the stack's top pointer to the new node.
- Ensures thread safety using a lock during the operation.

#### Pop Operation:
- Checks if the stack is empty; returns false if it is.
- Retrieves the element from the top node.
- Updates the stack's top pointer to the next node.
- Frees the memory of the removed node.


### SGL Queue:
#### Insert Operation:
- Creates a new node to store the given element.
- Checks if the queue is empty:
- If empty, the new node becomes both the head and the tail of the queue.
- Otherwise, the new node is added to the end of the queue, and the tail pointer is updated.

#### Remove Operation:
- Checks if the queue is empty:
- If empty, resets the tail pointer and returns false.
- Retrieves the element from the head node.
- Updates the head pointer to the next node.
- Frees the memory of the removed node.


### Treiber stack:
### Push Operation
- Creates a new node with the given element and sets its next pointer to the current top atomically.
- Uses the CAS (Compare-and-Swap) operation to atomically update the top pointer.
- If CAS fails, the operation reloads the top and retries until successful.

#### Pop Operation
- Atomically loads the current top node of the stack.
- Returns false if the stack is empty.
- Uses CAS to atomically update the top pointer to the next node.
- If CAS fails or the stack becomes empty during retries, the operation continues to attempt until successful.
- Stores the popped element, deletes the node, and returns true

### M&S Queue:
#### Insert Operation
- Create a new node with the given element.
- Atomically check the current tail.
- If the tail's next is nullptr, link the new node and update the tail using CAS. If not, advance the tail to the next node.

#### Remove Operation
- Atomically load the head node.
- If the queue is empty, return false.
- Atomically update the head to the next node and retrieve its element.
- Retry if the CAS operation fails.

### Treiber stack with elimination array:
#### Push Operation
- Create a new node and set its next pointer to the current stack top.
- Attempt to push the node onto the stack atomically using CAS.
- If CAS fails, attempt to place the element in a randomly chosen slot in the elimination array.
- If elimination succeeds (i.e., the element is consumed by a pop), clean up and exit.
- If elimination fails, reset the slot and retry the stack push.

#### Pop Operation
- Atomically load the current top node.
- If the stack is empty, attempt to match a PUSH operation in the elimination array and retrieve the element.
- If the stack is not empty, attempt to pop the top node atomically using CAS.
- If CAS fails, reload the top pointer and retry


### SGL with elimination array:
#### Push Operation
- Attempt to acquire a lock and push the element onto the stack.
- If lock acquisition fails, insert the element into a random slot in the elimination array.
- If eliminated, clean up and exit; otherwise, retry.

#### Pop Operation
- Attempt to acquire a lock and pop the top element from the stack.
- If lock acquisition fails, attempt to match a PUSH operation in the elimination array.
- If a match is found, retrieve the element; otherwise, retry

### SGL with flat combining:
#### Push Operation
- Acquire lock to ensure mutual exclusion.
- Handle elimination array for matching PUSH and POP. If no match, push element onto stack.
- Release lock after operation.
- Retry if element is not consumed by randomly selecting an elimination array index.

#### Pop Operation
- Acquire lock to ensure mutual exclusion.
- Handle elimination array for matching PUSH and POP, or pop from stack.
- Return element or false if stack is empty.
- Release lock after operation.
- Retry if lock acquisition fails by eliminating with a random index.

### Test cases:
- Each thread pushes an element from the input vector to the stack.
- Each thread pops an element from the stack to the output vector.
- Repeat until both vectors are fully processed.
- Ensure indices do not exceed vector bounds.
- Comparing the output vector elements with the input vector to find duplication and abnormalities in the algorithm using linux commands, if it passes the conditon showing `success` at the end

#### Example output:
```
Running: ./container -i 1K_entry.txt -o out.txt -t 4 --queue=mns
option --> i:1K_entry.txt
option --> o:out.txt
option --> t:4
option --> queue: mns
Elapsed (ns): 17287419
Elapsed (s): 0.017287
Done!!!

 Performance counter stats for './container -i 1K_entry.txt -o out.txt -t 4 --queue=mns':

             60088      cache-misses              #   12.404 % of all cache refs
            484440      cache-references
               199      page-faults
           6776104      branch-instructions
            200812      branch-misses             #    2.96% of all branches

       0.030726407 seconds time elapsed

       0.006096000 seconds user
       0.054869000 seconds sys


Sorting out.txt
1010 out.txt
1000 out.txt
Comparing out.txt with 1K_entry.txt
Success: Output matches input
-----------------------------------------
```

### Stacks(1K elements)
#### 1 Thread
| Algorithm         | Runtime (us) | L1 cache hit (%) |
|:--------------:|:-----:|:-----------:|
| SGL |  437 |        37 |
| Treiber  |  354 |        43 |
| SGL elimination |  456 |    56     |
| Treiber elimination |  544 |    51     |
| flat combining |  723 |    50    |
#### 2 Threads
| Algorithm         | Runtime (us) | L1 cache hit (%) |
|:--------------:|:-----:|:-----------:|
| SGL |  601 |        54 |
| Treiber  |  590 |        60 |
| SGL elimination |  541 |    45     |
| Treiber elimination |  423 |    45     |
| flat combining |  814 |    44     |

#### 3 Threads
| Algorithm         | Runtime (us) | L1 cache hit (%) |
|:--------------:|:-----:|:-----------:|
| SGL |  692 |        60 |
| Treiber  |  629 |        53 |
| SGL elimination |  557 |    54     |
| Treiber elimination |  452 |    54     |
| flat combining |  712 |    53     |

#### 4 Threads
| Algorithm         | Runtime (us) | L1 cache hit (%) |
|:--------------:|:-----:|:-----------:|
| SGL |  961 |        57 |
| Treiber  |  644 |        50 |
| SGL elimination |  498 |    58     |
| Treiber elimination |  403 |    58     |
| flat combining |  782 |    72     |

#### 8 Threads
| Algorithm         | Runtime (us) | L1 cache hit (%) |
|:--------------:|:-----:|:-----------:|
| SGL |  1382 |        67 |
| Treiber  |  887 |        68 |
| SGL elimination |  820 |    58     |
| Treiber elimination |  774 |    70     |
| flat combining |  983 |    60     |

#### 16 Threads
| Algorithm         | Runtime (us) | L1 cache hit (%) |
|:--------------:|:-----:|:-----------:|
| SGL |  1859 |        66 |
| Treiber  |  1440 |        73 |
| SGL elimination |  1438 |    73     |
| Treiber elimination |  1577 |    69     |
| flat combining |  1722 |    63     |

### Performance comparison
- As the number of threads increases, the runtime for all mechanisms increases due to conctext switching and memory contention. This is especially noticeable as we move from simpler mechanisms like Single Global Lock (SGL) to advanced techniques like Flat Combining
- Mechanisms like Trieber’s Stack show significant performance degradation with an increasing thread count. This is because more threads simultaneously attempt to access the top node address, leading to contention on the top node and reduced overall performance.
-  Elimination mechanisms experience less contention compared to Trieber’s Stack by offloading operations to an elimination array. Threads spin on this array instead of contending for the top node, reducing contention overhead
- Although flat combinging reduces contention, the runtime for Flat Combining is higher because the lock holder must process all pending push and pop operations stored in the elimination array before releasing the lock
- Single Global Lock (SGL) exhibits the highest runtime among the mechanisms. The overhead of lock handover is considerably more expensive than the spin-and-acquire approach used in other mechanisms, leading to degraded performance as the thread count increases


### Queue(1K elements)
#### 1 Thread
| Algorithm         | Runtime (us) | L1 cache hit (%) |
|:--------------:|:-----:|:-----------:|
| SGL |  576 |        47 |
| M&S  |  7684 |        75 |

#### 2 Threads
| Algorithm         | Runtime (us) | L1 cache hit (%) |
|:--------------:|:-----:|:-----------:|
| SGL |  586 |        53 |
| M&S  |  15741 |        49 |
#### 3 Threads
| Algorithm         | Runtime (us) | L1 cache hit (%) |
|:--------------:|:-----:|:-----------:|
| SGL |  845 |        55 |
| M&S  |  12041 |        84 |

#### 4 Threads
| Algorithm         | Runtime (us) | L1 cache hit (%) |
|:--------------:|:-----:|:-----------:|
| SGL |  1178 |        60 |
| M&S  |  17287 |        88 |

#### 8 Threads
| Algorithm         | Runtime (us) | L1 cache hit (%) |
|:--------------:|:-----:|:-----------:|
| SGL |  2106 |        47 |
| M&S  |  24060 |        77 |

#### 16 Threads
| Algorithm         | Runtime (us) | L1 cache hit (%) |
|:--------------:|:-----:|:-----------:|
| SGL |  1498 |        70 |
| M&S  |  22899 |        74 |

### Performance comparison
- The runtime of both SGL and M&S increases as the number of threads grows for the same test sample. This behavior highlights the effects of contention, context-switching overhead, and thread spinning when comparing head and tail nodes in the queue buffer.
- The runtime for M&S is higher than SGL due to its Compare-And-Swap (CAS) mechanism. CAS involves Read-Write-Modify (RWM) operations on both the head and tail nodes, which may lead to additional overhead compared to SGL’s single-lock mechanism
- SGL Cache hits increase with the number of threads, indicating efficient memory access and reduced contention
- M&S (Michael and Scott): Cache hits decrease with the number of threads due to increased cache contention caused by the CAS operations on shared memory regions
- For M&S cache hits are decresing with number of threads due to cache contention as we discussed above
- Considering the trade-offs, if faster execution and improved cache efficiency are desired as the number of threads increases, the SGL mechanism is more suitable

## Code structure:

- `main`: The function performs several key tasks, including handling command-line arguments, reading and writing files, creating and managing threads, measuring execution time, and performing operations on a data structure (stack or queue)

- `insert_remove_sgl_stack`: Function to handle stack insertions and removals for a specific buffer type.

- `insert_remove_sgl_queue`: Function to handle queue insertions and removals for a specific buffer type.

- `insert_remove_treiber`: Function to handle Treiber stack insertions and removals for a specific buffer type.

- `insert_remove_mns`: Function to handle MNS queue insertions and removals for a specific buffer type.

- `insert_remove_treiber_elim`: Function to handle Treiber stack elimination insertions and removals for a specific buffer type.

- `insert_remove_sgl_elim`: Function to handle stack elimination insertions and removals for a specific buffer type.
- `push and pop` : The push and pop for each algorithm which have there self explanatory characteristics

## File description:
- `concurrent_containers.cpp`: program reads data from an input file, processes it using multiple threads and different buffer types (stack or queue), and writes the results to an output file, while measuring and displaying the execution time
- `buffer.cpp` : code implements several lock-free and elimination-based data structures in C++, including stack, queue, Treiber stack, and M&S queue, using atomic operations and Compare-and-Swap (CAS) to ensure thread safety without blocking. It also includes an advanced elimination approach for stack operations that helps in reducing contention.
- `parallelized_code.cpp` : A file is a collection of data or information stored on a storage device, typically organized in a specific format, and accessed by a program or user for reading, writing, or manipulation.

## Bugs:
- IO stream is not thread safe
- The esisting code is not stable have bugs, csn see segmentation faults and infinite loop and other boundary conditions.
- Garbage collect is not taken care
- ABA problem might not be addressed properly in few cases
- 

## How to run:
- Run `./test_cases.sh`
- Edit the `test_cases.sh` file by changing the parameters such as `input_files`, `num_threads`, `stack_types` and `queue_types`.
- For quick verification of algorithm make use of `Makefile` by running `make`
- Then `./container -h` will give you information about the usage of the command and how to pass the parameters accordingly

## References:
- https://max-inden.de/post/2020-03-28-elimination-backoff-stack/


## Machine capabilities:
Used a machine having 16 cores, so tested thread counts upto 16 starting from 1
```
jihs6098@ecen4313-fl23-0:~/finalproject-JithendraHS$ lscpu
Architecture:                       x86_64
CPU op-mode(s):                     32-bit, 64-bit
Byte Order:                         Little Endian
Address sizes:                      46 bits physical, 48 bits virtual
CPU(s):                             16
On-line CPU(s) list:                0-15
Thread(s) per core:                 1
Core(s) per socket:                 16
Socket(s):                          1
NUMA node(s):                       1
Vendor ID:                          GenuineIntel
CPU family:                         6
Model:                              85
Model name:                         Intel(R) Xeon(R) Gold 6226R CPU @ 2.90GHz
Stepping:                           7
CPU MHz:                            2899.998
BogoMIPS:                           5799.99
Virtualization:                     VT-x
Hypervisor vendor:                  KVM
Virtualization type:                full
L1d cache:                          512 KiB
L1i cache:                          512 KiB
L2 cache:                           64 MiB
L3 cache:                           16 MiB
NUMA node0 CPU(s):                  0-15
```

## Sample output:
```
jihs6098@ecen4313-fl23-0:~/finalproject-JithendraHS$ ./test_script.sh
*****Cleaning*****
rm -f *.o concurrent_containers command_handling buffer parallelized_code
g++-11 -Wall -Werror --std=c++20 -pthread  -g -c -o concurrent_containers.o concurrent_containers.cpp
*****Object file created concurrent_containers.o
g++-11 -Wall -Werror --std=c++20 -pthread  -g -c -o command_handling.o command_handling.cpp
*****Object file created command_handling.o
g++-11 -Wall -Werror --std=c++20 -pthread  -g -c -o buffer.o buffer.cpp
*****Object file created buffer.o
g++-11 -Wall -Werror --std=c++20 -pthread  -g -c -o parallelized_code.o parallelized_code.cpp
*****Object file created parallelized_code.o
g++-11 -Wall -Werror --std=c++20 -pthread  -g -o container concurrent_containers.o command_handling.o buffer.o parallelized_code.o
Usage: ./container [-i source.txt] [-o out.txt] [-t NUMTHREADS] [--stack=<sgl,treiber,sgl_elim,treiber_elim,stack_flat>] [--queue=<sgl,mns>]
-i : file containing elements to insert into stack or queue
-o : file to store remaining elements in stack or queue
-t : Number of threads for parallelism
--stack : stack type (e.g., sgl, treiber)
--queue : queue type (e.g., sgl, m&s)
--pop : # of elements to pop from the stack or queue
Running: ./container -i 1K_entry.txt -o out.txt -t 4 --stack=sgl
option --> i:1K_entry.txt
option --> o:out.txt
option --> t:4
option --> stack: sgl
Elapsed (ns): 961355
Elapsed (s): 0.000961
Done!!!

 Performance counter stats for './container -i 1K_entry.txt -o out.txt -t 4 --stack=sgl':

             53801      cache-misses              #   43.193 % of all cache refs
            124559      cache-references
               191      page-faults
           1345187      branch-instructions
             32854      branch-misses             #    2.44% of all branches

       0.014135304 seconds time elapsed

       0.008257000 seconds user
       0.008257000 seconds sys


Sorting out.txt
1010 out.txt
1000 out.txt
Comparing out.txt with 1K_entry.txt
Success: Output matches input
-----------------------------------------
Running: ./container -i 1K_entry.txt -o out.txt -t 4 --stack=treiber
option --> i:1K_entry.txt
option --> o:out.txt
option --> t:4
option --> stack: treiber
Elapsed (ns): 644186
Elapsed (s): 0.000644
Done!!!

 Performance counter stats for './container -i 1K_entry.txt -o out.txt -t 4 --stack=treiber':

             56561      cache-misses              #   50.284 % of all cache refs
            112482      cache-references
               190      page-faults
           1266783      branch-instructions
             33004      branch-misses             #    2.61% of all branches

       0.031058034 seconds time elapsed

       0.000000000 seconds user
       0.032113000 seconds sys


Sorting out.txt
1010 out.txt
1000 out.txt
Comparing out.txt with 1K_entry.txt
Success: Output matches input
-----------------------------------------
Running: ./container -i 1K_entry.txt -o out.txt -t 4 --stack=sgl_elim
option --> i:1K_entry.txt
option --> o:out.txt
option --> t:4
option --> stack: sgl_elim
Elapsed (ns): 498399
Elapsed (s): 0.000498
Done!!!

 Performance counter stats for './container -i 1K_entry.txt -o out.txt -t 4 --stack=sgl_elim':

             48948      cache-misses              #   42.574 % of all cache refs
            114971      cache-references
               189      page-faults
           1313501      branch-instructions
             32265      branch-misses             #    2.46% of all branches

       0.025644507 seconds time elapsed

       0.000000000 seconds user
       0.026501000 seconds sys


Sorting out.txt
1010 out.txt
1000 out.txt
Comparing out.txt with 1K_entry.txt
Success: Output matches input
-----------------------------------------
Running: ./container -i 1K_entry.txt -o out.txt -t 4 --stack=treiber_elim
option --> i:1K_entry.txt
option --> o:out.txt
option --> t:4
option --> stack: treiber_elim
Elapsed (ns): 402676
Elapsed (s): 0.000403
Done!!!

 Performance counter stats for './container -i 1K_entry.txt -o out.txt -t 4 --stack=treiber_elim':

             43398      cache-misses              #   42.368 % of all cache refs
            102430      cache-references
               189      page-faults
           1258090      branch-instructions
             32256      branch-misses             #    2.56% of all branches

       0.014618132 seconds time elapsed

       0.000000000 seconds user
       0.015428000 seconds sys


Sorting out.txt
1010 out.txt
1000 out.txt
Comparing out.txt with 1K_entry.txt
Success: Output matches input
-----------------------------------------
Running: ./container -i 1K_entry.txt -o out.txt -t 4 --stack=stack_flat
option --> i:1K_entry.txt
option --> o:out.txt
option --> t:4
option --> stack: stack_flat
Elapsed (ns): 781532
Elapsed (s): 0.000782
Done!!!

 Performance counter stats for './container -i 1K_entry.txt -o out.txt -t 4 --stack=stack_flat':

             34745      cache-misses              #   28.603 % of all cache refs
            121472      cache-references
               204      page-faults
           1583633      branch-instructions
             42138      branch-misses             #    2.66% of all branches

       0.007899307 seconds time elapsed

       0.000000000 seconds user
       0.007146000 seconds sys


Sorting out.txt
1010 out.txt
1000 out.txt
Comparing out.txt with 1K_entry.txt
Success: Output matches input
-----------------------------------------
Running: ./container -i 1K_entry.txt -o out.txt -t 4 --queue=sgl
option --> i:1K_entry.txt
option --> o:out.txt
option --> t:4
option --> queue: sgl
Elapsed (ns): 1178345
Elapsed (s): 0.001178
Done!!!

 Performance counter stats for './container -i 1K_entry.txt -o out.txt -t 4 --queue=sgl':

             57748      cache-misses              #   39.496 % of all cache refs
            146212      cache-references
               192      page-faults
           1404089      branch-instructions
             34956      branch-misses             #    2.49% of all branches

       0.014507091 seconds time elapsed

       0.008745000 seconds user
       0.008745000 seconds sys


Sorting out.txt
1010 out.txt
1000 out.txt
Comparing out.txt with 1K_entry.txt
Success: Output matches input
-----------------------------------------
Running: ./container -i 1K_entry.txt -o out.txt -t 4 --queue=mns
option --> i:1K_entry.txt
option --> o:out.txt
option --> t:4
option --> queue: mns
Elapsed (ns): 17287419
Elapsed (s): 0.017287
Done!!!

 Performance counter stats for './container -i 1K_entry.txt -o out.txt -t 4 --queue=mns':

             60088      cache-misses              #   12.404 % of all cache refs
            484440      cache-references
               199      page-faults
           6776104      branch-instructions
            200812      branch-misses             #    2.96% of all branches

       0.030726407 seconds time elapsed

       0.006096000 seconds user
       0.054869000 seconds sys


Sorting out.txt
1010 out.txt
1000 out.txt
Comparing out.txt with 1K_entry.txt
Success: Output matches input
-----------------------------------------

```