#include "sequential.h"

void ws_initialise_threads(){}

void ws_start_threads()
{
    unsigned int y;

    printf("Started\n");
    
    for( y = 0; y < HEIGHT; y++)
    {
        compute_line( y, 0);
    }
    
    printf("Finished\n");
}
