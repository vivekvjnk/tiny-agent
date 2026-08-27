Think of **C++ templates** as **compile-time code generators**.

Since you know C and Python:

* **C**: you often duplicate code or use macros / `void *`.
* **Python**: one function naturally works with many types at runtime.
* **C++ templates**: one generic definition is specialized by the compiler for concrete types at compile time.

## Function template

```cpp
template <typename T>
T max(T a, T b)
{
    return a > b ? a : b;
}
```

Usage:

```cpp
max<int>(10, 20);
max<float>(1.2f, 3.4f);
```

The compiler effectively generates type-specific versions:

```cpp
int max(int, int);
float max(float, float);
```

So you get **genericity like Python**, but with **type checking and performance like C**.

## Class template

```cpp
template <typename T>
class Buffer {
    T data[100];
};
```

Now:

```cpp
Buffer<int> int_buffer;
Buffer<float> float_buffer;
```

are different concrete types.

## The key mental model

Templates are basically:

> **"Give the compiler a pattern for generating type-specific code."**

The compiler substitutes `T` with actual types during compilation.

## Why templates matter

They power most of modern C++:

```cpp
std::vector<int>
std::vector<float>
std::array<uint8_t, 256>
std::unique_ptr<Device>
```

For an embedded C programmer, templates are especially useful because they provide **compile-time abstraction without necessarily paying runtime overhead**.

Example:

```cpp
template <int SIZE>
struct RingBuffer {
    uint8_t data[SIZE];
};
```

```cpp
RingBuffer<128> uart_rx;
RingBuffer<512> ethernet_rx;
```

`SIZE` is known at compile time, so this can be much better than a generic dynamically sized structure.

**One-line summary:**
**Templates let C++ write generic code that is expanded and type-checked at compile time.**
