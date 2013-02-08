#include <stdio.h>

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
    
    Complex c;
    
    c.re = 0;
    c.im = -0.1;
    
    printf("res = %c\n", is_member( &c));
    
    compute_plane( &c_max, &c_min, &c_factor);

    return (0);
}

/* -------------------------------------------------------------------------- */
void initialise( Complex *c_max, Complex *c_min, Complex *c_factor)
{
    c_min->re = -2.0;
    c_min->im = -1.2;
    
    c_max->re = 1.0;
    c_max->im = c_min->im + (c_max->re - c_min->re) * HEIGHT / WIDTH;

    /* used to convert x, y of the raster plane into a complex */
    c_factor->re = (c_max->re - c_min->re) / (WIDTH - 1);
    c_factor->im = (c_max->im - c_min->im) / (HEIGHT - 1);
    
    printf("min: ");    print_complex( c_min);
    printf("max: ");    print_complex( c_max);
    printf("factor: "); print_complex( c_factor);
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
    char plane[WIDTH][HEIGHT];  /* array to hold the generated image */
    
    FILE *fp = fopen("out.ppm", "w+");
    fprintf(fp, "P2\n%d %d\n15\n", WIDTH, HEIGHT);
    
    /* classic nested for loop approach */
    for(y = 0; y < WIDTH; ++y)    
    {
        c_cur.im = convert_y_coord( c_max->im, c_factor->im, y);
        for(x = 0; x < HEIGHT; ++x)
        {
            c_cur.re = convert_x_coord( c_min->re, c_factor->re, x);
            
            //print_complex(&c_cur);
            
            plane[x][y] = is_member( &c_cur);
            //printf("%c ", plane[x][y]);
            
            if( plane[x][y] == '*'){
                fprintf(fp, "1   ");
            }
            else{
                fprintf(fp, "255 ");
            }
        }
        fprintf(fp, "\n");
        //printf("\n");
    }
    
    fclose(fp);    
}

/* -------------------------------------------------------------------------- */
char is_member(Complex *c)
{
    unsigned int i;
    Complex z;

    z.re = c->re; 
    z.im = c->im;

    for(i = 0; i < MAX_ITERATIONS; ++i)
    {
        //printf("i=%d ", i);
        //print_complex( &z);
        
        //printf("res=%g ",((z.re * z.re) + (z.im * z.im)));
        if( ((z.re * z.re) + (z.im * z.im)) > 4)
        {
            /* c is not a member of the set */
            return ' ';
        }
        z = julia_func( &z, c);
        //printf("\n");
    }
    
    //printf("\n");
    
    /* if it gets this far c is a member of the set */
    return '*';
}

/* -------------------------------------------------------------------------- */
/* calculates the next value of: z = z * z + c */
Complex julia_func(Complex *z, Complex *c)
{
    double z_im_p, z_re_p;
    Complex res;
    
    z_re_p = z->re * z->re;
    z_im_p = z->im * z->im;
    
    res.im = 2 * z->re * z->im + c->im;
    res.re = z_re_p - z_im_p + c->re;

    return res;
}

/* -------------------------------------------------------------------------- */
/* DEBUG FUNCTIONS:                                                           */
/* -------------------------------------------------------------------------- */
void print_complex(Complex *com)
{
    printf("(%g + %g*i) ", com->re, com->im);
}

void write_to_ppm( char plane[WIDTH][HEIGHT])
{
    
}
