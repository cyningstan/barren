/*======================================================================
 * CWG: Cyningstan Wargame Engine.
 * An engine for simple turn-based wargames.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 04-Sep-2020
 *
 * Testbed Program and Demonstration Game.
 */

/* Standard C Headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

/* Project Headers */
#include "cwg.h"
#include "battle.h"
#include "utype.h"
#include "terrain.h"
#include "unit.h"
#include "map.h"

/*----------------------------------------------------------------------
 * File Level Variables.
 */

/** @var cwg is the Cwg object. */
static Cwg *cwg;

/** @var battle is the battle. */
static Battle *battle;

/** @var utypes is the array of unit types. */
static UnitType *utypes[CWG_UTYPES];

/** @var terrain is the array of terrain types. */
static Terrain *terrain[CWG_TERRAIN];

/*----------------------------------------------------------------------
 * Service Level Functions.
 */

/**
 * Exit with an error message.
 * @param message is the error message.
 */
static void fatal_error (char *message)
{
    printf ("Error: %s\n", message);
    exit (1);
}

/*----------------------------------------------------------------------
 * Level 3 Functions.
 */

/**
 * Initialise a unit type.
 * @param name is the unit type name.
 * @param cost is the cost to build.
 * @param hits is the hit points.
 * @param power is the attack power.
 * @param range is the attack range.
 * @param armour is the protective armour.
 * @param moves is the movement points.
 * @param build is 1 if this unit type can build the others.
 * @return a new unit type.
 */
static UnitType *initutype (char *name, int cost, int hits, int power,
			    int range, int armour, int moves, int build)
{
    /* local variables */
    UnitType *utype; /* unit type to return */
    int u; /* unit type counter */

    /* attempt to reserve memory */
    if (! (utype = new_UnitType ()))
	fatal_error ("Cannot create unit type.");

    /* set the simple attributes */
    strcpy (utype->name, name);
    utype->cost = cost;
    utype->hits = hits;
    utype->power = power;
    utype->range = range;
    utype->armour = armour;
    utype->moves = moves;

    /* set as a builder */
    if (build)
	for (u = 0; u < CWG_UTYPES && battle->utypes[u]; ++u)
	    utype->builds[u] = 1;
    
    /* return the unit type */
    return utype;
}

/**
 * Initialise a terrain type.
 * @param name is the terrain type name.
 * @param mov0 is the movement cost for unit type 0.
 * @param mov1 is the movement cost for unit type 1.
 * @param mov2 is the movement cost for unit type 2.
 * @param mov3 is the movement cost for unit type 3.
 * @param def0 is the defence bonus for unit type 0.
 * @param def1 is the defence bonus for unit type 1.
 * @param def2 is the defence bonus for unit type 2.
 * @param def3 is the defence bonus for unit type 3.
 * @return a new terrain type.
 */
static Terrain *initterrain (char *name, int mov0, int mov1, int mov2,
			     int mov3, int def0, int def1, int def2,
			     int def3)
{
    /* local variables */
    Terrain *terrain; /* the terrain to create */
    
    /* reserve the memory */
    if (! (terrain = new_Terrain ()))
	fatal_error ("Cannot create terrain.");

    /* set the name */
    strcpy (terrain->name, name);

    /* set the movement cost */
    terrain->moves[0] = mov0;
    terrain->moves[1] = mov1;
    terrain->moves[2] = mov2;
    terrain->moves[3] = mov3;

    /* set the defence bonus */
    terrain->defence[0] = def0;
    terrain->defence[1] = def1;
    terrain->defence[2] = def2;
    terrain->defence[3] = def3;

    /* return the terrain */
    return terrain;
}

/**
 * Initialise a new map with open terrain.
 */
static void initmap (void)
{
    if (! (battle->map = new_Map ()))
	fatal_error ("Cannot create map.");
    if (! battle->map->size (battle->map, 20, 12))
	fatal_error ("Cannot initialise map.");
}

/**
 * Add hills to 50% of maps.
 */
