# GDB debugging 

- Disable ASLR([Address Space Layout Randomization](resources/aslr.md))
- .config debug_info should be true to get full debug information

- Show the C code at breakpoint hit
`(gdb) list`
- Show assembly code at breakpoint hit
`(gdb) disassemble`

- Show register values at breakpoint
`(gdb) info registers

- Show arguments passed to the break function
`(gdb) info args`

- Show local variables 
`(gdb) info locals`


## GDB TUI mode
- Split view mode; Top window with source code and bottom gdb console window


# Useful tricks
## 1. Add breakpoint on a panic
```bash
(gdb) break panic # or hbreak (stands for hardware break)
```
## 2. Trigger kernel panic
```bash
echo c > /proc/sysrq-trigger
```


## Device drivers
- Concept of constructor and destructor 
    - How pluggable driver architecture works? 


- Three types of drivers
    - Character driver
    - Network drivers
    - Block drivers
### System calls in drivers
- Major two types
    - File operations and IO operations

