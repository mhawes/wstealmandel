#ifndef RTWORKSTEAL_H
#define RTWORKSTEAL_H

#define WORKER_COUNT 3        /* the total number of worker threads */

#define MAX_ESTIMATE 2147483647

#include <time.h>
#include <pthread.h>

/* HAS TO BE INCLUDED LAST!!! */
#include "mandelbrot.h"

typedef enum { THREAD_WORKING, 
               THIEF_SIG, 
               IS_THIEF,
               VICTIM_SIG, 
               IS_VICTIM, 
               WORK_FINISHED 
             } ThreadStatus;

typedef struct ThreadInfo{
    char t_id;
    ThreadStatus status;
    unsigned long estimated_complete;    
    unsigned int end, curr;
    pthread_mutex_t finish_line_mut;
    pthread_cond_t  finish_line_cond;
} ThreadInfo;

/* -------------------------------------------------------------------------- */

void *rt_render_thread       ( void*);
void *rt_monitor_thread      ( void*);

void rt_broadcast_finished   ();
void rt_distribute           ( ThreadInfo *, ThreadInfo *);

ThreadInfo *rt_wait_for_complete();
ThreadInfo *rt_find_victim   ();

void rt_become_thief         ( ThreadInfo *);
void rt_become_victim        ();

unsigned int rt_compute_work ( ThreadInfo *);
void rt_update_estimate      ( ThreadInfo *, unsigned long);


/* -------------------------------------------------------------------------- */

void ws_initialise_threads   ();
void ws_start_threads        ();


void rt_print_status( ThreadInfo *);
void rt_print_workload( ThreadInfo *);
#endif /* RTWORKSTEAL_H */