static void addhillstomap (void)
{
    /* local variables */
    int x, /* x coordinate */
	y, /* y coordinate */
	h; /* counter of hill squares */
    Map *map; /* convenience pointer to battle map */

    /* only 50% chance of hills */
    if (rand () % 2)
	return;

    /* procedurally generate hill over 10% of surface */
    map = battle->map;
    x = rand () % 20;
    y = rand () % 12;
    h = 0;
    while (h < 24) {
	if (x >= 0 && x < 20 &&
	    y >= 0 && y < 12 &&
	    map->terrain[x + map->width * y] == 0) {
	    map->terrain[x + map->width * y] = 1;
	    ++h;
	}
	if (rand () % 8) {
	    x += rand () % 3 - 1;
	    y += rand () % 3 - 1;
	} else {
	    x = rand () % 20;
	    y = rand () % 12;
	}
    }
}

/**
 * Add forests to 50% of maps.
 */
static void addforesttomap (void)
{
    /* local variables */
    int x, /* x coordinate */
	y, /* y coordinate */
	f; /* counter of forest squares */
    Map *map; /* convenience pointer to battle map */

    /* only 50% chance of forests */
    if (rand () % 2)
	return;

    /* procedurally generate woods over 10% of surface */
    map = battle->map;
    x = rand () % 20;
    y = rand () % 12;
    f = 0;
    while (f < 24) {
	if (x >= 0 && x < 20 &&
	    y >= 0 && y < 12 &&
	    map->terrain[x + map->width * y] == 0) {
	    map->terrain[x + map->width * y] = 2;
	    ++f;
	}
	if (rand () % 8) {
	    x += rand () % 3 - 1;
	    y += rand () % 3 - 1;
	} else {
	    x = rand () % 20;
	    y = rand () % 12;
	}
    }
}

/**
 * Add forts to 50% of maps.
 */
static void addfortstomap (void)
{
    /* local variables */
    int x, /* x coordinate */
	y, /* y coordinate */
	f; /* counter of fort squares */
    Map *map; /* convenience pointer to battle map */

    /* only 50% chance of forts */
    if (rand () % 2)
	return;

    /* procedurally generate four forts */
    map = battle->map;
    f = 0;
    while (f < 4) {
	x = 5 * f + rand () % 5;
	y = rand () % 12;
	if (map->terrain[x + map->width * y] == 0) {
	    map->terrain[x + map->width * y] = 2;
	    map->points[x + map->width * y] = 1;
	    ++f;
	}
    }
}

/**
 * Add the game units to the map.
 */
static void addunitstomap (void)
{
    /* local variables */
    int s, /* side counter */
	u, /* unit counter */
	x, /* x coordinate for unit */
	y; /* y coordinate for unit */
    Unit *unit; /* convenience variable */
    Map *map; /* convenience pointer to battle map */

    /* initialise convenience pointers */
    map = battle->map;

    /* loop through all units */
    for (s = 0; s < 2; ++s)
	for (u = 0; u < 16; ++u) {

	    /* reserve memory and set unit type */
	    unit = battle->units[u + 16 * s] = new_Unit ();
	    unit->utype = u / 5;
	    unit->side = s;

	    /* set name and stats */
	    sprintf (unit->name, "%s %d",
		     battle->utypes[unit->utype]->name, 1 + (u % 5));
	    unit->hits = battle->utypes[unit->utype]->hits;
	    unit->moves = battle->utypes[unit->utype]->moves;

	    /* find a nice place */
	    do {
		x = rand () % 5;
		if (s)
		    x = 19 - x;
		y = rand () % 12;
	    } while (map->units[x + map->width * y] != CWG_NO_UNIT ||
		     map->terrain[x + map->width * y] != 0);
	    unit->x = x;
	    unit->y = y;
	    map->units[x + map->width * y] = u + 16 * s;
	}
}

/*
 * Initialise the Battle
 */
