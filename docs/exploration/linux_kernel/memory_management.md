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
