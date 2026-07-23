# AGENTS.md

C multiprocess/multithread simulation (42 curriculum). A `codexion` binary
models N "coders" competing for shared USB dongles while compiling/debugging/
refactoring, with FIFO or EDF scheduling.

## Build & run
- `make` — builds `./codexion` from the 8 `.c` files in `coders/` (`cc -pthread`,
  `-Wall -Wextra -Werror`). There is no separate test/lint target.
- Run requires exactly 8 args, in this order:
  `./codexion <cdrs> <brn> <tc> <td> <tr> <rqc> <cldwn> <sch>`
  - first 7 are non-negative integers; last is `fifo` or `edf` (else "Wrong Format").
  - `cdrs` (coders, index 0) and `rqc` (compiles required, index 5) must be > 0,
    otherwise the parser returns NULL and the program exits with format error.
- `make run` runs a sample: `./codexion 5 800 200 200 200 4 20 fifo`.
- `make re` / `make fclean` for rebuild / clean. `codexion` is gitignored.

## Architecture (non-obvious)
- All code lives in `coders/`; `coders/coders.h` is the single shared header
  defining `t_sim`, `t_coder`, `t_dongle` and the index constants
  (`NBR_CDRS=0, TT_BRNT=1, TT_CMPL=2, TT_DEBG=3, TT_RFCT=4, REQ_CMP=5,
  CLDOWN=6, SCH=7`) used to read `sim->data`.
- `coders/main.c` is the entrypoint: `parser` -> `init_codex` -> `monitor_routine`
  -> `cleanup`. The monitor thread spawns/watches coder threads.
- Dongle model: each dongle has exactly **two** possible claimants (left/right
  neighbour coder), so its wait queue is a fixed `queue[2]` sorted by priority —
  no heap/priority queue. Priority key: FIFO = enqueue time, EDF = burnout
  deadline (`last_compile + TT_BRNT`); **smaller = higher priority** in both.
  See `coders/dongle.c` header for full contract.
- A coder needs **2 dongles** (left + right) per compile cycle; watch for the
  deadlock/starvation dynamics the `note` file describes.

## Conventions
- 42-style: every file begins with the `/* 42 header */` comment block and
  follows Norm (K&R-ish brace style, short functions/lines) as enforced by
  `-Wall -Wextra -Werror`. Preserve the header and style when editing.
- `coders/my_dongle.c` exists but is empty (0 bytes) — not part of the build
  (`SRCS` in Makefile omits it). Do not assume it compiles.
- `viz.py` and `en.subject.pdf` are analysis/subject artifacts, not build inputs.
