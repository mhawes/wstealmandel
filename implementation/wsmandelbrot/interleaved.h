#ifndef INTERLEAVED_H
#define INTERLEAVED_H

#define WORKER_COUNT 4        /* the total number of worker threads */


#include <pthread.h>

/* HAS TO BE INCLUDED LAST!!! */
#include "mandelbrot.h"

typedef struct thread_info_t{
    char t_id;
    unsigned int start_y, end_y;
} thread_info_t;

void *in_worker_thread       ( void*);
void in_initialise           ( thread_info_t*);

/* -------------------------------------------------------------------------- */

void ws_initialise_threads   ();
void ws_start_threads        ();

/* -------------------------------------------------------------------------- */

#endif /* INTERLEAVED_H */
