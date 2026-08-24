# Virtual memory 
## Supervisor Address Translation and Protection(SATP) Register 
- Running process passes address of instruction memoy pointers to the processor
- Virtual memory map consists of multiple page tables of 512 elements
- Processor takes the memory passed by the process and navigate through multiple levels of virtual memory page tables to finally reach the physical memory address

- Page table entry
    - Contains hardware level informations like, validity of a page, if the page is a user page, is page readable, is page writable, PPN[0-2] : parts required to construct physical address from virtual address

## Process memory layout 
A process is logically divided into the following parts, known as segments:
- *Text*: the instructions of the program.
- *Data*: the static variables used by the program.
- *Heap*: an area from which programs can dynamically allocate extra memory.
- *Stack*: a piece of memory that grows and shrinks as functions are called and return and that is used to allocate storage for local variables and function call linkage information.

## sbrk() and brk() system calls
- brk : Program Break; Stands for the Process(program) break point in the virtual memory layout;
- Initially *program break* pointer points to the memory location right after un-initialized data segment(which is represented by *end*. Code can access *end* pointer by defining a end[] array at the beginning)
- Through brk() system call, we can increase/decrease size of the heap allocation for the process
- The call sbrk(0) returns the current setting of the program break without changing it.
"""
After the program break is increased, the program may access any address in
the newly allocated area, but no physical memory pages are allocated yet. The ker-
nel automatically allocates new physical pages on the first attempt by the process to
access addresses in those pages.
"""

### Doubts 
?. When fork() is executed by a process, does the kernel create the duplicate process with completely new memory allocation, then copy everything from parent process to duplicate?
- Yes. The child inherits following from parent
    - Data, stack and heap segments
- But the program *Text* is shared by the two processes
    - *Text* area is marked as read only. Hence it is ok to share between processes

?. What if the child process need to execute a completely different program?
- Often child processes use *execve()* system call to load and execute entirely different program
- *execve()* call destroys:
    - text, data, stack and heap segments
- Replace them with new segments based on the code of new program

?. How virtual memory fascilitiate *shared memory* IPC? What features of virtual memory are exploited here?
