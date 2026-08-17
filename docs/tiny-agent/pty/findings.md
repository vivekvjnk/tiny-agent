# PTY Harness — Week 1 Findings

## How to build and run

Requires: `gcc`, `make`, `libutil` (ships with glibc — available on any standard Linux).

```bash
cd tiny-agent
make              # compiles to build/pty_test
./build/pty_test  # runs the harness against /bin/sh
make clean        # rm -rf build
```

Expected output:

```
[cmd] echo 'PTY harness alive'
[out] PTY harness alive
$
[cmd] ls /
[out] bin  boot  dev  etc  home  ...
$
[cmd] uname -a
[out] Linux <hostname> ...
$
[cmd] echo exit_code=$?
[out] exit_code=0
$
[cmd] exit
[out]
[done]
```

The trailing `$ ` after each output block is the shell prompt bleeding through — expected, see "What bleeds through" below.

## What was tested

`tiny-agent/src/pty.c` — minimal `forkpty()` + read/write loop.
Ran 5 commands: echo, ls, uname, exit_code check, exit.

## What works

- `forkpty()` + `execl("/bin/sh")` — spawns shell cleanly
- Write command → read output — works end-to-end
- Multi-line output (ls /) — select() + read() accumulates correctly
- Exit handling — SIGCHLD + waitpid() cleans up without hang
- First-line strip (echoed command) — `strchr(buf, '\n') + 1` adequate for now

## Synchronization primitive (key finding)

**Quiet-period timeout works.** 120ms of silence on master fd = command complete.
This is portable and does not require prompt string parsing.

Tradeoff:
- Slow commands (find, make) will never look "done" until they finish naturally — fine.
- Interactive programs (vim, python REPL) will timeout and send more input — problematic, deferred.
- 120ms adds latency per command. For agent use, acceptable. Can tune.

## What bleeds through

The shell prompt (`$ `) appears at the end of each command's output block.
Current first-line strip only removes the echoed command; the trailing `$ ` prompt remains.

Two options for Phase 2:
1. **Parse and strip the prompt string.** Fragile — varies by shell and user config.
   Setting `PS1="$ "` in child environment makes it predictable.
2. **Leave the prompt in output.** The LLM reads it as part of the shell state.
   Could be useful signal: "prompt appeared = shell is idle."

Decision deferred. Option 2 is simpler and gives the LLM more context.

## What is NOT answered yet

- Interactive programs (vim, less, top): will corrupt the read loop. Need separate handling path.
- Binary output / ANSI escape codes: not stripped yet. `ls --color` would pollute output.
- stderr: currently merged via PTY. Explicit separation requires separate fd.
- Exit code retrieval: `echo exit_code=$?` works but is a hack. Real approach: parse `$?` after each command.
- Command cancellation: no SIGINT path yet.
- PTY window size (SIGWINCH): not handled. Affects programs that query terminal size.

## Next questions for Phase 2 (LLM integration)

1. Should the agent strip ANSI codes before sending to LLM? Yes.
2. Should the LLM see the prompt string? Probably yes — useful context.
3. How does the agent detect "shell is stuck waiting for input"? Timeout + heuristic.
4. What's the max output size before truncation? Need a policy.
