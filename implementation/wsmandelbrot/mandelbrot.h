#include <stdio.h>
#include <math.h>
#include <pthread.h>

#include "deque.h"

#define WORKER_COUNT 4        /* the total number of worker threads */

/* dimensions of the raster plane */
#define HEIGHT 2200
#define WIDTH  2200

#define PPM_BLACK 0     /* the value of black in a grey-scale pgm file */

#define MAX_ITERATIONS 127  /* between: 1-127 */

/* struct for nicely containing Complex numbers */
typedef struct Complex{
    double re, im;
} Complex;

typedef struct ThreadLoad{
    Deque *deq;
    Complex *c_max, *c_min, *c_factor;
} ThreadLoad;

/* -------------------------------------------------------------------------- */
/* function declarations */

void initialise                 ( Complex *, Complex *, Complex *);
inline double convert_x_coord   ( double, double, unsigned int);
inline double convert_y_coord   ( double, double, unsigned int);
inline char is_outside_rad2     ( Complex *);
void compute_plane              ( Complex *, Complex *, Complex *);
char is_member                  ( Complex);
Complex julia_func              ( Complex, Complex);

/* thread functions */
void *compute_work              ( void*); 

/* UTIL FUNCTIONS */
void print_complex              ( Complex *);
void write_to_ppm               ( unsigned int[HEIGHT][WIDTH]);