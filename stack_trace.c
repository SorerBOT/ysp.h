#include <stdio.h>
#include <dlfcn.h>
#include <ptrauth.h>

void func1();
void func2();
void func3();
void func4();

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

void func4()
{

    void* pc_signed = get_pc();
    void* pc = ptrauth_strip(pc_signed, ptrauth_key_return_address);
    Dl_info info = {0};
    dladdr(pc, &info);

    printf("rip_signed = %p\n", pc_signed);
    printf("rip_unsigned = %p\n", pc);
    printf("Current executing function %s\n", info.dli_sname);

    void* rbp_signed = __builtin_frame_address(0);
    void* rbp = ptrauth_strip(rbp_signed, ptrauth_key_frame_pointer);
    void** rip_signed_ptr = (void**)((char*)rbp + 8);
    void* rip_signed = *rip_signed_ptr;
    void* rip = ptrauth_strip(rip_signed, ptrauth_key_return_address);

    info = (Dl_info){0};
    dladdr(rip, &info);
    printf("rbp_signed = %p\n", rbp_signed);
    printf("rbp_unsigned = %p\n", rbp);
    printf("rip_signed = %p\n", rip_signed);
    printf("rip_unsigned = %p\n", rip);
    printf("Current executing function %s\n", info.dli_sname);
}

int main(void)
{
    func1();
    return 0;
}

