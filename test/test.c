#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct statue
{
    void (*now_statue)(struct statue *);
    void *p;
    bool run;
    int return_val;
};

void A(struct statue *s);
void B(struct statue *s);

void A(struct statue *s)
{
    s->now_statue = B;
    printf("A");
}

void B(struct statue *s)
{
    s->now_statue = A;
    printf("B");
}

int main(int argc, char *argv[])
{
    struct statue s = {NULL, NULL, true, 0};
    s.now_statue = A;

    while(s.run)
        if(s.now_statue!=NULL)
            s.now_statue(&s);

    return s.return_val;
}

