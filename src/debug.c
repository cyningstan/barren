/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * By Cyningstan
 * Copyright (C) Damian Gareth Walker 2021
 *
 * Debug Logging Module
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <io.h>

/*----------------------------------------------------------------------
 * File Level Variables.
 */

/**
 * @var log The debug log file.
 */
static FILE *log;

/*----------------------------------------------------------------------
 * Top Level Functions.
 */

/**
 * Open the debug log file.
 */
void debug_open (void)
{
    log = fopen ("barren.log", "a");
    setbuf (log, NULL);
}

/**
 * Log a single line to the log file.
 * @param string The string to log.
 */
void debug_log (char *message)
{
    char tm[20]; /* time message */
    time_t t;
    struct tm *ts;

    /* add the message to the log */
    t = time (0);
    ts = localtime (&t);
    strftime (tm, 20, "%Y-%m-%d %H:%M:%S", ts);
    fprintf (log, "%s %s\n", tm, message);
    fprintf (log, "\n");
    fflush (log);
}

/**
 * Close the debug log file.
 */
void debug_close (void)
{
    fclose (log);
}
