#ifndef SEQUENTIAL_H
#define SEQUENTIAL_H

#define WORKER_COUNT 1

/* HAS TO BE INCLUDED LAST!!! */
#include "mandelbrot.h"

void ws_initialise_threads   ();
void ws_start_threads        ();

#endif /* SEQUENTIAL_H */
