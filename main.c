#include <stdio.h>

#include "test_port.h"
#include "test_segments.h"

int main()
{
    test_segments();
    test();
    printf("Test Finish!\n");
    return 0;
}
