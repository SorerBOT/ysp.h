#include <time.h>

#define YSP_IMPLEMENTATION
#include "../src/ysp.h"

/*
 * This example SIMULATES a profiling session of a Neural Network training
 * which is supposedly composed of the following parts
 *      - processing input
 *      - gradient descent which simply runs backprop + some kind of optimal step size estimator iteratively
 */

void busy_wait(size_t duration);
void process_inputs();
void gradient_descent();
void backprop();
void step_size_estimator();

void busy_wait(size_t duration)
{
    time_t initial_time = time(NULL);

    while (1)
    {
        if (time(NULL) - initial_time > duration)
        {
            break;
        }
    }
}

void process_inputs()
{
    busy_wait(2);    
}

void backprop()
{
    busy_wait(10);
}

void step_size_estimator()
{
    busy_wait(1);
}

void gradient_descent()
{
    for (size_t i = 0; i < 3; ++i)
    {
        backprop();
        step_size_estimator();
    }
}

int main(void)
{
    process_inputs();
    gradient_descent();
    return 0;
}
