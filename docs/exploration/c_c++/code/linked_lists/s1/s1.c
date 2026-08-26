/*
### **1. Linked Lists: Lock-Free RCU Unlink Engine**

**Concept:** Intrusive Doubly Linked Lists & Read-Copy-Update (RCU)

**Scenario:** Linux drivers manage dynamic data structures (e.g., loaded kernel modules) using intrusive lists (`struct list_head`). Safe node unlinking while lockless readers traverse the list requires strict memory ordering and RCU grace-period handling.

**Problem:** Implement an intrusive doubly linked list node deletion function in C (`list_del_rcu_safe`).

* The intrusive list uses a standard kernel-style definition:
```c
struct list_head {
    struct list_head *next, *prev;
};

```


* Implement `void list_del_rcu_safe(struct list_head *entry)` to unlink `entry` from a circular doubly linked list without corrupting concurrent lockless readers traversing forward via `next` pointers.


* Ensure proper CPU memory barrier placement (e.g., `WRITE_ONCE` semantics) so that any concurrent reader reading `node->next` never observes an invalid pointer or a broken chain.
* **Constraints:** Do not use heavy spinlocks during traversal; design specifically for reader-side RCU semantics.
*/

// Header setup
#define container_of(ptr, type, member) \
((type *) ((char *)(ptr) - offsetof(type,member)))

#define WRITE_ONCE(x, value)\
(*(volatile (typeof(x) *)(&x)) = value)

#define READ_ONCE(x)\
(*(volatile const (typeof(x) *)(&x)))

struct node{
    struct dll *next;
    struct dll *prev;
};

struct dll{
    int data;
    struct node *link;
};

int list_del_rcu_safe(struct dll *element){

    struct dll *next = element->link->next;
    struct dll *prev = element->link->prev;

    // replace next nodes prev element with prev node pointer
    next->link->prev = prev;

    // replace prev nodes next pointer with next pointer. Use WRITE_ONCE
    WRITE_ONCE(prev->link->next,next);


    // avoid element -> prev -> next bugs by poisoning prev value
    WRITE_ONCE(element->link->prev, (struct dll *)0xDEADBEEF);
}

