#include "mandelbrot.h"

static unsigned int plane[HEIGHT][WIDTH];/* array to hold the generated image */
static Pixel plane_col[HEIGHT][WIDTH];/* array to hold the generated image in colour */
static Deque deques[WORKER_COUNT]; /* Set of deques to fill. One per trhead. */
pthread_t threads[WORKER_COUNT]; /* set of threads to execute the deques */

pthread_mutex_t started_mut;
char started_count = 0;

/* -------------------------------------------------------------------------- */
/* CODE STARTS HERE:                                                          */
/* -------------------------------------------------------------------------- */

int
main()
{
    Complex c_max;
    Complex c_min;
    Complex c_factor;

/* TEST CODE */
/*    Deque d;
    
    de_initialise( &d, 0);
    
    de_test_deque( &d);   
    
    //int rc = pthread_create(&t0, NULL, compute_work, (void *)&d);
*/
/* END HERE */

    /* ------------------------------------- */
    initialise( &c_max, &c_min, &c_factor);
    
    compute_plane( &c_max, &c_min, &c_factor);

    /* write it out to a PGM file */
    //write_to_pgm();
    
    write_to_ppm();
    
    pthread_exit(NULL);
    return (0);    
}

/* -------------------------------------------------------------------------- */
void initialise( Complex *c_max, Complex *c_min, Complex *c_factor)
{
    c_min->re = -2;
    c_max->re = 2;

    c_min->im = -2;
    c_max->im = c_min->im + (c_max->re - c_min->re) * HEIGHT / WIDTH;

    /* used to convert x, y of the raster plane into a complex number */
    c_factor->re = (c_max->re - c_min->re) / (WIDTH - 1);
    c_factor->im = (c_max->im - c_min->im) / (HEIGHT - 1);
    
    /* set a seed for the rand function */
    srand( 938672); /* FIXME */
    
    pthread_mutex_init( &started_mut, NULL); 
}

/* -------------------------------------------------------------------------- */
inline double convert_y_coord( double im_max, 
                        double im_factor, 
                        unsigned int y)
{
    return im_max - y * im_factor;
}

/* -------------------------------------------------------------------------- */
inline double convert_x_coord( double re_min, 
                        double re_factor, 
                        unsigned int x)
{
    return re_min + x * re_factor;
}

/* -------------------------------------------------------------------------- */
void compute_plane( Complex *c_max, 
                    Complex *c_min, 
                    Complex *c_factor)
{
    pthread_attr_t attr;
    void *status;
    unsigned int x, y, i = 0;
    char rad_flag = 0;
    Line line;

    /* initialise all deques */
    for (i = 0; i < WORKER_COUNT; i++)
    {
        de_initialise( &deques[i], i);
    }

/* THIS NEEDS SERIOUSLY CLEANING UP! TODO */  
    for( y = 0; y < HEIGHT ; y++)
    {
        line.y = y;
        
        /* distribute the current line y to one of the deques */
        de_push_bottom( &deques[0], line);
    }

    /* Initialize and set thread detached attribute */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    ThreadLoad tl[WORKER_COUNT];
    for (i = 0; i < WORKER_COUNT; i++)
    {
        tl[i].t_id = i;
        tl[i].c_max = c_max;
        tl[i].c_min = c_min;
        tl[i].c_factor = c_factor;
        tl[i].deq = &deques[i];
    }

    for(i=0; i<WORKER_COUNT; i++) {
        pthread_create(&threads[i], &attr, worker_thread, (void *)&tl[i]);
    }
    
    for(i=0; i<WORKER_COUNT; i++) {
        pthread_join(threads[i], &status);
    }

    /* Free attribute and wait for the other threads */
    pthread_attr_destroy( &attr);
}

/* -------------------------------------------------------------------------- */
/* returns PPM_BLACK if in the mandelbrot set. 
 * returns the iteration count when not in the set. This produces a nice 
 * gradient effect.
 */
