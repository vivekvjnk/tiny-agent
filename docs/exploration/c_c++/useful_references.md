# GCC Source code

1. [memcpy](https://github.com/gcc-mirror/gcc/blob/master/libgcc/memcpy.c)
```c
/* Public domain.  */
#include <stddef.h>

void *
memcpy (void *dest, const void *src, size_t len)
{
  char *d = dest;
  const char *s = src;
  while (len--)
    *d++ = *s++;
  return dest;
}
```
- copies first len bytes from src to dest
- character pointer allows aliasing to any datatype according to c standard
- support for unaligned memory: byte copy support unaligned memory
 
2. [memcmp](https://github.com/gcc-mirror/gcc/blob/master/libgcc/memcmp.c)
```c
/* Public domain.  */
#include <stddef.h>

int
memcmp (const void *str1, const void *str2, size_t count)
{
  const unsigned char *s1 = str1;
  const unsigned char *s2 = str2;

  while (count-- > 0)
    {
      if (*s1++ != *s2++)
	  return s1[-1] < s2[-1] ? -1 : 1;
    }
  return 0;
}
```
- As per C standard, if first differing byte of str1 is less than that of str2, memcmp should return -1, otherwise +1. This is why we have a ternary operator in the code.


## [MALLOC](https://github.com/iromise/glibc/blob/master/malloc/malloc.c)

- **Chunks**: Core concept; Allocates a chunk of memory and returns pointer to user data section of the chunk
- **Bins**: Free chunks are grouped into bins
    - Fast bins: Kept as simple LIFO; very small and frequent allocations; 
    - Small bins: Medium sized chunks; Kept in doubly linked lists
    - Large bins: For larger chunks
    - Unsorted bins: Holding area for bins that have been freed but not categorized into any of the above 3
- Concept of *Arenas* to support multi-threading
    - Heap is divided into arenas for each thread
    - This allows individual threads to run undisturbed as long as they don't run out of their arena memory
    - Each arena has it's own locks

```c
struct malloc_chunk {

    INTERNAL_SIZE_T mchunk_prev_size; /* Size of previous chunk (if free).  */
    INTERNAL_SIZE_T mchunk_size;      /* Size in bytes, including overhead. */

    struct malloc_chunk *fd; /* double links -- used only if free. */
    struct malloc_chunk *bk;

    /* Only used for large blocks: pointer to next larger size.  */
    struct malloc_chunk *fd_nextsize; /* double links -- used only if free. */
    struct malloc_chunk *bk_nextsize;
};

/*
   malloc_chunk details:

    An allocated chunk looks like this:
    
    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |             Size of previous chunk, if unallocated (P clear)  |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |             Size of chunk, in bytes                     |A|M|P|
      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |             User data starts here...                          .
        .                                                               .
        .             (malloc_usable_size() bytes)                      .
        .                                                               |
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |             (size of chunk, but used for application data)    |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |             Size of next chunk, in bytes                |A|0|1|
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    Where "chunk" is the front of the chunk for the purpose of most of
    the malloc code, but "mem" is the pointer that is returned to the
    user.  "Nextchunk" is the beginning of the next contiguous chunk.

    Chunks always begin on even word boundaries, so the mem portion
    (which is returned to the user) is also on an even word boundary, and
    thus at least double-word aligned.

    Free chunks are stored in circular doubly-linked lists, and look like this:

    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |             Size of previous chunk, if unallocated (P clear)  |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `head:' |             Size of chunk, in bytes                     |A|0|P|
      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |             Forward pointer to next chunk in list             |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |             Back pointer to previous chunk in list            |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |             Unused space (may be 0 bytes long)                .
        .                                                               .
        .                                                               |
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `foot:' |             Size of chunk, in bytes                           |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |             Size of next chunk, in bytes                |A|0|0|
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    The P (PREV_INUSE) bit, stored in the unused low-order bit of the
    chunk size (which is always a multiple of two words), is an in-use
    bit for the *previous* chunk.  If that bit is *clear*, then the
    word before the current chunk size contains the previous chunk
    size, and can be used to find the front of the previous chunk.
    The very first chunk allocated always has this bit set,
    preventing access to non-existent (or non-owned) memory. If
    prev_inuse is set for any given chunk, then you CANNOT determine
    the size of the previous chunk, and might even get a memory
    addressing fault when trying to do so.

    The A (NON_MAIN_ARENA) bit is cleared for chunks on the initial,
    main arena, described by the main_arena variable.  When additional
    threads are spawned, each thread receives its own arena (up to a
    configurable limit, after which arenas are reused for multiple
    threads), and the chunks in these arenas have the A bit set.  To
    find the arena for a chunk on such a non-main arena, heap_for_ptr
    performs a bit mask operation and indirection through the ar_ptr
    member of the per-heap header heap_info (see arena.c).

    Note that the `foot' of the current chunk is actually represented
    as the prev_size of the NEXT chunk. This makes it easier to
    deal with alignments etc but can be very confusing when trying
    to extend or adapt this code.

    The three exceptions to all this are:

     1. The special chunk `top' doesn't bother using the
    trailing size field since there is no next contiguous chunk
    that would have to index off it. After initialization, `top'
    is forced to always exist.  If it would become less than
    MINSIZE bytes long, it is replenished.

     2. Chunks allocated via mmap, which have the second-lowest-order
    bit M (IS_MMAPPED) set in their size fields.  Because they are
    allocated one-by-one, each must contain its own trailing size
    field.  If the M bit is set, the other bits are ignored
    (because mmapped chunks are neither in an arena, nor adjacent
    to a freed chunk).  The M bit is also used for chunks which
    originally came from a dumped heap via malloc_set_state in
    hooks.c.

     3. Chunks in fastbins are treated as allocated chunks from the
    point of view of the chunk allocator.  They are consolidated
    with their neighbors only in bulk, in malloc_consolidate.
*/
```

```c
void *__libc_malloc(size_t bytes) {
    mstate ar_ptr;
    void * victim;

    void *(*hook)(size_t, const void *) = atomic_forced_read(__malloc_hook);
    if (__builtin_expect(hook != NULL, 0))
        return (*hook)(bytes, RETURN_ADDRESS(0));

    arena_get(ar_ptr, bytes);

    victim = _int_malloc(ar_ptr, bytes);
    /* Retry with another arena only if we were able to find a usable arena
       before.  */
    if (!victim && ar_ptr != NULL) {
        LIBC_PROBE(memory_malloc_retry, 1, bytes);
        ar_ptr = arena_get_retry(ar_ptr, bytes);
        victim = _int_malloc(ar_ptr, bytes);
    }

    if (ar_ptr != NULL) __libc_lock_unlock(ar_ptr->mutex);

    assert(!victim || chunk_is_mmapped(mem2chunk(victim)) ||
           ar_ptr == arena_for_chunk(mem2chunk(victim)));
    return victim;
}

libc_hidden_def(__libc_malloc)

    void __libc_free(void *mem) {
    mstate    ar_ptr;
    mchunkptr p; /* chunk corresponding to mem */

    void (*hook)(void *, const void *) = atomic_forced_read(__free_hook);
    if (__builtin_expect(hook != NULL, 0)) {
        (*hook)(mem, RETURN_ADDRESS(0));
        return;
    }

    if (mem == 0) /* free(0) has no effect */
        return;

    p = mem2chunk(mem);

    if (chunk_is_mmapped(p)) /* release mmapped memory. */
    {
        /* See if the dynamic brk/mmap threshold needs adjusting.
       Dumped fake mmapped chunks do not affect the threshold.  */
        if (!mp_.no_dyn_threshold && chunksize_nomask(p) > mp_.mmap_threshold &&
            chunksize_nomask(p) <= DEFAULT_MMAP_THRESHOLD_MAX &&
            !DUMPED_MAIN_ARENA_CHUNK(p)) {
            mp_.mmap_threshold = chunksize(p);
            mp_.trim_threshold = 2 * mp_.mmap_threshold;
            LIBC_PROBE(memory_mallopt_free_dyn_thresholds, 2,
                       mp_.mmap_threshold, mp_.trim_threshold);
        }
        munmap_chunk(p);
        return;
    }

    ar_ptr = arena_for_chunk(p);
    _int_free(ar_ptr, p, 0);
}
libc_hidden_def(__libc_free)
```


