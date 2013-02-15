#include "deque.h"

/* -------------------------------------------------------------------------- */
void de_initialise( Deque *d, char thread_id)
{
    d->t_id = thread_id;
    
    d->mem_size = INIT_MEM_SIZE;
    
    d->top = 0;     /* points to the NEXT position of top */
    d->bot = 0;     /* points to the bototm of the queue */
    
    /* allocates the initial block of memory */
    d->queue = (Line *)malloc(d->mem_size * sizeof(Line));
    
    /* initialise of EMPTY and ABORT vals. */
    empty.status = LINE_EMPTY;
    abort_steal.status = LINE_ABORT;
}

/* -------------------------------------------------------------------------- */
/* This function grows the array to the current value of mem_size.
 * It does so by allocating a new array and copying the results from
 * the old array over. 
 */
void de_re_allocate( Deque *d, int old_size)
{
    int i, j = 0, size = d->top - d->bot;
    Line *temp_q = (Line *)malloc(d->mem_size * sizeof(Line));
    
//    printf("REALLOCATION: %d\n", d->mem_size);
//    printf("b: %d t:%d\n", d->bot, d->top);
    
    for( i = d->bot; i < d->top; i++)
    {
        temp_q[i % d->mem_size] = d->queue[i % old_size];
    }
    
    de_free_queue( d);
    d->queue = temp_q;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* This is here for adding initial vals to the queue and doesn't get called 
 * during work-stealing at any point.
 *
 * returns 1 if the push needed to be aborted.
 * returns 0 if completed correctly.
 */
char de_push_top( Deque *d, Line line)
{
    int size;
    
    size = d->top - d->bot;

    /* in this case we need to grow the array */
    int mem_s = d->mem_size;
    if(size >= mem_s){
        d->mem_size = mem_s * 2;
        de_re_allocate( d, mem_s);
    }
    
    line.status = LINE_NORMAL;
    d->queue[d->top % d->mem_size] = line;
    
    if( !cas_top( d, d->top, d->top + 1)){
        /* abort */
        return 1;
    }
    
    return 0;
}

/* -------------------------------------------------------------------------- */
/* Takes the bottom member of the queue and increments the bottom counter.
 * This function has a chance to shrink the size of the queue.
 */
Line de_pop_bottom( Deque *d)
{
    Line l;
    int size = d->top - d->bot;
    
    if( d->bot >= d->top){
        return empty;
    }
    
    /* in this case we want to shrink the array */
    de_attempt_shrink( d, size);
    
    l = d->queue[d->bot % d->mem_size]; 
    d->bot++;
    
    return l;
}

/* -------------------------------------------------------------------------- */
/* Takes the top member of the queue.
 */
Line de_steal( Deque *d)
{
    int size;
    size = d->top - d->bot;
    
    /* in this case we only have the bottom member therefore do not 
     * want to steal.
     */
    if(size <= 1){
        return empty;
    }
    
    Line l = d->queue[(d->top - 1) % d->mem_size];
    
    if( !cas_top( d, d->top, d->top - 1)){
        l = abort_steal;
    }
    return l;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
char de_attempt_shrink( Deque *d, int size)
{
    int mem_s = d->mem_size;
    if( size < mem_s / 2 && mem_s > INIT_MEM_SIZE){
        d->mem_size = mem_s / 2;
        de_re_allocate( d, mem_s);
        return 1;
    }
    return 0;
}

/* -------------------------------------------------------------------------- */
char cas_top( Deque *d, int old, int new)
{
    char pre_cond;
    
    /* need to sort out atomic task here. ASM.... TODO*/
    pre_cond = (d->top == old);
    if(pre_cond == 1){
        d->top = new;
    }
    /* TODO */
    
    return pre_cond;
}

/* -------------------------------------------------------------------------- */
void de_free_queue( Deque *d)
{
    free(d->queue);
}

/* -------------------------------------------------------------------------- */
/* UTIL FUNCTIONS: */
/* -------------------------------------------------------------------------- */
void de_print_deque( Deque *d)
{
    int size = d->top - d->bot;
    int i;
    Line l;
    
    printf("Deque for thread id: %d\n", d->t_id);
    printf("  Bot: %d Top: %d\n", d->bot, d->top);
    printf("  Size:           %d\n", size);
    printf("  Size in Memory: %d\n", d->mem_size);
    
    printf("  Members:\n");
    if( size >= 0){
        for(i = 0; i < d->mem_size; i++)
        {
            l = d->queue[i];
            printf( "    i:%d [y:%d xs:%d xe:%d]", i, l.y, l.x_sta, l.x_end);
            if(i == d->bot % d->mem_size){ printf(" <- bot");}
            if(i == d->top - 1 % d->mem_size){ printf(" <- top");}
            printf("\n");
        }
    }
    else{
        printf("    Deque empty\n");
    }
    printf("\n");
}
