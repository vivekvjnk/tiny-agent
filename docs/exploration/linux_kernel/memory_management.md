# Supervisor Address Translation and Protection(SATP) Register 
- Running process passes address of instruction memoy pointers to the processor
- Virtual memory map consists of multiple page tables of 512 elements
- Processor takes the memory passed by the process and navigate through multiple levels of virtual memory page tables to finally reach the physical memory address

- Page table entry
    - Contains hardware level informations like, validity of a page, if the page is a user page, is page readable, is page writable, PPN[0-2] : parts required to construct physical address from virtual address