static void initbattle (void)
{
    /* local variables */
    int c; /* general counter */

    /* create the battle */
    if (! (battle = new_Battle (utypes, terrain)))
	fatal_error ("Cannot create battle.");
    for (c = 0; c < 2; ++c)
	battle->resources[c] = 12;
    for (c = 0; c < 3; ++c)
	battle->builds[c] = 1;
    
    /* initialise the unit types */
    battle->utypes[0] = initutype ("Infantry", 4, 6, 3, 1, 2, 2, 0);
    battle->utypes[1] = initutype ("Cavalry", 6, 4, 3, 1, 2, 4, 0);
    battle->utypes[2] = initutype ("Artillery", 8, 3, 6, 3, 0, 2, 0);
    battle->utypes[3] = initutype ("Camp", 0, 3, 0, 0, 0, 2, 1);

    /* initialise the terrain types */
    battle->terrain[0] = initterrain ("Open", 1, 1, 1, 0, 0, 0, 0, 0);
    battle->terrain[1] = initterrain ("Hill", 1, 1, 2, 0, 1, 1, 1, 1);
    battle->terrain[2] = initterrain ("Wood", 2, 2, 0, 0, 2, 2, 2, 2);
    battle->terrain[3] = initterrain ("Fort", 2, 0, 0, 0, 4, 4, 4, 4);
}

/*----------------------------------------------------------------------
 * Level 2 Functions.
 */

/**
 * Attempt to load a game.
 * @return 1 if the game was loaded, 0 if not.
 */
static int loadgame (void)
{
    return 0;
}

/**
 * Attempt to create a game.
 * @return 1 if the game was created, 0 if not.
 */
static int creategame (void)
{
    initbattle ();
    initmap ();
    addhillstomap ();
    addforesttomap ();
    addfortstomap ();
    addunitstomap ();
    return 1;
}

/**
 * Get a command from the console.
 * @return a pointer to the allocated string
 */
static char *getcommand (void)
{
    char *command;
    int len;
    printf ("Command> ");
    command = malloc (81);
    if (! fgets (command, 80, stdin))
	strcpy (command, "quit");
    if ((len = strlen (command)) && command[len - 1] == '\n')
	command[len - 1] = '\0';
    return command;
}

/**
 * Display the current state of the map.
 */
static void showmap (void)
{
    int x, y, /* map coordinate counters */
	unitid; /* id of unit on the map */
    Unit *unit; /* unit to examine */
    char abbrev[3]; /* abbbreviated name */
    Map *map; /* convenience pointer to battle map */

    /* loop through the whole map */
    map = battle->map;
    for (y = 0; y < 12; ++y) {
	for (x = 0; x < 20; ++x)
	    if ((unitid = map->units[x + map->width * y]) != -1 &&
		(unit = battle->units[unitid])) {
		sprintf (abbrev, "%c%c", unit->name[0], unit->name[2]);
		printf ("%s", abbrev);
	    } else if (map->terrain[x + map->width * y] == 0)
		printf ("..");
	    else if (map->terrain[x + map->width * y] == 1)
		printf ("^^");
	    else if (map->terrain[x + map->width * y] == 2)
		printf ("**");
	    else if (map->terrain[x + map->width * y] == 3)
		printf ("[]");
	printf ("\n");
    }
}

/**
 * Describe a location.
 */
static void describe (char *command)
{
    /* local variables */
    char *token; /* coordinates in string form, hopefully */
    int x, /* x coordinate */
	y, /* y coordinate */
	unitid, /* unit id on map */
	terrainid; /* terrain at location */
    Unit *unit; /* unit at location */

    /* begin the tokenisation */
    if (! (token = strtok (command, " "))) {
	printf ("Error tokenising %s.\n", command);
	return;
    }

    /* check for the existence of coordinates */
    if (! (token = strtok (NULL, " "))) {
	printf ("Usage: desc X,Y\n");
	return;
    }

    /* check for validity of coordinate token */
    if (sscanf (token, "%d,%d", &x, &y) != 2) {
	printf ("Usage: desc X,Y\n");
	return;
    }

    /* check that coordinates are on the map */
    if (x < 0 || x >= battle->map->width ||
	y < 0 || y >= battle->map->height) {
	printf ("Error: coordinates must be between 0,0 and %d,%d\n",
		battle->map->width - 1, battle->map->height - 1);
	return;
    }

    /* show any unit stats */
    if ((unitid = battle->map->units[x + battle->map->width * y])
	!= -1) {
	unit = battle->units[unitid];
	printf ("Unit: %s h:%d/%d p:%d r:%d a:%d m:%d/%d\n",
		unit->name,
		unit->hits,
		battle->utypes[unit->utype]->hits,
		battle->utypes[unit->utype]->power,
		battle->utypes[unit->utype]->range,
		battle->utypes[unit->utype]->armour,
		unit->moves,
		battle->utypes[unit->utype]->moves);
    }

    /* show any terrain type */
    terrainid = battle->map->terrain[x + battle->map->width * y];
    printf ("Terrain: %s\n", battle->terrain[terrainid]->name);
}

