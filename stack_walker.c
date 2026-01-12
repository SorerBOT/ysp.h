#include <stdio.h>
#include <ptrauth.h>
#include <dlfcn.h>

void print_stack_trace()
{
    void* rbp_signed = __builtin_frame_address(0);
    void* rbp = ptrauth_strip(rbp_signed, ptrauth_key_frame_pointer);

    void* rip_signed = *(void**)((char*)rbp + 8);
    void* rip = ptrauth_strip(rip_signed, ptrauth_key_return_address);

    Dl_info info = {0};
    dladdr(rip, &info);
    printf("Currently executing function: %s\n", info.dli_sname);

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
    print_stack_trace();
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
    shalom();
    return 0;
}
