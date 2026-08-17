// mailbox scan using SIMD instructions

/*objective: 
Find all messages satisfying:

``` c
(flags & required_flags) == required_flags &&
payload_size >= min_size
```
*/
#define SIMD_LANES_32 4
#define MAX_MESSAGE_COUNT 128

struct Message {
    uint32_t type;
    uint32_t flags;
    uint32_t payload_size;
    uint32_t correlation_id;
};


struct MessageBlock {
    uint32_t flags[MAX_MESSAGE_COUNT];
    uint32_t payload_size[MAX_MESSAGE_COUNT];
};

/*
 * Fill unused SIMD lanes after the valid messages.
 *
 * Valid messages:
 *     [0 ... message_count-1]
 *
 * Tail:
 *     [message_count ... rounded_count-1]
 *
 * Zero-filled tail is intended to prevent out-of-bounds access
 * when the SIMD loop processes a complete vector.
 */
void fill_message_tail(struct MessageBlock *block,
                       size_t message_count)
{
    size_t rounded_count =
        ((message_count + SIMD_LANES_32 - 1) / SIMD_LANES_32)
        * SIMD_LANES_32;

    for (size_t i = message_count; i < rounded_count; ++i) {
        block->flags[i] = 0;
        block->payload_size[i] = 0;
    }
}