/**
 * Give a unit a movement order.
 */
static void moveunit (char *command)
{
    /* local variables */
    char *token; /* for tokenising command */
    int x1, /* unit x coordinate */
	y1, /* unit y coordinate */
	x2, /* target x coordinate */
	y2, /* target y coordinate */
	unitid; /* unit id on map */
    Unit *unit; /* unit to order */

    /* check initial token */
    if (! (token = strtok (command, " "))) {
	printf ("Error tokenising %s.\n", command);
	return;
    }

    /* look for unit to order */
    if (! (token = strtok (NULL, " ")) ||
	sscanf (token, "%d,%d", &x1, &y1) != 2) {
	printf ("Usage: order X,Y X,Y\n");
	return;
    }

    /* look for destination */
    if (! (token = strtok (NULL, " ")) ||
	sscanf (token, "%d,%d", &x2, &y2) != 2) {
	printf ("Usage: order X,Y X,Y\n");
	return;
    }

    /* check that coordinates are on the map */
    if (x2 < 0 || x2 >= battle->map->width ||
	y2 < 0 || y2 >= battle->map->height) {
	printf ("Error: coordinates must be between 0,0 and %d,%d\n",
		battle->map->width - 1, battle->map->height - 1);
	return;
    }

    /* check for unit */
    if ((unitid = battle->map->units[x1 + battle->map->width * y1]) == -1 ||
	! (unit = battle->units[unitid])) {
	printf ("Error: no unit at %d,%d.\n", x1, y1);
	return;
    }

    /* move unit */
    if (battle->move (battle, unit, x2, y2, NULL))
	showmap ();
}

/**
 * Give a unit an attack order.
 */
static void attackunit (char *command)
{
    /* local variables */
    char *token; /* for tokenising command */
    int x1, /* unit x coordinate */
	y1, /* unit y coordinate */
	x2, /* target x coordinate */
	y2, /* target y coordinate */
	unitid, /* id of unit attacking on map */
	targetid; /* id of target unit on map */
    Unit *unit, /* unit to order */
	*target; /* target of attack */
    Map *map; /* convenience pointer to map */

    /* check initial token */
    if (! (token = strtok (command, " "))) {
	printf ("Error tokenising %s.\n", command);
	return;
    }

    /* look for unit to order */
    if (! (token = strtok (NULL, " ")) ||
	sscanf (token, "%d,%d", &x1, &y1) != 2) {
	printf ("Usage: order X,Y X,Y\n");
	return;
    }

    /* look for destination */
    if (! (token = strtok (NULL, " ")) ||
	sscanf (token, "%d,%d", &x2, &y2) != 2) {
	printf ("Usage: order X,Y X,Y\n");
	return;
    }

    /* check that coordinates are on the map */
    map = battle->map;
    if (x2 < 0 || x2 >= map->width ||
	y2 < 0 || y2 >= map->height) {
	printf ("Error: coordinates must be between 0,0 and %d,%d\n",
		map->width - 1, map->height - 1);
	return;
    }

    /* check for unit */
    if ((unitid = map->units[x1 + map->width * y1]) == -1) {
	printf ("Error: no unit at %d,%d.\n", x1, y1);
	return;
    }
    unit = battle->units[unitid];

    /* check for target and attack if allowed */
    if ((targetid = map->units[x2 + map->width * y2]) != -1 &&
	(target = battle->units[targetid]) &&
	battle->attack (battle, unit, target, NULL)) {
	if (target->hits == 0) {
	    printf ("Unit %d,%d destroys unit %d,%d.\n",
		    x1, y1, x2, y2);
	    showmap ();
	} else {
	    printf ("Unit %d,%d attacks unit %d,%d.\n", x1, y1, x2, y2);
	    if (unit->hits == 0) {
		printf ("Unit %d,%d is destroyed.\n", x1, y1);
		unit->destroy (unit);
		battle->units[targetid] = NULL;
	    }
	}
	return;
    }

    /* no unit to attack */
    printf ("No unit to attack at %d,%d.\n", x2, y2);
}

