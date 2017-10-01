#ifndef __BASE_STATUS_H__
#define __BASE_STATUS_H__
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

#define SINGAL_STATE(s) \
    if(s.now_status!=NULL) \
        s.now_status(&s);

#define STATE_MACHINE(s, init_status) \
    s.now_status = init_status; \
    while(s.run) \
        SINGAL_STATE(s);

#endif	//__BASE_STATUS_H__
