/*======================================================================
 * CWG: Cyningstan Wargame Engine.
 * An engine for simple turn-based wargames.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 19-Sep-2020.
 *
 * Battle Module.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* project headers */
#include "cwg.h"
#include "battle.h"
#include "utype.h"
#include "terrain.h"
#include "map.h"
#include "unit.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/** @var levels Difficulty levels for both sides. */
static int levels[2] = {0, 0};

/** @var movement Algorithm used for unit movement. */
static int movement = CWG_MOVE_SMART;

/*----------------------------------------------------------------------
 * Private Level 1 Functions.
 */

/**
 * Create a grid of travel costs from each map square to a destination.
 * @param battle is the battlefield.
 * @param unit is the unit concerned.
 * @param dx is the destination x coordinate.
 * @param dy is the destination y coordinate.
 * @return an array of costs for each square.
 */
static int *getpathcosts (Battle *battle, Unit *unit, int dx, int dy)
{
    /* local variables */
    int *costs, /* travel cost grid */
	width, /* width of the map */
	height, /* height of the map */
	sx, /* start x coordinate of sweep */
	sy, /* start y coordinate of sweep */
	ex, /* end x coordinate of sweep */
	ey, /* end y coordinate of sweep */
	x, /* current x coordinate of sweep */
	y, /* current y coordinate of sweep */
	dir, /* current direction of sweep */
	xo, /* x offset to examine */
	yo, /* y offset to examine */
	pos, /* 1-dimensional position with offset */
	moves, /* movement cost into a square */
	best, /* best cost met so far */
	modified; /* 1 if costs modified this sweep */
    Map *map; /* map of the battlefield */
    Terrain *terrain; /* terrain type of a map square */

    /* initialise */
    map = battle->map;
    width = map->width;
    height = map->height;
    if (! (costs = calloc (width * height, sizeof (int))))
	return NULL;

    /* shortcut for path to adjacent square */
    if (abs (unit->x - dx) <= 1 && abs (unit->y - dy) <= 1) {
	terrain = battle->terrain
	    [(int) map->terrain[dx + width * dy]];
	if (! (moves = terrain->moves[unit->utype]))
	    costs[dx + width * dy] = 0;
	else if (map->units[dx + width * dy] != CWG_NO_UNIT)
	    costs[dx + width * dy] = 0;
	else
	    costs[dx + width * dy] = moves;
	return costs;
    }

    /* outer loop - sweep map until all costs finalised */
    costs[dx + width * dy] = 1;
    sx = sy = 0;
    ex = width - 1;
    ey = height - 1;
    dir = 1;
    do {

	/* inner loop - perform a single sweep */
	modified = 0;
	for (x = sx; x != ex + dir; x += dir)
	    for (y = sy; y != ey + dir; y += dir) {
 
		/* don't check the destination or blocked squares */
		if (x == dx && y == dy)
		    continue;
		terrain = battle->terrain
		    [(int) map->terrain[x + width * y]];
		if (! (moves = terrain->moves[unit->utype]))
		    continue;

		/* inner inner loop - look around */
		best = 0;
		for (xo = -1; xo <= 1; ++xo)
		    for (yo = -1; yo <= 1; ++yo) {
			pos = (x + xo) + width * (y + yo);
			if (xo == 0 && yo == 0)
			    continue;
			if (x + xo < 0 || x + xo >= width ||
			    y + yo < 0 || y + yo >= height)
			    continue;
			if (map->units[x + width * y] != CWG_NO_UNIT)
			    continue;
			if ((costs[pos] > 0 ||
			     (x + xo == dx && y + yo == dy)) &&
			    (costs[pos] < best ||
			     best == 0))
			    best = costs[pos];
		    }

		/* modify square if better cost found */
		if (costs[x + width * y] != best + moves && best != 0) {
		    costs[x + width * y] = best + moves;
		    modified = 1;
		}
	    }

	/* reverse direction for next sweep */
	sx ^= ex;
	ex ^= sx;
	sx ^= ex;
	sy ^= ey;
	ey ^= sy;
	sy ^= ey;
	dir = -dir;
    } while (modified);

    /* all done */
    return costs;
}

