#include <stdio.h>
#include <ptrauth.h>
#include <dlfcn.h>
#include <signal.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/mman.h>

#define YSP_TIMER_HZ_99_MICRO_SECOND 10101

typedef void ysp_instruction_t;

typedef struct
{
    size_t depth;
    const ysp_instruction_t* instructions[];
} ysp_sample_t;

typedef struct
{
    size_t samples_offset;
    size_t samples_count;
    ysp_sample_t samples[];
} ysp_profiler_data_t;

ysp_profiler_data_t* profiler = NULL;

void print_stack_trace(int signo, struct __siginfo * siginfo, void * ctx)
{
    ysp_sample_t* sample = (ysp_instruction_t*)(((char*)profiler->samples) + profiler->samples_offset);
    *sample = (ysp_sample_t)
    {
        .depth = 0
    };

    ucontext_t* uctx = (ucontext_t*) ctx;
    ysp_instruction_t* uctx_rip_signed = (ysp_instruction_t*) uctx->uc_mcontext->__ss.__pc;
    ysp_instruction_t* uctx_rip = ptrauth_strip(uctx_rip_signed, ptrauth_key_return_address);

    sample->instructions[sample->depth] = uctx_rip;
    sample->depth++;

    ysp_instruction_t* rbp_signed = __builtin_frame_address(0);
    ysp_instruction_t* rbp = ptrauth_strip(rbp_signed, ptrauth_key_frame_pointer);

    ysp_instruction_t* rip_signed = *(ysp_instruction_t**)((char*)rbp + 8);
    ysp_instruction_t* rip = ptrauth_strip(rip_signed, ptrauth_key_return_address);

    while (1)
    {
        ysp_instruction_t* calling_rbp_signed = *((ysp_instruction_t**) rbp);
        rbp = ptrauth_strip(calling_rbp_signed, ptrauth_key_frame_pointer);

        if (rbp == NULL)
        {
            break;
        }

        ysp_instruction_t* calling_rip_signed = *(ysp_instruction_t**)((char*)rbp + 8);
        ysp_instruction_t* calling_rip = ptrauth_strip(calling_rip_signed, ptrauth_key_return_address);

        sample->instructions[sample->depth] = calling_rip;
        sample->depth++;
    }

    profiler->samples_offset += sizeof(ysp_sample_t) + sample->depth * sizeof(ysp_instruction_t*);
}
double calc_pi()
{
    long long iterations = 1000000; // Needs many iterations!
    double pi_approx = 0.0;
    double sign = 1.0; // For alternating signs

    for (long long i = 0; i < iterations; i++) {
        pi_approx += sign * (4.0 / (2.0 * i + 1.0));
        sign *= -1.0; // Alternate sign for next term
    }

    return pi_approx;
}

void shalom3()
{
    printf("PI = %lf\n", calc_pi());
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
    profiler = mmap(NULL, 1000 * 1000 * sizeof(char),
            PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_ANON,
            0, 0);

    if (profiler == NULL)
    {
        perror("YSP: mmap() error\n");
        exit(EXIT_FAILURE);
    }

    *profiler = (ysp_profiler_data_t)
    {
        .samples_offset = 0
    };

    struct sigaction act = {0};
    act.sa_sigaction = print_stack_trace;
    sigaction(SIGPROF, &act, NULL);

    struct itimerval time_val = {0};
    time_val.it_value.tv_usec = YSP_TIMER_HZ_99_MICRO_SECOND / 10;
    time_val.it_interval.tv_usec = YSP_TIMER_HZ_99_MICRO_SECOND / 10;
    setitimer(ITIMER_PROF, &time_val, NULL);

    shalom();

    ysp_sample_t* sample = profiler->samples;
    void* last_addr = ((char*)sample) + profiler->samples_offset;
    while (sample < (ysp_sample_t*)last_addr)
    {
        for (size_t i = 0; i < sample->depth; ++i)
        {
            Dl_info info = {0};
            dladdr(sample->instructions[i], &info);
            printf("func: %s\n", info.dli_sname);
        }

        sample = sample + (sizeof(ysp_sample_t) + sample->depth * sizeof(ysp_instruction_t*));
    }

    return 0;
}
