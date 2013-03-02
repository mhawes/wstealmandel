#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

#include "deque.h"

#define WORKER_COUNT 4        /* the total number of worker threads */

/* dimensions of the raster plane */
//#define HEIGHT 18500
//#define WIDTH  18500

#define HEIGHT 10000
#define WIDTH  10000

#define PPM_BLACK 0     /* the value of black in a grey-scale ppm file */

#define MAX_ITERATIONS 70  /* between: 1-127 */

/* struct for nicely containing Complex numbers */
typedef struct Complex{
    double re, im;
} Complex;

typedef struct ThreadLoad{
    char t_id;
    Deque *deq;
    Complex *c_max, *c_min, *c_factor;
} ThreadLoad;

typedef struct Pixel{
    char red, blue;
} Pixel;

/* -------------------------------------------------------------------------- */
/* function declarations */

void initialise                 ( Complex *, Complex *, Complex *);
inline double convert_x_coord   ( double, double, unsigned int);
inline double convert_y_coord   ( double, double, unsigned int);
inline char is_outside_rad2     ( Complex *);
void compute_plane              ( Complex *, Complex *, Complex *);
char is_member                  ( Complex);
Complex julia_func              ( Complex, Complex);

/* worker thread functions */
void *worker_thread             ( void*);
int compute_deque              ( Deque *, Complex *, Complex *, Complex *);
char become_thief               ( Deque *);
char victimise                  ( Deque *, Deque *);

Deque *random_deque             ( char[WORKER_COUNT]);

/* UTIL FUNCTIONS */
void print_complex              ( Complex *);
void write_to_pgm               ();
void write_to_ppm               ();