/**
 * Create a dummy path costs array for quick movement.
 * @param  battle The battlefield.
 * @param  unit   The unit concerned.
 * @param  dx     The destination x coordinate.
 * @param  dy     The destination y coordinate.
 * @return        A dummy array that can be freed safely.
 */
static int *ignorepathcosts (Battle *battle, Unit *unit, int dx, int dy)
{
    return malloc (1);
}

/**
 * Move unit to the next square along its path.
 * @param battle is the battlefield.
 * @param unit is the unit to move.
 * @param dx is the destination x coordinate.
 * @param dy is the destination y coordinate.
 * @param costs is the grid of movement costs.
 * @return 1 on success, 0 on failure.
 */
static int movenextsquare (Battle *battle, Unit *unit, int dx, int dy,
			   int *costs)
{
    /* local variables */
    int xo, /* x offset to check */
	yo, /* y offset to check */
	xdir, /* x direction towards map centre */
	ydir, /* y direction towards map centre */
	best, /* best cost of potential next square */
	cost, /* cost of potential next square */
	x, /* current x coordinate of unit */
	y, /* current y coordinate of unit */
	width, /* width of the map */
	height, /* height of the map */
	nx, /* next x coordinate */
	ny; /* next y coordinate */
    Map *map; /* map of the battlefield */
    Terrain *terrain; /* terrain to move into */

    /* initialise */
    map = battle->map;
    width = map->width;
    height = map->height;
    x = unit->x;
    y = unit->y;
    nx = ny = -1;
    best = 0;
    xdir = (x < width / 2) - (x >= width / 2);
    ydir = (y < height / 2) - (y >= height / 2);

    /* validate */
    if (x == dx && y == dy)
	return 0;

    /* find the best adjacent square */
    for (xo = -xdir; xo != 2 * xdir; xo += xdir)
	for (yo = -ydir; yo != 2 * ydir; yo += ydir) {
	    if (x + xo < 0 || x + xo >= width ||
		y + yo < 0 || y + yo >= height ||
		(xo == 0 && yo == 0))
		continue;
	    if (! (cost = costs[(x + xo) + width * (y + yo)]));
	    else if (cost < best || best == 0) {
		best = cost;
		nx = x + xo;
		ny = y + yo;
	    } else if (cost <= best && (xo == 0 || yo == 0)) {
		best = cost;
		nx = x + xo;
		ny = y + yo;
	    }
	}

    /* return now if we couldn't find a square */
    if (nx == -1 || ny == -1)
	return 0;

    /* return now if the next square is occupied */
    else if (map->units[nx + width * ny] != CWG_NO_UNIT)
	return 0;

    /* return if we lack enough movement points */
    terrain = battle->terrain[(int) map->terrain[nx + width * ny]];
    cost = terrain->moves[unit->utype];
    if (cost == 0 || cost > unit->moves)
	return 0;

    /* otherwise move the unit */
    map->units[nx + width * ny] = map->units[x + width * y];
    map->units[x + width * y] = CWG_NO_UNIT;
    unit->x = nx;
    unit->y = ny;
    unit->moves -= cost;
    return 1;
}

/**
 * Move unit to the next square along its path (quick method).
 * @param  battle The battlefield.
 * @param  unit   The unit to move.
 * @param  dx     The destination x coordinate.
 * @param  dy     The destination y coordinate.
 * @param  costs  The dummy grid of movement costs.
 * @return        1 on success, 0 on failure.
 */
