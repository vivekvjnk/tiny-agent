# Tiny Agent --- Senior Embedded Systems Exploration Protocol

## 1. Purpose

This document defines the stable format for a continuing daily
problem-solving process under the **Tiny Agent** project.

The goal is not to complete a fixed number of LeetCode problems each
day. The goal is to spend **one focused hour per day** developing
senior-level systems intuition through practical,
implementation-oriented problems.

The problems should progressively connect:

**CPU architecture → memory → Linux → concurrency → IPC → networking →
runtime design → tiny-agent architecture**

Tiny Agent is the recurring laboratory. When a concept naturally maps to
the tiny-agent runtime, the problem should use that context.

The existing URP model provides the architectural backdrop: a minimal,
stateful, mailbox-driven, asynchronous agent primitive with a scheduler
external to the primitive. Its C reference model includes a ring-buffer
mailbox and scheduler/event loop.

## 2. High-Level Objective

Develop the ability expected from a **senior embedded software engineer
interviewing at FAANG-level companies**:

-   reason from software behavior down to hardware;
-   understand CPU, memory, OS, and concurrency mechanisms rather than
    merely using APIs;
-   identify bottlenecks and measure them;
-   design data structures and execution models around hardware
    realities;
-   understand when parallelism helps and when it does not;
-   write efficient, robust C/C++ and systems code;
-   explain design trade-offs clearly;
-   connect low-level mechanisms to real runtime architecture.

The emphasis is **deep intuition over problem-count**.

## 3. Nature of the Problems

Problems are intentionally more substantial than conventional LeetCode
exercises.

A problem may take several days. Completion within one session is
**not** required.

A problem can ask the engineer to:

-   implement an algorithm;
-   inspect generated assembly;
-   benchmark competing implementations;
-   profile a workload;
-   investigate Linux behavior;
-   design a data structure;
-   reason about cache/memory behavior;
-   implement multithreading or synchronization;
-   use SIMD or CUDA;
-   investigate an OS primitive;
-   modify a tiny-agent component;
-   explain observed performance;
-   derive a scalability limit;
-   compare architectural alternatives.

The problem should create a reason to explore the underlying system
rather than simply test API knowledge.

## 4. The One-Hour Rule

Each session has one hard constraint:

> **Spend approximately one focused hour on the current exploration.**

The one-hour boundary is a stopping point, not a completion deadline.

At the end of an hour, record:

1.  What was understood?
2.  What was implemented?
3.  What was measured?
4.  What surprised us?
5.  What remains unexplained?
6.  What should be investigated next?

If the problem is unfinished, continue it in the next session.

Do **not** replace a deep problem merely because it spans multiple days.

## 5. Stable Question Format

Every new exploration should use the following structure.

### Problem N --- `<short descriptive title>`

#### Context

A concise engineering scenario.

Preferably connect it to Tiny Agent, URP, the scheduler, mailbox,
workspace, transport, or another realistic systems component.

#### High-Level Objective

State the engineering capability being developed.

Example:

> Understand how memory layout and SIMD execution affect the performance
> of a message-processing runtime.

#### Problem

Give the concrete engineering task.

The problem should contain enough information to begin implementation
without requiring a pre-existing solution.

#### Constraints

Specify relevant constraints such as:

-   language;
-   platform;
-   input size;
-   latency/throughput requirements;
-   memory limits;
-   threading model;
-   hardware assumptions.

Do not over-constrain the solution when exploration is part of the
objective.

#### Exploration Goals

List the concepts the problem is intended to expose.

Examples:

-   cache locality;
-   SIMD;
-   branch prediction;
-   atomics;
-   false sharing;
-   virtual memory;
-   system calls;
-   epoll;
-   IPC;
-   CUDA memory hierarchy.

#### Required Work

Separate the work into concrete stages.

Typical progression:

1.  Build a naïve/reference implementation.
2.  Make it correct.
3.  Measure it.
4.  Identify the bottleneck.
5.  Implement an improved version.
6.  Measure again.
7.  Explain the difference.
8.  Investigate the underlying mechanism.

#### Questions to Answer

Include the conceptual questions that should be answered during
exploration.

These should test **why**, not merely **what API**.

#### Senior-Level Extension

Add one harder architectural question.

Examples:

-   redesign the data structure;
-   remove a scalability bottleneck;
-   reason about multicore behavior;
-   integrate the solution into Tiny Agent;
-   determine when an optimization becomes counterproductive.

