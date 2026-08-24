In low-level C, an expression with side effects triggers a direct modification of the hardware state—specifically CPU registers, stack memory, or heap memory—in addition to yielding a value for immediate computation.
Here is exactly what happens behind the scenes at the machine level.
## Memory and Hardware Dynamics

* Lvalue Evaluation: The CPU calculates the exact memory address (RAM or stack pointer offset) of the target variable.
* The Mutation: A CPU instruction (STR in ARM or MOV in x86) writes a new bit pattern directly to that memory address.
* The Value Yield: The expression evaluates to the assigned value, which is held in a high-speed CPU register (like eax or r0) for immediate use by subsequent operators.

Example: y = (x = 5);
1. CPU loads immediate value 5 into register.
2. CPU stores 5 from register into memory address of 'x' (The Side Effect).
3. The expression (x = 5) evaluates to 5, which remains in the register.
4. CPU stores that register value 5 into memory address of 'y'.

## The Concept of "Sequence Points"
In C, side effects do not always happen instantly; they are guaranteed to complete only at specific anchors called Sequence Points. Between sequence points, the compiler is free to reorder instructions for optimization.

* Semicolons (;): Marks a statement boundary where all pending memory writes must be finished.
* Logical AND/OR (&&, ||): Evaluates the left side completely, performs its side effects, and uses that result to decide whether to evaluate the right side.
* Comma Operator (,): Evaluates the left expression, discards its value but applies its side effects, then evaluates the right side.
* Function Calls: All arguments are fully evaluated, and their side effects are applied, right before execution jumps to the function body.

## The Danger of Undefined Behaviour
If you attempt to modify a memory location more than once—or read it and modify it simultaneously—within a single sequence point, the C standard declares it Undefined Behaviour (UB). The compiler can generate unpredictable assembly.

* Bad: i = i++; (Modifies i twice without an intervening sequence point).
* Bad: arr[i] = i++; (It is unspecified whether the index i is evaluated before or after the increment side effect takes place).

## Volatile Memory Access
Normally, the compiler optimizes code by keeping values in registers and delaying writes to RAM. The volatile keyword changes this dynamic entirely.

* Hardware Mapping: It tells the compiler that the memory address maps to hardware peripherals (like a sensor register).
* Guaranteed Execution: Every single read or write expression is treated as a critical side effect. The compiler is forbidden from optimizing them out or caching them in registers.
