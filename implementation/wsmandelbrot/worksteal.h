#ifndef WORKSTEAL_H
#define WORKSTEAL_H

#define WORKER_COUNT 4        /* the total number of worker threads */

#include <pthread.h>
#include "deque.h"

/* HAS TO BE INCLUDED LAST!!! */
#include "mandelbrot.h"

void *ws_worker_thread       ( void*);

char ws_become_thief         ( deque_t *);
char ws_victimise            ( deque_t *, deque_t *);

void ws_distribute_lines     ();
deque_t *ws_random_deque       ( char[WORKER_COUNT]);
unsigned int ws_compute_deque( deque_t *deq);

/* -------------------------------------------------------------------------- */

void ws_initialise_threads   ();
void ws_start_threads        ();

/* -------------------------------------------------------------------------- */

#endif /* WORKSTEAL_H */
