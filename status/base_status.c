#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "base_status.h"

void A(status_t *s);
void B(status_t *s);

void A(status_t *s)
{
    s->now_status = B;
    printf("A");
}

void B(status_t *s)
{
    s->now_status = A;
    printf("B");
}

int main(int argc, char *argv[])
{
    status_t s = {NULL, NULL, true, 0};

    STATE_MACHINE(s, A);

    return s.return_val;
}