char is_member(Complex c)
{
    char i;
    Complex z;

    z.re = c.re; 
    z.im = c.im;

    for(i = MAX_ITERATIONS; i >= 0; i--)
    {
        if( ((z.re * z.re) + (z.im * z.im)) > 4)
        {
            /* c is not a member of the set */
            return i;
        }
        
        z = julia_func( z, c);
    }
    
    /* if it gets this far c is a member of the set */
    return MAX_ITERATIONS;
}

/* -------------------------------------------------------------------------- */
/* returns 1 if outside the circle of radius 2 */
inline char is_outside_rad2( Complex *c)
{
    /* if the number is outside the radius 2 we know it cant be in the set */
    return( sqrt((c->re * c->re) + (c->im * c->im)) > 2);
}

/* -------------------------------------------------------------------------- */
/* calculates the next value of: z = z^2 + c */
Complex julia_func(Complex z, Complex c)
{
    Complex res;
    
    res.im = 2 * z.re * z.im + c.im;
    res.re = (z.re * z.re) - (z.im *z.im)  + c.re;

    return res;
}
/* -------------------------------------------------------------------------- */
/* thread functions */
/* -------------------------------------------------------------------------- */
void *worker_thread( void *thread_load)
{
    ThreadLoad *tl = (ThreadLoad *) thread_load;
    int stealable = 1;
    Deque *deq = tl->deq;

    printf("T_id %d started\n", deq->t_id); 
    
    pthread_mutex_lock(&started_mut);
    started_count++;
    pthread_mutex_unlock (&started_mut);
    
    while(stealable == 1)
    {
        /* This is where the work is done. */
        compute_deque( deq, tl->c_max, tl->c_min, tl->c_factor);

        /* After this the thread turns into a thief */
        stealable = become_thief( deq);
    }
    
    printf("T_id %d finished\n", deq->t_id);
    
    pthread_exit(NULL);
}

/* -------------------------------------------------------------------------- */
void compute_deque( Deque *deq, Complex *c_max, Complex *c_min, Complex *c_factor)
{
//    printf( "T %d BECAME WORKER\n", deq->t_id);

    Complex c_cur;  /* used as the current c value */
    unsigned int x;
    Line line_cur;

    while(1)
    {
        line_cur = de_pop_bottom( deq);
        
        if(line_cur.status == LINE_EMPTY){
            break;
        }
        
        c_cur.im = convert_y_coord( c_max->im, c_factor->im, line_cur.y);
        for( x = 0; x < WIDTH; x++)
        {
            c_cur.re = convert_x_coord( c_min->re, c_factor->re, x);
            
            if( is_outside_rad2( &c_cur)){
                plane_col[line_cur.y][x].red = 0;  /* make the outer grey */
                plane_col[line_cur.y][x].blue = 0;  /* make the outer grey */
            }
            else{
                /* negate shade of grey depending on the thread which processed it. */ 
                /*if( deq->t_id % 2 == 0){
                    plane[line_cur.y][x] = is_member( c_cur);
                } 
                else {
                    plane[line_cur.y][x] = MAX_ITERATIONS - is_member( c_cur);
                }*/
                
                /* place colour pixel */
                switch( deq->t_id % 4){
                    case 0:
                        plane_col[line_cur.y][x].red = is_member( c_cur);
                        plane_col[line_cur.y][x].blue = 0;
                        break;
                    case 1:
                        plane_col[line_cur.y][x].red = MAX_ITERATIONS - is_member( c_cur);
                        plane_col[line_cur.y][x].blue = 0;
                        break;
                    case 2:
                        plane_col[line_cur.y][x].red = 0;
                        plane_col[line_cur.y][x].blue = is_member( c_cur);
                        break;
                    case 3:
                        plane_col[line_cur.y][x].red = 0;
                        plane_col[line_cur.y][x].blue = MAX_ITERATIONS - is_member( c_cur);
                }
            }
        }
    }

}

/* -------------------------------------------------------------------------- */
/* returns 1 if found work and needs to return to worker mode.
 * returns 0 if no work is found in the entire network and the thread can finish.
 */
