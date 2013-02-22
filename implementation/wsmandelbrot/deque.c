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
    
    /* mutex for top access */
    pthread_mutex_init( &d->top_mutex, NULL); 
}

/* -------------------------------------------------------------------------- */
/* This function grows the array to the current value of mem_size.
 * It does so by allocating a new array and copying the results from
 * the old array over. 
 */
void de_re_allocate( Deque *d, int old_size)
{
    int i, j = 0, size = d->bot - d->top;
    Line *temp_q = (Line *)malloc(d->mem_size * sizeof(Line));
    
//    printf("REALLOCATION: %d\n", d->mem_size);
//    printf("b: %d t:%d\n", d->bot, d->top);
    
    for( i = d->top; i < d->bot; i++)
    {
        temp_q[i % d->mem_size] = d->queue[i % old_size];
    }
    
    de_free_queue( d);
    d->queue = temp_q;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void de_push_bottom( Deque *d, Line line)
{
    int size = d->bot - d->top;
    
    int mem_s = d->mem_size;
    if(size >= mem_s){
        d->mem_size = mem_s * 2;
        de_re_allocate( d, mem_s);
    }
    
    line.status = LINE_NORMAL;
    d->queue[ d->bot % d->mem_size] = line;
    
    d->bot++;
}

/* -------------------------------------------------------------------------- */
/* Takes the bottom member of the queue and increments the bottom counter.
 * This function has a chance to shrink the size of the queue.
 */
Line de_pop_bottom( Deque *d)
{
    Line l;
    d->bot--;

    int size = d->bot - d->top;
    
    if( size < 0){
        d->bot = d->top;
        return empty;
    }

    l = d->queue[d->bot % d->mem_size]; 
        
    if( size > 0){
        /* in this case we want to shrink the array */
        //de_attempt_shrink( d, size);
        
        return l;
    }
    if( !de_cas_top( d, d->top, d->top + 1)){
        l = empty;
        d->bot = d->top + 1;
    }
    return l;
}

/* -------------------------------------------------------------------------- */
/* Takes the top member of the queue.
 */
Line de_steal( Deque *d)
{
    int size = d->bot - d->top;
    
    /* in this case we only have the bottom member therefore do not 
     * want to steal.
     */
    if(size <= 0){
        return empty;
    }
    
    Line l = d->queue[d->top % d->mem_size];
    
    if( !de_cas_top( d, d->top, d->top + 1)){
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
char de_cas_top( Deque *d, int old, int new)
{
    char pre_cond;
    
    if( pthread_mutex_trylock(&d->top_mutex) == 0)
    {
    
        /* need to sort out atomic task here. ASM.... TODO*/
        pre_cond = (d->top == old);
        if(pre_cond == 1){
            d->top = new;
        }
        pthread_mutex_unlock (&d->top_mutex);
    }
    
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
    int size = d->bot - d->top;
    int i;
    Line l;
    
    printf("Deque for thread id: %d\n", d->t_id);
    printf("  Bot: %d Top: %d\n", d->bot, d->top);
    printf("  Size:           %d\n", size);
    printf("  Size in Memory: %d\n", d->mem_size);
    
    printf("  Members:\n");
    if( size >= 0){
        for(i = d->mem_size; i >= 0 ; i--)
        {
            l = d->queue[i];
            printf( "    i:%d [y:%d]", i, l.y);
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

void de_test_deque( Deque *d)
{
    Line l;

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
