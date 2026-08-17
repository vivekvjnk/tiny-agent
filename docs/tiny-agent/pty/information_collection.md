# What is a shell?

## Linux Terminal Execution Model

```text
                USER SPACE
┌───────────────────────────────────────┐
│                                       │
│  User                                │
│    ↓                                  │
│  Terminal (TTY / PTY)                │
│    ↓                                  │
│  Shell (sh/bash/ash)                 │
│    ↓                                  │
│  Linux Programs (ls, ps, grep, etc.) │
│                                       │
└───────────────────────────────────────┘
                │
                │ System Calls
                ▼
┌───────────────────────────────────────┐
│             LINUX KERNEL             │
│                                       │
│  - Process Management                 │
│  - Memory Management                  │
│  - Filesystems                        │
│  - Device Drivers                     │
│  - TTY / PTY Subsystem                │
│                                       │
└───────────────────────────────────────┘
```

---

## Shell Execution Flow

```text
User Input
    ↓
Shell reads command
    ↓
Shell invokes:
    fork()
    execve()
    ↓
Kernel starts program
    ↓
Program stdout/stderr
    ↓
Terminal displays output
```

---

## Standard File Descriptors

| FD | Name   | Purpose       |
| -- | ------ | ------------- |
| 0  | stdin  | input stream  |
| 1  | stdout | output stream |
| 2  | stderr | error stream  |

These file descriptors are typically connected to a terminal.

---

## TTY vs PTY

### TTY (Real Terminal)

Represents hardware-backed terminal interfaces.

Examples:

```text
/dev/tty1
/dev/ttyS0
```

Used for:

* physical consoles
* serial terminals

---

### PTY (Pseudo Terminal)

Software-emulated terminal interface.

Implemented as:

```text
PTY Master  ↔  PTY Slave
```

Example slave device:

```text
/dev/pts/0
```

---

## PTY Communication Model

```text
Program / Controller
        ↓
    PTY Master
        ↕
    PTY Slave
        ↓
      Shell
        ↓
 Linux Programs
```

The PTY slave behaves like a real terminal device, allowing shells and programs to operate normally while being controlled programmatically through the PTY master.


# How does shell channel stdout/in/err to the terminal?
The shell manages standard streams by assigning three reserved file descriptors (FDs)—which are essentially unique integer IDs—to every process it starts. By default, these descriptors point to your terminal, but the shell can "rewire" them to files or other processes through redirection and piping. [1, 2, 3, 4] 
## The Standard File Descriptors

| FD [5, 6, 7, 8, 9] | Name | Default Source/Sink | Description |
|---|---|---|---|
| 0 | stdin | Keyboard | Standard input: where the program reads data. |
| 1 | stdout | Screen | Standard output: where the program sends regular data. |
| 2 | stderr | Screen | Standard error: where the program sends error messages. |

## How the Shell Manipulates FDs
When you run a command, the shell uses specific operators to change where these integers point before the program starts. [2, 10] 

* Redirection (>, <, 2>): The shell replaces the default terminal target of a descriptor with a specific file.
* command > file: Redirects stdout (1) to a file.
   * command 2> error.log: Redirects stderr (2) to a file.
   * command < input.txt: Redirects stdin (0) to read from a file.
* FD Duplication (>&, <&): This tells the shell to make one descriptor point to the same location as another.
* 2>&1: A common pattern that redirects stderr (2) to wherever stdout (1) is currently going.
* Piping (|): The shell connects the stdout (1) of one process directly to the stdin (0) of another, creating a "pipe" between them. [6, 7, 9, 10, 11, 12, 13, 14, 15] 

## Under the Hood: The Process

   1. Forking: When you type a command, the shell creates a copy of itself (a child process).
   2. Redirection: Before the new command actually runs, the child process opens the files you specified and uses system calls (like dup2) to assign them to FDs 0, 1, or 2.
   3. Execution: The child process "transforms" into the intended program. Because it inherits the modified FDs from the shell, it automatically writes to your file instead of the screen without ever knowing the difference. [4, 10, 16, 17, 18] 


