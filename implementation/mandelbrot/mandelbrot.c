#include "mandelbrot.h"

/* -------------------------------------------------------------------------- */
/* CODE STARTS HERE:                                                          */
/* -------------------------------------------------------------------------- */

int
main()
{
    Complex c_max;
    Complex c_min;
    Complex c_factor;

    /* ------------------------------------- */
    initialise( &c_max, &c_min, &c_factor);
    
    compute_plane( &c_max, &c_min, &c_factor);

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
    unsigned int x, y;
    Complex c_cur;      /* used as the current z value */
    
    static char plane[HEIGHT][WIDTH];  /* array to hold the generated image */

    /* classic nested for loop approach */
    for(y = 0; y < HEIGHT; y++)    
    {
        c_cur.im = convert_y_coord( c_max->im, c_factor->im, y);
        
        for(x = 0; x < WIDTH; x++)
        {
            c_cur.re = convert_x_coord( c_min->re, c_factor->re, x);
            
            if( is_outside_rad2( &c_cur)){
                plane[y][x] = MAX_ITERATIONS / 2;  /* make the outer grey */
            }
            else{
                /* compute c_cur checking if it is in the set */
                plane[y][x] = is_member( c_cur);
            }
        }
    }
    
    /* write it out to a PPM file */
    write_to_ppm( plane);
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
/* UTIL FUNCTIONS:                                                           */
/* -------------------------------------------------------------------------- */
void print_complex(Complex *com)
{
    printf("(%g + %g*i) ", com->re, com->im);
}

/* -------------------------------------------------------------------------- */
void write_to_ppm( char plane[HEIGHT][WIDTH])
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