static int quicknextsquare (Battle *battle, Unit *unit, int dx, int dy,
			   int *costs)
{
    int xdir, /* x direction of travel */
	ydir, /* y direction of travel */
	hand, /* left-hand or right-hand search order */
	step, /* step of terrain check, -1/1 */
	stepc, /* step cost */
	stepx, /* potential next step X coordinate */
	stepy, /* potential next step Y coordinate */
	stepl, /* potential next step as a single value */
	best, /* best movement cost found so far */
	bestx, /* best next step X coordinate */
	besty, /* best next step Y coordinate */
	bestl; /* best location as a single value */
    Terrain *terrain; /* terrain to check */

    /* get general direction of travel */
    xdir = (dx > unit->x) - (dx < unit->x);
    ydir = (dy > unit->y) - (dy < unit->y);
    bestx = unit->x + xdir;
    besty = unit->y + ydir;
    bestl = bestx + battle->map->width * besty;
    if (battle->map->units[bestl] == CWG_NO_UNIT) {
	terrain = battle->terrain[battle->map->terrain[bestl]];
	best = terrain->moves[unit->utype];
    } else
	best = 0;

    /* if more than one square away, work out quickest direction */
    if (battle->map->distance (unit->x, unit->y, dx, dy, 0) > 1) {

	/* check oblique routes */
	hand = (2 * rand () % 2) - 1;
	for (step = -1; step <= 1; step += 2) {

	    /* where is the next step? */
	    if (xdir && ydir) {
		stepx = unit->x + xdir * (step == hand);
		stepy = unit->y + ydir * (step != hand);
	    } else if (xdir) {
		stepx = unit->x + xdir;
		stepy = unit->y + hand * step;
	    } else if (ydir) {
		stepy = unit->y + ydir;
		stepx = unit->x + hand * step;
	    }
	    if (stepx < 0 || stepx >= battle->map->width
		|| stepy < 0 || stepy >= battle->map->height)
		continue;
	    stepl = stepx + battle->map->width * stepy;

	    /* check the next step */
	    if (battle->map->units[stepl] == CWG_NO_UNIT) {
		terrain = battle->terrain[battle->map->terrain[stepl]];
		stepc = terrain->moves[unit->utype];
	    } else
		stepc = 0;
	    if ((stepc < best || best == 0) && stepc != 0) {
		best = stepc;
		bestx = stepx;
		besty = stepy;
	    }
	}
    }
    bestl = bestx + battle->map->width * besty;
    
    /* return now if we couldn't find a square */
    if (best == 0)
	return 0;

    /* return now if the next square is occupied */
    else if (battle->map->units[bestl] != CWG_NO_UNIT)
	return 0;

    /* return if we lack enough movement points */
    terrain = battle->terrain[bestl];
    if (best == 0 || best > unit->moves)
	return 0;

    /* otherwise move the unit */
    battle->map->units[bestl]
	= battle->map->units[unit->x + battle->map->width * unit->y];
    battle->map->units[unit->x + battle->map->width * unit->y]
	= CWG_NO_UNIT;
    unit->x = bestx;
    unit->y = besty;
    unit->moves -= best;
    return 1;
}

/**
 * Have one unit fire upon another (no return fire).
 * @param battle is the battlefield map.
 * @param unit is the unit attacking.
 * @param target is its target.
 * @return 1 if firing was possible, 0 if not.
 */
static int fireupon (Battle *battle, Unit *unit, Unit *target)
{
    /* local variables */
    Terrain *terrain; /* target terrain */
    Map *map; /* the battlefield map */
    int ux, /* unit x coordinate */
	uy, /* unit y coordinate */
	tx, /* x coordinate of target */
	ty, /* y coordinate of target */
	distance, /* distance from unit to target */
	attack, /* attack value of target */
	defence, /* defence value of target */
	damage, /* damage caused to target */
	roll, /* damage roll */
	r; /* roll counter for higher difficulty */

    /* initialise unit and terrain information */
    map = battle->map;
    ux = unit->x;
    uy = unit->y;
    tx = target->x;
    ty = target->y;
    terrain = battle->terrain[(int) map->terrain[tx + map->width * ty]];

    /* validation */
    distance = abs (ux - tx) > abs (uy - ty) ?
	abs (ux - tx) : abs (uy - ty);
    if (distance > battle->utypes[unit->utype]->range)
	return 0;
    if (! unit->moves)
	return 0;

    /* calculate attack and defence values */
    attack = battle->utypes[unit->utype]->power;
    defence = battle->utypes[target->utype]->armour +
	terrain->defence[target->utype];

    /* calculate damage */
    damage = 0;
    for (r = 0; r <= levels[unit->side]; ++r) {
	roll = rand () % (1 + attack) - rand () % (1 + defence);
	if (roll > damage)
	    damage = roll;
    }
    damage = (damage < 0) ? 0 : damage;
    damage = damage > target->hits ? target->hits : damage;

    /* reduce target hits and remove from map if killed */
    target->hits -= damage;
    if (target->hits == 0)
	map->units[tx + map->width * ty] = CWG_NO_UNIT;

    /* firing successful */
    unit->moves = 0;
    return 1;
}

