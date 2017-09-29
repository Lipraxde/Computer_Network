#ifndef __STATUS_H__
#define __STATUS_H__
#include <stdbool.h>

struct status
{
    void (*now_status)(struct status *);
    void *p;
    bool run;
    int return_val;
};

typedef struct status status_t;
typedef void (*state_func)(status_t *);

#define STATE_MACHINE(s, init_status) \
    s.now_status = init_status; \
    while(s.run) \
        if(s.now_status!=NULL) \
            s.now_status(&s);

#endif	//__STATUS_H__
