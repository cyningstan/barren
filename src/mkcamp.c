/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * Created: 11-Jan-2021.
 *
 * Campaign generation program.
 */

/*----------------------------------------------------------------------
 * Headers
 */

/* ANSI C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* compiler-specific headers */
#include <conio.h>

/* project-specific headers */
#include "cwg.h"
#include "utype.h"
#include "terrain.h"
#include "map.h"
#include "unit.h"
#include "battle.h"
#include "campaign.h"
#include "scenario.h"
#include "cgalib.h"
#include "fatal.h"

/*----------------------------------------------------------------------
 * Constants.
 */

/** @const BUFSIZE is the maximum input buffer size */
#define BUFSIZE 1024

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/** @enum read_state is the state of the line-reading FSM */
typedef enum read_state {
    STATE_NONE, /* not reading anything in particular */
    STATE_UNITTYPE, /* reading a unit type */
    STATE_TERRAIN, /* reading a terrain type */
    STATE_SCENARIO, /* reading a scenario */
    STATE_LAST /* placeholder */
} ReadState;

/** @var cwg The Cwg object. */
static Cwg *cwg = NULL;

/*----------------------------------------------------------------------
 * File Level Variables.
 */

/** @var input is the input file handle. */
static FILE *input;

/** @var filename is the base filename without extension. */
static char *filename;

/** @var line is the number of the input line being processed */
static int line = 0;

/** @var campaign is the campaign to build */
static Campaign *campaign;

/** @var unittype is a unit type being built */
static UnitType *unittype;

/** @var terrain is a terrain type being built. */
static Terrain *terrain;

/** @var scenario is a scenario being built. */
static Scenario *scenario;

/** @var bitmaps is temporary bitmap storage */
static Bitmap *bitmaps[20][8];

/**
 * @var mapchars is an array of characters for map definitions.
 * Note that this array of characters is *not* a string (no NUL).
 */
static char mapchars[12] = {
    '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0'
};

/** @var headers The possible output headers. */
static char *headers[] = {
    "BAR100C", /* obsolete */
    "BAR101C", /* obsolete */
    "BAR102C" /* corrected target square bug */
};

/** @var version Version for header output. */
static int version = 2;

/*----------------------------------------------------------------------
 * Level 4 Functions.
 */

/**
 * Add a unit type if it doesn't already exist.
 * @param name The name of the unit.
 * @return the unit ID.
 */
static int addunittype (char *name)
{
    int b, /* bitmap counter */
	u; /* current unit type count */
    UnitType *newunittype;

    /* count existing unit types */
    for (u = 0; u < CWG_UTYPES; ++u) {

	/* we're at the ebd of the unit list, add the unit */
	if (! campaign->unittypes[u]) {
	    if (! (newunittype = new_UnitType ()))
		fatalerror (FATAL_MEMORY);
	    strcpy (newunittype->name, name);
	    campaign->unittypes[u] = newunittype;
	    for (b = 0; b < 4; ++b)
		campaign->unitbitmaps[b + 4 * u] = bitmaps[b][u];
	    break;
	}

	/* unit name already exists */
	else if (! strcmp (campaign->unittypes[u]->name, name))
	    break;
    }

    /* return unit type id */
    return u;
}

/**
 * Ensure a scenario has a battle, and grab a pointer to it.
 * @return a pointer to the battle.
 */
