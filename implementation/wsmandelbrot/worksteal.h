#ifndef WORKSTEAL_H
#define WORKSTEAL_H

#define WORKER_COUNT 4        /* the total number of worker threads */

#include <pthread.h>

#include "deque.h"

/* HAS TO BE INCLUDED LAST!!! */
#include "mandelbrot.h"

void *ws_worker_thread       ( void*);
char ws_become_thief         ( Deque *);
char ws_victimise            ( Deque *, Deque *);

Deque *ws_random_deque       ( char[WORKER_COUNT]);
unsigned int ws_compute_deque( Deque *deq);

void ws_initialise_threads   ();
void ws_distribute_lines     ();
void ws_start_threads        ();

#endif /* WORKSTEAL_H */