/*----------------------------------------------------------------------
 * Public Level Functions.
 */

/**
 * Destroy a battle when the game is over.
 * @param battle is the battle to destroy.
 */
static void destroy (Battle *battle)
{
    int c; /* unit counter */
    if (battle) {
	for (c = 0; c < CWG_UNITS; ++c)
	    if (battle->units[c])
		battle->units[c]->destroy (battle->units[c]);
	if (battle->map)
	    battle->map->destroy (battle->map);
	free (battle);
    }
}

/**
 * Clone a battle state.
 * @param battle is the battle to clone.
 * @return a copy of the battle.
 */
static Battle *clone (Battle *battle)
{
    /* local variables */
    Battle *newbattle; /* the copy of the battle */
    Unit *unit; /* pointer to a unit to be cloned */
    int c; /* array counter */

    /* reserve memory for the new battle */
    if (! (newbattle = new_Battle (battle->utypes, battle->terrain)))
	return NULL;

    /* clone the simple attributes */
    newbattle->start = battle->start;
    newbattle->side = battle->side;
    newbattle->resources[0] = battle->resources[0];
    newbattle->resources[1] = battle->resources[1];

    /* clone the arrays */
    for (c = 0; c < CWG_UTYPES; ++c)
	newbattle->builds[c] = battle->builds[c];
    for (c = 0; c < CWG_UNITS; ++c)
	if ((unit = battle->units[c]))
	    newbattle->units[c] = unit->clone (unit);
	else
	    newbattle->units[c] = NULL;

    /* clone the map */
    newbattle->map = battle->map->clone (battle->map);

    /* return the new battle */
    return newbattle;
}

/**
 * Write the battle to an already open file.
 * @param battle is the battle to write.
 * @param output is the output file.
 * @return 1 on success, 0 on failure.
 */
static int write (Battle *battle, FILE *output)
{
    /* local variables */
    int c, /* array counter */
	s = 1, /* success flag */
	unitid; /* unit ID */
    Cwg *cwg; /* pointer to cwg object */

    /* get the Cwg object to use its utilities */
    cwg = get_Cwg ();

    /* write who started and who is playing now */
    s &= cwg->writeint (&battle->start, output);
    s &= cwg->writeint (&battle->side, output);

    /* write the economic data */
    s &= cwg->writeint (&battle->resources[0], output);
    s &= cwg->writeint (&battle->resources[1], output);

    /* write out the arrays */
    for (c = 0; c < CWG_UTYPES; ++c)
	s &= cwg->writeint (&battle->builds[c], output);

    /* write the map */
    s &= battle->map->write (battle->map, output);

    /* write the units on the map */
    for (c = 0; c < CWG_UNITS; ++c) {
	if (battle->units[c]) {
	    unitid = c;
	    s &= cwg->writeint (&unitid, output);
	    s &= battle->units[c]->write (battle->units[c], output);
	}
    }
    unitid = CWG_NO_UNIT;
    s &= cwg->writeint (&unitid, output);

    /* return */
    return s;
}

/**
 * Read the battle from an already open file.
 * @param battle is the battle to read.
 * @param input is the input file.
 * @return 1 on success, 0 on failure.
 */
