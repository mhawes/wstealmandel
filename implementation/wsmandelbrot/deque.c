#include "deque.h"

/* -------------------------------------------------------------------------- */
void de_initialise( deque_t *d, char thread_id)
{
    d->t_id = thread_id;
    
    d->mem_size = INIT_MEM_SIZE;
    
    d->top = 0;     /* points to the NEXT position of top */
    d->bot = 0;     /* points to the bototm of the queue */
    
    /* allocates the initial block of memory */
    d->queue = (line_t *)malloc(d->mem_size * sizeof(line_t));
    
    /* initialise of EMPTY and ABORT vals. */
    empty.status = LINE_EMPTY;
    abort_steal.status = LINE_ABORT;
    
    /* mutex for top access */
    pthread_mutex_init( &d->top_mutex, NULL); 
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* Pushes a line onto the bottom of the queue.
 */
void de_push_bottom( deque_t *d, line_t line)
{
    int size = d->bot - d->top;
    
    de_attempt_grow( d, size);
    
    line.status = LINE_NORMAL;
    d->queue[ d->bot % d->mem_size] = line;
    
    d->bot++;
}

/* -------------------------------------------------------------------------- */
/* Takes the bottom member of the queue and increments the bottom counter.
 * This function has a chance to shrink the size of the queue.
 */
line_t de_pop_bottom( deque_t *d)
{
    line_t l;
    d->bot--;

    int size = d->bot - d->top;
    
    if( size < 0){
        d->bot = d->top;
        return empty;
    }

    l = d->queue[d->bot % d->mem_size]; 
        
    if( size > 0){
        /* in this case we want to attempt to shrink the array */
        de_attempt_shrink( d, size);
        
        return l;
    }
    
    /* in this case bottom is also top and lock needs to be checked for */    
    if( pthread_mutex_trylock(&d->top_mutex) == 0){
        pthread_mutex_unlock (&d->top_mutex);
        return l;
    }
    else {
        l = empty;
        d->bot = d->top + 1;
    }

    return l;
}

/* -------------------------------------------------------------------------- */
/* Takes the top member of the queue.
 */
line_t de_steal( deque_t *d)
{
    int size = d->bot - d->top;
    line_t l;
    
    /* in this case we only have the bottom member therefore do not 
     * want to steal.
     */
    if(size <= 0){
        return empty;
    }

    /* mutex for top element needs to be enforced here. */
    /* If the mutex is locked an abort signal line is returned */    
    if( pthread_mutex_trylock(&d->top_mutex) == 0)
    {
        l = d->queue[d->top % d->mem_size];
        d->top++;
        pthread_mutex_unlock (&d->top_mutex);
    }
    else{
        l = abort_steal;
    }
    
    return l;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
char de_attempt_shrink( deque_t *d, int size)
{
    int mem_s = d->mem_size;
    if( size <= mem_s / 2 && mem_s > INIT_MEM_SIZE){
        d->mem_size = mem_s / 2;
        de_re_allocate( d, mem_s);
        return 1;
    }
    return 0;
}

/* -------------------------------------------------------------------------- */
char de_attempt_grow( deque_t *d, int size)
{
    int mem_s = d->mem_size;
    
    if(size >= mem_s){
        d->mem_size = mem_s * 2;
        de_re_allocate( d, mem_s);
    }
}

/* -------------------------------------------------------------------------- */
/* This function grows the array to the current value of mem_size.
 * It does so by allocating a new array and copying the results from
 * the old array over. 
 */
void de_re_allocate( deque_t *d, int old_size)
{
    pthread_mutex_lock(&d->top_mutex);
    
    int i, j = 0, size = d->bot - d->top;
    line_t *temp_q = (line_t *)malloc(d->mem_size * sizeof(line_t));
    
    for( i = d->top; i < d->bot; i++)
    {
        temp_q[i % d->mem_size] = d->queue[i % old_size];
    }
    
    de_free_queue( d);
    d->queue = temp_q;
    
    pthread_mutex_unlock (&d->top_mutex);
}

/* -------------------------------------------------------------------------- */
void de_free_queue( deque_t *d)
{
    free(d->queue);
}

/* -------------------------------------------------------------------------- */
/* UTIL FUNCTIONS: */
/* -------------------------------------------------------------------------- */
void de_print_deque( deque_t *d)
{
    int size = d->bot - d->top;
    int i;
    line_t l;
    
    printf("Deque for thread id: %d\n", d->t_id);
    printf("  Bot: %d Top: %d\n", d->bot, d->top);
    printf("  Size:           %d\n", size);
    printf("  Size in Memory: %d\n", d->mem_size);
    
    printf("  Members:\n");
    if( size >= 0){
        for(i = d->mem_size - 1; i >= 0 ; i--)
        {
            l = d->queue[i];
            if( i < 10){ printf( "    i:%d ",i);}
            else{        printf( "    i:%d",i);}
            printf( " [y:%d]", l.y);
            if(i == d->bot % d->mem_size){ printf(" <- bot");}
            if(i == d->top % d->mem_size){ printf(" <- top");}
            printf("\n");
        }
    }
    else{
        printf("    Deque empty\n");
    }
    printf("\n");
}

void de_test_deque( deque_t *d)
{
    line_t l;

    /* fill up the deque */
    int i;
    for( i = 0; i < 700; i++)
    {
        l.y = i;
        
        if(i % 2 == 0){ de_pop_bottom( d); }
        if(i % 3 == 0){ de_steal( d); }

        de_push_bottom( d, l);
    }
    
    
    
    while(1)
    {
        de_print_deque( d);
    
        l = de_pop_bottom( d);
        if( l.status == LINE_EMPTY){
            printf("GOT EMPTY SIGNAL\n\n");
            break;
        }
    }
}
