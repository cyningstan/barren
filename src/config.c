/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 23-Jan-2021.
 *
 * Init File Hander Module.
 */

/*----------------------------------------------------------------------
 * Headers.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* project-specific headers */
#include "cwg.h"
#include "barren.h"
#include "fatal.h"
#include "config.h"

/*----------------------------------------------------------------------
 * File Level Variables.
 */

/** @var config The configuration - there should be only one. */
static Config *config = NULL;

/** @var cwg A pointer to the Cwg library. */
static Cwg *cwg;

/*----------------------------------------------------------------------
 * Public Methods.
 */

/**
 * Destroy the configuration when no longer needed.
 */
static void destroy (void)
{
    if (config) {
        free (config);
        config = NULL;
    }
}

/**
 * Load the configuration from a file.
 */
static void load (void)
{
    /* local variables */
    FILE *input; /* init file handle */
    char header[8]; /* header read from init file */

    /* if there's an input file, load the values from it */
    if ((input = fopen ("barren.ini", "rb"))) {

        /* read and verify header */
        if (! fread (header, 8, 1, input))
            fatalerror (FATAL_INVALIDINIT);
        if (strcmp (header, "BAR100I"))
            fatalerror (FATAL_INVALIDINIT);

        /* read the data proper */
        if (! cwg->readstring (config->campaignfile, input))
            fatalerror (FATAL_INVALIDINIT);
        if (! cwg->readint (&config->playertypes[0], input))
            fatalerror (FATAL_INVALIDINIT);
        if (! cwg->readint (&config->playertypes[1], input))
            fatalerror (FATAL_INVALIDINIT);
	if (! cwg->readstring (config->gamefile, input))
	    fatalerror (FATAL_INVALIDINIT);

        /* close the file */
        fclose (input);
    }
}

/**
 * Save values to the initialisation file and clean up.
 */
static void save (void)
{
    /* local variables */
    FILE *output; /* init file handle */

    /* attempt to open the output file */
    if ((output = fopen ("barren.ini", "wb"))) {

	/* write header */
	fwrite ("BAR100I", 8, 1, output);

	/* write the data proper */
	cwg->writestring (config->campaignfile, output);
	cwg->writeint (&config->playertypes[0], output);
	cwg->writeint (&config->playertypes[1], output);
	cwg->writestring (config->gamefile, output);

	/* close the file */
	fclose (output);
    }
}

/*----------------------------------------------------------------------
 * Top Level Function Definitions.
 */

/**
 * Construct a new configuration.
 * @return The new configuration.
 */
Config *new_Config (void)
{
    /* reserve memory for configuration */
    if (config)
	return config;
    else if (! (config = malloc (sizeof (Config))))
	fatalerror (FATAL_MEMORY);

    /* initialise methods */
    config->destroy = destroy;
    config->load = load;
    config->save = save;

    /* initialise properties */
    strcpy (config->campaignfile, "BARREN.CAM");
    config->playertypes[0] = PLAYER_HUMAN;
    config->playertypes[1] = PLAYER_COMPUTER;
    *config->gamefile = '\0';

    /* get Cwg object (for I/O) */
    cwg = get_Cwg ();

    /* return the configuration */
    return config;
}