static int read (Battle *battle, FILE *input)
{
    /* local variables */
    int c, /* array counter */
	s = 1, /* success flag */
	unitid; /* unit ID */
    Cwg *cwg; /* pointer to cwg object */

    /* get the Cwg object to use its utilities */
    cwg = get_Cwg ();

    /* read who started and who is playing now */
    s &= cwg->readint (&battle->start, input);
    s &= cwg->readint (&battle->side, input);

    /* read the economic data */
    s &= cwg->readint (&battle->resources[0], input);
    s &= cwg->readint (&battle->resources[1], input);

    /* read out the arrays */
    for (c = 0; c < CWG_UTYPES; ++c)
	s &= cwg->readint (&battle->builds[c], input);

    /* read the map */
    if (battle->map)
	battle->map->destroy (battle->map);
    battle->map = new_Map ();
    s &= battle->map->read (battle->map, input);

    /* read the units on the map */
    for (c = 0; c < CWG_UNITS; ++c)
	if (battle->units[c]) {
	    free (battle->units[c]);
	    battle->units[c] = NULL;
	}
    for (cwg->readint (&unitid, input);
	 unitid != CWG_NO_UNIT && s;
	 cwg->readint (&unitid, input)) {
	battle->units[unitid] = new_Unit ();
	s &= battle->units[unitid]->read (battle->units[unitid], input);
    }

    /* return */
    return s;
}

/**
 * Move a unit towards a destination.
 * @param battle is the battle to affect.
 * @param unit is the unit.
 * @param x is the x coordinate to move to.
 * @param y is the y coordinate to move to.
 * @return 1 if successful, 0 if not.
 */
static int move (Battle *battle, Unit *unit, int x, int y,
	     MoveHook hook)
{
    /* local variables */
    int *costs = NULL, /* a grid of total costs to destination */
	oldx, /* previous x coordinate for display hook */
	oldy; /* previous y coordinate for display hook */
    int *(*prepare) (Battle *, Unit *, int, int);
    int (*next) (Battle *, Unit *, int, int, int *);

    /* validation */
    if (! battle->utypes)
	return 0;
    if (! battle->terrain)
	return 0;
    if (unit->side != battle->side)
	return 0;
    if (! unit->moves)
	return 0;

    /* select the movement algorithm */
    if (movement == CWG_MOVE_QUICK) {
	prepare = ignorepathcosts;
	next = quicknextsquare;
    } else {
	prepare = getpathcosts;
	next = movenextsquare;
    }

    /* work out the costs grid */
    if (! (costs = prepare (battle, unit, x, y)))
	return 0;

    /* traverse the costs grid */
    oldx = unit->x;
    oldy = unit->y;
    while (next (battle, unit, x, y, costs)) {
	if (hook)
	    hook (unit, oldx, oldy);
	oldx = unit->x;
	oldy = unit->y;
    }

    /* return success */
    if (costs) free (costs);
    return 1;
}

/**
 * Attack one unit with another.
 * @param battle is the battle to affect.
 * @param unit is the unit to move.
 * @param target is the target to attack.
 * @return 1 if successful, 0 if not.
 */
static int attack (Battle *battle, Unit *unit, Unit *target,
	       AttackHook hook)
{
    CwgAttackResult result; /* result of attack */

    /* validation */
    if (! battle->utypes)
	return 0;
    if (! battle->terrain)
	return 0;
    if (unit->side != battle->side)
	return 0;
    if (target->side == battle->side)
	return 0;
    if (! unit->moves)
	return 0;

    /* unit fires on target */
    result = CWG_ATT_NO_RETURN;
    if (! fireupon (battle, unit, target))
	return 0;
    if (target->hits) {
	result = fireupon (battle, target, unit);
	if (! unit->hits)
	    result = CWG_ATT_ATTR_KILLED;
    } else
	result = CWG_ATT_DEFR_KILLED;
    
    /* call hook and return */
    if (hook)
	hook (unit, target, result);
    return 1;
}

