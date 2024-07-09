/*======================================================================
 * Anarchic Kingdom
 * A light strategy game set in medieval times.
 * Copyright (C) Damian Gareth Walker 2021.
 *
 * Beta Test Handler Module.
 */

/*----------------------------------------------------------------------
 * Included Headers.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* project headers */
#include "fatal.h"

/*----------------------------------------------------------------------
 * Public Functions.
 */

/**
 * Check that this beta test version has not expired.
 */
void beta_check (void)
{
    time_t rawtime; /* current time in raw format */
    struct tm *strtime; /* current time in struct tm format */
    char date[10]; /* current date as a string */

    /* check that beta is not expired */
    time (&rawtime);
    strtime = localtime (&rawtime);
    date[8] = '\0'; /* buggy strftime doesn't terminate */
    strftime (date, 9, "%Y%m%d", strtime);
    if (strcmp (date, "20230430") > 0)
	fatalerror (FATAL_EXPIRED);
}
