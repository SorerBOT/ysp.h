#include <stdio.h>
#include <ptrauth.h>
#include <dlfcn.h>
#include <signal.h>
#include <sys/time.h>

void print_stack_trace(int signo, struct __siginfo * siginfo, void * ctx)
{

    ucontext_t* uctx = (ucontext_t*) ctx;
    void* uctx_rip_signed = (void*) uctx->uc_mcontext->__ss.__pc;
    void* uctx_rip = ptrauth_strip(uctx_rip_signed, ptrauth_key_return_address);

    Dl_info info = {0};
    dladdr(uctx_rip, &info);

    printf("Currently executing function: %s\n", info.dli_sname);

    void* rbp_signed = __builtin_frame_address(0);
    void* rbp = ptrauth_strip(rbp_signed, ptrauth_key_frame_pointer);

    void* rip_signed = *(void**)((char*)rbp + 8);
    void* rip = ptrauth_strip(rip_signed, ptrauth_key_return_address);

    info = (Dl_info ){0};
    dladdr(rip, &info);

    do
    {
        void* calling_rbp_signed = *((void**) rbp);
        rbp = ptrauth_strip(calling_rbp_signed, ptrauth_key_frame_pointer);

        void* calling_rip_signed = *(void**)((char*)rbp + 8);
        void* calling_rip = ptrauth_strip(calling_rip_signed, ptrauth_key_return_address);

        info = (Dl_info) {0};
        dladdr(calling_rip, &info);
        printf("Called by function: %s\n", info.dli_sname);
    } while (info.dli_sname != NULL);
}
void shalom3()
{
    int x;
    while (1)
    {
        ++x;
    }
}
void shalom2()
{
    shalom3();
}
void shalom()
{
    printf("SHALOM\n");
    shalom2();
}

int main(void)
{
    struct sigaction act = {0};
    act.sa_sigaction = print_stack_trace;
    sigaction(SIGPROF, &act, NULL);

    struct itimerval time_val = {0};
    time_val.it_value.tv_sec = 1;
    time_val.it_interval.tv_sec = 1;
    setitimer(ITIMER_PROF, &time_val, NULL);

    shalom();




    return 0;
}
