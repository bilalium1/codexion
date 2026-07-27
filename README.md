# CODEXION - 42

> This project has been created as part of 42 curriculum by blemrabe

![cat](https://media1.tenor.com/m/qMH5o_XizbcAAAAd/but-here%27s-the-coder.gif)

this is what the coders look like btw ^^

### Description

Codexion simulates multiple coders sitting in a circular co-working hub, competing for shared USB dongles to compile their code. Each coder cycles through three phases: compile, debug, and refactor. Compiling requires holding two dongles simultaneously — the one on their left and the one on their right in the ring.

**How the ring works**: coders and dongles alternate in a circle. Coder `i` sits between dongle `i` (left) and dongle `(i+1) % n` (right). Each dongle is shared by exactly two neighbouring coders. This means at most half the coders can compile at the same time — the others wait.

**How dongles decide who gets them**: each dongle maintains a queue of at most two waiting coders (its two neighbours). When a dongle is released, it grants access to the next coder in the queue based on the scheduling policy — either FIFO (first to arrive) or EDF (earliest burnout deadline first). After being released, a dongle enters a cooldown period before it can be claimed again.

**Deadlock prevention**: coders acquire their two dongles in a fixed order — even-indexed coders take left then right, odd-indexed coders take right then left. This breaks the circular wait that would otherwise cause deadlock.

**Burnout**: if a coder does not start compiling within `time_to_burnout` milliseconds since their last compile, they burn out and the simulation stops. A monitor thread checks this continuously.

**End condition**: the simulation stops when either a coder burns out, or every coder has compiled at least `number_of_compiles_required` times.

### Instructions

- Build the project from the root directory:
```
make
```

- Run the simulation with the required arguments:
```
./codexion <cdrs> <brn> <tc> <td> <tr> <rqc> <cldwn> <sch>
```

| Argument | Description |
|---|---|
| `cdrs` | Number of coders (and dongles) |
| `brn` | Time to burnout in ms |
| `tc` | Time to compile in ms |
| `td` | Time to debug in ms |
| `tr` | Time to refactor in ms |
| `rqc` | Number of compiles required per coder |
| `cldwn` | Dongle cooldown time in ms |
| `sch` | Scheduler: `fifo` or `edf` |

- Example:
```
./codexion 5 800 200 200 200 4 20 fifo
```

### Resources


- [Pthreads — POSIX threads explained (Linux man page)](https://man7.org/linux/man-pages/man7/pthreads.7.html)
- [pthread_create(3) — Linux man page](https://man7.org/linux/man-pages/man3/pthread_create.3.html)
- [cppreference — std::thread (concepts apply to pthreads)](https://en.cppreference.com/w/cpp/thread/thread)

Thank you for reading!

![sonic](.sonic.gif)

Made by b//
