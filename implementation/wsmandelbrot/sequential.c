#include "sequential.h"

void ws_initialise_threads(){}

void ws_start_threads()
{
    unsigned int y;

#if TRACE >= 2
    trace_event("Started\n");
#endif
    
    for( y = 0; y < HEIGHT; y++)
    {
        compute_line( y, 0);
    }

#if TRACE >= 2   
    trace_event("Finished\n");
#endif
}
