#include <stdio.h>
#include <ptrauth.h>
#include <dlfcn.h>
#include <signal.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>

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
    ysp_sample_t* sample = (ysp_sample_t*)(((char*)profiler->samples) + profiler->samples_offset);
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
void busy_wait_5_seconds()
{
    time_t initial_time = time(NULL);

    while (1)
    {
        if (time(NULL) - initial_time > 5)
        {
            break;
        }
    }
}

void shalom3()
{
    busy_wait_5_seconds();
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

int ysp_compare_samples(const void* first, const void* second)
{
    const ysp_sample_t* first_sample = (const ysp_sample_t*)first;
    const ysp_sample_t* second_sample = (const ysp_sample_t*)second;

    if (first_sample->depth == second_sample->depth)
    {
        for (size_t i = 0; i < first_sample->depth; ++i)
        {
            if (first_sample->instructions[i] != second_sample->instructions[])
        }
    }
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
    time_val.it_value.tv_usec = YSP_TIMER_HZ_99_MICRO_SECOND;
    time_val.it_interval.tv_usec = YSP_TIMER_HZ_99_MICRO_SECOND;
    setitimer(ITIMER_PROF, &time_val, NULL);

    shalom();

    size_t sample_ptrs_count = 0;
    ysp_sample_t** sample_ptrs = (ysp_sample_t**)(((char*)profiler->samples) + profiler->samples_offset + 1);

    ysp_sample_t* sample = profiler->samples;
    void* last_addr = ((char*)sample) + profiler->samples_offset;
    while (sample < (ysp_sample_t*)last_addr)
    {
        sample_ptrs[sample_ptrs_count] = sample;
        sample_ptrs_count++;
        sample = (ysp_sample_t*)(((char*)sample) + (sizeof(ysp_sample_t) + sample->depth * sizeof(ysp_instruction_t*)));
    }

    //qsort(sample_ptrs, sample_ptrs_count, sizeof(ysp_sample_t*), );
    return 0;
}
