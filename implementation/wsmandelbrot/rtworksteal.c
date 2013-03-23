#include "rtworksteal.h"

pthread_t threads[WORKER_COUNT]; /* set of threads to execute the deques */
pthread_t monitor;

pthread_mutex_t thief_mut;  /* used to make sure we have 1 thief at a time */

/* used to stop/restart the victim/thief threads */
pthread_mutex_t vi_stop_mut; 
pthread_mutex_t th_stop_mut; 
pthread_cond_t  vi_stop_cond;
pthread_cond_t  th_stop_cond;

pthread_barrier_t stop_bar;

thread_info_t thread_infos[WORKER_COUNT];

char finish_sig;

/* -------------------------------------------------------------------------- */
/* thread functions */
/* -------------------------------------------------------------------------- */
void *rt_render_thread( void *t_info)
{
    int work_count = 0;
    thread_info_t *info = (thread_info_t *) t_info;

    printf("T_id %d started\n", info->t_id); 
    
    do
    {
        /* This is where the work is done. */
        work_count += rt_compute_work( info);

        /* After this the thread turns into a thief */
        if( info->curr >= info->end){
            rt_become_thief( info);
        }
    } while(finish_sig != 1);
    
    printf("T_id %d finished computing %d lines\n", info->t_id, work_count);

    pthread_exit(NULL);
}

/* -------------------------------------------------------------------------- */
void *rt_monitor_thread( void *null)
{
    printf("Monitor started\n"); 
    
    char i, is_work = 1;
    thread_info_t *thief;
    thread_info_t *victim;
    
    do
    {
        //printf("Monitor waiting for thief\n");
        
        thief = rt_wait_for_complete();

        if( thief == NULL){
            rt_broadcast_finished();
//            printf("Monitor detected no work");
            break;
        }

        //printf("Monitor found thief: %d\n", thief->t_id);

        victim = rt_find_victim();
        
        if( victim == NULL){
            rt_broadcast_finished();
            //printf("Monitor detected no work after no victim\n");
            break;
        }
        //printf("Monitor found victim: %d\n", victim->t_id);
/*
        rt_print_status(thief);
        rt_print_workload( thief);
        rt_print_status(victim);
        rt_print_workload( victim);
*/
        /* Wait for the finish_line mutex to be unlocked */
        /*pthread_mutex_lock( &victim->finish_line_mut);
        while( victim->status == IS_VICTIM)
        {
            pthread_cond_wait(&victim->finish_line_cond, &victim->finish_line_mut);
        }
        pthread_mutex_unlock( &victim->finish_line_mut); */

        /* wait at this barrier for the victim to finish the line */        
        pthread_barrier_wait(&victim->line_bar);
        
        rt_distribute( thief, victim);
        
        /* let threads continue */
        pthread_barrier_wait(&stop_bar);
        
    } while( is_work == 1);

    printf("Monitor finished\n");

    pthread_exit(NULL);
}

/* -------------------------------------------------------------------------- */
void rt_distribute( thread_info_t *thief, thread_info_t *victim)
{
    unsigned int block; 
    unsigned int victim_count;
    
    victim_count = victim->end - victim->curr;
    /* Steal exactly half of the victims work */
    
    /*
    printf("VICTIM b: "); 
    rt_print_workload( victim);
    printf("THIEF b:  "); 
    rt_print_workload( thief);
    */
    
    if(victim->curr < victim->end){
        block = victim_count / 2;

        thief->end    = victim->end;    
        thief->curr  = victim->curr + block + 1;
        
        victim->end   = victim->curr + block;
    }
    
    thief->status  = THREAD_WORKING;
    victim->status = THREAD_WORKING;
    
    /*
    printf( "steal-count: %u\n", block);
    printf("VICTIM: "); 
    rt_print_workload( victim);
    printf("THIEF:  "); 
    rt_print_workload( thief);
    */
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* Sets all threads to finished status */
void rt_broadcast_finished()
{
    char i;
    
    //printf("FINISH BROADCAST\n");
    
    for( i = 0; i < WORKER_COUNT; i++)
    {
        
        //rt_print_status(&thread_infos[i]);
        //rt_print_workload(&thread_infos[i]);
 
        finish_sig = 1;
        //pthread_barrier_wait(&thread_infos[i].line_bar);
        
        thread_infos[i].status = WORK_FINISHED;
        
        /*
        pthread_mutex_unlock( &th_stop_mut);
        pthread_mutex_unlock( &vi_stop_mut);       
        pthread_mutex_unlock( &thief_mut);
        */
    }        
    
    pthread_barrier_wait(&stop_bar);
}

/* -------------------------------------------------------------------------- */
/* Cycles through all the thread infos until it finds one that has a complete 
 * signal. 
 *
 * Returns the thread_info of the complete thread.
 */
thread_info_t *rt_wait_for_complete()
{
    char i;
    thread_info_t *ti = NULL;
    
    do
    {
        for( i = 0; i < WORKER_COUNT; i++)
        {
            if( thread_infos[i].status == THIEF_SIG){
                thread_infos[i].status = IS_THIEF;
                return &thread_infos[i];
            }
        }
    } while( ti == NULL && finish_sig != 1); /* repeat until no work-detected */
    
    return ti;
}

/* -------------------------------------------------------------------------- */
/* Finds the thread with the highest estimated complete time */
thread_info_t *rt_find_victim()
{
    char i, count = 0;

    thread_info_t *result = &thread_infos[0]; /* take t_id 0 as the default */
    
    for(i = 0; i < WORKER_COUNT; i++)
    {   
        if( thread_infos[i].curr >= thread_infos[i].end){
            count++;
        }
        else if( thread_infos[i].estimated_complete > result->estimated_complete)
        {
            result = &thread_infos[i];
        }
    }
    
    if( count == WORKER_COUNT){
        return NULL;
    }
    result->status = VICTIM_SIG;
    
    return result;
}

/* -------------------------------------------------------------------------- */
unsigned int rt_compute_work( thread_info_t *ti)
{
    unsigned int i;
    unsigned int work_count = 0;

    struct timeval tv_start, tv_end;

    while( ti->curr <= ti->end && finish_sig != 1)
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
        
        gettimeofday(&tv_end, NULL);
        
        /* THIS IS NOT NICE! FIXME is there a better way to do this? */
        rt_update_estimate( ti, ((tv_end.tv_sec - tv_start.tv_sec)*1000000 + 
                                tv_end.tv_usec - tv_start.tv_usec) / 100);
    }
    
    return work_count;
}

