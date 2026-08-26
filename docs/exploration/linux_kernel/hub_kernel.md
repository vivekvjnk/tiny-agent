# Concepts

- shared vs static libraries in unix based OS
- Shared Libraries 
    - Less memory usage: runtime and storage 
    - Easy to update a shared library; All consumers refer to the same library
    - Dynamic linker: explore implementation of linux dynamic linker
        - Dynamic linker links any program that uses the shared library to correct memory address of the shared libraries


## Type of filesystems 
[Reference](https://youtu.be/Sk9TatW9ino?si=kQxtC_NvPfkdGaaV)
1. Block backed file systems
    - Fixed size
    - Block device of fixed size
    - Formatted and interpretted through a second device driver(eg:ext2,ex4)
    - eg: flash
2. Pipe backed file systems
    - Protocol based
    - Talks to a pipe through a protocol
    - Other end of the pipe is a program(eg:samba) providing the file system
    - eg: Network FS
3. RAM File systems
    - Automatically resize themselves to the size of content
    - Files are stored in RAM memory
    - Highly memory efficient
    - Stores in page cache
    - eg: RAMFS
4. Synthetic file system
    - No backing store 
    - eg: /proc 

* RAM Disk is not RAMFS. It is a block filesystem. RAM Disk(RD) is mapped to a block driver(eg:ext2). Through this driver system interacts with the RAM Disk. 
    - Earlier versions of linux used init.rd to store kernel in RAM 
    - New versions use init.ramfs

# References 
- The Linux Programming Interfaces (TLPI) - Michael Kerrisk