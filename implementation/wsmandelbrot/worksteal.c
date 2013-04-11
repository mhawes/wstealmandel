#include "worksteal.h"

static deque_t deques[WORKER_COUNT]; /* Set of deques to fill. One per trhead. */
pthread_t threads[WORKER_COUNT]; /* set of threads to execute the deques */

/* -------------------------------------------------------------------------- */
/* thread function */
/* -------------------------------------------------------------------------- */
void *ws_worker_thread( void *t_deq)
{
    deque_t *deq = (deque_t *) t_deq;
    char stealable = 1;
    int work_count = 0;

    printf("T_id %d started\n", deq->t_id); 
    
    do
    {
        /* This is where the work is done. */
        work_count += ws_compute_deque( deq);

        /* After this the thread turns into a thief */
        stealable = ws_become_thief( deq);
    } while(stealable == 1);
    
    
    printf("T_id %d finished computing %d lines\n", deq->t_id, work_count);

    pthread_exit(NULL);
}

/* -------------------------------------------------------------------------- */
/* this initialises the deques which are passed to the threads. */
void ws_initialise_threads()
{
    char i;

    /* initialise all deques */
    for (i = 0; i < WORKER_COUNT; i++)
    {
        de_initialise( &deques[i], i);
    }
    
    ws_distribute_lines();
    
    /* set a seed for the rand function */
    srand( 938672);
}

/* -------------------------------------------------------------------------- */
void ws_distribute_lines()
{
    unsigned int y, distribution = HEIGHT / WORKER_COUNT;
    char i = -1;
    line_t line;
    
    line.status = LINE_NORMAL;
    
    /* initialise deques with work before starting */
    for( y = 0; y < HEIGHT ; y++)
    {
        line.y = y;
        
        if( y % distribution == 0){
            i++;
        }
        
        /* distribute the current line y to one of the deques */
        de_push_bottom( &deques[i], line); 
    }
}

/* -------------------------------------------------------------------------- */
void ws_start_threads()
{
    pthread_attr_t attr;
    void *status;
    char i;

    /* Initialize and set thread detached attribute */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    /* start threads */
    for(i=0; i<WORKER_COUNT; i++) {
        pthread_create(&threads[i], &attr, ws_worker_thread, (void *)&deques[i]);
    }
    
    /* join point */
    for(i=0; i<WORKER_COUNT; i++) {
        pthread_join(threads[i], &status);
    }
    
    /* Free attribute and wait for the other threads */
    pthread_attr_destroy( &attr);
}

/* -------------------------------------------------------------------------- */
unsigned int ws_compute_deque( deque_t *deq)
{
//    printf( "T %d BECAME WORKER\n", deq->t_id);
    unsigned int work_count = 0;
    line_t line_cur;

    while(1)
    {
        line_cur = de_pop_bottom( deq);
        
        if(line_cur.status == LINE_EMPTY){
            break;
        }
        
        compute_line( line_cur.y, deq->t_id);
        
        work_count++;
    }
    
    return work_count;
}

/* -------------------------------------------------------------------------- */
/* returns 1 if found work and needs to return to worker mode.
 * returns 0 if no work is found in the entire network and the thread can finish.
 */
char ws_become_thief( deque_t *deq)
{
//    printf( "T %d BECAME THIEF\n", deq->t_id);

    int i;
    char result = 0, ex_count = 1;
    deque_t *victim;
    char exclude_set[WORKER_COUNT];

    /* initialise the exclude set */
    for(i = 0; i < WORKER_COUNT; i++)
    {
        exclude_set[i] = 0;
    }
    /* add this threads deque to the exclude set */
    exclude_set[deq->t_id] = 1;

    /* while no work has been stolen.
     * in the case that no work is available 0 is 
     * returned and the loop is broken.
     */
    while(result == 0)
    {
        /* if the exclude set is not exhausted get another victim at random */
        if( ex_count < WORKER_COUNT ){
            victim = ws_random_deque( exclude_set);
            result = ws_victimise( deq, victim);
        }
        else{
            return 0;
        }

        /* add the unsuccessful victim to the exclude set */
        exclude_set[victim->t_id] = 1;
        ex_count++;
    }
    
    return 1;
}

/* -------------------------------------------------------------------------- */
/* returns 1 if work has been stolen and placed in the deque ready for computing
 * returns 0 if no work is found at this victim.
 */
char ws_victimise( deque_t *deq, deque_t *victim)
{
    char result = 0;
    line_t line;
    int steal_size;
    int fill_count = 0;

    line = de_steal( victim);
    
    /* check if the victim is empty first */
    if( line.status == LINE_EMPTY ){
        return 0;
    }
    
    /* evaluate victim size to work out how much to steal */
    steal_size = (victim->bot - victim->top) / 2;
    
    /* loop until we get an empty line or the right amount of work is stolen */
    while( line.status != LINE_EMPTY )
    {
        /* if we have a normal line push it onto this threads deque */
        if( line.status == LINE_NORMAL){
            de_push_bottom( deq, line);
            /* when we have stolen the right amount of work from a victim give up */
            if( fill_count < steal_size){
                line = de_steal( victim);
                fill_count++;
            }
            else{
                return 1;
            }
        }

        /* if the thread was blocked try again */
        if( line.status == LINE_ABORT){
            line = de_steal( victim);
        }
    }

    return 0;
}

/* -------------------------------------------------------------------------- */
/* 
 * Generates a random number between 0 and WORKER_COUNT (not inclusive)
 * and returns a deque that is NOT the same as any in the exclude_set.
 * NOTE: this function takes no responsibility for an exclude set that is fully 
 *       set.
 */
deque_t *ws_random_deque( char exclude_set[WORKER_COUNT])
{
    int i, j;

    do
    {
        i = rand() % WORKER_COUNT;  
    } while(exclude_set[i] == 1);
    
    return &deques[i];
}