static Battle *getscenariobattle (void)
{
    /* local variables */
    Battle *battle; /* the battle found or created */

    /* make sure we've begun defining a scenario */
    if (! scenario) {
	printf ("No scenario defined for line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* see if we need to make a battle now */
    if (! (battle = scenario->battle)) {
	if (! (battle = new_Battle (campaign->unittypes,
				    campaign->terrain))) {
	    printf ("Cannot create battle on line %d.\n", line);
	    fatalerror (FATAL_MEMORY);
	}
	scenario->battle = battle;
    }

    /* return the battle */
    return battle;
}

/*----------------------------------------------------------------------
 * Level 3 Functions.
 */

/**
 * Get a number from the input line.
 * @return the number.
 */
static int getnumberfromline (void)
{
    /* local variables */
    char *token; /* token from the command line */

    /* grab the next token */
    if (! (token = strtok (NULL, " \r\n"))) {
	printf ("Number missing on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* return the numeric value */
    return atoi (token);
}

/**
 * Look up a unit type among those already defined.
 * @return index of unit type in campaigns array.
 */
static int unittypelookup (void)
{
    /* local variables */
    char *token, /* token from the command line */
	*name; /* the name part of the token */
    UnitType *searchunit; /* pointer to unit searched */
    int u; /* unit type counter */

    /* grab the next token */
    if (! (token = strtok (NULL, "\r\n"))) {
	printf ("Unit type name missing on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* skip any initial + on a name */
    if (*token == '+')
	name = &token[1];
    else
	name = token;

    /* search for unit by name */
    searchunit = NULL;
    for (u = 0; u < CWG_UTYPES && ! searchunit; ++u)
	if (campaign->unittypes[u] &&
	    ! strcmp (campaign->unittypes[u]->name, name))
	    return u;

    /* not found - add the unit if prefixed by a + */
    if (*token == '+')    
	return addunittype (name);

    /* deal with unit type not found */
    printf ("Unrecognised unit type on line %d.\n", line);
    fatalerror (FATAL_INVALIDDATA);

    /* dummy return to keep compiler happy */
    return -1;
}

/**
 * Look up a terrain type among those already defined.
 * @return index of terrain type in campaigns array.
 */
static int terrainlookup (void)
{
    /* local variables */
    char *token; /* token from the command line */
    Terrain *searchterrain; /* pointer to unit searched */
    int t; /* unit type counter */

    /* grab the next token */
    if (! (token = strtok (NULL, "\r\n"))) {
	printf ("Terrain name missing on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* search for terrain by name */
    searchterrain = NULL;
    for (t = 0; t < CWG_TERRAIN && ! searchterrain; ++t)
	if (campaign->terrain[t] &&
	    ! strcmp (campaign->terrain[t]->name, token))
	    return t;

    /* deal with unit type not found */
    printf ("Unrecognised terrain type on line %d.\n", line);
    fatalerror (FATAL_INVALIDDATA);

    /* dummy return to keep compiler happy */
    return -1;
}

/**
 * Look up a player name from the next parameter on a line.
 * @return 0 or 1 for the player.
 */
static int playernamelookup (void) {

    /* local variables */
    char *token; /* token from the command line */
    int player = -1; /* player number found */

    /* grab the next token */
    if (! (token = strtok (NULL, " \r\n"))) {
	printf ("Player value missing on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* check against recognised player names */
    if (! strcmp (token, campaign->corpnames[0]))
	player = 0;
    else if (! strcmp (token, campaign->corpnames[1]))
	player = 1;

    /* deal with player name not found */
    else {
	printf ("Unknown player %s on line %d.\n", token, line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* return the player number */
    return player;
}

/**
 * Get text from the command line, and wrap it.
 * @return the wrapped text.
 */
static char *getwrappedtext (void) {

    /* local variables */
    char *token, /* token from the command line */
	*cptr, /* pointer to character in token */
	*spc; /* pointer to last space */
    int l; /* current line length */

    /* grab the next token */
    if (! (token = strtok (NULL, "\r\n"))) {
	printf ("Text value missing on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* wrap it */
    spc = NULL;
    l = 0;
    for (cptr = token; *cptr; ++cptr) {

	/* increment line length */
	++l;

	/* perform the wrap if the line is long enough */
	if (l > 32) {
	    if (*cptr == ' ')
		spc = cptr;
	    if (! spc) {
		printf ("Text unwrappable in line %d.\n", line);
		fatalerror (FATAL_INVALIDDATA);
	    }
	    *spc = '\n';
	    l = cptr - spc;
	    spc = NULL;
	}

	/* check for hard line breaks */
	if (*cptr == '\\') {
	    *cptr = '\n';
	    l = 0;
	    spc = NULL;
	}

	/* check for spaces */
	else if (*cptr == ' ')
	    spc = cptr;

    }

    /* return the wrapped string */
    return token;
}

/**
 * Ensure a scenario has a map, and grab a pointer to it.
 * @return a pointer to the map.
 */
static Map *getscenariomap (void)
{
    /* local variables */
    Battle *battle; /* the battle found or created */

    /* first check for the existence of the battle */
    battle = getscenariobattle ();

    /* then check the existence of the map for that battle */
    if (! battle->map)
	battle->map = new_Map ();
    if (! battle->map) {
	printf ("Cannot create map on line %d.\n", line);
	fatalerror (FATAL_MEMORY);
    }

    /* return the map */
    return battle->map;
}

/*----------------------------------------------------------------------
 * Level 2 Functions.
 */

/**
 * Process a 'unittype' command.
 * @param state is the current state.
 * @return the state after processing.
 */
static ReadState processunittypeline (ReadState state)
{
    /* local variables */
    char *token; /* unit type name token */
    int utypeid; /* id of unit type */

    /* get unit type */
    if (! (token = strtok (NULL, "\r\n"))) {
	printf ("Unit type name missing on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* initialise the unit type */
    utypeid = addunittype (token);
    unittype = campaign->unittypes[utypeid];

    /* return the new state */
    return STATE_UNITTYPE;
}

/**
 * Process a 'terrain' command.
 * @param state is the current state.
 * @return the state after processing.
 */
static ReadState processterrainline (ReadState state)
{
    /* local variables */
    char *token; /* unit type name token */
    int b, /* bitmap counter */
	t; /* current terrain type count */

    /* get unit type */
    if (! (token = strtok (NULL, "\r\n"))) {
	printf ("Terrain type name missing on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* initialise the terrain type */
    if (! (terrain = new_Terrain ()))
	fatalerror (FATAL_MEMORY);
    strcpy (terrain->name, token);

    /* count the terrain types */
    for (t = 0; t < CWG_TERRAIN; ++t)
	if (! campaign->terrain[t])
	    break;

    /* get the unit type bitmaps */

    /* add the terrain type and bitmaps to the campaign */
    campaign->terrain[t] = terrain;
    for (b = 0; b < 16; ++b)
	campaign->terrainbitmaps[b + 16 * t] = bitmaps[4 + b][t];

    /* deal with unrecognised command */
    return STATE_TERRAIN;
}

/**
 * Process a 'scenario' command.
 * @param state is the current state.
 * @return the state after processing.
 */
static ReadState processscenarioline (ReadState state)
{
    /* local variables */
    int id; /* id of scenario */

    /* Create scenario */
    scenario = new_Scenario (campaign);
    scenario->battle = new_Battle (campaign->unittypes,
				   campaign->terrain);
    id = getnumberfromline ();
    campaign->scenarios[id - 1] = scenario;

    /* deal with unrecognised command */
    return STATE_SCENARIO;
}

/**
 * Process a 'name' command.
 */
static void processnameline (void)
{
    /* local variables */
    char *token; /* campaign name token */

    /* get the name from the next token */
    if (! (token = strtok (NULL, "\r\n"))) {
	printf ("Campaign name missing on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* set the campaign name */
    strcpy (campaign->name, token);
}

/**
 * Process a 'player' command.
 */
static void processplayerline (void)
{
    char *token; /* player detail token */
    int side; /* player number 0 or 1 */

    /* get the player number from the next token */
    if (! (token = strtok (NULL, " "))) {
	printf ("Player number or name missing on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }
    side = atoi (token) - 1;
    if (side < 0 || side > 1) {
	printf ("Player number invalid on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* get the player name from the next token */
    if (! (token = strtok (NULL, "\r\n"))) {
	printf ("Player name missing on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }
    if (strlen (token) < 1 || strlen (token) > 8) {
	printf ("Player name invalid on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }
    strcpy (campaign->corpnames[side], token);
}

/**
 * Process a 'human' command.
 */
static void processhumanline (void)
{
    int side; /* player number 0 or 1 */
    side = playernamelookup ();
    campaign->singleplayer = 1 + side;
}

/**
 * Process a 'gatherer' line.
 */
static void processgathererline (void)
{
    campaign->gatherer = unittypelookup ();
}

/**
 * Process a 'resource' line.
 */
static void processresourceline (void)
{
    campaign->resource = terrainlookup ();
}

/**
 * Process a 'cost' command.
 */
static void processcostline (void)
{
    /* local variables */
    char *token; /* unit cost token */

    /* get the cost from the next token */
    if (! (token = strtok (NULL, "\r\n"))) {
	printf ("Cost value missing on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* set the unit type hits */
    unittype->cost = atoi (token);
}

/**
 * Process a 'hits' command.
 */
static void processhitsline (void)
{
    /* local variables */
    char *token; /* unit hits token */

    /* get the hits from the next token */
    if (! (token = strtok (NULL, "\r\n"))) {
	printf ("Hits value missing on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* set the unit type hits */
    unittype->hits = atoi (token);
}

/**
 * Process a 'power' command.
 */
static void processpowerline (void)
{
    /* local variables */
    char *token; /* unit power token */

    /* get the power from the next token */
    if (! (token = strtok (NULL, "\r\n"))) {
	printf ("Power value missing on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* set the unit type power */
    unittype->power = atoi (token);
}

/**
 * Process a 'range' command.
 */
static void processrangeline (void)
{
    /* local variables */
    char *token; /* unit range token */

    /* get the range from the next token */
    if (! (token = strtok (NULL, "\r\n"))) {
	printf ("Range value missing on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* set the unit type range */
    unittype->range = atoi (token);
}

/**
 * Process a 'armour' command.
 */
static void processarmourline (void)
{
    /* local variables */
    char *token; /* unit armour token */

    /* get the armour from the next token */
    if (! (token = strtok (NULL, "\r\n"))) {
	printf ("Armour value missing on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* set the unit type armour */
    unittype->armour = atoi (token);
}

/**
 * Process a 'moves' command for a unit type.
 */
static void processunitmovesline (void)
{
    /* local variables */
    char *token; /* unit moves token */

    /* get the moves from the next token */
    if (! (token = strtok (NULL, "\r\n"))) {
	printf ("Moves value missing on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* set the unit type moves */
    unittype->moves = atoi (token);
}

/**
 * Process a 'build' line for a unit type.
 */
static void processunitbuildline (void)
{
    unittype->builds[unittypelookup ()] = 1;
}

/**
 * Process a 'map' command in a terrain section.
 */
static void processterrainmapline (void)
{
    /* local variables */
    char *token; /* parameter token */
    int t; /* terrain counter */

    /* get the parameter from the next token */
    if (! (token = strtok (NULL, "\r\n"))) {
	printf ("Value missing on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* set the map character */
    for (t = 0; t < CWG_TERRAIN; ++t)
	if (campaign->terrain[t] == terrain)
	    break;
    mapchars[t] = *token;
}

/**
 * Process a 'defence' command in a terrain section.
 */
static void processdefenceline (void)
{
    /* local variables */
    char *token; /* parameter token */
    int defence, /* defence factor */
	u; /* unit type index */

    /* get the defence parameter from the next token */
    if (! (token = strtok (NULL, " \r\n"))) {
	printf ("Defence value missing on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }
    defence = atoi (token);

    /* get the unit type parameter from the rest of the input line */
    u = unittypelookup ();

    /* set the terrain's defence for this unit */
    terrain->defence[u] = defence;
}

/**
 * Process a 'moves' command in a terrain section.
 */
static void processterrainmovesline (void)
{
    /* local variables */
    char *token; /* parameter token */
    int moves, /* moves factor */
	u; /* unit type index */

    /* get the moves parameter from the next token */
    if (! (token = strtok (NULL, " \r\n"))) {
	printf ("Moves value missing on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }
    moves = atoi (token);

    /* get the unit type parameter from the rest of the input line */
    u = unittypelookup ();

    /* set the terrain's movement cost for this unit */
    terrain->moves[u] = moves;
}

/**
 * Process a 'briefing' command in a scenario section.
 */
static void processbriefingline (void)
{
    /* local variables */
    char *briefing; /* wrapped briefing */
    int player; /* player number */

    /* get the player parameter from the next token */
    player = playernamelookup ();
    briefing = getwrappedtext ();

    /* set the appropriate briefing */
    strcpy (scenario->text[SCENARIO_BRIEFING_1 + player], briefing);
}

/**
 * Process a 'victory' command in a scenario section.
 */
static void processvictoryline (void)
{
    /* local variables */
    char *debriefing; /* wrapped debriefing */
    int player; /* player number */

    /* get the player parameter from the next token */
    player = playernamelookup ();
    debriefing = getwrappedtext ();

    /* set the appropriate briefing */
    strcpy (scenario->text[SCENARIO_VICTORY_1 + player], debriefing);
}

/**
 * Process a 'defeat' command in a scenario section.
 */
static void processdefeatline (void)
{
    /* local variables */
    char *debriefing; /* wrapped debriefing */
    int player; /* player number */

    /* get the player parameter from the next token */
    player = playernamelookup ();
    debriefing = getwrappedtext ();

    /* set the appropriate briefing */
    strcpy (scenario->text[SCENARIO_DEFEAT_1 + player], debriefing);
}

/**
 * Process a 'next' command in a scenario section.
 */
static void processnextline (void)
{
    /* local variables */
    char *token; /* parameter token */
    int player, /* player number */
	next; /* next scenario ID */

    /* get the player parameter from the next token */
    player = playernamelookup ();

    /* get the next scenario upon that player's victory */
    if (! (token = strtok (NULL, "\r\n"))) {
	printf ("Next scenario value missing on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }
    next = atoi (token);

    /* set the next scenario value */
    scenario->next[player] = next;
}

/**
 * Process a 'resources' command in a scenario section.
 */
static void processresourcesline (void)
{
    /* local variables */
    Battle *battle; /* battle for this scenario */
    char *token; /* parameter token */
    int player, /* player number */
	resources; /* resources value */

    /* ensure scenario has a battle set up */
    battle = getscenariobattle ();

    /* get the player parameter from the next token */
    player = playernamelookup ();

    /* get the next scenario upon that player's victory */
    if (! (token = strtok (NULL, "\r\n"))) {
	printf ("Next scenario value missing on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }
    resources = atoi (token);

    /* set the next scenario value */
    battle->resources[player] = resources;
}

/**
 * Process a 'map' command in a scenario section.
 */
static void processscenariomapline (void)
{
    /* local variables */
    Map *map; /* map for that battle */
    int width, /* width of map */
	height, /* height of map */
	x, /* x coordinate of map sweep */
	y; /* y coordinate of map sweep */
    char *mapstr, /* map string token */
	*tptr; /* pointer to terrain character */
    int t; /* terrain index */

    /* ensure scenario has a battle set up, with a map */
    map = getscenariomap ();

    /* get width and height, setting and verifying map dimensions */
    width = getnumberfromline ();
    height = getnumberfromline ();
    if (! map->size (map, width, height)) {
	printf ("Invalid map size %dx%d set on line %d.\n",
		width, height, line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* get the map string */
    if (! (mapstr = strtok (NULL, "\r\n"))) {
	printf ("Map string missing on line %d.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }
    if (strlen (mapstr) != width * height) {
	printf ("Invalid map length %d (should be %d) on line %d.\n",
		strlen (mapstr), width * height, line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* set the map terrain from the map string */
    for (x = 0; x < width; ++x)
	for (y = 0; y < height; ++y) {
	    if (! (tptr = strchr (mapchars, mapstr[x + width * y]))) {
		printf ("Invalid map character '%c' in line %d.\n",
			mapstr[x + width * y], line);
		fatalerror (FATAL_INVALIDDATA);
	    }
	    t = tptr - mapchars;
	    map->terrain[x + width * y] = (char) t;
	}
}

/**
 * Process a 'build' command in a scenario section.
 */
static void processscenariobuildline (void)
{
    Battle *battle; /* the scenario's battle */
    battle = getscenariobattle ();
    battle->builds[unittypelookup ()] = 1;
}

/**
 * Process a 'unit' command in a scenario section.
 */
static void processunitline (void)
{
    /* local variables */
    Map *map; /* the ma on which to place the unit */
    int x, /* x position of unit */
	y, /* y position of unit */
	player, /* player to whom the unit belongs */
	u, /* new unit id */
	ut; /* unit type id */
    Unit *unit; /* new unit */

    /* ensure that the map exists */
    map = getscenariomap ();

    /* grab and verify the coordinates */
    x = getnumberfromline ();
    y = getnumberfromline ();
    if (x < 0 || x >= map->width ||
	y < 0 || y >= map->height) {
	printf ("Location %d,%d off %dx%d map in line %d.\n", x, y,
		map->width, map->height, line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* grab the side name */
    player = playernamelookup ();

    /* find free unit slot */
    for (u = 0; u < CWG_UNITS; ++u)
	if (! scenario->battle->units[u])
	    break;
    if (u == CWG_UNITS) {
	printf ("Too may units (%d) in line %d.\n", u, line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* identify the unit type and create the unit */
    ut = unittypelookup ();
    unit = scenario->battle->units[u] = new_Unit ();
    strcpy (unit->name, campaign->unittypes[ut]->name);
    unit->side = player;
    unit->utype = ut;
    unit->x = x;
    unit->y = y;
    map->units[x + map->width * y] = u;
    unit->hits = campaign->unittypes[ut]->hits;
    unit->moves = campaign->unittypes[ut]->moves;
}

/**
 * Process a 'point' command in a scenario section.
 */
static void processpointline (void)
{
    /* local variables */
    Map *map; /* the ma on which to place the unit */
    int x, /* x coordinate of victory point */
	y; /* y coordinate of victory point */

    /* ensure that the map exists */
    map = getscenariomap ();

    /* grab and verify the coordinates */
    x = getnumberfromline ();
    y = getnumberfromline ();
    if (x < 0 || x >= map->width ||
	y < 0 || y >= map->height) {
	printf ("Location %d,%d off %dx%d map.\n", x, y,
		map->width, map->height);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* add the point to the map */
    map->points[x + map->width * y] = 1;
}

/*----------------------------------------------------------------------
 * Level 1 Functions.
 */

/**
 * Initialise the command line options.
 * @param argc is the argument count.
 * @param argv is the array of arguments.
 */
static void initialiseoptions (int argc, char **argv)
{
    if (argc != 2)
	fatalerror (FATAL_COMMAND_LINE);
    filename = argv[1];
}

/**
 * Initialise the campaign.
 */
static void initialisecampaign (void)
{
    campaign = new_Campaign ();
}

/**
 * Initialise the screen.
 */
static void initialisescreen (void)
{
    /* local variables */
    int bc, /* bitmap column counter */
	br; /* bitmap row counter */
    char header[8], /* input file header */
	*bytes, /* buffer for the input screen */
	*picfilename; /* name of PIC file */
    Screen *screen; /* the CGALIB screen */

    /* initialise the screen itself */
    if (! (screen = scr_create (5)))
	fatalerror (FATAL_DISPLAY);
    scr_palette (screen, 5, 8);
    fatalscreen (screen);

    /* load the campaign graphics */
    if (! (picfilename = malloc (strlen (filename) + 5)))
	fatalerror (FATAL_MEMORY);
    sprintf (picfilename, "%s.pic", filename);
    if (! (input = fopen (picfilename, "rb")))
	fatalerror (FATAL_DISPLAY);
    if (! fread (header, 7, 1, input)) {
        printf ("Cannot read header from %s.\n", picfilename);
        fatalerror (FATAL_DISPLAY);
    }
    if (! (bytes = malloc (16192))) {
        printf ("Cannot reserve memory for screen.\n");
        fatalerror (FATAL_DISPLAY);
    }
    if (! fread (bytes, 16192, 1, input)) {
        printf ("Cannot read bytes from %s.\n", picfilename);
        fatalerror (FATAL_DISPLAY);
    }
    fclose (input);
    _fmemcpy ((char far *) 0xb8000000, bytes, 16192);
    free (bytes);

    /* initialise the unit-terrain bitmaps */
    for (bc = 0; bc < 20; ++bc)
	for (br = 0; br < 8; ++br) {
	    if (! (bitmaps[bc][br] = bit_create (16, 16)))
		fatalerror (FATAL_MEMORY);
	    scr_get (screen, bitmaps[bc][br], 16 * bc, 16 * br);
	}

    /* initialise the corporate bitmaps */
    for (bc = 0; bc < 2; ++bc) {
	if (! (campaign->corpbitmaps[bc] = bit_create (32, 24)))
	    fatalerror (FATAL_MEMORY);
	scr_get (screen, campaign->corpbitmaps[bc], 32 * bc, 128);
    }

    /* clean up screen */
    scr_destroy (screen);
    fatalscreen (NULL);
    free (picfilename);
}

/**
 * Open the input file.
 */
static void openinputfile (void)
{
    /* local variables */
    char *inputfilename;

    /* ascertain the filename */
    if (! (inputfilename = malloc (strlen (filename) + 5)))
	fatalerror (FATAL_MEMORY);
    sprintf (inputfilename, "%s.cmi", filename);

    /* attempt to open the input file */
    if (! (input = fopen (inputfilename, "r")))
	fatalerror (FATAL_INVALIDDATA);

    /* clean up */
    free (inputfilename);
}

/**
 * Process a single input line.
 * @return 1 if a line was read, 0 if EOF was met.
 */
static int processinputline (void)
{
    /* local variables */
    char buf[BUFSIZE], /* input buffer */
	*pos, /* position of character we're searching for */
	*token; /* initial command token */
    static ReadState state = STATE_NONE; /* reading state */

    /* read a line into the buffer and validate it */
    if (! fgets (buf, BUFSIZE, input))
	return 0;
    ++line;
    if (! strchr (buf, '\n')) {
	printf ("Line %d too long.\n", line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* filter out comments and blank lines */
    if ((pos = strchr (buf, '#')))
	*pos = '\n';
    if (*buf == '\n')
	return 1;

    /* tokenise the line */
    token = strtok (buf, " \r\n");
    
    /* process change-of-state commands */
    if (! strcmp (token, "unittype"))
	state = processunittypeline (state);
    else if (! strcmp (token, "terrain"))
	state = processterrainline (state);
    else if (! strcmp (token, "scenario"))
	state = processscenarioline (state);

    /* process campaign commands */
    else if (! strcmp (token, "name") && state == STATE_NONE)
	processnameline ();
    else if (! strcmp (token, "player") && state == STATE_NONE)
	processplayerline ();
    else if (! strcmp (token, "human") && state == STATE_NONE)
	processhumanline ();

    /* process various unit type commands */
    else if (! strcmp (token, "cost") && state == STATE_UNITTYPE)
	processcostline ();
    else if (! strcmp (token, "hits") && state == STATE_UNITTYPE)
	processhitsline ();
    else if (! strcmp (token, "power") && state == STATE_UNITTYPE)
	processpowerline ();
    else if (! strcmp (token, "range") && state == STATE_UNITTYPE)
	processrangeline ();
    else if (! strcmp (token, "armour") && state == STATE_UNITTYPE)
	processarmourline ();
    else if (! strcmp (token, "moves") && state == STATE_UNITTYPE)
	processunitmovesline ();
    else if (! strcmp (token, "build") && state == STATE_UNITTYPE)
	processunitbuildline ();

    /* process various terrain commands */
    else if (! strcmp (token, "map") && state == STATE_TERRAIN)
	processterrainmapline ();
    else if (! strcmp (token, "defence") && state == STATE_TERRAIN)
	processdefenceline ();
    else if (! strcmp (token, "moves") && state == STATE_TERRAIN)
	processterrainmovesline ();

    /* resource related commands */
    else if (! strcmp (token, "gatherer"))
	processgathererline ();
    else if (! strcmp (token, "resource"))
	processresourceline ();

    /* process various scenario commands */
    else if (! strcmp (token, "briefing") && state == STATE_SCENARIO)
	processbriefingline ();
    else if (! strcmp (token, "victory") && state == STATE_SCENARIO)
	processvictoryline ();
    else if (! strcmp (token, "defeat") && state == STATE_SCENARIO)
	processdefeatline ();
    else if (! strcmp (token, "next") && state == STATE_SCENARIO)
	processnextline ();
    else if (! strcmp (token, "resources") && state == STATE_SCENARIO)
	processresourcesline ();
    else if (! strcmp (token, "map") && state == STATE_SCENARIO)
	processscenariomapline ();
    else if (! strcmp (token, "build") && state == STATE_SCENARIO)
	processscenariobuildline ();
    else if (! strcmp (token, "unit") && state == STATE_SCENARIO)
	processunitline ();
    else if (! strcmp (token, "point") && state == STATE_SCENARIO)
	processpointline ();

    /* process unrecognised commands */
    else {
	printf ("Unrecognised command: %s on line %d.\n", token, line);
	fatalerror (FATAL_INVALIDDATA);
    }

    /* tell the calling process that we didn't reach EOF */
    return 1;
}

/**
 * Close the input file.
 */
static void closeinputfile (void)
{
    fclose (input);
}

/**
 * Save the campaign.
 */
static void savecampaign (void)
{
    /* local variables */
    FILE *output; /* output file */
    char *outputfilename; /* name of output file */

    /* ascertain the filename */
    if (! (outputfilename = malloc (strlen (filename) + 5)))
	fatalerror (FATAL_MEMORY);
    sprintf (outputfilename, "%s.cam", filename);

    /* attempt to open the output file and output the header */
    if (! (output = fopen (outputfilename, "wb")))
	fatalerror (FATAL_NODATA);
    if (! (fwrite (headers[version], 8, 1, output)))
	fatalerror (FATAL_NODATA);

    /* write the rest of the campaign data */
    if (! (campaign->write (campaign, output)))
	fatalerror (FATAL_NODATA);

    /* close the output file and clean up */
    fclose (output);
    free (outputfilename);
}

/**
 * Close the screen bitmaps.
 */
static void closescreen (void)
{
    /* local variables */
    int bc, /* bitmap column counter */
	br; /* bitmap row counter */

    /* free up the bitmaps */
    for (bc = 0; bc < 20; ++bc)
	for (br = 0; br < 12; ++br)
	    bit_destroy (bitmaps[bc][br]);
}

/*----------------------------------------------------------------------
 * Top Level Function.
 */

/**
 * Main Program.
 * @param argc is the command line argument count.
 * @param argv is the array of command line arguments.
 * @return 0 on success, >0 on failure.
 */
int main (int argc, char **argv)
{
    /* initialisation */
    initialiseoptions (argc, argv);
    cwg = get_Cwg ();
    initialisecampaign ();
    initialisescreen ();

    /* process input file */
    openinputfile ();
    while (processinputline ());
    closeinputfile ();

    /* complete and save the campaign */
    savecampaign ();

    /* clean up */
    closescreen ();
    campaign->destroy (campaign);
    cwg->destroy ();
    return 0;
}
