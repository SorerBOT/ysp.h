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
    push    rbp             ; Save caller's base pointer
    mov     rbp, rsp        ; Set base pointer to current stack

    ; --- Body ---
    sub     rsp, 4          ; ALLOCATE 4 bytes on the stack for local variable. RSP is now [RBP - 4]
    mov     [rbp-4], edi    ; Store first arg (EDI) into our local stack variable
    add     [rbp-4], esi    ; Add second arg (ESI) directly to that stack memory
    mov     eax, [rbp-4]    ; Move the result from stack into EAX for return

    ; --- Epilogue ---
    mov     rsp, rbp        ; Restore RSP (effectively freeing the 4 bytes)
    pop     rbp             ; Restore caller's RBP
    ret
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
