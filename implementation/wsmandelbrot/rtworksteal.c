#include "rtworksteal.h"

pthread_t threads[WORKER_COUNT]; /* set of threads to execute the deques */
pthread_t monitor;
pthread_attr_t attr;
thread_info_t thread_infos[WORKER_COUNT];

pthread_mutex_t thief_mut;  /* used to make sure we have 1 thief at a time */

/* used to stop/restart the victim/thief threads  FIXME*/
/*pthread_mutex_t vi_stop_mut; 
pthread_mutex_t th_stop_mut; 
pthread_cond_t  vi_stop_cond;
pthread_cond_t  th_stop_cond;
*/

pthread_mutex_t mon_wait_mut; 
pthread_cond_t  mon_wait_cond;
pthread_mutex_t thief_sig_mut;
pthread_cond_t  thief_sig_cond;

pthread_mutex_t the_one_thief_mut;

pthread_barrier_t stop_bar;

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
        if( info->curr >= info->end){
            /* this is when a thread becomes a thief */
            rt_become_thief( info);
        }
        else{
            /* This is where the work is done. */
            work_count += rt_compute_work( info);
        }
    } while(finish_sig != 1);
    
    info->sta_finished = 1;

    printf("T_id %d finished computing %d lines\n", info->t_id, work_count);

    pthread_exit(NULL);
}

/* -------------------------------------------------------------------------- */
void *rt_monitor_thread( void *null)
{
    printf("Monitor started\n"); 

    void *status;
    
    char i, is_work = 1;
    char alive_count = WORKER_COUNT;    // used to catch straglers
    
    
    thread_info_t *thief;
    thread_info_t *victim;
    
    /* start threads */
    for(i = 0; i < WORKER_COUNT; i++) {
        pthread_create(&threads[i], &attr, rt_render_thread, (void *)&thread_infos[i]);
    }
    
    do
    {
        printf("Monitor waiting for thief\n");
        
        thief = rt_wait_for_thief();

        if( thief == NULL){
            rt_broadcast_finished();
            printf("Monitor detected no work");
            break;
        }
        
        printf("Monitor found thief: %d\n", thief->t_id);

        victim = rt_find_victim();
        
        if( victim == NULL){
            rt_broadcast_finished();
            printf("Monitor detected no work after no victim\n");
            break;
        }
        
        printf("Monitor found victim: %d\n", victim->t_id);

        /* Wait for the finish_line mutex to be unlocked */
        pthread_mutex_lock( &mon_wait_mut);
        printf("Monitor waiting for victim to finish line\n");
        victim->sta_v_sig = 1;
        rt_print_workload( victim);
        while( victim->sta_v_sig)
        {
            pthread_cond_wait(&mon_wait_cond, &mon_wait_mut);
        }
        pthread_mutex_unlock( &mon_wait_mut); 
        printf("Monitor got signal from victim\n");

/*
        rt_print_status(thief);
        rt_print_workload( thief);
        rt_print_status(victim);
        rt_print_workload( victim);
*/

        pthread_mutex_lock( &thief->stop_mut);        
        pthread_mutex_lock( &victim->stop_mut);

        rt_distribute( thief, victim);
        
        /* restart the threads */
        pthread_cond_signal( &victim->stop_cond);
        pthread_cond_signal( &thief->stop_cond);
        pthread_mutex_unlock( &victim->stop_mut);
        pthread_mutex_unlock( &thief->stop_mut);
        
        printf("Monitor finished distribution and restarted threads\n");
        
        /* let threads continue */
        //pthread_barrier_wait(&stop_bar);
        
    } while( is_work == 1);

    printf("Monitor is cleaning up the straglers\n");
    /* wait for stranglers before joining */
    do
    {
        //thief = rt_wait_for_thief();
        pthread_cond_signal( &thief_sig_cond);
        
        for( i = 0; i < WORKER_COUNT; i++)
        {
            pthread_cond_signal( &thread_infos[i].stop_cond);
            pthread_cond_signal( &thread_infos[i].stop_cond);
        
            if( thread_infos[i].sta_finished){
                printf("%d\n", alive_count);
                alive_count--;
            }
        }
    } while( alive_count > 0);
    

    /* join point */
    for(i = 0; i < WORKER_COUNT; i++) {
        pthread_join(threads[i], &status);
    }

    printf("Monitor finished\n");

    pthread_exit(NULL);
}

