#include "rtworksteal.h"

pthread_t threads[WORKER_COUNT]; /* set of threads to execute the deques */
pthread_t monitor;

pthread_mutex_t thief_mut;  /* used to make sure we have 1 thief at a time */

/* used to stop/restart the victim/thief threads */
pthread_mutex_t vi_stop_mut; 
pthread_mutex_t th_stop_mut; 
pthread_cond_t  vi_stop_cond;
pthread_cond_t  th_stop_cond;

ThreadInfo thread_infos[WORKER_COUNT];

char finish_sig;

/* -------------------------------------------------------------------------- */
/* thread functions */
/* -------------------------------------------------------------------------- */
void *rt_render_thread( void *t_info)
{
    int work_count = 0;
    ThreadInfo *info = (ThreadInfo *) t_info;

    printf("T_id %d started\n", info->t_id); 
    
    do
    {
        /* This is where the work is done. */
        work_count += rt_compute_work( info);

        /* After this the thread turns into a thief */
        rt_become_thief( info);
    } while(info->status != WORK_FINISHED);
    
    printf("T_id %d finished computing %d lines\n", info->t_id, work_count);

    pthread_exit(NULL);
}

/* -------------------------------------------------------------------------- */
void *rt_monitor_thread( void *null)
{
    printf("Monitor started\n"); 
    
    char i, is_work = 1;
    ThreadInfo *thief;
    ThreadInfo *victim;
    
    do
    {
        /* Lock as a mechanism for stopping the victim until this is done */ 
        printf("Monitor waiting for thief\n");
        
        thief = rt_wait_for_complete();

        if( thief == NULL){
            rt_broadcast_finished();
            printf("Monitor detected no work");
            break;
        }
        printf("Monitor found thief: %d\n", thief->t_id);

        victim = rt_find_victim();
        printf("Monitor found victim: %d\n", victim->t_id);

        /* if the victim found has an est time of 0 we know all threads are 
         * thieves thus there is no work left 
         */
        if( victim->estimated_complete == 0){
            printf( "No work detected\n");
            is_work = 0;
            
            rt_broadcast_finished();
        }
        else {        
            /* Wait for the finish_line mutex to be unlocked */
            pthread_mutex_lock( &victim->finish_line_mut);
            while( victim->status == IS_VICTIM)
            {
                pthread_cond_wait(&victim->finish_line_cond, &victim->finish_line_mut);
            }
            pthread_mutex_unlock( &victim->finish_line_mut);
            
            rt_distribute( thief, victim);
        }
        
        /* let threads continue */
        pthread_cond_signal( &th_stop_cond);
        pthread_cond_signal( &vi_stop_cond);
        
    } while( is_work == 1);

    printf("Monitor finished\n");

    pthread_exit(NULL);
}