/**
 * Give a unit a create order.
 */
static void createunit (char *command)
{
    char *token; /* for tokenising command */
    int x1, /* unit x coordinate */
	y1, /* unit y coordinate */
	x2, /* target x coordinate */
	y2, /* target y coordinate */
	ut, /* unit type counter */
	u, /* unit counter */
	srcid, /* id of unit to order */
	tgt, /* id of unit created */
	tgttype; /* target unit type id */
    Unit *src; /* unit to order */
    Map *map; /* convenience pointer to battle map */

    /* check initial token */
    if (! (token = strtok (command, " "))) {
	printf ("Error tokenising %s.\n", command);
	return;
    }

    /* look for unit to order */
    if (! (token = strtok (NULL, " ")) ||
	sscanf (token, "%d,%d", &x1, &y1) != 2) {
	printf ("Usage: restore X,Y X,Y\n");
	return;
    }

    /* look for destination */
    if (! (token = strtok (NULL, " ")) ||
	sscanf (token, "%d,%d", &x2, &y2) != 2) {
	printf ("Usage: restore X,Y X,Y\n");
	return;
    }

    /* look for unit type */
    if (! (token = strtok (NULL, " "))) {
	printf ("Usage: restore X,Y X,Y\n");
	return;
    } else {
	tgttype = -1;
	for (ut = 0; ut < 4 && ! tgttype; ++ut) {
	    if (! strcmp (battle->utypes[ut]->name, token))
		tgttype = ut;
	}
	if (! tgttype) {
	    printf ("Error: no such unit type as %s.\n", token);
	    return;
	}
    }

    /* check for unit */
    map = battle->map;
    if ((srcid = map->units[x1 + map->width * y1]) != -1 &&
	(src = battle->units[srcid])) {
	printf ("Error: no unit at %d,%d.\n", x1, y1);
	return;
    }

    /* check that target square is empty */
    if (map->units[x2 + map->width * y2] != CWG_NO_UNIT) {
	printf ("Cannot create %s at %d,%d.\n", token, x2, y2);
	return;
    }

    /* check there is space in the unit array */
    for (u = 0; u < 32; ++u)
	if (! battle->units[u])
	    break;
    if (u == 32) {
	printf ("No room for more units.\n");
	return;
    }

    /* create unit */
    if ((tgt = battle->create (battle, src, tgttype, x2, y2, NULL))
	== -1) {
	printf ("Cannot create %s at %d,%d.\n", token, x2, y2);
	return;
    }

    /* successful - replace old unit with new and name it */
    strcpy (battle->units[tgt]->name, token);
    showmap ();
}

/**
 * Give a unit a restore order.
 */
