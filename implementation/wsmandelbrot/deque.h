/* 
 * Implementation of a double ended queue of line structs which is non-blocking.
 * The deque is cirular. (see paper)....
 * 
 * The deque grows and shrinks accordingly.
 */
 
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define LINE_NORMAL 0
#define LINE_EMPTY 1
#define LINE_ABORT 2

#define INIT_MEM_SIZE 10 /* the initial number of allocated Line slots */
 
typedef struct Line{
    char status;
    unsigned int y, x_sta, x_end;
} Line;

typedef struct Deque{
    char t_id;
    
    int mem_size; 
    int top, bot;

    Line *queue;
} Deque;

static Line empty, abort_steal;

/* function defs */

void de_initialise     ( Deque *, char);

/* ------------------------------------- */
Line de_steal          ( Deque *);
char de_push_top       ( Deque *, Line);
Line de_pop_bottom     ( Deque *);
/* ------------------------------------- */

char cas_top           ( Deque *, int, int);

char de_attempt_shrink ( Deque *, int);

void de_re_allocate    ( Deque *, int );
void de_free_queue     ( Deque *d);

/* UTIL FUNCTIONS */
void de_print_deque( Deque *);
