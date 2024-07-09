/*======================================================================
 * barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * Created: 01-Jan-2021.
 *
 * Campaign Handler Module.
 */

/*----------------------------------------------------------------------
 * Headers
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* project specific headers */
#include "cgalib.h"
#include "cwg.h"
#include "battle.h"
#include "utype.h"
#include "terrain.h"
#include "map.h"
#include "unit.h"
#include "campaign.h"
#include "scenario.h"

/*----------------------------------------------------------------------
 * Data Definition.
 */

/** @var cwg The CWG object */
static Cwg *cwg = NULL;

/*----------------------------------------------------------------------
 * Level 1 Private Function Definitions.
 */

/**
 * Destroy the subsidiary objects in the campaign.
 * @param campaign The campaign object.
 */
static void destroyobjects (Campaign *campaign)
{
    int c; /* counter for unit types, terrain, etc. */

    for (c = 0; c < CWG_UTYPES; ++c)
	if (campaign->unittypes[c])
	    campaign->unittypes[c]->destroy (campaign->unittypes[c]);
    for (c = 0; c < 4 * CWG_UTYPES; ++c)
	if (campaign->unitbitmaps[c])
	    bit_destroy (campaign->unitbitmaps[c]);
    for (c = 0; c < CWG_TERRAIN; ++c)
	if (campaign->terrain[c])
	    campaign->terrain[c]->destroy (campaign->terrain[c]);
    for (c = 0; c < 16 * CWG_TERRAIN; ++c)
	if (campaign->terrainbitmaps[c])
	    bit_destroy (campaign->terrainbitmaps[c]);
    for (c = 0; c < BARREN_SCENARIOS; ++c)
	if (campaign->scenarios[c])
	    campaign->scenarios[c]->destroy (campaign->scenarios[c]);
}

/**
 * Initialise the campaign attributes.
 * @param capaign The campaign to initialise.
 */
static void initialiseattributes (Campaign *campaign)
{
    int c; /* counter for unit types, terrain, etc. */

    *campaign->filename = '\0';
    *campaign->name = '\0';
    strcpy (campaign->corpnames[0], "Nuvutech");
    strcpy (campaign->corpnames[1], "Avuscorp");
    for (c = 0; c < CWG_UTYPES; ++c)
	campaign->unittypes[c] = NULL;
    for (c = 0; c < 4 * CWG_UTYPES; ++c)
	campaign->unitbitmaps[c] = NULL;
    for (c = 0; c < CWG_TERRAIN; ++c)
	campaign->terrain[c] = NULL;
    for (c = 0; c < 16 * CWG_TERRAIN; ++c)
	campaign->terrainbitmaps[c] = NULL;
    for (c = 0; c < BARREN_SCENARIOS; ++c)
	campaign->scenarios[c] = NULL;
    campaign->gatherer = -1;
    campaign->resource = -1;
    campaign->singleplayer = 0;
}

/*----------------------------------------------------------------------
 * Public Methods.
 */

/**
 * Destroy the campaign when no longer needed.
 * @param campaign is the campaign to destroy.
 */
static void destroy (Campaign *campaign)
{
    if (campaign) {
	destroyobjects (campaign);
	free (campaign);
    }
}

/**
 * Clear the data from a campaign.
 * @param campaign is the campaign to clear.
 */
static void clear (Campaign *campaign)
{
    destroyobjects (campaign);
    initialiseattributes (campaign);
}

/**
 * Save a campaign to an already-open file.
 * @param campaign is the campaign to save.
 * @param output is the already-open output file.
 * @return 1 if successful, 0 if not.
 */
static int write (Campaign *campaign, FILE *output)
{
    /* local variables */
    int r = 1, /* return code from trivial write */
	u, /* unit type counter */
	t, /* terrain type counter */
	b, /* bitmap counter */
	s; /* scenario counter */
    UnitType *unittype; /* pointer to unit types */
    Terrain *terrain; /* pointer to terrain */
    Bitmap *bitmap; /* pointer to bitmaps */
    Scenario *scenario; /* pointer to scenarios */

    /* write the campaign name */
    if (campaign->name)
	r &= cwg->writestring (campaign->name, output);
    else
	r &= cwg->writestring ("", output);

    /* write the corporation details */
    r &= cwg->writeint (&campaign->singleplayer, output);
    r &= cwg->writestring (campaign->corpnames[0], output);
    r &= cwg->writestring (campaign->corpnames[1], output);

    /* write the unit types */
    for (u = 0; u < CWG_UTYPES; ++u)
	if (campaign->unittypes[u]) {
	    r &= cwg->writeint (&u, output);
	    unittype = campaign->unittypes[u];
	    r &= unittype->write (unittype, output);
	    for (b = 0; b < 4; ++b) {
		bitmap = campaign->unitbitmaps[b + 4 * u];
		bit_write (bitmap, output);
	    }
	}
    r &= cwg->writeint (&u, output);

    /* write the gatherer unit */
    r &= cwg->writeint (&campaign->gatherer, output);

    /* write the terrain types */
    for (t = 0; t < CWG_TERRAIN; ++t)
	if (campaign->terrain[t]) {
	    cwg->writeint (&t, output);
	    terrain = campaign->terrain[t];
	    r &= terrain->write (terrain, output);
	    for (b = 0; b < 16; ++b) {
		bitmap = campaign->terrainbitmaps[b + 16 * t];
		bit_write (bitmap, output);
	    }
	}
    r &= cwg->writeint (&t, output);

    /* write the resource terrain */
    r &= cwg->writeint (&campaign->resource, output);

    /* write the corporate logos */
    bit_write (campaign->corpbitmaps[0], output);
    bit_write (campaign->corpbitmaps[1], output);

    /* write the scenarios */
    for (s = 0; s < BARREN_SCENARIOS; ++s)
	if (campaign->scenarios[s]) {
	    cwg->writeint (&s, output);
	    scenario = campaign->scenarios[s];
	    r &= scenario->write (scenario, output);
	}
    r &= cwg->writeint (&s, output);

    /* return the code for success */
    return r;
}
    
