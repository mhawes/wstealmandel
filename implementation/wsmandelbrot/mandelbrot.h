#ifndef MANDELBROT_H
#define MANDELBROT_H

#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* depending on the make rule applied this attaches a different scheduler */
#ifdef rtmandel
#include "rtworksteal.h"
#endif
#ifdef wsmandel
#include "worksteal.h"
#endif
#ifdef semandel
#include "sequential.h"
#endif
#ifdef nsmandel
#include "noscheduling.h"
#endif
#ifdef inmandel
#include "interleaved.h"
#endif


/* dimensions of the raster plane */
#define HEIGHT 10000
#define WIDTH  10000

#define PPM_BLACK 0     /* the value of black in a grey-scale ppm file */

#define MAX_ITERATIONS 70  /* between: 1-127 */

/* struct for nicely containing Complex numbers */
typedef struct complex_t{
    double re, im;
} complex_t;

typedef struct pixel_t{
    char t_id, val;
} pixel_t;

/* ARGUMENT ENUMS */
typedef enum {OFF, GREYSCALE, REDSCALE, DISTRIBUTION} output_arg_t;

/* -------------------------------------------------------------------------- */
/* function declarations */

void initialise                 ( );
inline double convert_x_coord   ( unsigned int);
inline double convert_y_coord   ( unsigned int);
inline char is_outside_rad2     ( complex_t);
char is_member                  ( complex_t);
complex_t julia_func              ( complex_t, complex_t);
void compute_line               ( unsigned int, char);

/* UTIL FUNCTIONS */
void handle_arguments           ( int, char *[]);
void print_usage                ();

void perhaps_print              ();

void print_complex              ( complex_t *);

void write_to_ppm_dist          ();
void write_to_ppm_greyscale     ();
void write_to_ppm_redscale      ();

char get_red_val                ( pixel_t);
char get_blue_val               ( pixel_t);

inline float fast_inv_sqrt      ( float);

void trace_event                ( const char *,...);
inline void trace_start         ();
#endif /* MANDELBROT_H */
