#include "mandelbrot.h"

static Pixel plane_col[HEIGHT][WIDTH];/* array to hold the generated image */

/* globals used to control the limits raster plane */
Complex c_max;  /* maximum value of c */
Complex c_min;  /* minimum value of c */
Complex c_factor; /* value used to calculate space between each sample on the 
                    raster plane */

/* ARGUMENT SETTINGS */
Output_Arg out_arg;
char outfile[21] = "out.ppm";


/* -------------------------------------------------------------------------- */
/* CODE STARTS HERE:                                                          */
/* -------------------------------------------------------------------------- */

int
main( int argc, char *argv[])
{
    unsigned long long elapsed;

    handle_arguments( argc, argv);
    
    struct timeval tv_start, tv_end;
    gettimeofday(&tv_start, NULL);

    initialise();
    
    ws_initialise_threads();
    ws_start_threads();
    
    gettimeofday(&tv_end, NULL);

    elapsed = (tv_end.tv_sec - tv_start.tv_sec)*1000000 + tv_end.tv_usec - tv_start.tv_usec;
//    printf( "TOOK: %llu us\n", elapsed);

    perhaps_print();
    
    pthread_exit(NULL);
    return (0);    
}

/* -------------------------------------------------------------------------- */
void initialise( )
{
    c_min.re = -2;
    c_max.re = 2;

    c_min.im = -2;
    c_max.im = c_min.im + (c_max.re - c_min.re) * HEIGHT / WIDTH;

    /* used to convert x, y of the raster plane into a complex number */
    c_factor.re = (c_max.re - c_min.re) / (WIDTH - 1);
    c_factor.im = (c_max.im - c_min.im) / (HEIGHT - 1);
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
/* 
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
inline char is_outside_rad2( Complex c)
{
    /* if the number is outside the radius 2 we know it cant be in the set */
    return( sqrt((c.re * c.re) + (c.im * c.im)) > 2);
}

/* -------------------------------------------------------------------------- */
/* calculates the next value of: z = z^2 + c */
Complex julia_func(Complex z, Complex c)
{
    Complex res;
    
    res.im = 2 * z.re * z.im + c.im;
    res.re = (z.re * z.re) - (z.im * z.im)  + c.re;

    return res;
}

/* -------------------------------------------------------------------------- */
/* Computes a line of the raster plane */
void compute_line( unsigned int y, char t_id)
{
    Complex c_cur; 
    unsigned int x;

    c_cur.im = convert_y_coord( c_max.im, c_factor.im, y);
    for( x = 0; x < WIDTH; x++)
    {
        c_cur.re = convert_x_coord( c_min.re, c_factor.re, x);
        
        plane_col[y][x].t_id = t_id; /* assign this thread id to the pixel */
        
        if( is_outside_rad2( c_cur)){
            /* set this to the cannonical boundary setting */
            plane_col[y][x].t_id = WORKER_COUNT; 
            plane_col[y][x].val = PPM_BLACK;  /* make the outer black */
        }
        else{
            plane_col[y][x].val= is_member( c_cur);
        }
    }
}

/* -------------------------------------------------------------------------- */
/* UTIL FUNCTIONS:                                                           */
/* -------------------------------------------------------------------------- */
/* A simple function to parse command line arguments to allow some dynamic 
 * control from the user.
 *
 * NOTE: repeated arguments overwrite the previous and the rightmost repetition 
 *       is used.
 */
void handle_arguments( int argc, char *argv[])
{
    int i;
    char substr[11];

    if( argc == 1){ /* in this case no arguments have been passed */
        /* USE DEFAULT SETTINGS */
        out_arg = OFF;
    }
    else{
        for( i = 1; i < argc; i ++)
        {
            /* apply rules here */
            /* OUTMODE RULES: */
            if( strcmp(argv[i],     "--outmode=greyscale")    == 0){
                out_arg = GREYSCALE;
            }
            else if( strcmp(argv[i],"--outmode=redscale")     == 0){
                out_arg = REDSCALE;
            }
            else if( strcmp(argv[i],"--outmode=distribution") == 0){
                out_arg = DISTRIBUTION;
            }
            
            /* OUTFILE RULE: */
            else if( strncmp(argv[i],"--outfile=", 10) == 0){
                strncpy( outfile, argv[i] + 10, 15);
            }
            
            /* USAGE RULE: */
            else if( strcmp(argv[i],"--help")          == 0){
                print_usage();
                exit(0);
            }
            
            /* ERROR RULE: */
            else{
                print_usage();
                exit(0);
            }
        }
    }
}

