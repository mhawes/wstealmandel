#include "noscheduling.h"

pthread_t threads[WORKER_COUNT]; /* set of threads to execute */
thread_info_t infos[WORKER_COUNT];

/* -------------------------------------------------------------------------- */

void *ws_worker_thread( void* tia)
{
    int y;
    thread_info_t *ti = (thread_info_t *) tia;

    printf("T_id %d started\n", ti->t_id); 

    for(y = ti->start_y; y < ti->end_y; y++)
    {
        compute_line( y, ti->t_id);
    }
    
    printf("T_id %d finished computing %d lines\n", ti->t_id, ti->end_y - ti->start_y);

    pthread_exit(NULL);
}

/* -------------------------------------------------------------------------- */

void ws_initialise_threads()
{
    unsigned int i, distribution = HEIGHT / WORKER_COUNT;
    thread_info_t ti;

    /* distribute the work evenly in blocks */
    for( i = 0; i < WORKER_COUNT; i++)
    {
        ti.t_id    = i;
        ti.start_y = i * distribution;
        ti.end_y   = ((i + 1) * distribution);
        
        if( i == WORKER_COUNT - 1){
            ti.end_y = HEIGHT;
        }

        infos[i] = ti;
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
        pthread_create(&threads[i], &attr, ws_worker_thread, (void *)&infos[i]);
    }
    
    /* join point */
    for(i=0; i<WORKER_COUNT; i++) {
        pthread_join(threads[i], &status);
    }
    
    /* Free attribute */
    pthread_attr_destroy( &attr);
}

