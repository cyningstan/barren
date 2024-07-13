/*======================================================================
 * CWG: Cyningstan Wargame Engine.
 * An engine for simple turn-based wargames.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * Created: 08-Oct-2021.
 *
 * Main CWG Object Module.
 */

/*----------------------------------------------------------------------
 * Includes
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* project headers */
#include "cwg.h"

/*----------------------------------------------------------------------
 * Data definitions.
 */

/** @var cwg The CWG object. */
static Cwg *cwg = NULL;

/** @var log The debug log output file */
static FILE *log = NULL;

/*----------------------------------------------------------------------
 * Public Method Definitions.
 */

/**
 * Destroy CWG when it is no longer needed.
 */
static void destroy (void)
{
    if (cwg)
	free (cwg);
}

/**
 * Write an integer as a byte to an already open output file.
 * @param  value  A pointer to the integer variable to write.
 * @param  output The output file handle.
 * @return        1 if successful, 0 if not.
 */
static int writeint (int *value, FILE *output)
{
    unsigned char c; /* character to read */
    c = (char) (*value & 0xff);
    return fwrite (&c, 1, 1, output);
}

/**
 * Write a string to an already open output file.
 * @param  value  A pointer to the string buffer to write.
 * @param  output The output file handle.
 * @return        1 if successful, 0 if not.
 */
static int writestring (char *value, FILE *output)
{
    unsigned char length; /* length byte */
    length = (char) strlen (value);
    if (! (fwrite (&length, 1, 1, output)))
	return 0;
    if (length)
	return fwrite (value, (int) length, 1, output);
    return 1;
}

/**
 * Read a byte from an already open input file
 * and store it in an integer variable.
 * @param  value A pointer to the integer variable to store into.
 * @param  input The input file handle.
 * @return       1 if successful, 0 if not.
 */
static int readint (int *value, FILE *input)
{
    unsigned char c; /* character to read */
    if (! (fread (&c, 1, 1, input)))
	return 0;
    *value = (int) c;
    return 1;
}

/**
 * Read a string from an already open input file
 * and store it in a string buffer.
 * @param  value A pointer to the string buffer to store into.
 * @param  input The input file handle.
 * @return       1 if successful, 0 if not.
 */
static int readstring (char *value, FILE *input)
{
    unsigned char length; /* length byte */
    if (! (fread (&length, 1, 1, input)))
	return 0;
    if (length &&
	! (fread (value, length, 1, input)))
	return 0;
    value[(int) length] = '\0';
    return 1;
}

/**
 * Open the debug log file.
 */
static void debugopen (void)
{
    log = fopen ("cwg.log", "a");
    setbuf (log, NULL);
}

/**
 * Log a single line to the log file.
 * @param string The string to log.
 */
static void debuglog (char *message)
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
static void debugclose (void)
{
    fclose (log);
}
/*----------------------------------------------------------------------
 * Top Level Function Definitions.
 */

/**
 * Constructor for the CWG object.
 * @return A pointer to the new CWG.
 */
Cwg *get_Cwg (void)
{
    /* only one Cwg should be in existence at once. */
    if (cwg)
	return cwg;

    /* try to reserve memory for the Cwg */
    if (! (cwg = malloc (sizeof (Cwg))))
	return NULL;

    /* initialise the methods */
    cwg->destroy = destroy;
    cwg->readint = readint;
    cwg->readstring = readstring;
    cwg->writeint = writeint;
    cwg->writestring = writestring;
    cwg->debug_open = debugopen;
    cwg->debug_log = debuglog;
    cwg->debug_close = debugclose;

    /* return the new Cwg */
    return cwg;
}