/* -------------------------------------------------------------------------- */
void rt_distribute( ThreadInfo *thief, ThreadInfo *victim)
{
    unsigned int block; 
    unsigned int victim_count;
    
    victim_count = victim->end - victim->curr;
    /* Steal exactly half of the victims work */
    
    printf("VICTIM b: "); 
    rt_print_workload( victim);
    printf("THIEF b:  "); 
    rt_print_workload( thief);
    
    
    if(victim->curr < victim->end - 1){
        block = victim_count / 2;

        thief->end    = victim->end;    
        thief->curr  = victim->curr + block + 1;
        
        victim->end   = victim->curr + block;
        
        thief->status  = THREAD_WORKING;
    }

    victim->status = THREAD_WORKING;

    printf( "steal-count: %u\n", block);
    printf("VICTIM: "); 
    rt_print_workload( victim);
    printf("THIEF:  "); 
    rt_print_workload( thief);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* Sets all threads to finished status */
void rt_broadcast_finished()
{
    char i;
    for( i = 0; i < WORKER_COUNT; i++)
    {
        thread_infos[i].status = WORK_FINISHED;
        rt_print_status(&thread_infos[i]);
        rt_print_workload(&thread_infos[i]);
 
        pthread_mutex_unlock( &th_stop_mut);
        pthread_mutex_unlock( &vi_stop_mut);       
        pthread_mutex_unlock( &thief_mut);
    }        
}

/* -------------------------------------------------------------------------- */
/* Cycles through all the thread infos until it finds one that has a complete 
 * signal. 
 *
 * Returns the thread_info of the complete thread.
 */
ThreadInfo *rt_wait_for_complete()
{
    char i;
    unsigned int count = 0;
    
    do
    {
        for( i = 0; i < WORKER_COUNT; i++)
        {
            if( thread_infos[i].status == THIEF_SIG){
                thread_infos[i].status = IS_THIEF;
                return &thread_infos[i];
            }
            else if( thread_infos[i].status == WORK_FINISHED ){
                count++;
            }
        }
    } while( count < WORKER_COUNT); /* repeat until no work-detected */
    
    return NULL;
}

/* -------------------------------------------------------------------------- */
/* Finds the thread with the highest estimated complete time */
ThreadInfo *rt_find_victim()
{
    char i;
    ThreadInfo *result = &thread_infos[0]; /* take t_id 0 as the default */
    
    for(i = 1; i < WORKER_COUNT; i++)
    {
        if(thread_infos[i].estimated_complete > result->estimated_complete)
        {
            result = &thread_infos[i];
        }
    }
    result->status = VICTIM_SIG;
    
    return result;
}

/* -------------------------------------------------------------------------- */
unsigned int rt_compute_work( ThreadInfo *ti)
{
    unsigned int i;
    unsigned int work_count = 0;

    struct timeval tv_start, tv_end;

    while( ti->curr <= ti->end)
//    for( i = ti->curr; i <= ti->end; i++)
    {
        i = ti->curr;
        
        gettimeofday(&tv_start, NULL);
    
        compute_line( i, ti->t_id);
        ti->curr++;
        work_count++;
        
        /* When this thread gets the victim signal it needs to stop working */
        if( ti->status == VICTIM_SIG)
        {
            rt_become_victim( ti);
            break;
        }
        else if( ti->status != THREAD_WORKING)
        {
            break;
        }
        
        gettimeofday(&tv_end, NULL);
        
        /* THIS IS NOT NICE! FIXME is there a better way to do this? */
        rt_update_estimate( ti, ((tv_end.tv_sec - tv_start.tv_sec)*1000000 + 
                                tv_end.tv_usec - tv_start.tv_usec) / 100);
    } 

    return work_count;
}

/* -------------------------------------------------------------------------- */
void rt_update_estimate( ThreadInfo *ti, unsigned long time)
{
    unsigned int work_count = ti->end - ti->curr;
    
    ti->estimated_complete = work_count * time;
}

/* -------------------------------------------------------------------------- */
/* this initialises the threads. */
void ws_initialise_threads()
{
    char i;
    unsigned int block = HEIGHT / WORKER_COUNT;
    unsigned int prev_end = 0;
    
    /* initialise the global mutexes */
    pthread_mutex_init( &thief_mut, NULL);
    pthread_mutex_init( &th_stop_mut, NULL);
    pthread_mutex_init( &vi_stop_mut, NULL);
    pthread_cond_init( &th_stop_cond, NULL);
    pthread_cond_init( &vi_stop_cond, NULL);
    
    for( i = 0; i < WORKER_COUNT; i++)
    {
        thread_infos[i].t_id = i;
        thread_infos[i].estimated_complete = MAX_ESTIMATE;
        thread_infos[i].status = THREAD_WORKING;        

        pthread_mutex_init( &thread_infos[i].finish_line_mut, NULL);
        pthread_cond_init( &thread_infos[i].finish_line_cond, NULL);

        /* if this is the last thread distribute right upto the last line 
           This is in-case the height isn't divisable by the worker count */
        thread_infos[i].curr = prev_end;
        if( i == WORKER_COUNT - 1){
            thread_infos[i].end   = HEIGHT - 1;
        }
        else{
            prev_end = block * (i + 1);
            thread_infos[i].end = prev_end;
            prev_end++;
        }
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
    
    pthread_create(&monitor, &attr, rt_monitor_thread, (void *)NULL);
    for(i = 0; i < WORKER_COUNT; i++) {
        pthread_create(&threads[i], &attr, rt_render_thread, (void *)&thread_infos[i]);
    }
    
    /* join point */
    pthread_join( monitor, &status);
    for(i = 0; i < WORKER_COUNT; i++) {
        pthread_join(threads[i], &status);
    }
    
    /* Free attribute and wait for the other threads */
    pthread_attr_destroy( &attr);
}

/* -------------------------------------------------------------------------- */
/* Issues the work complete signal and waits until its status is changed from
 * THIEF_SIG.
 */
void rt_become_thief( ThreadInfo *ti)
{
    /* Set the estimated time to 0. This helps the monitor detect no-remaining work. */
    ti->estimated_complete = 0;

    /* Attempt to lock the thief mutex.
     * This ensures only one thread can become a thief at a time */
    pthread_mutex_lock( &thief_mut);
    ti->status = THIEF_SIG;
    
    /* Stop mutex makes sure we are stopped here. */
    pthread_mutex_lock( &th_stop_mut);
    while (ti->status == IS_THIEF)
    {
        pthread_cond_wait(&th_stop_cond, &th_stop_mut);
    }
    
    pthread_mutex_unlock( &th_stop_mut);
    
    pthread_mutex_unlock( &thief_mut);    
}

/* -------------------------------------------------------------------------- */
/*
 */
void rt_become_victim( ThreadInfo *ti)
{
    ti->status = IS_VICTIM;

    pthread_cond_signal( &ti->finish_line_cond);

    /* make this wait for the stop mutex to be unlocked */    
    pthread_mutex_lock( &vi_stop_mut);
    
    while (ti->status == IS_VICTIM)
    {
        pthread_cond_wait(&vi_stop_cond, &vi_stop_mut);
    }
    
    pthread_mutex_unlock( &vi_stop_mut);
}

/* -------------------------------------------------------------------------- */
/* UTIL FUNCTIONS: */
/* -------------------------------------------------------------------------- */
void rt_print_status( ThreadInfo *ti)
{
    printf("t_id %d status: ", ti->t_id);
    switch( ti->status)
    {
        case WORK_FINISHED :
            printf("FINISHED\n");
            break;
        case THIEF_SIG :
            printf("THIEF (waiting)\n");
            break;
        case IS_THIEF :
            printf("THIEF\n");
            break;
        case VICTIM_SIG :
            printf("VICTIM (waiting)\n");
            break;
        case IS_VICTIM :
            printf("VICTIM\n");
            break;
        case THREAD_WORKING :
            printf("WORKING\n");
    }
}

void rt_print_workload( ThreadInfo *ti)
{
    printf("t_id %d curr: %u end: %u\n",ti->t_id,
                                        ti->curr,
                                        ti->end);
}

