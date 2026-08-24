




/*
* **3.3. Check Power of Two and Nearest Power Alignment**
* Write a macro or static inline function `is_power_of_two(uint32_t x)` that evaluates to true/false in $O(1)$.
* Write `uint32_t align_up_power_of_two(uint32_t x)` which rounds up a 32-bit integer to the nearest power of 2 (e.g., `13` $\rightarrow$ `16`, `16` $\rightarrow$ `16`).
*/


int is_power_of_two(uint32_t x){
    /*
    Key logic: Any power of two number -> only one bit is set in the binary representation of number
        - negate 1 from the number -> All bits to the right would be flipped
        - (number & (number -1)) != 0 -> number is power of 2; otherwise number is not power of 2
        - 0 is an edge case.  
    */
    return (x!=0)&&(x & (x -1));
}

uint32_t align_up_power_of_two(uint32_t x) {
    if (x == 0) return 1; // 0 rounds up to 2^0 = 1
    
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;
    
    return x;
}