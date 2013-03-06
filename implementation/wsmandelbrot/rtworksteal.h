#ifndef RTWORKSTEAL_H
#define RTWORKSTEAL_H

#define RT_WORKER_COUNT 1        /* the total number of worker threads */

#define MAX_ESTIMATE 255

#include <pthread.h>

/* HAS TO BE INCLUDED LAST!!! */
#include "mandelbrot.h"



typedef struct ThreadInfo{
    char t_id;
    char estimated_complete;    
    unsigned int start, end, curr;
} ThreadInfo;


/* -------------------------------------------------------------------------- */

void *rt_render_thread       ( void*);
void *rt_monitor_thread      ( void*);

char rt_become_thief         ();
char rt_victimise            ();

unsigned int rt_compute_work ( ThreadInfo *);

/* -------------------------------------------------------------------------- */

void ws_initialise_threads   ();
void ws_start_threads        ();

#endif /* RTWORKSTEAL_H */