[1] [https://stackoverflow.com](https://stackoverflow.com/questions/7082001/how-do-file-descriptors-work)
[2] [https://www.youtube.com](https://www.youtube.com/watch?v=WpmQnOmdE8s)
[3] [https://www.youtube.com](https://www.youtube.com/watch?v=epENw4XxcFg)
[4] [https://www.reddit.com](https://www.reddit.com/r/linux/comments/152hkal/what_is_the_reality_of_stdout_stdin_and_stderr/)
[5] [https://unix.stackexchange.com](https://unix.stackexchange.com/questions/757194/when-and-where-are-the-standard-in-out-and-error-file-descriptors-first-opened)
[6] [https://javascript.plainenglish.io](https://javascript.plainenglish.io/mastering-linux-standard-i-o-streams-stdin-stdout-and-stderr-explained-with-examples-640c9924800e)
[7] [https://www.ibm.com](https://www.ibm.com/docs/en/aix/7.1.0?topic=redirection-standard-input-standard-output-standard-error-files)
[8] [https://www.youtube.com](https://www.youtube.com/watch?v=icuV2CR3Ghg)
[9] [https://www.redhat.com](https://www.redhat.com/en/blog/linux-shell-redirection-pipelining)
[10] [https://medium.com](https://medium.com/@pash4stud2/understanding-bash-redirection-with-file-descriptors-0ea510dc411f)
[11] [https://thoughtbot.com](https://thoughtbot.com/blog/input-output-redirection-in-the-shell)
[12] [https://hwchiu.medium.com](https://hwchiu.medium.com/differences-between-file-2-1-and-2-1-file-in-bash-redirection-d772660f12e2)
[13] [https://learning.lpi.org](https://learning.lpi.org/en/learning-materials/101-500/103/103.4/103.4_01/)
[14] [https://www.linuxjournal.com](https://www.linuxjournal.com/content/working-stdin-and-stdout)
[15] [https://stackoverflow.com](https://stackoverflow.com/questions/10508843/what-is-dev-null-21)
[16] [https://stackoverflow.com](https://stackoverflow.com/questions/5256599/what-are-file-descriptors-explained-in-simple-terms)
[17] [https://www.youtube.com](https://www.youtube.com/watch?v=FuiLk7uH9Jw&t=7)
[18] [https://www.youtube.com](https://www.youtube.com/watch?v=9FuWfNdOnsY)



## 1. How often is the stdin file descriptor read?
In standard shell operations, there is no fixed "sample time" for file updates or reading from a file descriptor. The process is event-driven and demand-based, rather than periodic. [1, 2, 3] 
The shell (or any program) reads from stdin (FD 0) only when a command explicitly asks for it. It is not an automated "background" scan. [4, 5, 6] 

* On Demand: A program calls a read() system call when it needs data. If no data is available (e.g., you haven't typed anything yet), the program typically "blocks" or pauses until you press a key or send data through a pipe.
* Immediate Response: Once data is provided, the operating system kernel notifies the process immediately. There is no waiting for a next "cycle" or clock tick. [1, 2, 3, 6, 7, 8] 

## 2. Is there a "sample time" for file updates?
File updates are handled by the operating system's kernel, and the shell receives these updates based on specific triggers:

* Buffer Flushing: For performance, most shells and programs use a buffer. Data might not appear in a file or on the screen instantly; it usually waits until the buffer is full or the program manually calls fflush().
* Notification Mechanisms: If you are using a tool that seems to sample files (like tail -f or a file watcher), it isn't actually "sampling" at a fixed rate. Modern Linux systems use inotify, an event-based system where the kernel sends a "wake up" signal to the shell the exact millisecond a file is modified. [3, 9, 10] 

## Summary of FD Behavior

| Action [1, 4, 9, 11] | Frequency/Timing |
|---|---|
| Reading (stdin) | Occurs only when the program executes a read command. |
| Writing (stdout/err) | Happens whenever the program generates output, though often delayed by buffering. |
| Update Latency | Near-zero; the kernel passes data between processes as fast as the CPU allows. |

Are you trying to monitor a log file in real-time or optimize a script's input processing speed?

[1] [https://medium.com](https://medium.com/geekculture/developer-diaries-processes-files-and-file-descriptors-in-linux-ebf007fb78f8)
[2] [https://www.lenovo.com](https://www.lenovo.com/gb/en/glossary/stdin/)
[3] [https://av.tib.eu](https://av.tib.eu/media/13946)
[4] [https://medium.com](https://medium.com/@chaitanyalakhchaura12/things-i-learned-about-stdin-and-stdout-in-linux-39d004c1385e)
[5] [https://learn.microsoft.com](https://learn.microsoft.com/en-us/cpp/c-runtime-library/stdin-stdout-stderr?view=msvc-170)
[6] [https://superuser.com](https://superuser.com/questions/1882461/how-does-this-standard-input-operator-0-work-in-linux)
[7] [https://en.wikipedia.org](https://en.wikipedia.org/wiki/Standard_streams)
[8] [https://www.youtube.com](https://www.youtube.com/watch?v=94n6ihmN3eQ&t=6)
[9] [https://www.ibm.com](https://www.ibm.com/docs/ssw_aix_71/com.ibm.aix.genprogc/using_file_descriptors.htm)
[10] [https://www.ibm.com](https://www.ibm.com/docs/en/i/7.4.0?topic=functions-gets-read-line)
[11] [https://stackoverflow.com](https://stackoverflow.com/questions/79635545/file-descriptor-creation-in-linux-takes-considerable-time-to-execute-at-256-512)


## FD Management

| Feature | stdin (FD 0) | stdout (FD 1) / stderr (FD 2) |
| --- | --- | --- |
| **Source/Sink** | Keyboard, File, or Pipe | Screen, File, or Pipe |
| **End of Stream** | `read()` returns 0 bytes | Closing the FD (close write-end) |
| **Data Removal** | Kernel removes data once read | Kernel moves data to the next destination |
| **Bash's Role** | Uses `dup2()` to set the source | Uses `dup2()` to set the destination |