#### Expected Output

Define what should exist at the end of the exploration.

For example:

-   working C implementation;
-   benchmark;
-   assembly inspection;
-   performance comparison;
-   short engineering explanation;
-   architectural recommendation.

The output is evidence of understanding, not the primary objective.

## 6. Difficulty and Progression

Problems should gradually move through layers of the system.

### Layer 1 --- Machine

-   C/C++ memory model
-   pointers
-   alignment
-   SIMD
-   assembly
-   cache hierarchy
-   branch prediction
-   atomics
-   lock-free structures

### Layer 2 --- OS

-   processes and threads
-   virtual memory
-   mmap
-   system calls
-   file descriptors
-   signals
-   scheduling
-   epoll
-   pipes
-   shared memory
-   IPC

### Layer 3 --- Systems

-   producer/consumer systems
-   thread pools
-   event loops
-   work queues
-   backpressure
-   contention
-   zero-copy design
-   networking
-   observability
-   performance engineering

### Layer 4 --- Tiny Agent

-   mailbox
-   scheduler
-   event loop
-   agent lifecycle
-   message transport
-   state persistence
-   capability dispatch
-   workspace interaction
-   failure handling
-   multi-agent execution

### Layer 5 --- Architecture

Eventually combine multiple layers in a single problem.

Example:

> Design a Linux-based scheduler for thousands of tiny-agent mailboxes
> and determine whether threads, processes, or an event-driven
> architecture is appropriate.

## 7. Tiny Agent Relevance Rule

Tiny Agent should be used whenever it provides a meaningful engineering
context.

Do **not** force a Tiny Agent connection where it becomes artificial.

If a problem is better explored independently, explicitly provide **2--3
possible Tiny Agent exploration directions** that could be pursued
afterward.

For example:

> This CUDA problem is primarily about GPU architecture. Possible Tiny
> Agent connections:
>
> 1.  GPU-assisted telemetry aggregation.
> 2.  Parallel workspace indexing.
> 3.  Batch processing of agent events.

The purpose is to maintain a strong systems foundation rather than
turning every exercise into a Tiny Agent feature.

## 8. Problem Selection Principles

Prefer problems that have:

-   measurable performance;
-   observable system behavior;
-   multiple reasonable implementations;
-   non-obvious trade-offs;
-   opportunities to inspect what the machine actually does;
-   realistic embedded/systems relevance;
-   a path from simple implementation to advanced optimization.

Avoid problems whose primary difficulty is:

-   obscure syntax;
-   API memorization;
-   mathematical trickery without systems relevance;
-   arbitrary LeetCode puzzle patterns.

Classic algorithmic problems are still useful when they expose an
important systems concept.

## 9. Session Continuity

A new Tiny Agent chat should be able to continue this process without
reconstructing the methodology.

At the beginning of a new chat, the assistant should:

1.  read this protocol;
2.  determine the current exploration/problem;
3.  continue from the user's last recorded state;
4.  avoid unnecessarily restarting completed work;
5.  provide the next one-hour challenge when the previous exploration is
    complete.

The active problem should therefore be treated as a **small ongoing
engineering investigation**, not a disposable daily question.

## 10. Session State

When ending a session, maintain a compact state record:

``` text
Problem:
Status:
Implemented:
Measured:
Key discovery:
Unresolved:
Next step:
```

This record is sufficient to resume the exploration in another chat.

## 11. Day 1 Starting Point

The first exploration deliberately begins at the lowest level.

### Exploration A --- x86 SIMD

**Theme:** SIMD and data-oriented message processing.

Use a Tiny Agent mailbox/message descriptor workload to explore:

-   scalar vs AVX2;
-   AoS vs SoA;
-   vector width;
-   alignment;
-   compiler auto-vectorization;
-   generated assembly;
-   cache behavior;
-   preserving mailbox semantics.

### Exploration B --- CUDA

**Theme:** Massive parallel event aggregation.

Use a large Tiny Agent event stream to explore:

-   GPU thread decomposition;
-   global memory;
-   coalescing;
-   atomic contention;
-   shared-memory aggregation;
-   reduction;
-   host/device transfer;
-   workload-size crossover.

These are deliberately substantial explorations and may span multiple
sessions.

------------------------------------------------------------------------

## 12. Core Principle

> **Do not optimize for finishing problems. Optimize for understanding
> the machine well enough to design better systems.**

The daily one-hour session is simply the mechanism that makes that
exploration sustainable.
