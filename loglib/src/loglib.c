/*======================================================================
 * LOGLIB
 * A small library for logging debug information in C projects.
 * Copyright (C) Damian Gareth Walker 2022
 * 
 * Main library module.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

/* project headers */
#include "loglib.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/** @var output is the output file. */
static FILE *output = NULL;

/*----------------------------------------------------------------------
 * Top Level Function Definitions.
 */

/**
 * Construct a new log.
 * @param file The name of the source file.
 */
void log_open (char *file)
{
    char logfile[255], /* the log filename */
	*ptr; /* pointer to special character in log filename */

    /* return if a log file is already open */
    if (output)
	return;

    /* work out the filename */
    if ((ptr = strrchr (file, '/')) ||
	(ptr = strrchr (file, '\\')))
	strcpy (logfile, ptr + 1);
    else
	strcpy (logfile, file);
    if ((ptr = strchr (logfile, '.')))
	strcpy (ptr, ".log");
    else
	strcat (logfile, ".log");

    /* open the file and output an initial message */
    if ((output = fopen (logfile, "a"))) {
	setbuf (output, NULL);
	log_printf ("Logging from %s", file);
    }
}

/**
 * Close the log file and destroy the instance.
 */
void log_close (void)
{
    if (output)
	fclose (output);
}

/**
 * Write to the log file.
 * @param format The format of the output.
 * @param ...    The arguments to print with the format.
 */
void log_printf (char *format, ...)
{
    va_list args; /* argument list */
    char tm[20]; /* time output */
    time_t t; /* time variable */
    struct tm *ts; /* time structure */

    /* ensure we have a stream to output to */
    if (! output)
	return;

    /* output the date and time */
    t = time (0);
    ts = localtime (&t);
    strftime (tm, 20, "%Y-%m-%d %H:%M:%S", ts);
    fprintf (output, "%s ", tm);

    /* output the arguments in the rest of the line */
    va_start (args, format);
    vfprintf (output, format, args);
    va_end (args);
    fprintf (output, "\n");

    /* flush the buffer */
    fflush (output);
}