/**
 * Create a unit.
 * @param battle is the battle to affect.
 * @param src is the unit doing the building.
 * @param utype is the unit type to build.
 * @param x is the x coordinate to place the new unit.
 * @param y is the y coordinate to place the new unit.
 * @return -1 on failure or the unit id on success.
 */
static int create (Battle *battle, Unit *src, int utype,
		   int x, int y, CreateHook hook)
{
    /* local variables */
    UnitType *srctype; /* builder unit type */
    Map *map; /* the battlefield map */
    Unit *tgt; /* the newly built unit */
    int c; /* array counter */

    /* validation */
    if (! battle->utypes)
	return CWG_NO_UNIT;
    if (! battle->terrain)
	return CWG_NO_UNIT;

    /* initialisation */
    srctype = battle->utypes[src->utype];
    map = battle->map;

    /* more validation */
    if (battle->side != src->side)
	return CWG_NO_UNIT;
    if (map->units[x + map->width * y] != CWG_NO_UNIT)
	return CWG_NO_UNIT;
    if (! srctype->builds[utype])
	return CWG_NO_UNIT;
    if (src->moves < srctype->moves)
	return CWG_NO_UNIT;
    if (battle->resources[battle->side] < battle->utypes[utype]->cost)
	return CWG_NO_UNIT;
    if (map->distance (x, y, src->x, src->y, 0) > 1)
	return CWG_NO_UNIT;
    if (battle->terrain[map->terrain[x + map->width * y]]->moves[utype]
	== 0)
	return CWG_NO_UNIT;

    /* ensure that there is room for one more unit */
    for (c = 0; c < CWG_UNITS; ++c)
	if (! battle->units[c])
	    break;
    if (c == CWG_UNITS)
	return CWG_NO_UNIT;

    /* place the new unit - leave moves at 0 */
    if (! (tgt = battle->units[c] = new_Unit ()))
	return CWG_NO_UNIT;
    tgt->side = battle->side;
    tgt->utype = utype;
    tgt->x = x;
    tgt->y = y;
    battle->map->units[x + map->width * y] = c;
    tgt->hits = battle->utypes[utype]->hits;

    /* update the src unit and the resources */
    src->moves = 0;
    battle->resources[battle->side] -= battle->utypes[utype]->cost;

    /* run hook at return */
    if (hook)
	hook (src, tgt);
    return c;
}

/**
 * Restore a unit.
 * @param battle is the battle to affect.
 * @param builder is the unit doing the repairing.
 * @param built is the unit to repair.
 * @return 1 if successful, 0 if not.
 */
static int restore (Battle *battle, Unit *builder, Unit *built,
		RestoreHook hook)
{
    /* local variables */
    UnitType *srctype, /* unit type of the builder */
	*tgttype; /* unit type of the unit to repair */
    int sx, /* x coordinate of the builder */
	sy, /* y coordinate of the builder */
	tx, /* x coordinate of the target */
	ty, /* y coordinate of the target */
	distance; /* distance from source to target */

    /* validation */
    if (! battle->utypes)
	return 0;
    if (! battle->terrain)
	return 0;

    /* initialisation */
    srctype = battle->utypes[builder->utype];
    sx = builder->x;
    sy = builder->y;
    tgttype = battle->utypes[built->utype];
    tx = built->x;
    ty = built->y;
    distance = abs (sx - tx) > abs (sy - ty) ?
	abs (sx - tx) : abs (sy - ty);

    /* more validation */
    if (distance != 1)
	return 0;
    if (! srctype->builds[built->utype])
	return 0;
    if (builder->moves < srctype->moves / 2)
	return 0;
    if (builder->side != battle->side)
	return 0;
    if (built->side != battle->side)
	return 0;
    if (battle->resources[battle->side] < tgttype->cost / 2)
	return 0;
    if (built->hits >= tgttype->hits)
	return 0;

    /* do the repair */
    built->hits = tgttype->hits;
    builder->moves -= srctype->moves / 2;
    battle->resources[battle->side] -= tgttype->cost / 2;

    /* run the hook and return */
    if (hook)
	hook (builder, built);
    return 1;
}