/**
 * Load a campaign from an already-open file.
 * @param campaign The campaign to load.
 * @param summary  0 to load full campaign, 1 for summary only.
 * @param input    The already-open input file.
 * @return         1 if successful, 0 if not.
 */
static int read (Campaign *campaign, int summary, FILE *input)
{
    int r = 1, /* return value */
	u, /* unit counter */
	t, /* terrain counter */
	b, /* bitmap counter */
	s; /* scenario counter */
    UnitType *unittype; /* unit type loaded */
    Terrain *terrain; /* terrain type loaded */
    Scenario *scenario; /* scenario loaded */

    /* read the campaign name and corporation details */
    if (! cwg->readstring (campaign->name, input))
	return 0;
    if (! cwg->readint (&campaign->singleplayer, input))
	return 0;
    if (! cwg->readstring (campaign->corpnames[0], input))
	return 0;
    if (! cwg->readstring (campaign->corpnames[1], input))
	return 0;

    /* stop here if only summary is needed */
    if (summary)
	return 1;

    /* read unit types and their graphics */
    for (u = 0; u < CWG_UTYPES; ++u)
	if ((unittype = campaign->unittypes[u])) {
	    unittype->destroy (unittype);
	    campaign->unittypes[u] = NULL;
	}
    for (cwg->readint (&u, input);
	 u != CWG_UTYPES;
	 cwg->readint (&u, input)) {
	unittype = campaign->unittypes[u] = new_UnitType ();
	r &= unittype->read (unittype, input);
	for (b = 0; b < 4; ++b)
	    campaign->unitbitmaps[b + 4 * u] = bit_read (input);
    }

    /* read the gatherer unit type id */
    r &= cwg->readint (&campaign->gatherer, input);

    /* read terrain types */
    for (t = 0; t < CWG_TERRAIN; ++t)
	if ((terrain = campaign->terrain[t])) {
	    terrain->destroy (terrain);
	    campaign->terrain[t] = NULL;
	}
    for (cwg->readint (&t, input);
	 t != CWG_TERRAIN;
	 cwg->readint (&t, input)) {
	terrain = campaign->terrain[t] = new_Terrain ();
	r &= terrain->read (terrain, input);
	for (b = 0; b < 16; ++b)
	    campaign->terrainbitmaps[b + 16 * t] = bit_read (input);
    }

    /* read the resource terrain type id */
    r &= cwg->readint (&campaign->resource, input);

    /* read the corporate logos */
    campaign->corpbitmaps[0] = bit_read (input);
    campaign->corpbitmaps[1] = bit_read (input);

    /* read scenarios */
    for (s = 0; s < BARREN_SCENARIOS; ++s)
	if ((scenario = campaign->scenarios[s])) {
	    scenario->destroy (scenario);
	    campaign->scenarios[s] = NULL;
	}
    for (cwg->readint (&s, input);
	 s != BARREN_SCENARIOS;
	 cwg->readint (&s, input)) {
	scenario = campaign->scenarios[s] =
	    new_Scenario (campaign);
	r &= scenario->read (scenario, input);
    }

    /* return success */
    return r;
}

/**
 * Load a campaign file. The filename attribute is used.
 * @param campaign The campaign to load.
 * @param summary  0 to load full campaign, 1 for summary only.
 * @return         1 if successful, 0 if not.
 */
static int load (Campaign *campaign, int summary)
{
    FILE *input; /* the input file handle */
    char header[8]; /* header read in from file */

    /* open the file */
    if (! *campaign->filename)
	return 0;
    if (! (input = fopen (campaign->filename, "rb")))
	return 0;

    /* read the header */
    if (! fread (header, 8, 1, input) ||
	(strcmp (header, "BAR102C") &&
	 strcmp (header, "BAR101C") &&
	 strcmp (header, "BAR100C"))) {
	fclose (input);
	return 0;
    }

    /* read the rest of the campaign file */
    if (! campaign->read (campaign, summary, input)) {
	fclose (input);
	return 0;
    }

    /* close the file and return */
    fclose (input);
    return 1;
}

/*----------------------------------------------------------------------
 * Constructors.
 */

/**
 * Campaign constructor.
 * @return the new campaign.
 */
Campaign *new_Campaign (void)
{
    /* local variables */
    Campaign *campaign; /* the campaign to return */

    /* get the Cwg object from the main program */
    cwg = get_Cwg ();

    /* reserve memory */
    if (! (campaign = malloc (sizeof (Campaign))))
	return NULL;

    /* initialise methods */
    campaign->destroy = destroy;
    campaign->clear = clear;
    campaign->write = write;
    campaign->read = read;
    campaign->load = load;

    /* initialise attributes */
    initialiseattributes (campaign);

    /* return the new campaign */
    return campaign;
}
