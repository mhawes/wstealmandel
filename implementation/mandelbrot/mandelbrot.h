#define HEIGHT 2890
#define WIDTH  2890

#define MAX_ITERATIONS 30

/* struct for nicely containing Complex numbers */
typedef struct Complex{
    double re, im;
} Complex;


void initialise                 ( Complex *, Complex *, Complex *);
inline double convert_x_coord   ( double, double, unsigned int);
inline double convert_y_coord   ( double, double, unsigned int);
void compute_plane              ( Complex *, Complex *, Complex *);
char is_member                  ( Complex *);
Complex julia_func              ( Complex *, Complex *);

/* DEBUG FUNCTIONS */
void print_complex              ( Complex *);
void write_to_ppm               ( char[WIDTH][HEIGHT]);