/**
 * Check for victory on the battlefield.
 * @param battle is the battle to check.
 * @return -1 for ongoing battle, or victorious side 0 or 1.
 */
static int victory (Battle *battle)
{
    int l, /* location counter */
	pointcount[3], /* count of victory points per side & total */
	unitcount[2], /* count of units per side */
	vp, /* flag for current location being victory point */
	s; /* side counter */
    Unit *unit; /* convenience pointer to current unit */

    /* count units per side and victory points occupied */
    pointcount[0] = pointcount[1] = pointcount[2] = 0;
    unitcount[0] = unitcount[1] = 0;
    for (l = 0; l < battle->map->width * battle->map->height; ++l) {
	if ((vp = battle->map->points[l]))
	    ++pointcount[2];
	if (battle->map->units[l] != CWG_NO_UNIT) {
	    unit = battle->units[battle->map->units[l]];
	    if (unit->hits)
		++unitcount[unit->side];
	    if (vp)
		++pointcount[unit->side];
	}
    }

    /* does one side have all the victory points? */
    for (s = 0; s < 2; ++s)
	if (pointcount[s] == pointcount[2] && pointcount[2])
	    return s;
	else if (unitcount[1 - s] == 0)
	    return s;

    /* if we reach this point, neither side has won yet */
    return -1;
}

/**
 * End a player's turn.
 * @param battle is the battle to affect.
 * @return 1 if successful, 0 if not.
 */
static int turn (Battle *battle)
{
    /* local variables */
    Unit *unit; /* unit to be refreshed */
    int c; /* unit counter */

    /* validation */
    if (! battle->utypes)
	return 0;
    if (! battle->terrain)
	return 0;

    /* switch side */
    battle->side = ! battle->side;

    /* remove dead units from the battle */
    for (c = 0; c < CWG_UNITS; ++c)
	if ((unit = battle->units[c]) &&
	    unit->hits == 0) {
	    unit->destroy (unit);
	    battle->units[c] = NULL;
	}

    /* restore side's units' movement points */
    for (c = 0; c < CWG_UNITS; ++c)
	if ((unit = battle->units[c]) &&
	    unit->hits &&
	    unit->side == battle->side)
	    unit->moves = battle->utypes[unit->utype]->moves;

    /* return with success */
    return 1;
}

/**
 * Set the difficulty level for one side.
 * @param side The side for which to set the difficulty.
 * @param level The difficulty level.
 */
static void setlevel (int side, int level)
{
    levels[side] = level;
}

/**
 * Set the movement algorithm.
 * @param algorithm The algorithm to use for movement:
 */
static void setmovement (int algorithm)
{
    movement = algorithm;
}

/*----------------------------------------------------------------------
 * Constructor and any other class methods.
 */

/**
 * Default constructor.
 * @returns a new Battle object.
 */
Battle *new_Battle (UnitType **utypes, Terrain **terrain)
{
    /* local variables */
    Battle *battle; /* the battle to create */
    int c; /* general purpose counter */

    /* reserve memory */
    if (! (battle = malloc (sizeof (Battle))))
	return NULL;

    /* initialise attributes and static variables */
    battle->start = 0;
    battle->side = 0;
    battle->resources[0] = 0;
    battle->resources[1] = 0;
    for (c = 0; c < CWG_UTYPES; ++c)
	battle->builds[c] = 0;
    for (c = 0; c < CWG_UNITS; ++c)
	battle->units[c] = NULL;
    battle->utypes = utypes;
    battle->terrain = terrain;
    battle->map = NULL;

    /* initialise methods */
    battle->destroy = destroy;
    battle->clone = clone;
    battle->write = write;
    battle->read = read;
    battle->move = move;
    battle->attack = attack;
    battle->create = create;
    battle->restore = restore;
    battle->victory = victory;
    battle->turn = turn;
    battle->setlevel = setlevel;
    battle->setmovement = setmovement;

    /* return the new battle */
    return battle;
}
