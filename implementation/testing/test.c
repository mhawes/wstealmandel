#include "deque.h"

#include <stdio.h>

int main()
{
    /* create a deque */
    Deque d;
    Line l;
    
    int i;

    printf("Test Suite.\n");
    
    l.y = 1;
    l.status = LINE_NORMAL;
    
    de_initialise( &d, 0);
    
    /* print start state of deque */
    de_print_deque( &d);
    
    de_push_bottom( &d, l);
    
    /* print after one push */
    de_print_deque( &d);
    
    /* push 9 more times */
    for( i = 0; i < 9; i++)
    {
        de_push_bottom( &d, l);
    }
    
    /* print after ten pushes */
    de_print_deque( &d);
    
    /* steal once */
    de_steal( &d);
    de_print_deque( &d);
    
    /* steal 3 more times */
    for( i = 0; i < 3; i++)
    {
        de_steal( &d);
    }
    de_print_deque( &d);
    
    /* push 2 times */
    de_push_bottom( &d, l);
    de_push_bottom( &d, l);
    de_print_deque( &d);
    
    /* pop once */
    de_pop_bottom( &d);
    de_print_deque( &d);
    
    /* push 10 more times to force the grow */
    for( i = 0; i < 10; i++)
    {
        de_push_bottom( &d, l);
    }
    de_print_deque( &d);
    
    /* steal 7 times to force the shrink */
    for( i = 0; i < 6; i++)
    {
        de_steal( &d);
    }
    de_pop_bottom( &d);
    de_print_deque( &d);
    
    return 0;
}