char become_thief( Deque *deq)
{
//    printf( "T %d BECAME THIEF\n", deq->t_id);

    int i;
    int try_count = 0;
    char result = 0, ex_count = 1;
    Deque *victim;
    
    char exclude_set[WORKER_COUNT];
    for(i = 0; i < WORKER_COUNT; i++)
    {
        exclude_set[i] = 0;
    }
    exclude_set[deq->t_id] = 1;

    while( try_count < 1000 && result == 0)
    {
        
        if( ex_count < WORKER_COUNT ){
            victim = random_deque( exclude_set);
            result = victimise( deq, victim);
        }
        else{
            return 0;
        }

        /* add it to the exclude set providing all threads are started */
        if( started_count == WORKER_COUNT ){
            exclude_set[victim->t_id] = 1;
            ex_count++;
        }
        if(result == 1){
            try_count = 0;
        }
        else{
            try_count++;
        }

    }
    
    return 1;
}

/* -------------------------------------------------------------------------- */
/* returns 1 if work has been stolen and placed in the deque ready for computing
 * returns 0 if no work is found at this victim.
 */
char victimise( Deque *deq, Deque *victim)
{
    char result = 0;
    Line line;
    int steal_size;
    int fill_count = 0;

    line = de_steal( victim);
    
    if( line.status == LINE_EMPTY ){
        return 0;
    }
    
    /* evaluate victim size to work out how much to steal */
    steal_size = (victim->bot - victim->top) / WORKER_COUNT;
    
    while( line.status != LINE_EMPTY )
    {
        /* if we have a normal line push it onto this threads deque */
        if( line.status == LINE_NORMAL){
            //printf("T %d STOLE FROM T %d and got normal signal\n", deq->t_id, victim->t_id);
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
            usleep(100); /* sleep for a bit before trying again */
            line = de_steal( victim);
        }
    }

    return 0;
}

/* -------------------------------------------------------------------------- */
/* 
 * Generates a random number between 0 and WORKER_COUNT (not inclusive)
 * and returns a deque that is NOT the same as any in the exclude_set.
 * this function takes no responsibility for an exclude set that is fully 
 * set.
 */
Deque *random_deque( char exclude_set[WORKER_COUNT])
{
    int i, j;

    do
    {
        i = rand() % WORKER_COUNT;  
    } while(exclude_set[i] == 1);
    
    return &deques[i];
}

/* -------------------------------------------------------------------------- */
/* UTIL FUNCTIONS:                                                           */
/* -------------------------------------------------------------------------- */
void print_complex(Complex *com)
{
    printf("(%g + %g*i) ", com->re, com->im);
}

/* -------------------------------------------------------------------------- */
void write_to_pgm()
{
    unsigned int x, y;

    FILE *fp = fopen("out.ppm", "w+");
    fprintf(fp, "P2\n%d %d\n%d\n", WIDTH, HEIGHT, MAX_ITERATIONS);
    
    /* classic nested for loop approach */
    for(y = 0; y < HEIGHT; ++y)    
    {
        for(x = 0; x < WIDTH; ++x)
        {
            fprintf(fp, "%i ", plane[y][x]);
        }
        fprintf(fp, "\n");
    }
    
    fclose(fp); 
}

/* -------------------------------------------------------------------------- */
void write_to_ppm()
{
    unsigned int x, y;

    FILE *fp = fopen("out.ppm", "w+");
    fprintf(fp, "P3\n%d %d\n%d\n", WIDTH, HEIGHT, MAX_ITERATIONS);
    
    /* classic nested for loop approach */
    for(y = 0; y < HEIGHT; ++y)    
    {
        for(x = 0; x < WIDTH; ++x)
        {
            fprintf(fp, "%i 0 %i ", plane_col[y][x].red,
                                     plane_col[y][x].blue);
        }
        fprintf(fp, "\n");
    }
    
    fclose(fp); 
}