/* -------------------------------------------------------------------------- */
void rt_distribute( thread_info_t *thief, thread_info_t *victim)
{
    unsigned int block; 
    unsigned int victim_count;
    
    /* calculate the work count of the victim */
    victim_count = victim->end - victim->curr;
    
    /*
    printf("VICTIM b: "); 
    rt_print_workload( victim);
    printf("THIEF b:  "); 
    rt_print_workload( thief);
    */

    /* Steal exactly half of the victims work */    
    if(victim->curr < victim->end){
        block = victim_count / 2;

        /* re-assign the work */
        thief->end  = victim->end;    
        thief->curr = victim->curr + block + 1;
        victim->end = victim->curr + block;
    }
    
//    thief->status  = THREAD_WORKING;
//    victim->status = THREAD_WORKING;
    
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
    
    //    pthread_barrier_wait(&stop_bar);
}

/* -------------------------------------------------------------------------- */
/* Cycles through all the thread infos until it finds one that has a complete 
 * signal. 
 *
 * Returns the thread_info of the complete thread.
 */
thread_info_t *rt_wait_for_thief()
{
    char i;
    thread_info_t *ti = NULL;
    
    do
    {
        for( i = 0; i < WORKER_COUNT; i++)
        {
            if( thread_infos[i].status == THIEF_SIG){
                thread_infos[i].status = IS_THIEF;
                ti = &thread_infos[i];
                pthread_cond_signal( &thief_sig_cond);
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
        else if( thread_infos[i].estimated_complete > result->estimated_complete &&
                 thread_infos[i].sta_working)
        {
            thread_infos[i].sta_working = 0;
            result = &thread_infos[i];
        }
    }
    
    /* if I have counted WORKER_COUNT worth of threads that are empty it means
     * we can complete
     */
    if( count == WORKER_COUNT){
        return NULL;
    }
    
    /* set the vitim sig and let the victim finish its current line */
    result->status = VICTIM_SIG;
    
    return result;
}

/* -------------------------------------------------------------------------- */
unsigned int rt_compute_work( thread_info_t *ti)
{
    unsigned int i;
    unsigned int work_count = 0;

    struct timeval tv_start, tv_end;

    printf("t_id %d started doing work\n", ti->t_id);
    ti->sta_working = 0;

    while( ti->curr <= ti->end && finish_sig != 1 && ti->sta_v_sig == 0)
    {
        i = ti->curr;
        
        gettimeofday(&tv_start, NULL);
    
        compute_line( i, ti->t_id);
        ti->curr++;
        work_count++;
        
        gettimeofday(&tv_end, NULL);
        
        /* THIS IS NOT NICE! FIXME is there a better way to do this? */
        rt_update_estimate( ti, ((tv_end.tv_sec - tv_start.tv_sec)*1000000 + 
                                tv_end.tv_usec - tv_start.tv_usec) / 100);
    }
    
    /* When this thread gets the victim signal it needs to stop working now */
    if( ti->sta_v_sig)
    {
        ti->sta_v_sig = 0;
        rt_become_victim( ti);
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
    /*pthread_mutex_init( &thief_mut, NULL);
    pthread_mutex_init( &th_stop_mut, NULL);
    pthread_mutex_init( &vi_stop_mut, NULL);
    pthread_cond_init( &th_stop_cond, NULL);
    pthread_cond_init( &vi_stop_cond, NULL);*/
    
    pthread_mutex_init( &mon_wait_mut, NULL);
    pthread_cond_init( &mon_wait_cond, NULL);
    pthread_mutex_init( &thief_sig_mut, NULL);
    pthread_cond_init( &thief_sig_cond, NULL);

    pthread_mutex_init( &the_one_thief_mut, NULL);

    finish_sig = 0;
    
//    pthread_barrier_init(&stop_bar, NULL, 3);
    
    for( i = 0; i < WORKER_COUNT; i++)
    {
        thread_infos[i].t_id = i;
        thread_infos[i].estimated_complete = MAX_ESTIMATE;
        thread_infos[i].status = THREAD_WORKING;

        thread_infos[i].sta_t_sig = 0;
        thread_infos[i].sta_v_sig = 0;
        thread_infos[i].sta_stop = 0;
        thread_infos[i].sta_working = 0;

        pthread_mutex_init( &thread_infos[i].stop_mut, NULL);
        pthread_cond_init( &thread_infos[i].stop_cond, NULL);
        
        //pthread_barrier_init( &thread_infos[i].line_bar, NULL, 2);

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
    void *status;

    /* Initialize and set thread detached attribute */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    /* start threads */
    /* NB: the monitor thread has responsibility for starting the workers */
    pthread_create(&monitor, &attr, rt_monitor_thread, (void *)NULL);
    
    /* join the monitor thread */
    pthread_join(monitor, &status);

    /* Free attribute and wait for the other threads */
    pthread_attr_destroy( &attr);    
}

/* -------------------------------------------------------------------------- */
/* Issues the work complete signal and waits until its status is changed from
 * THIEF_SIG.
 */
void rt_become_thief( thread_info_t *ti)
{
    pthread_mutex_lock( &the_one_thief_mut);
    printf("t_id %d locked the_one_thief_mut\n", ti->t_id);

    /* Attempt to lock the thief_sig mutex.
     * Wait until chosen as a thief by the monitor.
     */ 
    
    pthread_mutex_lock( &thief_sig_mut);
    printf("t_id %d locked thief_sig_mut\n", ti->t_id);
    
    ti->status = THIEF_SIG;
    ti->sta_t_sig = 1;

    /* Set the estimated time to 0. 
     * This stops the monitor from picking the thief as a victim 
     */
    ti->estimated_complete = 0;
    
    while (ti->sta_t_sig)
    {
        pthread_cond_wait(&thief_sig_cond, &thief_sig_mut);
        ti->sta_t_sig = 0;
    }
    pthread_mutex_unlock( &thief_sig_mut);
    printf("t_id %d un-locked thief_sig_mut\n", ti->t_id);
    
    /* make the thread wait while the distribution is being done */
    pthread_mutex_lock( &ti->stop_mut);
    printf("t_id %d stopped (T)\n", ti->t_id);
    ti->status = THIEF_SIG;
    ti->sta_stop = 1;
    while (ti->sta_stop)
    {
        pthread_cond_wait(&ti->stop_cond, &ti->stop_mut);
        ti->sta_stop = 0;
    }
    
    /* set the status to working */    
    ti->status = THREAD_WORKING;
    
    pthread_mutex_unlock( &ti->stop_mut);
    pthread_mutex_unlock( &the_one_thief_mut);
    printf("t_id %d un-locked the_one_thief_mut\n", ti->t_id);
    printf("t_id %d re-started (T)\n", ti->t_id);
    
    
    
    /* Stop mutex makes sure we are stopped here. */
    /*pthread_mutex_lock( &th_stop_mut);
    while (ti->status == IS_THIEF)
    {
        pthread_cond_wait(&th_stop_cond, &th_stop_mut);
    }
    
    pthread_mutex_unlock( &th_stop_mut); */
    
    /* wait for thief, victim, and monitor to call this function */
//    pthread_barrier_wait(&stop_bar);
    
    /* Allow waiting thieves to become a thief */
//    pthread_mutex_unlock( &thief_mut);    
}

/* -------------------------------------------------------------------------- */
/*
 */
void rt_become_victim( thread_info_t *ti)
{
    /* make the thread wait while the distribution is being done */
    pthread_mutex_lock( &ti->stop_mut);
    
    /* signal the monitor that it can start distributing */
    pthread_cond_signal(&mon_wait_cond);    
    
    printf("t_id %d stopped (V)\n", ti->t_id);
    ti->status = IS_VICTIM;
    ti->sta_stop = 1;
    while (ti->sta_stop)
    {
        pthread_cond_wait(&ti->stop_cond, &ti->stop_mut);
        ti->sta_stop = 0;
    }
    /* set the status to working */    
    ti->status = THREAD_WORKING;
    
    pthread_mutex_unlock( &ti->stop_mut);
    printf("t_id %d re-started (V)\n", ti->t_id);

//    pthread_cond_signal( &ti->finish_line_cond);

    /* when monitor and victim reach this barrier the line has ended */
//    pthread_barrier_wait(&ti->line_bar);

    /* make this wait for the stop mutex to be unlocked */    
    /*pthread_mutex_lock( &vi_stop_mut);
        
    while (ti->status == IS_VICTIM)
    {
        pthread_cond_wait(&vi_stop_cond, &vi_stop_mut);
    }
    
    pthread_mutex_unlock( &vi_stop_mut);*/
    
    /* wait for thief, victim, and monitor to call this function */
    //pthread_barrier_wait(&stop_bar);
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
            printf("THIEF SIG\n");
            break;
        case IS_THIEF :
            printf("THIEF\n");
            break;
        case VICTIM_SIG :
            printf("VICTIM SIG\n");
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

