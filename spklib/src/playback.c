/*======================================================================
 * SPKLIB
 * A PC Speaker Sound Library
 *
 * Copyright (C) Damian Gareth Walker 2021.
 *
 * Simple utility to play a tune.
 */

/*----------------------------------------------------------------------
 * Included headers.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* project headers */
#include "tune.h"

/*----------------------------------------------------------------------
 * Data definitions.
 */

/** @var tune is the tune to play */
static Tune *tune;

/*----------------------------------------------------------------------
 * Level 1 routines.
 */

/**
 * Initialise the program by loading the named tune.
 * @param argc is the argument count.
 * @param argv is the argument array.
 * @return 1 on success, 0 on error.
 */
static int initialise (int argc, char **argv)
{
    FILE *input; /* the input file */
    char header[8]; /* header read from file */
    int success = 1;

    /* check parameters */
    if (argc != 2) {
	printf ("Usage: %s TUNEFILE\n", argv[0]);
	exit (0);
    }

    /* create and load the tune */
    if (! (tune = new_Tune ()))
	return 0;
    else if (! (input = fopen (argv[1], "rb")))
	return 0;
    else if (! fread (header, 8, 1, input))
	success = 0;
    else if (strcmp (header, "SPK100T"))
	success = 0;
    else if (! tune->read (tune, input))
	success = 0;

    /* ready to play! */
    fclose (input);
    return success;
}

/*----------------------------------------------------------------------
 * Top level routines.
 */

/**
 * Main program.
 * @param argc is the argument count.
 * @param argv is the argument array.
 * @return 0 on success, >0 on error.
 */
int main (int argc, char **argv)
{
    if (! initialise (argc, argv))
	return 1;
    tune->play (tune, NULL);
    return 0;
}
