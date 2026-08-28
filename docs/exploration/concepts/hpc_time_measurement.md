
- x86 has instruction `rdtsc`(read time stamp counter)
    - 64 bit counter
    - counts number of nano-seconds passed from epoch
        - In unix `epoch` is 1st Jan 1970
    - Initial value is synchronized through 
        - NTP or CMOS 
    
ns_boot = ns_at_boot + TSC * Factor 

At beginning Factor = 1/3 if frequency is 3GHz

Time calculation should not be complicated. Otherwise the calculation itslef would trigger drift.
Usually use bit manipulation to achieve this multiplication operation.

1/3 ~ 85/256 

This can be composed into two operations 
1. multiply by 85 
2. Divide by 256 
    - Converted into bit shift
    - number >> 8

ns_time = boot + (TSC * Factor) >> 8

This operation would take some clock cycles

we need to compensate for this

ns = time + ((TSC - offset) * factor) >> shift

time: last high quality time reading from sync source
offset: counter value when we got the last sync source

`rdtsc` doesn't measure frequency of the core(since core frequency may vary).
Now there's a base clock and `rdtsc` reads the base clock.
Still there are minor shifts/offsets in the clock.

To compensate these, the `factor` is modified.

Most of these counters are only accessible in privilaged mode. system call is required to access them.
But there are special read only area in the memory, which can be accessed from unprivilaged code as well. This enables user space programs to know the time without expensive system calls. 

shift, factor, time and offset : These are the mapped values 

Once ther mapped values are available, the user space libraries would execute the equation 
$ ns = time + ((TSC - offset) * factor ) >> shift $ 
This whole operation takes around 35ns to complete
Compensate for that as well

Typical workflow using time stamp

rdtsc -> A
|
|
| ---- Some code
|
|
rdtsc -> B

time taken for "some code" = ((B - A) * factor))>>shift

rdtscp : AMD version of the instruction. This instruction ensures all instructions previous to this should be finished(unlike rdtsc from intel)
Through this approach, rdtscp resolves the instruction reordering inconsistencies in the time measurement
Load - Store fences should be placed before and after rdtscp to make it more accurate. Otherwise, there are chances for the code outside the section of interest to leak inside. One example is `cpuid`. This instruction simply reads the cpu id. But it act as a fence.


Reference: https://www.youtube.com/watch?v=xs5iOwkX9fU&t=859s


Other projects from Matt Godbolt
1. [Compiler Explorer](https://godbolt.org/)
2. [BBC Micro Emulator](https://github.com/mattgodbolt/jsbeeb)  (https://bbc.xania.org/)
