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
On a serious note, we do have to recall some assembler for this one, but nothing that even the faintest of hearts wouldn't manage. Firstly, let us recall how a typical function definition looks like in assembly:
```asm
section .text
global add_numbers

add_numbers:
    ; --- Prologue ---
    push    rbp             ; Save the caller's base pointer
    mov     rbp, rsp        ; Set the base pointer to the current stack pointer

    ; --- Body ---
    mov     rax, rdi        ; Move first argument (RDI) into the accumulator (RAX)
    add     rax, rsi        ; Add second argument (RSI) to the accumulator (RAX)
                            ; Result is now in RAX (Destination is FIRST operand)

    ; --- Epilogue ---
    mov     rsp, rbp        ; Restore the stack pointer (deallocate locals if any)
    pop     rbp             ; Restore the caller's base pointer
    ret                     ; Return control to caller
```
Now the interesting parts here are the prologue its sibling, the epilogue. Their role is quite straightforward once we recall what the `stack` is, conveniently we have the following diagram I borrowed from gemini:
```
      +------------------+  <-- High Address
      |  ...             |
      |  Caller's Stack  |
      +------------------+
      |  Return Address  |  <-- [rbp + 8] (Pushed by 'call')
      +------------------+
RBP-> |   Saved RBP      |  <-- [rbp]     (Pushed by 'push rbp')
      +------------------+
      |  Local Variable  |  <-- [rbp - 4]
      |      ...         |
      +------------------+
RSP-> |   Top of Stack   |  <-- Current Stack Pointer
      +------------------+  <-- Low Address
```