/* -------------------------------------------------------------------------- */
void rt_update_estimate( thread_info_t *ti, unsigned long time)
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
    
    finish_sig = 0;
    
    pthread_barrier_init(&stop_bar, NULL, 3);
    
    for( i = 0; i < WORKER_COUNT; i++)
    {
        thread_infos[i].t_id = i;
        thread_infos[i].estimated_complete = MAX_ESTIMATE;
        thread_infos[i].status = THREAD_WORKING;        

        pthread_mutex_init( &thread_infos[i].finish_line_mut, NULL);
        pthread_cond_init( &thread_infos[i].finish_line_cond, NULL);
        
        pthread_barrier_init( &thread_infos[i].line_bar, NULL, 2);

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
    
    for(i = 0; i < WORKER_COUNT; i++) {
        pthread_join(threads[i], &status);
    }
    pthread_join( monitor, &status);    
    
    /* Free attribute and wait for the other threads */
    pthread_attr_destroy( &attr);
}

/* -------------------------------------------------------------------------- */
/* Issues the work complete signal and waits until its status is changed from
 * THIEF_SIG.
 */
void rt_become_thief( thread_info_t *ti)
{
    /* Set the estimated time to 0. 
     * This stops the monitor from picking the thief as a victim 
     */
    ti->estimated_complete = 0;

    /* Attempt to lock the thief mutex.
     * This ensures only one thread can become a thief at a time 
     * The others have to wait.
     */
//    pthread_mutex_lock( &thief_mut);
    ti->status = THIEF_SIG;
    
    /* Stop mutex makes sure we are stopped here. */
    /*pthread_mutex_lock( &th_stop_mut);
    while (ti->status == IS_THIEF)
    {
        pthread_cond_wait(&th_stop_cond, &th_stop_mut);
    }
    
    pthread_mutex_unlock( &th_stop_mut); */
    
    /* wait for thief, victim, and monitor to call this function */
    pthread_barrier_wait(&stop_bar);
    
    /* Allow waiting thieves to become a thief */
//    pthread_mutex_unlock( &thief_mut);    
}

/* -------------------------------------------------------------------------- */
/*
 */
void rt_become_victim( thread_info_t *ti)
{
    ti->status = IS_VICTIM;

//    pthread_cond_signal( &ti->finish_line_cond);

    /* when monitor and victim reach this barrier the line has ended */
    pthread_barrier_wait(&ti->line_bar);

    /* make this wait for the stop mutex to be unlocked */    
    /*pthread_mutex_lock( &vi_stop_mut);
        
    while (ti->status == IS_VICTIM)
    {
        pthread_cond_wait(&vi_stop_cond, &vi_stop_mut);
    }
    
    pthread_mutex_unlock( &vi_stop_mut);*/
    
    /* wait for thief, victim, and monitor to call this function */
    pthread_barrier_wait(&stop_bar);
}

/* -------------------------------------------------------------------------- */
/* UTIL FUNCTIONS: */
/* -------------------------------------------------------------------------- */
void rt_print_status( thread_info_t *ti)
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

void rt_print_workload( thread_info_t *ti)
{
    printf("t_id %d curr: %u end: %u\n",ti->t_id,
                                        ti->curr,
                                        ti->end);
}