static void restoreunit (char *command)
{
    /* local variables */
    char *token; /* for tokenising command */
    int x1, /* unit x coordinate */
	y1, /* unit y coordinate */
	x2, /* target x coordinate */
	y2, /* target y coordinate */
	unitid, /* id of unit to order */
	targetid; /* id of the target to restore */
    Unit *unit, /* unit to order */
	*target; /* target to restore */
    Map *map; /* convenience pointer to the battle map */

    /* check initial token */
    if (! (token = strtok (command, " "))) {
	printf ("Error tokenising %s.\n", command);
	return;
    }

    /* look for unit to order */
    if (! (token = strtok (NULL, " ")) ||
	sscanf (token, "%d,%d", &x1, &y1) != 2) {
	printf ("Usage: restore X,Y X,Y\n");
	return;
    }

    /* look for destination */
    if (! (token = strtok (NULL, " ")) ||
	sscanf (token, "%d,%d", &x2, &y2) != 2) {
	printf ("Usage: restore X,Y X,Y\n");
	return;
    }

    /* check for unit */
    map = battle->map;
    if ((unitid = map->units[x1 + map->width * y1]) == -1 ||
	! (unit = battle->units[unitid])) {
	printf ("Error: no unit at %d,%d.\n", x1, y1);
	return;
    }

    /* check for target and restore if allowed */
    if ((targetid = map->units[x2 + map->width * y2]) &&
	(target = battle->units[targetid]) &&
	battle->restore (battle, unit, target, NULL))
	printf ("Unit %d,%d restores unit %d,%d.\n",
		x1, y1, x2, y2);
    else
	printf ("Cannot restore unit %d,%d.\n", x2, y2);
}

/**
 * End a turn.
 */
static void endturn (void)
{
    battle->turn (battle);
}

/**
 * Display battle information.
 */
static void battleinfo (void)
{
    /* local variables */
    int ut; /* Unit type */

    /* print all the things */
    printf ("s:%d r:%d/%d", battle->side,
	    battle->resources[0],
	    battle->resources[1]);
    for (ut = 0; ut < 4; ++ut)
	if (battle->builds[ut])
	    printf (" b:%s", battle->utypes[ut]->name);
    printf ("\n");
}

/*----------------------------------------------------------------------
 * Level 1 Functions.
 */

/**
 * Initialise the program.
 * @return 1 if successful, 0 if not.
 */
static int initialise (void)
{
    srand (time (NULL));
    cwg = get_Cwg ();
    if (loadgame ())
	return 1;
    if (creategame ())
	return 1;
    return 0;
}

/**
 * Play a turn of the game.
 * @return 1 if game is ongoing, 0 if it is over.
 */
static int playturn (void)
{
    /* local variables */
    char *command, /* command entered */
	*cmdcopy, /* a copy of the command */
	*token; /* the first token */

    /* get the command and look at the first token */
    command = getcommand ();
    cmdcopy = malloc (strlen (command) + 1);
    strcpy (cmdcopy, command);

    /* interpret the first token */
    if (! (token = strtok (command, " ")))
	printf ("Please enter a command.\n");
    else if (! strcmp (command, "quit")) {
	free (command);
	free (cmdcopy);
	return 0;
    } else if (! strcmp (command, "map"))
	showmap ();
    else if (! strcmp (command, "desc"))
	describe (cmdcopy);
    else if (! strcmp (command, "move"))
	moveunit (cmdcopy);
    else if (! strcmp (command, "attack"))
	attackunit (cmdcopy);
    else if (! strcmp (command, "create"))
	createunit (cmdcopy);
    else if (! strcmp (command, "restore"))
	restoreunit (cmdcopy);
    else if (! strcmp (command, "turn"))
	endturn ();
    else if (! strcmp (command, "info"))
	battleinfo ();
    else
	printf ("Unrecognised command %s.\n", command);

    /* free memory and return */
    free (command);
    free (cmdcopy);
    return 1;
}

/**
 * Clean up when the program is done.
 */
static void cleanup (void)
{
    int ut, /* unit type count */
	t; /* terrain count */
    for (ut = 0; ut < 4; ++ut)
	battle->utypes[ut]->destroy (battle->utypes[ut]);
    for (t = 0; t < 4; ++t)
	battle->terrain[t]->destroy (battle->terrain[t]);
    battle->map->destroy (battle->map);
    battle->destroy (battle);
    cwg->destroy ();
}

/*----------------------------------------------------------------------
 * Top Level Function.
 */

/**
 * Main Program.
 * @return 0 on a successful run.
 */
int main (void)
{
    if (initialise ()) {
	while (playturn ());
	cleanup ();
	return 0;
    }
    return 1;
}
