#include <time.h>

#define YSP_IMPLEMENTATION
#include "../src/ysp.h"

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
    shalom2();
}

int main(void)
{

    shalom();


    return 0;
}
