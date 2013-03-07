#ifndef MANDELBROT_H
#define MANDELBROT_H

#include <math.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* TODO sort out the makefile to insert these options when compiling objects */
//#ifdef rtmandel
#include "rtworksteal.h"
//#endif
#ifdef wsmandel
#include "worksteal.h"
#endif

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

typedef struct Pixel{
    char t_id, val;
} Pixel;

/* ARGUMENT ENUMS */
typedef enum {OFF, GREYSCALE, REDSCALE, DISTRIBUTION} Output_Arg;

/* -------------------------------------------------------------------------- */
/* function declarations */

void initialise                 ( );
inline double convert_x_coord   ( double, double, unsigned int);
inline double convert_y_coord   ( double, double, unsigned int);
inline char is_outside_rad2     ( Complex);
char is_member                  ( Complex);
Complex julia_func              ( Complex, Complex);
void compute_line               ( unsigned int, char);

/* UTIL FUNCTIONS */
void handle_arguments           ( int, char *[]);
void print_usage                ();

void perhaps_print              ();

void print_complex              ( Complex *);
void write_to_pgm               ();

void write_to_ppm               ();
void write_to_ppm_redscale      ();

char get_red_val                ( Pixel);
char get_blue_val               ( Pixel);

#endif /* MANDELBROT_H */
