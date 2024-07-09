/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * Created: 03-Jan-2021.
 *
 * Scenario Handler Module.
 */

/*----------------------------------------------------------------------
 * Headers.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* project specific headers */
#include "cwg.h"
#include "battle.h"
#include "scenario.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

static Cwg *cwg; /* the CWG object */

/*----------------------------------------------------------------------
 * Private methods.
 */

/**
 * Write a long string to an already open file.
 * @param value is the string to write.
 * @param output is the file to write to.
 * @return 1 if successful, 0 on failure.
 */
int writestring (char *value, FILE *output)
{
    char byte; /* byte of length string */
    byte = (char) (strlen (value) % 0x100);
    if (! fwrite (&byte, 1, 1, output))
	return 0;
    byte = (char) (strlen (value) / 0x100);
    if (! fwrite (&byte, 1, 1, output))
	return 0;
    if (! strlen (value))
	return 1;
    return fwrite (value, strlen (value), 1, output);
}

/**
 * Read a long string from an open file.
 * @param value is the string buffer to store the string.
 * @param input is the file to read from.
 * @return 1 on success, 0 on failure.
 */
int readstring (char *value, FILE *input)
{
    /* local variables */
    int len, /* string length */
	r = 1; /* return value */
    char byte; /* byte from string length */

    /* read the length */
    if (! fread (&byte, 1, 1, input))
	return 0;
    len = byte;
    if (! fread (&byte, 1, 1, input))
	return 0;
    len += 0x100 * byte;

    /* read the string and return it */
    if (len)
	r &= fread (value, len, 1, input);
    value[len] = '\0';
    return 1;
}

/*----------------------------------------------------------------------
 * Public methods.
 */

/**
 * Destroy scenario when it is no longer needed.
 * @param scenario is the scenario to destroy.
 */
static void destroy (Scenario *scenario)
{
    /* free up memory */
    if (scenario) {
	if (scenario->battle)
	    scenario->battle->destroy (scenario->battle);
	free (scenario);
    }
}

/**
 * Write scenario details to an already open file.
 * @param scenario is the scenario to save.
 * @param output is the output file.
 * @return 1 for success, 0 for failure.
 */
static int write (Scenario *scenario, FILE *output)
{
    /* local variables */
    int t, /* text counter */
	n, /* next scenario counter */
	r; /* return code from trivial write */

    /* write the battle */
    if (! scenario->battle)
	return 0;
    if (! scenario->battle->write (scenario->battle, output))
	return 0;

    /* write the scenario text */
    for (t = 0; t < 6; ++t) {
	if (scenario->text[t])
	    r = writestring (scenario->text[t], output);
	else
	    r = writestring ("", output);
	if (! r)
	    return 0;
    }

    /* write the next scenario IDs */
    for (n = 0; n < 2; ++n)
	if (! cwg->writeint (&scenario->next[n], output))
	    return 0;
    
    /* return success */
    return 1;
}

/**
 * Read scenario details from an already open file.
 * @param scenario is the scenario to load.
 * @param input is the input file.
 * @return 1 for success, 0 for failure.
 */
static int read (Scenario *scenario, FILE *input)
{
    /* local variables */
    int t, /* text counter */
	n, /* next scenario counter */
	r = 1; /* return value */

    /* read the battle */
    if (! scenario->battle)
	scenario->battle = new_Battle (scenario->campaign->unittypes,
				       scenario->campaign->terrain);
    if (! scenario->battle)
	return 0;
    r &= scenario->battle->read (scenario->battle, input);

    /* read the scenario text */
    for (t = 0; t < 6; ++t)
	r &= readstring (scenario->text[t], input);

    /* read the next scenario IDs */
    for (n = 0; n < 2; ++n)
	r &= cwg->readint (&scenario->next[n], input);

    /* return success */
    return r;
}

/*----------------------------------------------------------------------
 * Constructors.
 */

/**
 * Construct a scenario.
 * @return the new scenario.
 */
Scenario *new_Scenario (Campaign *campaign)
{
    /* local variables */
    Scenario *scenario;
    int t; /* text counter */

    /* reserve memory for the scenario */
    if (! (scenario = malloc (sizeof (Scenario))))
	return NULL;

    /* initialise the methods */
    scenario->destroy = destroy;
    scenario->write = write;
    scenario->read = read;

    /* initialise the attributes */
    scenario->campaign = campaign;
    scenario->battle = NULL;
    for (t = 0; t < 6; ++t)
	*scenario->text[t] = '\0';
    scenario->next[0] = scenario->next[1] = 0;

    /* get the CWG object */
    cwg = get_Cwg ();

    /* return the new scenario */
    return scenario;
}
