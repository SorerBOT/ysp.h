#include <stdio.h>
#include <dlfcn.h>
#include <ptrauth.h>
#include <signal.h>
#include <sys/time.h>

void print_stack_trace();

void func1();
void func2();
void func3();
size_t func4();
size_t func5();

void func1()
{
    func2();
}

void func2()
{
    func3();
}
void func3()
{
    func4();
}

void* get_pc()
{
    return __builtin_return_address(0);
}
size_t func4()
{
    return func5();
}
size_t func5()
{
    volatile size_t x = 0;
    for (volatile size_t i = 0; i < 1000 * 1000 * 1000 * 10 ;++i)
    {
        x += i;
    }
    return x;
}

void print_stack_trace()
{
    void* pc_signed = get_pc();
    void* pc = ptrauth_strip(pc_signed, ptrauth_key_return_address);
    Dl_info info = {0};
    dladdr(pc, &info);

    printf("rip_signed = %p\n", pc_signed);
    printf("rip_unsigned = %p\n", pc);
    printf("Current executing function %s\n", info.dli_sname);

    info = (Dl_info){0};
    void* rbp_signed = __builtin_frame_address(0);
    void* rbp = ptrauth_strip(rbp_signed, ptrauth_key_frame_pointer);
    void** rip_signed_ptr = (void**)((char*)rbp + 8);
    void* rip_signed = *rip_signed_ptr;
    void* rip = ptrauth_strip(rip_signed, ptrauth_key_return_address);
    info = (Dl_info){0};
    dladdr(rip, &info);
    printf("rbp_signed = %p\n", rbp_signed);
    printf("rip_signed = %p\n", rip_signed);

    do
    {
        printf("rbp = %p\n", rbp);
        printf("rip = %p\n", rip);
        printf("Called by function %s\n", info.dli_sname);

        rbp_signed = *((void**)rbp);
        rbp = ptrauth_strip(rbp_signed, ptrauth_key_frame_pointer);
        rip_signed = *(void**)((char*)rbp + 8);
        rip = ptrauth_strip(rip_signed, ptrauth_key_return_address);
        info = (Dl_info){0};
        dladdr(rip, &info);
    } while (info.dli_sname != NULL);
}

int main(void)
{
    struct sigaction sa = {0};
    struct itimerval timer;

    // 1. Configure the signal handler
    sa.sa_handler = print_stack_trace;
    // Restart system calls if interrupted by signal
    sa.sa_flags = SA_RESTART; 
    
    if (sigaction(SIGPROF, &sa, NULL) == -1) {
        perror("Error: cannot handle SIGPROF");
        return 1;
    }

    // 2. Configure the timer
    // The timer will expire after 500,000 microseconds (0.5 seconds)
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 500000;

    // And reset to this interval every time it expires
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 200;

    // 3. Start the timer using ITIMER_PROF
    if (setitimer(ITIMER_PROF, &timer, NULL) == -1) {
        perror("Error: setitimer");
        return 1;
    }
    func1();
    return 0;
}