/* -------------------------------------------------------------------------- */
void print_usage()
{
    printf( "Computing the Mandelbrot set in parallel using a workstealing technique. \n"
            "Author: Martin Hawes\n"
            "\n"
            "Usage:\n"
            "   * No Arguments\n"
            "       The raster-plane is computed but no output is done.\n"
            "\n"
            "   * --outmode=<mode>\n"
            "       Enables output of the raster-plane in a number of ppm configurations.\n"
            "       Default filename is \"out.ppm\".\n"
            "       modes:\n"
            "           greyscale    - Gradient of Black to White.\n"
            "           redscale     - Gradient of Black to Red.\n"
            "           distribution - Gradient per thread. \n"
            "\n"
            "   * --outfile=<file>\n"
            "       Specifies the name of the output file.\n"
            "       Maximum of 20 characters.\n"
            "\n"
            );
}

/* -------------------------------------------------------------------------- */
/* Prints to output file depending on the arguments passed to the program */
void perhaps_print()
{
    switch( out_arg){
        case GREYSCALE :
            write_to_pgm();
            break;
        case REDSCALE :
            write_to_ppm_redscale();
            break;
        case DISTRIBUTION :
            write_to_ppm();
    }
}

/* -------------------------------------------------------------------------- */
void print_complex(Complex *com)
{
    printf("(%g + %g*i) ", com->re, com->im);
}

/* -------------------------------------------------------------------------- */
void write_to_pgm()
{
    unsigned int x, y;

    FILE *fp = fopen( outfile, "w+");
    fprintf(fp, "P2\n%d %d\n%d\n", WIDTH, HEIGHT, MAX_ITERATIONS);
    
    /* classic nested for loop approach */
    for(y = 0; y < HEIGHT; ++y)    
    {
        for(x = 0; x < WIDTH; ++x)
        {
            fprintf(fp, "%i ", plane_col[y][x].val);
        }
        fprintf(fp, "\n");
    }
    
    fclose(fp); 
}

/* -------------------------------------------------------------------------- */
void write_to_ppm_redscale()
{
    unsigned int x, y;

    FILE *fp = fopen( outfile, "w+");
    fprintf(fp, "P3\n%d %d\n%d\n", WIDTH, HEIGHT, MAX_ITERATIONS);
    
    /* classic nested for loop approach */
    for(y = 0; y < HEIGHT; ++y)    
    {
        for(x = 0; x < WIDTH; ++x)
        {
            fprintf(fp, "%i 0 0 ", MAX_ITERATIONS - plane_col[y][x].val);
        }
        fprintf(fp, "\n");
    }
    
    fclose(fp); 
}

/* -------------------------------------------------------------------------- */
void write_to_ppm()
{
    unsigned int x, y;

    FILE *fp = fopen( outfile, "w+");
    fprintf(fp, "P3\n%d %d\n%d\n", WIDTH, HEIGHT, MAX_ITERATIONS);
    
    /* classic nested for loop approach */
    for(y = 0; y < HEIGHT; ++y)    
    {
        for(x = 0; x < WIDTH; ++x)
        {
            fprintf(fp, "%i 0 %i ", get_red_val(plane_col[y][x]),
                                    get_blue_val(plane_col[y][x]));
        }
        fprintf(fp, "\n");
    }
    
    fclose(fp); 
}

/* -------------------------------------------------------------------------- */
char get_red_val( Pixel pix)
{
    if( pix.t_id == WORKER_COUNT)
    {
        return pix.val;
    }

    if( pix.t_id % 4 == 0){
        return pix.val;
    }
    else if( pix.t_id % 4 == 1){
        return MAX_ITERATIONS - pix.val;
    }
    
    return 0;    
}

/* -------------------------------------------------------------------------- */
char get_blue_val( Pixel pix)
{
    if( pix.t_id == WORKER_COUNT)
    {
        return pix.val;
    }
    
    if( pix.t_id % 4 == 2){
        return pix.val;
    }
    else if( pix.t_id % 4 == 3){
        return MAX_ITERATIONS - pix.val;
    }
    
    return 0;
}

