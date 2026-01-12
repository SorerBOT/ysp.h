# ysp.h
WORK IN PROGRESS! :D
## How does it work?
Our profiler is a sampling profiler, which means that in order to learn how much time each function spent running, and which function called it, the profiler periodically checks, or "samples", what is the currently running function, and what other function called it, and which function called IT, recursively. Now imagine we have this amazingly completed calculation to perform:
```c
#include <stdio.h>

int heavy_calculation(int x);
int heavy_calculation_2(int x);

int main()
{
    int x = 15;
    int result = heavy_calculation(x);

    printf("%d ** %d = %d\n", x, x, result);
}

int heavy_calculation(int x)
{
    // calculate x**x
    int result = 0;
    for (int i = 0; i < x; ++i)
    {
        result += heavy_calculation_2(x);
    }
    return result;
}

int heavy_calculation_2(int x)
{
    int result = 0;
    for (int i = 0; i < x; ++i)
    {
        ++result;
    }
    return result;
}
```
If our profiler, ends up sampling while heavy_calculation_2 is being executed, then we would want the sample to resemble the following:  
`main` -> `heavy_calculation` -> `heavy_calculation_2`  
But how can we achieve that? STACK WALKING >>>>> ASSEMBLY <<<, COMPUTER ARCHITECTURE, REGISTERS!!111.  
### Stack Walking
On a serious note, we do have to recall some assembler for this one, but nothing that even the faintest of hearts wouldn't manage. Firstly, let us recall how a typical function definition looks like in assembly:
```asm
section .text
global add_numbers

add_numbers:
    ; --- Prologue ---
    push    rbp             ; Save caller's base pointer (the highest stack-address he possesses)
    mov     rbp, rsp        ; Set base pointer to current stack (the highest stack-address the CURRENT function possesses)

    ; --- Body ---
    sub     rsp, 4          ; ALLOCATE 4 bytes on the stack for local variable. RSP is now [RBP - 4]
    mov     [rbp-4], edi    ; Store first arg (EDI) into our local stack variable
    add     [rbp-4], esi    ; Add second arg (ESI) directly to that stack memory
    mov     eax, [rbp-4]    ; Move the result from stack into EAX for return

    ; --- Epilogue ---
    mov     rsp, rbp        ; Restore RSP (effectively freeing the 4 bytes)
    pop     rbp             ; Restore caller's RBP (so that when the calling function takes control, it knows the highest stack-address IT possesses)
    ret
```
Now the interesting parts here are the prologue its sibling, the epilogue. Their role is quite straightforward once we recall what the `stack` is, conveniently we have the following diagram I borrowed from gemini:
```
High Address (e.g., 0x...FFF0)
                        |
      +-------------------------------------+
      | ... Caller's Stack Data ...         |
      +=====================================+ <-- Stack boundary before 'call'
      |                                     |
      |        Return Address (RIP)         | <-- Pushed automatically by the
      |                                     |     'call' instruction.
      | Address: [rbp + 8]                  |
      +-------------------------------------+
RBP-> |                                     | <-- The 'mov rbp, rsp' makes RBP
      |        Saved Caller's RBP           |     point right here.
      |                                     |
      | Address: [rbp]                      | <-- Pushed explicitly by the first
      +=====================================+     line: 'push rbp'
      |                                     |
      |  Local Variable space (4 bytes)     | <-- Space created by 'sub rsp, 4'.
      |  (Holds value of EDI/ESI)           |
      |                                     |
RSP-> | Address: [rbp - 4]                  | <-- RSP points here during the body.
      +-------------------------------------+
                        |
                  Low Address (e.g., 0x...FFE4)
```
What we should learn from this diagram is that when a function is being called, it first pushes a `Return Address` which is simply the next instruction which needs to be executed after the function invocation is complete. It was previously stored in the `RIP` register, but we now need it for the current function, so storing it is the only option. We are primarily interested in this value, because if we have the address of an instruction inside a function we can usually lookup the function name using a function like `dladdr`. Additionally, the `RBP` register's role is to keep track of the highest stack-address, meaning the FIRST stack-address owned by the current function, the reason why we use ```asm push rbp``` as the first instruction of the function, is that we are on the brink of starting a new function, and the new function wants to know its limits, particularly what is the highest stack-address it is allowed to interact with and for that it needs to free up the `RBP` register and so as not to lose the previous, or calling function's `RBP`, we simply push it onto the stack. Lastly, the `RSP` register, serves as complementary to the `RBP` pointer, limiting the stack from the bottom, by indicating which is the lowest stack address accessible from the current function.  

Now that we got the rust off our assembly game, we can describe our sampling algorithm, called stack walking:
1. **Start**: get the current value of `RIP` which is just a register.
2. **Symbolize**: use `ldaddr` to get the current function's name.
3. **Stack Walk**:
    * Read `[RBP+8]` to get the previous `RIP`
    * Symbolize: use `dladdr` to get the calling function name.
    * Use `[RBP]` to get the calling function's `RBP`
4. Repeat until the calling function's `RBP` / `RIP` point to nonesense.

An important note is that we are not actually going to be using `dladdr` throughout the runtime of the program, but we are going to store all the addresses, and parse them using `dladdr` after the entire program finished executing, or something along those lines.

Another important note is that we would have to compile with the `-fno-omit-frame-pointer` flag, as we would not necessarily have `RBP` updated with the bottom of the stack (highest address) available to us otherwise.

### Profiling
Now that we are familiar with the idea of stack walking, the profiling process will simply consist of many-many different samplings. We can look at the following example, let us consider 7 samplings, at 7 different moments throughout our initial program's execution:
| Timestamp | Call stack |
| :--- | :--- |
| 0.1ms | Nil |
| 0.2ms | main |
| 0.3ms | main -> heavy_calculation |
| 0.4ms | main -> heavy_calculation -> heavy_calculation_2|
| 0.5ms | main -> heavy_calculation |
| 0.6ms | main |
| 0.7ms | Nil |

Looking at these samplings, we can calculate how long it took for each function to complete, we do so by subtracting the first timestamp, at which the function appears from the first timestamp where the function no longer appears. For example, we can calculate main's runtime using this formula:
$\text{main runtime} = 0.7 - 0.2 = 0.5$.  
