/* 
 * Implementation of a double ended queue of line structs which is non-blocking.
 * The deque is cirular. (see paper)....
 * 
 * The deque grows and shrinks accordingly.
 */

#ifndef DEQUE_H
#define DEQUE_H
 
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define LINE_NORMAL 0
#define LINE_EMPTY 1
#define LINE_ABORT 2

#define INIT_MEM_SIZE 10 /* the initial number of allocated Line slots */
 
typedef struct line_t{
    char status;
    unsigned int y;
} line_t;

typedef struct deque_t{
    char t_id;
    
    int mem_size; 
    int top, bot;

    line_t *queue;
    
    pthread_mutex_t top_mutex;
} deque_t;

static line_t empty, abort_steal;

/* function defs */

void de_initialise     ( deque_t *, char);

/* ------------------------------------- */
line_t de_steal        ( deque_t *);
void de_push_bottom    ( deque_t *, line_t);
line_t de_pop_bottom   ( deque_t *);
/* ------------------------------------- */

char de_attempt_shrink ( deque_t *, int);
char de_attempt_grow   ( deque_t *, int);
void de_re_allocate    ( deque_t *, int);
void de_free_queue     ( deque_t *d);

/* UTIL FUNCTIONS */
void de_print_deque( deque_t *);

#endif /* DEQUE_H */
