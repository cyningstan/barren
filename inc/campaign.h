/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * Created: 01-Jan-2021.
 *
 * Campaign Handler Header.
 */

/* types defined in this file */
typedef struct campaign Campaign;

#ifndef __CAMPAIGN_H__
#define __CAMPAIGN_H__

/* header dependencies */
#include "cwg.h"
#include "cgalib.h"
#include "utype.h"
#include "terrain.h"
#include "scenario.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/**
 * @const BARREN_SCENARIOS
 * The maximum number of scenarios in a campaign.
 */
#define BARREN_SCENARIOS 16

/**
 * @struct campaign holds data for a campaign.
 */
struct campaign {

    /*
     * Attributes.
     */

    /** @var filename The campaign filename. */
    char filename[13];

    /** @var name The name of the campaign. */
    char name[33];

    /**
     * @var singleplayer
     * 0 if the campaign is for 1 or 2 players.
     * 1 if the campaign is for 1 player 1st corporation.
     * 2 if the campaign is for 1 player 2nd corporation.
     */
    int singleplayer;

    /** @var corpnames The names of the corporations. */
    char corpnames[2][9];

    /** @var unittypes An array of unit types. */
    UnitType *unittypes[CWG_UTYPES];

    /** @var gatherer The unit type that gathers resources. */
    int gatherer;

    /** @var unitbitmaps An array of bitmaps for unit types. */
    Bitmap *unitbitmaps[4 * CWG_UTYPES];

    /** @var terrain An array of terrain types. */
    Terrain *terrain[CWG_TERRAIN];

    /** @var resource The terrain square that provides resources. */
    int resource;

    /** @var terrainbitmaps A array of bitmaps for terrain. */
    Bitmap *terrainbitmaps[16 * CWG_TERRAIN];

    /** @var corpbitmaps An array of corporate logo bitmaps. */
    Bitmap *corpbitmaps[2];

    /** @var scenarios An array of scenarios. */
    Scenario *scenarios[BARREN_SCENARIOS];

    /*
     * Methods.
     */

    /**
     * Destroy the campaign when no longer needed.
     * @param campaign is the campaign to destroy.
     */
    void (*destroy) (Campaign *campaign);

    /**
     * Clear the data from a campaign.
     * @param campaign is the campaign to clear.
     */
    void (*clear) (Campaign *campaign);

    /**
     * Save a campaign to an already-open file.
     * @param campaign is the campaign to save.
     * @param output is the already-open output file.
     * @return 1 if successful, 0 if not.
     */
    int (*write) (Campaign *campaign, FILE *output);

    /**
     * Load a campaign from an already-open file.
     * @param campaign The campaign to load.
     * @param summary  0 to load full campaign, 1 for summary only.
     * @param input    The already-open input file.
     * @return         1 if successful, 0 if not.
     */
    int (*read) (Campaign *campaign, int summary, FILE *input);

    /**
     * Load a campaign file. The filename attribute is used.
     * @param campaign The campaign to load.
     * @param summary  0 to load full campaign, 1 for summary only.
     * @return         1 if successful, 0 if not.
     */
    int (*load) (Campaign *campaign, int summary);

};

/*----------------------------------------------------------------------
 * Constructor Declarations.
 */

/**
 * Campaign constructor.
 * @return the new campaign.
 */
Campaign *new_Campaign (void);

#endif
