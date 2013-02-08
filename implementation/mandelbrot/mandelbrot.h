#include <stdio.h>
#include <math.h>

#define HEIGHT 8000
#define WIDTH  10000

#define PPM_WHITE 1
#define PPM_BLACK 0

#define MAX_ITERATIONS 25

/* struct for nicely containing Complex numbers */
typedef struct Complex{
    double re, im;
} Complex;

/* function declarations */

void initialise                 ( Complex *, Complex *, Complex *);
inline double convert_x_coord   ( double, double, unsigned int);
inline double convert_y_coord   ( double, double, unsigned int);
void compute_plane              ( Complex *, Complex *, Complex *);
char is_member                  ( Complex *);
Complex julia_func              ( Complex *, Complex *);

/* UTIL FUNCTIONS */
void print_complex              ( Complex *);
void write_to_ppm               ( char[HEIGHT][WIDTH]);
