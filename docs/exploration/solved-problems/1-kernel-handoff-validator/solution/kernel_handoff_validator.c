#include <stdint.h>
#include <stdbool.h>



typedef struct {
    char stage_name[8];       // "BL1", "BL2", "BL31", "BL32", "BL33", "KERNEL"
    uint32_t execution_level; // 1 = EL1, 2 = EL2, 3 = EL3
    bool is_secure;           // true = Secure World, false = Non-Secure World
    uint64_t load_address;    // Base memory address
    uint64_t image_size;      // Size in bytes
    bool is_in_sram;          // true = On-chip SRAM, false = System DRAM
} BootStage;

/* structure to easily map stage_name */

typedef union{
    uint8_t stage_name;
    char BL1;
    char BL2;
    char BL3;
    char BL1;

}

typedef struct {
    uint64_t x[4];            // CPU General Purpose Registers x0, x1, x2, x3
    uint32_t sctlr;           // System Control Register (SCTLR_EL1/EL2)
    uint8_t daif;             // Interrupt mask bits: Bit 3:D, Bit 2:A, Bit 1:I, Bit 0:F
    uint32_t dtb_magic;       // Header magic value at memory address pointed to by x0
} CpuState;

typedef struct {
    uint64_t child_base;      // Child bus base physical address
    uint64_t parent_base;     // Parent (CPU physical) base address
    uint64_t size;            // Window size in bytes
} BusRange;

typedef struct { 
    bool handoff_status;
    uint64_t out_dtb_phys_addr;
} HandoffResult;
HandoffResult handoff_validator(BootStage bootstage, CpuState cpustate, BusRange busrange)
{
    HandoffResult handoff_result = {0};

    return handoff_result;
}

/*
#### 1. Stage Sequence & Architectural Privilege Invariants

* **Chronological Order:** Stage execution order must follow `BL1` $\rightarrow$ `BL2` $\rightarrow$ `BL31` $\rightarrow$ `BL32` (Optional) $\rightarrow$ `BL33` $\rightarrow$ `KERNEL`.
* **Execution Levels & Security Rules:**
* **`BL1`**: Must execute in **Secure EL3** from **SRAM**.
* **`BL2`**: Must execute in **Secure EL1 or EL3** from **SRAM**.
* **`BL31`**: Must execute in **Secure EL3** (SRAM or DRAM).
* **`BL32`** (Optional OP-TEE): Must execute in **Secure EL1** from **DRAM**.
* **`BL33`** (U-Boot/Barebox): Must execute in **Non-Secure EL1 or EL2** from **DRAM**.
* **`KERNEL`**: Must execute in **Non-Secure EL1 or EL2** from **DRAM**.
*/
bool invariants_validator(BootStage bootstage)
{
    uint8_t* bs_stage_name = bootstage.stage_name;
    // BL is represented using and 8 bit character array. Define bit masks for comparison
    // check if more than one bit is active in bl_stage_name. If yes, simply reject it. 
    bool has_multiple_bits_set = (*bs_stage_name) * (*bs_stage_name - 1);
    if(has_multiple_bits_set)
        return 0;
    
    /*
    truth table
                EL1         EL2         EL3     DRAM    SRAM 
    BL1         0           0           1       0       1
    BL2         1           0           1       0       1
    BL31        0           0           1       1       1
    BL32        1           0           0       1       0
    BL33        1           1           0       1       0
    KERNEL      1           1           0       1       0

    Apply kmap simplification on this truth table

    Dropping this approach. Ofcourse, K-Map would improve the raw effciency. But in this sparse set, K-Map would introduce un-necessary complexity while giving little effciency gains. So we've decided to proceed with simple switch cases. Still, we would use the above truth table as reference for implementation.
    */   
}