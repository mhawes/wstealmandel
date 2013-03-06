#include "rtworksteal.h"

pthread_t threads[RT_WORKER_COUNT]; /* set of threads to execute the deques */
pthread_t monitor;

ThreadInfo thread_infos[RT_WORKER_COUNT];

/* -------------------------------------------------------------------------- */
/* thread function */
/* -------------------------------------------------------------------------- */
void *rt_render_thread( void *t_info)
{
    char stealable = 1;
    int work_count = 0;
    ThreadInfo *info = (ThreadInfo *) t_info;


//    printf("T_id %d started\n", info->t_id); 
    
    do
    {
        /* This is where the work is done. */
        work_count += rt_compute_work( info);

        /* After this the thread turns into a thief */
        stealable = rt_become_thief();
    } while(stealable == 1);
    
    
//    printf("T_id %d finished computing %d lines\n", info->t_id, work_count);

    pthread_exit(NULL);
}

/* -------------------------------------------------------------------------- */
unsigned int rt_compute_work( ThreadInfo *ti)
{
    char i;
    unsigned int start, end;
    unsigned int work_count = 0;

    printf("start: %d end: %d\n", ti->start, ti->end);
    i = ti->start;
    end = ti->end;
    
    printf("i = %d\n", i);
    
    
    /* FIXME What the fuck is wrong with this loop? Am I losing my mind? */
    for( i = 0; i < 80; i++);
    {
        printf("i = %d\n", i);

//        compute_line( y, ti->t_id);
        
        work_count++;
    }
    
    return work_count;
}

/* -------------------------------------------------------------------------- */
/* this initialises the threads. */
void ws_initialise_threads()
{
    char i;
    unsigned int block = HEIGHT / RT_WORKER_COUNT;
    unsigned int prev_end = 0;
    
    for( i = 0; i < RT_WORKER_COUNT; i++)
    {
        thread_infos[i].t_id = i;
        thread_infos[i].estimated_complete = MAX_ESTIMATE;

        thread_infos[i].curr = 0;

        /* if this is the last thread distribute right upto the last line 
           This is in-case the height isn't divisable by the worker count */
        thread_infos[i].start = prev_end;
        if( i == RT_WORKER_COUNT - 1){
            thread_infos[i].end   = HEIGHT;
        }
        else{
            prev_end = block * (i + 1);
            thread_infos[i].end = prev_end;
            prev_end++;
        }
        
//        printf("T: %d Start: %d End: %d\n",i, thread_infos[i].start, thread_infos[i].end);
    }
    
    rt_compute_work( &thread_infos[0]);
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
    for(i = 0; i < RT_WORKER_COUNT; i++) {
        pthread_create(&threads[i], &attr, rt_render_thread, (void *)&thread_infos[i]);
    }
    
    /* join point */
    for(i = 0; i < RT_WORKER_COUNT; i++) {
        pthread_join(threads[i], &status);
    }
    
    /* Free attribute and wait for the other threads */
    pthread_attr_destroy( &attr);
}

/* -------------------------------------------------------------------------- */
/* returns 1 if found work and needs to return to worker mode.
 * returns 0 if no work is found in the entire network and the thread can finish.
 */
char rt_become_thief( Deque *deq)
{
    return 0;
}

/* -------------------------------------------------------------------------- */
/* returns 1 if work has been stolen and placed in the deque ready for computing
 * returns 0 if no work is found at this victim.
 */
char rt_victimise( Deque *deq, Deque *victim)
{
    return 0;
}

