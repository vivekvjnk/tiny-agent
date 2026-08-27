#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>

// hash table datastructure
struct node{
    _Atomic uint32_t key;
    _Atomic uint32_t value;
};
#define EMPTY_KEY 0xFFFFFFFFU
#define DEFAULT_NODE {.key=EMPTY_KEY, .value=0}
#define MAX_SIZE 1024 // Make sure max size is a power of 2
#define MAX_SIZE_MASK (MAX_SIZE-1)
// define global hash table with fixed size

struct node g_hash_table[MAX_SIZE];
atomic_int_fast32_t g_last_key=0;

/*
two helper functions. One to insert item into the hash table, one to lookup item
*/

bool insert_item(uint32_t key,uint32_t value){
    uint32_t index = hash(key) & (MAX_SIZE_MASK);
    for(uint16_t i=0;i<MAX_SIZE;i++){
        uint32_t probe_index = (index + i) &(MAX_SIZE_MASK);
        uint32_t expected = EMPTY_KEY;
        
        if(atomic_compare_exchange_strong(&g_hash_table[probe_index].key,expected,key)){
            atomic_store(&g_hash_table[index].value, value);
            return true;
        }

        //slot was not empty. but key is same
        if(expected == key){
            atomic_store(&g_hash_table[index].value, value);
            return true;
        }
    }
    return false; // table is full
}

uint32_t lookup_item(uint32_t key){
    uint32_t index = hash(key) & (MAX_SIZE_MASK);
    for(uint16_t i=0;i<MAX_SIZE;i++){
        uint32_t probe_index = (index+i) & (MAX_SIZE_MASK);
        uint32_t k = atomic_load_explicit(&g_hash_table[probe_index].key,memory_order_acquire);

        if(k == key){
            return atomic_load_explicit(&g_hash_table[probe_index].value, memory_order_acquire);
        }
        
        if(k==EMPTY_KEY){
            break;
        }

    }
    return 0;
}
uint32_t hash(uint32_t key) {
    // Thomas wang 32 bit Mix function
    key = ~key + (key << 15); // key = (key << 15) - key - 1;
    key = key ^ (key >> 12);
    key = key + (key << 2);
    key = key ^ (key >> 4);
    key = (key + (key << 3)) + (key << 11); // key * 2053
    key = key ^ (key >> 16);
    return key;
}



