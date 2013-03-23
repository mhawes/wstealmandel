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
             } thread_status_t;

typedef struct thread_info_t{
    char t_id;
    thread_status_t status;
    unsigned long estimated_complete;    
    unsigned int end, curr;
    pthread_mutex_t finish_line_mut;
    pthread_cond_t  finish_line_cond;
    
    pthread_barrier_t line_bar;
} thread_info_t;

/* -------------------------------------------------------------------------- */

void *rt_render_thread       ( void*);
void *rt_monitor_thread      ( void*);

void rt_broadcast_finished   ();
void rt_distribute           ( thread_info_t *, thread_info_t *);

thread_info_t *rt_wait_for_complete();
thread_info_t *rt_find_victim   ();

void rt_become_thief         ( thread_info_t *);
void rt_become_victim        ();

unsigned int rt_compute_work ( thread_info_t *);
void rt_update_estimate      ( thread_info_t *, unsigned long);


/* -------------------------------------------------------------------------- */

void ws_initialise_threads   ();
void ws_start_threads        ();

/* -------------------------------------------------------------------------- */

void rt_print_status( thread_info_t *);
void rt_print_workload( thread_info_t *);

#endif /* RTWORKSTEAL_H */
