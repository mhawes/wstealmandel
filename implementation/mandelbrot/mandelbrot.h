#include <stdio.h>
#include <math.h>

#define HEIGHT 8000
#define WIDTH  8000

#define PPM_BLACK 0

/* between: 1-127 */
#define MAX_ITERATIONS 50

/* struct for nicely containing Complex numbers */
typedef struct Complex{
    double re, im;
} Complex;

/* function declarations */

void initialise                 ( Complex *, Complex *, Complex *);
inline double convert_x_coord   ( double, double, unsigned int);
inline double convert_y_coord   ( double, double, unsigned int);
inline char is_outside_rad2     ( Complex *);
void compute_plane              ( Complex *, Complex *, Complex *);
char is_member                  ( Complex);
Complex julia_func              ( Complex, Complex);

/* UTIL FUNCTIONS */
void print_complex              ( Complex *);
void write_to_ppm               ( char[HEIGHT][WIDTH]);
