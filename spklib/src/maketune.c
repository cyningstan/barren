/*======================================================================
 * SPKLIB
 * A PC Speaker Sound Library
 *
 * Copyright (C) Damian Gareth Walker 2021.
 *
 * Utility to make a tune file.
 */

/*----------------------------------------------------------------------
 * Included headers.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* project headers */
#include "speaker.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/** @var input is the input file. */
static FILE *input;

/** @var otuput is the output file. */
static FILE *output;

/** @var tune is the tune to read */
static Tune *tune;

/*----------------------------------------------------------------------
 * Level 1 Routines.
 */

/**
 * Initialise the program.
 * @param argc is the argument count.
 * @param argv is the argument list array.
 * @return 1 if successful, 0 if not.
 */
int initialise (int argc, char **argv)
{
    /* ensure we have the arguments */
    if (argc != 3) {
	printf ("Usage: %s INFILE OUTFILE\n", argv[0]);
	exit (0);
    }

    /* open the files */
    if (! (input = fopen (argv[1], "r")))
	return 0;
    if (! (output = fopen (argv[2], "wb")))
	return 0;

    /* create the tune */
    if (! (tune = new_Tune ()))
	return 0;

    /* all successful */
    return 1;
}

/**
 * Copy a note at a time from the input file to the output file.
 * @return 1 if an input line was read, 0 if not.
 */
int makenote (void)
{
    char line[81]; /* text line read from file */
    int pitch, /* pitch read from file */
	duration; /* duration read from file */

    /* attempt to read the line and process it */
    if (! (fgets (line, 80, input)))
	return 0;
    if (! sscanf (line, "%d,%d", &pitch, &duration))
	return 0;

    /* add the note to the tune */
    tune->add (tune, new_Note (pitch, duration));
    return 1;
}

/**
 * Close the files.
 */
void closedown (void)
{
    if (! fwrite ("SPK100T", 8, 1, output))
	return;
    if (! tune->write (tune, output))
	return;
    fclose (output);
    fclose (input);
}

/*----------------------------------------------------------------------
 * Top Level Routine.
 */

/**
 * Main Program.
 * @param argc is the argument count.
 * @param argv is the argument list array.
 * @return 0 if successful, >0 on error.
 */
int main (int argc, char **argv)
{
    if (! initialise (argc, argv))
	return 1;
    while (makenote ());
    closedown ();
    return 0;
}
