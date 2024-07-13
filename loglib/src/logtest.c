/*======================================================================
 * LOGLIB
 * A small library for logging debug information in C projects.
 * Copyright (C) Damian Gareth Walker 2022
 * 
 * Test and demonstration module
 */

/* standard C headers */
#include <stdio.h>

/* project headers */
#include "loglib.h"

/*----------------------------------------------------------------------
 * Top Level Function Definitions.
 */

/**
 * Main function. Outputs some lines to an arbitrary log.
 * @return 0 for successful run.
 */
int main (void)
{
    log_open (__FILE__);
    log_printf ("Single log message");
    log_printf ("A log message with an integer %d", 5);
    log_close ();
    return 0;
}
