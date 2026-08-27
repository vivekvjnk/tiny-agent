In C++,  is a sequence container that represents a dynamic array. Unlike standard static arrays, vectors can automatically resize themselves when elements are added or deleted. They are the go-to container in modern C++ for managing collections of sequential data due to their speed and ease of use. [1, 2, 3, 4, 5]  
To use vectors, you must include the  header file. [1, 6]  
# 🚀 Quick Code Example 
```cpp
#include <iostream>
#include <vector> // Required header

int main() {
    // 1. Initialization
    std::vector<int> numbers = {10, 20, 30};

    // 2. Add elements
    numbers.push_back(40);
    numbers.push_back(50);

    // 3. Access elements safely
    std::cout << "First element: " << numbers.at(0) << "\n"; 
    std::cout << "Last element: " << numbers.back() << "\n";

    // 4. Loop through the vector
    std::cout << "All elements: ";
    for (int num : numbers) {
        std::cout << num << " ";
    }
    
    return 0;
}
```

# ⚙️ Core Mechanics: Size vs. Capacity 
Understanding how a vector manages memory under the hood is critical for writing efficient C++ code: 

• Size (): The actual number of elements currently stored in the vector. 
• Capacity (): The total amount of memory allocated. A vector always allocates extra memory to handle future growth. When capacity runs out, the vector automatically reallocates a larger block of memory (usually doubling in size) and copies the elements over. [2, 3, 5]  

🛠️ Common Operations & Cheat Sheet 

1. Declaration & Initialization
```cpp
std::vector<int> v1;               // Empty vector
std::vector<int> v2(5);            // Vector of size 5, initialized to 0
std::vector<int> v3(5, 10);        // Vector of size 5, all initialized to 10
std::vector<int> v4 = {1, 2, 3};   // Initializer list (C++11 and later)
```

2. Modifying Elements 
• push_back(val) : Appends  to the end. 
• pop_back() : Removes the last element. 
• insert(iterator,val) : Inserts  at the specified iterator position (slower, requires shifting elements). 
• erase(iterator) : Removes an element at the specified position. 
• clear() : Removes all elements from the vector. [3, 4, 5, 7, 8]  

3. Accessing Elements 
• v[i]: Quick direct access. Fast, but does not check if the index  is valid. 
• v.at(i): Accesses element with bounds-checking. Throws an error (std::out_or_range) if i is invalid. 
• v.front() / v.back() : Returns the first / last element. 
• v.data() : Returns a raw pointer to the underlying array. [1, 3, 4, 5, 7, 8]  

⏱️ Performance (Big O Complexity) 

| Operation | Time Complexity | Notes  |
| --- | --- | --- |
| Random Access (v[i] or v.at(i)) | O(1) | Constant time, just like raw arrays.  |
| Insert/Delete at end (push_back) | Amortized O(1) | Fast, unless memory allocation triggers a resize.  |
| Insert/Delete in Middle (insert/erase) | O(n) | Linear time, because elements must shift.  |

💡 Performance Optimization Tips 

1. Use reserve() if you know the size: Reallocating memory is a heavy process. If you know you need to store 10,000 items, run  first. This allocates memory upfront and completely bypasses background resizing overhead. 
2. Pass by Reference in Functions: To avoid copying the entire vector when passing it to a function, pass it by reference (&) or constant reference (const &): [5]  
```cpp
void printVector(const std::vector<int>& v) { /* ... */ }
```
 

[1] https://www.geeksforgeeks.org/cpp/vector-in-cpp-stl/
[2] https://en.cppreference.com/cpp/container/vector
[3] https://www.youtube.com/watch?v=fk-AtA2vC9s
[4] https://learn.microsoft.com/en-us/cpp/standard-library/vector-class?view=msvc-170
[5] https://www.youtube.com/watch?v=iM_rIWmq6cE
[6] https://www.w3schools.com/cpp/cpp_vectors.asp
[7] https://coddy.tech/docs/cpp/vector
[8] https://www.youtube.com/watch?v=TaySu61K6dI

