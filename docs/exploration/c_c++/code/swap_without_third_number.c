/*
## 1. The Core Abstraction (Group Theory)
The swap algorithm works without a third variable because operations like addition, subtraction, XOR, and XNOR are Group Operations.

* In abstract algebra, these operations possess the Cancellation Property.
* When you combine two variables, one variable effectively acts as an information-preserving cipher key for the other.
* Because the overall system retains two variables at any given moment, the process is a bijection (a reversible, one-to-one mapping), allowing you to perfectly decode the original values.

## 2. The Generalization
This logic applies to any commutative (Abelian) group, meaning the swap algorithm can theoretically be performed using:

* Modular Arithmetic (clock arithmetic)
* Vector Addition (geometry and physics engines)
* Matrix Multiplication (under strict invertible constraints)

## 3. The Limits: Annihilators vs. Identities
The math breaks down when an operation introduces an element that destroys information:

* Identities (like 0 in addition or 1 in multiplication) are safe because they change nothing.
* Annihilators (like 0 in multiplication) act as a computational black hole. Performing a * 0 creates a true many-to-one mapping where the original value is permanently erased.

## 4. The Thermodynamic and Coding Reality

* Landauer's Principle: Erasing information is a physical process. Reversible operations (like XOR) conserve information, while operations involving an annihilator (like multiplying by zero) destroy entropy, forcing the CPU to flush transistor charges and release physical heat.
* Security & Vulnerabilities: In cryptography and hashing, accidental interaction with an annihilator creates an "Information Sink." This collapses the key space into a single predictable state, completely shattering the system's security.
*/

#include <stdio.h>

int main() {
    int a = 10, b = 20;

    printf("Before swap: a = %d, b = %d\n", a, b);

    // Swapping logic
    a = a ^ b;  // a now holds the XOR combination of both
    b = a ^ b;  // b gets the original value of a
    a = a ^ b;  // a gets the original value of b

    /* Same can be done using addition and multiplication
    // addition + subtraction
    a = a + b;  // a now holds the XOR combination of both
    b = a - b;  // b gets the original value of a
    a = a - b;  // a gets the original value of b

    //multiplication + division
    a = a * b;  // a now holds the XOR combination of both
    b = a / b;  // b gets the original value of a
    a = a / b;  // a gets the original value of b

    Xor method is preferred, because it is most efficient and avoids any overflows.
    */

    printf("After swap: a = %d, b = %d\n", a, b);

    return 0;
}
