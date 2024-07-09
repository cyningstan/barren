/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * AI Module.
 */

/*----------------------------------------------------------------------
 * Included Headers.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* project-specific headers */
#include "ai.h"
#include "barren.h"
#include "game.h"
#include "campaign.h"
#include "report.h"
#include "fatal.h"
#include "cwg.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/**
 * @struct unitcategories
 * Categorisation for unit types.
 */
typedef struct unitcategory {

    /** @var combatant 1 if the unit is capable of attacking. */
    int combatant;

    /** @var builder 1 if the unit is capable of building. */
    int builder;

    /** @var gatherer 1 if the unit is capable of mining. */
    int gatherer;

    /** @var mobile 1 if the unit is capable of moving. */
    int mobile;

} UnitCategory;

/**
 * @enum TargetType
 * Enumeration of target types.
 */
typedef enum {
    TARGET_ATTACK, /* an enemy unit to attack */
    TARGET_GATHERING, /* a resource to be gathered */
    TARGET_OCCUPATION, /* a victory point to occupy */
    TARGET_REPAIR, /* a damaged friendly unit to repair */
    TARGET_RALLY, /* a friendly build for damaged units to rally to */
    TARGET_DEFENCE /* a friendly static unit to defend */
} TargetType;

/**
 * @struct target
 * The data held for a potential target.
 */
typedef struct target Target;
struct target {

    /** @var type The type of the target. */
    int type;

    /** @var location The location of the target. */
    int location;

    /** @var importance The importance of the unit. */
    int importance;

    /** @var utype Preferred unit type. */
    int utype;

    /** 
     * @var requirements
     * Firepower or other quantity required for this target.
     */
    int requirements;

    /**
     * @var allocation
     * Firepower or other quantity allocated to this target.
     */
    int allocation;

    /**
     * @var next
     * A pointer to the next target in the list.
     */
    Target *next;

};

/**
 * @struct agent
 * Data held for an agent.
 */
typedef struct agent Agent;
struct agent {

    /**
     * @var count
     * A counter of the number of agents in the list.
     * Only the counter of the first node is valid.
     */
    int count;

    /**
     * @var unit
     * The unit number of the agent.
     */
    int unit;

    /**
     * @var location
     * Location of the agent as a single integer.
     */
    int location;

    /**
     * @var suitability
     * Suitability of this agent for the given target.
     */
    int suitability;

    /**
     * @var building
     * 1 if the unit should build for the target.
     */
    int building;

    /**
     * @var target
     * Target to which this unit is allocated.
     */
    Target *target;

    /**
     * @var next
     * A pointer to the next agent in the list.
     */
    Agent *next;

};

/**
 * @struct aidata
 * Private data for the AI.
 */
struct aidata {

    /** @var game The game to play. */
    Game *game;

    /** @var unitcats The unit categorisations. */
    UnitCategory unitcats[CWG_UTYPES];

    /** @var ourcentre The centre of friendly units. */
    int ourcentre;

    /** @var theircentre The centre of enemy units. */
    int theircentre;

    /**
     * @var maxpower
     * The maximum attack power of any unit on the battlefield.
     */
    int maxpower;

    /**
     * @var maxdist
     * The taxicab distance between opposite corners of the map.
     * Used as a scale for importance factors.
     */
    int maxdist;

    /** @var gatherers Number of friendly gatherers on the map. */
    int gatherers;

    /**
     * @var points
     * 0 if neither side has enough units to occupy victory points.
     * 1 if we are attempting to occupy/defend victory points.
     */
    int points;

    /** @var prompthook Pointer to an external prompt function. */
    PromptHook prompthook;

    /** @var loghook; Pointer to an external log function. */
    LogHook loghook;
};

/**
 * @var ai
 * The single AI instance.
 */
static AI *ai = NULL;

/*----------------------------------------------------------------------
 * CWG Display Hooks.
 */

/**
 * Add unit movements to the battle report.
 * @param unit The unit that has moved.
 * @param x    The x coordinate the unit moved from.
 * @param y    The y coordinate the unit moved from.
 */
static void movehook (Unit *unit, int x, int y)
{
    Game *game; /* convenience pointer to the game */
    int width; /* width of battle map */
    game = ai->data->game;
    width = game->battle->map->width;
    game->report->add (game->report, ACTION_MOVE, unit->utype, 0,
		       x + width * y, unit->x + width * unit->y);
}

/**
 * Add an attack to the battle report.
 * @param attacker The unit that attacked.
 * @param defender The unit that was attacked.
 */
static void attackhook (Unit *attacker, Unit *defender,
			CwgAttackResult result)
{
    Game *game; /* convenience pointer to game */
    int width; /* width of the battle map */
    game = ai->data->game;
    width = game->battle->map->width;
    game->report->add (game->report, ACTION_ATTACK + result,
		       attacker->utype, defender->utype,
		       attacker->x + width * attacker->y,
		       defender->x + width * defender->y); 
}

/**
 * Add unit construction to the game report.
 * @param builder The unit that is building.
 * @param built   The unit that was built.
 */
static void buildhook (Unit *builder, Unit *built)
{
    Game *game; /* convenience pointer to game */
    int width; /* width of the battle map */
    game = ai->data->game;
    width = game->battle->map->width;
    game->report->add (game->report, ACTION_BUILD,
		       builder->utype, built->utype,
		       builder->x + width * builder->y,
		       built->x + width * built->y);
    strcpy (built->name, game->campaign->unittypes[built->utype]->name);
}

/**
 * Log a repair in the battle report.
 * @param builder The unit that can build/repair.
 * @param built   The unit that was repaired.
 */
static void repairhook (Unit *builder, Unit *built)
{
    Game *game; /* convenience pointer to game */
    int width; /* width of the battle map */
    game = ai->data->game;
    width = game->battle->map->width;
    game->report->add (game->report, ACTION_REPAIR,
		       builder->utype, built->utype,
		       builder->x + width * builder->y,
		       built->x + width * built->y);
}

/*----------------------------------------------------------------------
 * Level 5 Private Function Definitions.
 */

/**
 * Calculate proximity of one square to another as an importance factor.
 * @param  first  The first square.
 * @param  second The second square.
 * @return        The importance factor 0..maxdist.
 */
static int proximityfactor (int first, int second)
{
    Map *map; /* pointer to the battle map */
    int x, /* longitudinal distance */
	y; /* latitudinal distance */
    map = ai->data->game->battle->map;
    y = abs ((first / map->width) - (second / map->width));
    x = abs ((first % map->width) - (second % map->width));
    return ai->data->maxdist - (x + y);
}

/*----------------------------------------------------------------------
 * Level 4 Private Function Definitions.
 */

/**
 * Reduce an importance factor to a scale of 0..maxdist.
 * @param  factor The factor to scale.
 * @param  max    The maximum value for the factor.
 * @return        The scaled value.
 */
static int scale (int factor, int max)
{
    return
	(int)
	((long) factor * (long) ai->data->maxdist / (long) max);
}

/**
 * Calculate a position near a target. Used to calculate position to
 * defend from or to build.
 * @param uloc      The current location of the unit to move
 * @param target    The target location.
 * @param direction The direction from the target (as another location).
 * @param range     The maximum number of squares from the target.
 * @param utype     The unit type to occupy this square.
 * @return          The best build location.
 */
static int nearby (int uloc, int target, int direction, int range,
		   int utype)
{
    int width, /* width of the map */
	height, /* height of the map */
	bestscore, /* best location score so far */
	bestlocation, /* best location so far */
	x, /* x coordinate of location being considered */
	y, /* y coordinate of location being considered */
	location, /* location being considered */
	score; /* score for the current location */
    Terrain *terrain; /* terrain of location being considered */
    Campaign *campaign; /* pointer to the campaign */
    Map *map; /* pointer to the map */

    /* initialise some variables */
    campaign = ai->data->game->campaign;
    map = ai->data->game->battle->map;
    width = map->width;
    height = map->height;
    bestscore = 0;
    bestlocation = direction;

    /* look at each location in range of the target */
    for (x = (target % width) - range; 
	 x <= (target % width) + range;
	 ++x) {
	for (y = (target / width) - range;
	     y <= (target / width) + range;
	     ++y) {

	    /* reject invalid locations */
	    if (x < 0 || x >= width || y < 0 || y >= height)
		continue; /* off the map */
	    location = x + width * y;
	    if (location == target)
		continue; /* right on the target */
	    if (map->units[location] != CWG_NO_UNIT &&
		location != uloc)
		continue; /* location occupied */
	    terrain = campaign->terrain[map->terrain[location]];
	    if (ai->data->unitcats[utype].mobile &&
		! terrain->moves[utype])
		continue; /* inaccessible to the unit */

	    /* score the location */
	    score = terrain->defence[utype] +
		proximityfactor (location, direction);
	    if (score > bestscore) {
		bestscore = score;
		bestlocation = location;
	    } else if (score >= bestscore && location == uloc) {
		bestscore = score;
		bestlocation = location;
	    }
	}
    }

    /* return the best location */
    return bestlocation;
}

/**
 * Ensure we have a report to record the action.
 */
static Report *initreport (void)
{
    Game *game; /* pointer to the game */
    game = ai->data->game;
    if (! game->report) {
	game->report = new_Report ();
	game->report->battle = game->battle->clone (game->battle);
    } else if (! game->report->battle)
	game->report->battle = game->battle->clone (game->battle);
    return game->report;
}

/*----------------------------------------------------------------------
 * Level 3 Private Function Definitions.
 */

/**
 * Return build ability as an importance factor.
 * @param  utype The unit type.
 * @return       The importance factor 0..maxdist.
 */
static int builderfactor (int utype)
{
    return scale (ai->data->unitcats[utype].builder, 1);
}

/**
 * Return gathering ability as an importance factor.
 * @param  utype The unit type.
 * @return       The importance factor 0..maxdist.
 */
static int gathererfactor (int utype)
{
    return scale (ai->data->unitcats[utype].gatherer, 1);
}

/**
 * Return resource as an importance factor.
 * @param  square The map square in question.
 * @return        The importance factor 0..maxdist.
 */
static int resourcefactor (int square)
{
    /* in future we might refine this to reduce resources' importance
       when no gatherers are on the battlefield. */
    int terrain, /* terrain type at the map square */
	resource; /* 1 if this is a resource, 0 otherwise */
    terrain = ai->data->game->battle->map->terrain[square];
    resource = (terrain == ai->data->game->campaign->resource);
    return scale (resource, 1);
}

/**
 * Return victory point as an importance factor.
 * @param  square The map square in question.
 * @return        The importance factor 0..maxdist.
 */
static int pointfactor (int square)
{
    /* in future we might adjust the importance depending on how many
       victory points are already occupied by our/their forces, and
       reduce the importance if not enough units exist to occupy all
       the victory points. */
    int point; /* 1 if there's a victory point here, 0 otherwise */
    point = (!! ai->data->game->battle->map->points[square]);
    return scale (point, 1);
}

/**
 * Return a unit's damage as an importance factor.
 * @param  unit The damaged unit.
 * @return      The importance factor 0..maxdist.
 */
static int damagefactor (Unit *unit)
{
    UnitType *utype; /* pointer to unit's type */
    utype = ai->data->game->campaign->unittypes[unit->utype];
    return scale (utype->hits - unit->hits, utype->hits);
}

/**
 * Return unit's static (non-mobile) nature as an importance factor.
 * @param  utype The unit type.
 * @return       The importance factor 0..maxdist.
 */
static int staticfactor (int utype)
{
    return scale (! ai->data->unitcats[utype].mobile, 1);
}

/**
 * Create a sortable index of agents.
 * @param  agents A linked list of agents.
 * @return        An array of pointers to agents.
 */
static Agent **indexagents (Agent *agents)
{
    Agent *agent, /* pointer to current agent */
	**index; /* index of agents */
    if (! (index = malloc (agents->count * sizeof (Agent *))))
	fatalerror (FATAL_MEMORY);
    for (agent = agents; agent; agent = agent->next)
	index[agents->count - agent->count] = agent;
    return index;
}

/**
 * Sort the index of agents by suitability.
 * @param count The number of agents.
 * @param index The index of agents.
 */
static void sortagents (int count, Agent **index)
{
    Agent *agent; /* temporary swap value */
    int sorted, /* 1 if the index is sorted */
	i; /* index counter */
    do {
	sorted = 1;
	for (i = 0; i < count - 1; ++i)
	    if (index[i]->suitability < index[i + 1]->suitability) {
		agent = index[i];
		index[i] = index[i + 1];
		index[i + 1] = agent;
		sorted = 0;
	    }
    } while (! sorted);
}

/**
 * Return a unit's firepower as an importance factor.
 * @param  utypeid The unit type to evaluate.
 * @return         The importance factor 0..maxdist.
 */
static int powerfactor (int utypeid)
{
    UnitType *utype; /* pointer to unit's type */
    utype = ai->data->game->campaign->unittypes[utypeid];
    return scale (utype->power * utype->range, ai->data->maxpower);
}

/**
 * Return as a factor a flag indicating whether a unit is in range of
 * a target location.
 * @param  unit     The unit in question.
 * @param  location The target location.
 * @return          0 if out of range, or maxdist if in range.
 */
static int rangefactor (Unit *unit, int location)
{
    Map *map; /* pointer to the battle map */
    UnitType *utype; /* the unit's type */
    int x, /* longitudinal distance */
	y, /* latitudinal distance */
	dist, /* calculated distance */
	size; /* largest dimension of the map */
    map = ai->data->game->battle->map;
    utype = ai->data->game->campaign->unittypes[unit->utype];
    size = (map->width > map->height) ? map->width : map->height;
    y = abs (unit->y - (location / map->width));
    x = abs (unit->x - (location % map->width));
    dist = ((x > y) ? x : y);
    return ai->data->maxdist * (dist <= utype->range);
}

/**
 * Find the best unit type to build.
 * @param  agent The agent building.
 * @return       The best unit type to build, or -1 not to build.
 */
static int bestbuildunittype (Agent *agent)
{
    int u, /* unit type counter */
	besttype = -1, /* best unit type found so far */
	bestscore = 0, /* best score found so far */
	score, /* score of current unit type */
	terrainid, /* id of terrain of target location */
	tunitid, /* target unit ID */
	tutypeid; /* target unit type ID */
    Campaign *campaign; /* pointer to the campaign */
    Battle *battle; /* pointer to the battle */
    UnitType *utype, /* prospective unit type */
	*autype, /* builder unit type */
	*tutype; /* target unit type */
    Terrain *terrain; /* terrain of target location */

    /* initialise variables */
    campaign = ai->data->game->campaign;
    battle = ai->data->game->battle;
    autype = campaign->unittypes[battle->units[agent->unit]->utype];
    besttype = -1;

    /* loop through all the unit types looking for the best one */
    for (u = 0; u < CWG_UTYPES; ++u) {
	utype = campaign->unittypes[u];

	/* reject units we can't afford */
	if (battle->resources[battle->side] < utype->cost)
	    continue;

	/* reject units we can't build in this scenario */
	if (! battle->builds[u])
	    continue;

	/* reject units that the agent can't build */
	if (! autype->builds[u])
	    continue;

	/* automatically accept the preferred unit type */
	if (u == agent->target->utype) {
	    besttype = u;
	    break;
	}

	/* work out a score for the unit type */
	switch (agent->target->type) {
	case TARGET_ATTACK:
	    score = utype->power
		* utype->range
		* ai->data->unitcats[u].mobile;
	    break;
	case TARGET_GATHERING:
	    score = (u == campaign->gatherer);
	    break;
	case TARGET_OCCUPATION:
	    terrainid = battle->map->terrain[agent->target->location];
	    terrain = campaign->terrain[terrainid];
	    score
		= (utype->power * utype->range)
		+ (utype->hits + utype->armour)
		+ terrain->defence[u];
	    score *= !! terrain->moves[u];
	    break;
	case TARGET_REPAIR:
	    tunitid = battle->map->units[agent->target->location];
	    if (tunitid == CWG_NO_UNIT)
		break;
	    tutypeid = battle->units[tunitid]->utype;
	    score = utype->builds[tutypeid];
	    break;
	case TARGET_RALLY:
	    tunitid = battle->map->units[agent->target->location];
	    if (tunitid == CWG_NO_UNIT)
		break;
	    tutype = campaign->unittypes[battle->units[tunitid]->utype];
	    score = tutype->builds[u];
	    break;
	case TARGET_DEFENCE:
	    score = (utype->power * utype->range);
	    break;
	}

	/* recommend this if it's the best (affordable) unit type */
	if (score > bestscore) {
	    bestscore = score;
	    besttype = u;
	}
    }

    /* return the best type identified */
    return besttype;
}

/**
 * Find the best positions to build at and from.
 * @param  agent    The agent that is building.
 * @param  utypeid  The unit type being built.
 * @param  buildpos Where to return the build AT position.
 * @param  frompos  Where to return the build FROM position.
 */
static void bestbuildpositions (Agent *agent, int utypeid,
				int *buildpos, int *frompos)
{
    Unit *targetunit; /* pointer to the target unit */
    int agenttypeid, /* id of the agent unit type */
	unitmobile, /* 1 if the unit we're building is mobile */
	agentmobile, /* 1 if the agent is mobile */
	targetunitid, /* id of the unit at the target location */
	targetmobile, /* 1 if the target unit is mobile */
	range; /* attack range of the unit to build */
    Campaign *campaign; /* pointer to the campaign */
    Battle *battle; /* pointer to the battle */
    Map *map; /* pointer to the map */

    /* initialise some variables */
    campaign = ai->data->game->campaign;
    battle = ai->data->game->battle;
    map = battle->map;
    agenttypeid = battle->units[map->units[agent->location]]->utype;
    unitmobile = ai->data->unitcats[utypeid].mobile;
    agentmobile = ai->data->unitcats[agenttypeid].mobile;
    targetunitid = map->units[agent->target->location];
    if (targetunitid != CWG_NO_UNIT) {
	targetunit = battle->units[targetunitid];
	targetmobile = ai->data->unitcats[targetunit->utype].mobile;
    }
    range = campaign->unittypes[utypeid]->range;

    /* work out where the unit should be built */
    if (unitmobile || ! agentmobile)
	*buildpos = nearby
	    (agent->location, agent->location, agent->target->location,
	     1, utypeid);
    else if (agent->target->type == TARGET_ATTACK)
	*buildpos = nearby
	    (agent->location, agent->target->location, agent->location,
	     range, utypeid);
    else if (agent->target->type == TARGET_GATHERING ||
	agent->target->type == TARGET_OCCUPATION)
	*buildpos = agent->target->location;
    else if (agent->target->type == TARGET_REPAIR && targetmobile)
	*buildpos = nearby
	    (agent->location, agent->location, agent->target->location,
	     1, utypeid);
    else if (agent->target->type == TARGET_REPAIR)
	*buildpos = nearby
	    (agent->location, agent->target->location, agent->location,
	     1, utypeid);
    else if (agent->target->type == TARGET_DEFENCE)
	*buildpos = nearby
	    (agent->location, agent->target->location,
	     ai->data->theircentre, 1,
	     utypeid);

    /* work out where to build from */
    *frompos = agentmobile
	? nearby (agent->location, *buildpos, agent->location,
		  1, agenttypeid)
	: agent->location;
}

/**
 * Find the best position for an agent to attack from.
 * @param  agent  The agent attacking.
 * @param  target The target being attacked.
 * @return        The best location.
 */
static int bestattackposition (Agent *agent, Target *target)
{
    Map *map; /* the battle map */
    Unit *unit; /* the agent unit */
    UnitType *utype; /* the agent unit type */
    Terrain *terrain; /* terrain on square being scanned */
    int x1, /* top left square x coordinate to scan */
	y1, /* top left square y coordinate to scan */
	x2, /* bottom right square x coordinate to scan */
	y2, /* bottom right square y coordinate to scan */
	x, /* x coordinate being scanned */
	y, /* y coordinate being scanned */
	dist, /* distance from target of square being scanned */
	score, /* score for square being scanned */
	best, /* best score scanned */
	bx, /* x coordinate of best square scanned */
	by, /* y coordinate of best square scanned */
	tx, /* target's x coordinate */
	ty; /* target's y coordinate */
    int pfactor, /* proximity factor */
	rfactor, /* range factor */
	afactor; /* armour factor */

    /* get target coordinates */
    map = ai->data->game->battle->map;
    tx = target->location % map->width;
    ty = target->location / map->width;

    /* get coordinates to scan */
    unit = ai->data->game->battle->units[agent->unit];
    utype = ai->data->game->campaign->unittypes[unit->utype];
    x1 = tx - utype->range;
    y1 = ty - utype->range;
    x2 = tx + utype->range;
    y2 = ty + utype->range;
    if (x1 < 0)
	x1 = 0;
    if (y1 < 0)
	y1 = 0;
    if (x2 >= map->width)
	x2 = map->width - 1;
    if (y2 >= map->height)
	y2 = map->height - 1;

    /* start gathering information about the best square */
    best = 0;
    bx = tx;
    by = ty;
    for (x = x1; x <= x2; ++x)
	for (y = y1; y <= y2; ++y) {
	    if (map->units[x + map->width * y] != CWG_NO_UNIT &&
		x != unit->x && y != unit->y)
		continue;
	    dist = (abs (x - tx) > abs (y - ty))
		? abs (x - tx)
		: abs (y - ty);
	    terrain = ai->data->game->campaign->terrain
		[map->terrain[x + map->width * y]];
	    if (! terrain->moves[unit->utype])
		continue;
	    pfactor = proximityfactor
		(agent->location, x + map->width * y);
	    rfactor = 2 * dist;
	    afactor = terrain->defence[unit->utype];
	    score = pfactor + rfactor + afactor;
	    if (score > best) {
		best = score;
		bx = x;
		by = y;
	    }
	}

    /* return the best square found */
    return bx + map->width * by;
}

/**
 * Find the best position for an agent to defend from.
 * @param  agent  The agent attacking.
 * @param  target The target being attacked.
 * @return        The best location.
 */
static int bestdefenceposition (Agent *agent, Target *target)
{
    Battle *battle; /* the battle */
    Map *map; /* the battle map */
    Unit *unit; /* the agent unit */
    UnitType *utype; /* the agent unit type */
    int x1, /* top left square x coordinate to scan */
	y1, /* top left square y coordinate to scan */
	x2, /* bottom right square x coordinate to scan */
	y2, /* bottom right square y coordinate to scan */
	x, /* x coordinate being scanned */
	y, /* y coordinate being scanned */
	score, /* score for square being scanned */
	best, /* best score scanned */
	bx, /* x coordinate of best square scanned */
	by, /* y coordinate of best square scanned */
	tx, /* target's x coordinate */
	ty; /* target's y coordinate */

    /* get target coordinates */
    battle = ai->data->game->battle;
    map = battle->map;
    tx = target->location % map->width;
    ty = target->location / map->width;

    /* get coordinates to scan */
    unit = ai->data->game->battle->units[agent->unit];
    utype = ai->data->game->campaign->unittypes[unit->utype];
    x1 = tx - utype->range;
    y1 = ty - utype->range;
    x2 = tx + utype->range;
    y2 = ty + utype->range;
    if (x1 < 0)
	x1 = 0;
    if (y1 < 0)
	y1 = 0;
    if (x2 >= map->width)
	x2 = map->width - 1;
    if (y2 >= map->height)
	y2 = map->height - 1;

    /* start gathering information about the best square */
    best = 0;
    bx = tx;
    by = ty;
    for (x = x1; x <= x2; ++x)
	for (y = y1; y <= y2; ++y) {
	    if (map->units[x + map->width * y] != CWG_NO_UNIT &&
		x != unit->x && y != unit->y)
		continue;
	    score = proximityfactor
		(ai->data->theircentre, x + map->width * y);
	    if (score > best) {
		best = score;
		bx = x;
		by = y;
	    }
	}

    /* return the best square found */
    return bx + map->width * by;
}

/**
 * Return the distance between two units, allowing diagonals.
 * @param  first  The first of the two units.
 * @param  second The second of the two units.
 * @return        The distance between them.
 */
static int distance (Unit *first, Unit *second)
{
    return (abs (first->x - second->x) > abs (first->y - second->y))
	? abs (first->x - second->x)
	: abs (first->y - second->y);
}

/*----------------------------------------------------------------------
 * Level 2 Private Function Definitions.
 */

/**
 * Target an enemy-controlled square for attack.
 * @param  targets The linked list of targets.
 * @param  square  The square to target.
 * @return         The updated linked list of targets.
 */
static Target *targetattack (Target *targets, int square)
{
    Target *target; /* new target */
    Battle *battle; /* the battle */
    UnitType *utype; /* target's unit type */
    int utypeid; /* unit type ID */

    /* create new target */
    if (! (target = malloc (sizeof (Target))))
	fatalerror (FATAL_MEMORY);

    /* get information about the target unit */
    battle = ai->data->game->battle;
    utypeid = battle->units[battle->map->units[square]]->utype;
    utype = ai->data->game->campaign->unittypes[utypeid];

    /* target the attack */
    target->type = TARGET_ATTACK;
    target->location = square;
    target->importance
	= builderfactor (utypeid)
	+ gathererfactor (utypeid)
	+ pointfactor (square)
	+ resourcefactor (square);
    target->importance /= 4;
    target->importance
	+= proximityfactor (square, ai->data->ourcentre);
    target->utype = utypeid;
    target->requirements
	= utype->hits
	+ utype->range * utype->power
	+ utype->armour;
    target->allocation = 0;
    target->next = targets;

    /* return the updated target list */
    return target;
}

/**
 * Target a resource square for gathering.
 * @param  targets The linked list of targets.
 * @param  square  The square to target.
 * @return         The updated linked list of targets.
 */
static Target *targetgathering (Target *targets, int square)
{
    Target *target; /* new target */
    Game *game; /* pointer to the game */

    /* create new target */
    if (! (target = malloc (sizeof (Target))))
	fatalerror (FATAL_MEMORY);

    /* target the resource */
    game = ai->data->game;
    target->type = TARGET_GATHERING;
    target->location = square;
    target->importance
	= proximityfactor (square, ai->data->ourcentre)
	+ resourcefactor (square);
    target->utype = ai->data->game->campaign->gatherer;
    target->requirements = 1;
    target->allocation = 0;
    target->next = targets;

    /* return the updated target list */
    return target;
}

/**
 * Target a victory point square for occupation.
 * @param targets The linked list of targets.
 * @param square  The square to target.
 * @return        The updated linked list of targets.
 */
static Target *targetpoint (Target *targets, int square)
{
    Target *target; /* new target */
    UnitType **unittypes; /* pointer to unit types */
    int ut; /* unit type counter */

    /* create new target */
    if (! (target = malloc (sizeof (Target))))
	fatalerror (FATAL_MEMORY);

    /* initialise convenience variables */
    unittypes = ai->data->game->campaign->unittypes;

    /* target the victory point */
    target->type = TARGET_OCCUPATION;
    target->location = square;
    target->importance
	= proximityfactor (square, ai->data->ourcentre)
	+ pointfactor (square);
    target->utype = 0;
    for (ut = 0; ut < CWG_UTYPES; ++ut)
	if (unittypes[ut]->power > unittypes[target->utype]->power)
	    target->utype = ut;
    target->requirements = ai->data->maxdist;
    target->allocation = 0;
    target->next = targets;

    /* return the updated target list */
    return target;
}

/**
 * Target a damaged friendly unit for repair.
 * @param  targets The linked list of targets.
 * @param  square  The square to target.
 * @return         The updated linked list of targets.
 */
static Target *targetrepair (Target *targets, int square)
{
    Target *target; /* new target */
    Battle *battle; /* the battle */
    UnitType *utype; /* target's unit type */
    Unit *unit; /* target unit */
    int utypeid; /* unit type ID */

    /* create new target */
    if (! (target = malloc (sizeof (Target))))
	fatalerror (FATAL_MEMORY);

    /* get information about the target unit */
    battle = ai->data->game->battle;
    unit = battle->units[battle->map->units[square]];
    utypeid = unit->utype;
    utype = ai->data->game->campaign->unittypes[utypeid];

    /* target the repair */
    target->type = TARGET_REPAIR;
    target->location = square;
    target->importance
	= damagefactor (unit)
	+ proximityfactor (square, ai->data->ourcentre);
    target->requirements = ai->data->maxdist;
    target->allocation = 0;
    target->next = targets;

    /* return the updated target list */
    return target;
}

/**
 * Target a friendly repair unit for rallying damaged units.
 * @param  targets The linked list of targets.
 * @param  square  The square to target.
 * @return         The updated linked list of targets.
 */
static Target *targetrally (Target *targets, int square)
{
    Target *target; /* new target */
    Battle *battle; /* the battle */
    UnitType *utype; /* target's unit type */
    Unit *unit; /* target unit */
    int utypeid; /* unit type ID */

    /* create new target */
    if (! (target = malloc (sizeof (Target))))
	fatalerror (FATAL_MEMORY);

    /* get information about the target unit */
    battle = ai->data->game->battle;
    unit = battle->units[battle->map->units[square]];
    utypeid = unit->utype;
    utype = ai->data->game->campaign->unittypes[utypeid];

    /* target the rallying point */
    target->type = TARGET_RALLY;
    target->location = square;
    target->importance
	= 2 * proximityfactor (square, ai->data->ourcentre);
    target->requirements = 2;
    target->allocation = 0;
    target->next = targets;

    /* return the updated target list */
    return target;
}

/**
 * Target a static non-combatant friendly unit for defence.
 * @param  targets The linked list of targets.
 * @param  square  The square to target.
 * @return         The updated linked list of targets.
 */
static Target *targetdefence (Target *targets, int square)
{
    Target *target; /* new target */
    Battle *battle; /* the battle */
    UnitType *utype, /* target's unit type */
	**unittypes; /* pointer to unit type list */
    int utypeid, /* unit type ID */
	ut; /* unit type counter */

    /* create new target */
    if (! (target = malloc (sizeof (Target))))
	fatalerror (FATAL_MEMORY);

    /* get information about the target unit */
    unittypes = ai->data->game->campaign->unittypes;
    battle = ai->data->game->battle;
    utypeid = battle->units[battle->map->units[square]]->utype;
    utype = ai->data->game->campaign->unittypes[utypeid];

    /* target the attack */
    target->type = TARGET_DEFENCE;
    target->location = square;
    target->importance
	= staticfactor (utypeid)
	+ resourcefactor (square)
	+ pointfactor (square);
    target->importance /= 3;
    target->importance
	+= proximityfactor (square, ai->data->theircentre);
    target->utype = 0;
    for (ut = 0; ut < CWG_UTYPES; ++ut)
	if (unittypes[ut]->power > unittypes[target->utype]->power)
	    target->utype = ut;
    target->requirements = 1;
    target->allocation = 0;
    target->next = targets;

    /* return the updated target list */
    return target;
}

/**
 * Sort the targets by descending order of importance.
 * @param  targets The linked list of targets to sort.
 * @return         The sorted linked list of targets.
 */
static Target *sorttargets (Target *targets)
{
    Target *prev, /* previous node */
	*curr, /* current node */
	*next; /* next node */
    int sorted = 0; /* 1 when sorted */

    /* main sorting loop */
    do {

	/* initialise iteration */
	sorted = 1;
	curr = targets;
	prev = NULL;

	/* compare each consecutive list pair */
	while (curr && curr->next) {
	    next = curr->next;

	    /* compare and swap if necessary */
	    if (curr->importance < next->importance) {
		curr->next = next->next;
		next->next = curr;
		if (prev)
		    prev->next = next;
		else
		    targets = next;
		sorted = 0;
	    }

	    /* move to next list pair */
	    prev = curr;
	    curr = curr->next;
	}
    } while (! sorted);

    /* return the sorted list */
    return targets;
}

/**
 * Get a list of potential agents.
 * @return A linked list of agents.
 */
static Agent *getagents (void)
{
    Agent *agents = NULL, /* the list of agents to return */
	*agent; /* an agent to add to the list */
    Battle *battle; /* pointer to the battle */
    Map *map; /* pointer to the battle map */
    Unit *unit; /* pointer to a unit on the map */
    int s, /* map square counter */
	count = 0; /* agent counter */

    /* initialise convenience variables */
    battle = ai->data->game->battle;
    map = battle->map;

    /* look at all of the squares on the map */
    for (s = 0; s < map->width * map->height; ++s) {

	/* identify any unit on the map */
	unit = (map->units[s] == CWG_NO_UNIT)
	    ? NULL
	    : battle->units[map->units[s]];

	/* if a friendly unit, add it to the agent list */
	if (unit && unit->side == battle->side) {
	    if (! (agent = malloc (sizeof (Agent))))
		fatalerror (FATAL_MEMORY);
	    agent->count = ++count;
	    agent->unit = map->units[s];
	    agent->location = s;
	    agent->suitability = 0;
	    agent->building = 0;
	    agent->target = NULL;
	    agent->next = agents;
	    agents = agent;
	}
    }

    /* return the agent list */
    return agents;
}

/**
 * Allocate an attacker to an attack target.
 * @param  target A pointer to the attack target in a linked list.
 * @param  agents A pointer to the linked list of agents.
 * @return        A pointer to the target to allocate against next.
 */
static Target *allocateattacker (Target *target, Agent *agents)
{
    Agent *agent, /* pointer to current agent */
	**index; /* index of agents */
    Battle *battle; /* pointer to the battle */
    Unit *unit; /* pointer to current unit */
    UnitCategory *unitcats; /* pointer to unit category data */
    UnitType *utype; /* pointer to current unit's type */
    int i; /* counter for index */

    /* initialise convenience variables */
    battle = ai->data->game->battle;
    unitcats = ai->data->unitcats;
    index = indexagents (agents);

    /* score all the agents for suitability */
    for (agent = agents; agent; agent = agent->next)

	/* agent already has a target - skip it */
	if (agent->target)
	    agent->suitability = 0;

	/* agent doesn't yet have a target */
	else {
	    unit = battle->units[agent->unit];

	    /* score builders */
	    if (unitcats[unit->utype].builder) {
		agent->suitability
		    = builderfactor (unit->utype);
		agent->building = 1;
	    }

	    /* score firepower for combatants */
	    else {
		agent->suitability
		    = powerfactor (unit->utype)
		    + ai->data->maxdist - damagefactor (unit);
		agent->suitability /= 2;
		agent->building = 0;
	    }

	    /* score proximity for mobile units */
	    if (unitcats[unit->utype].mobile)
		agent->suitability
		    += proximityfactor (agent->location,
					target->location);

	    /* score range for static units */
	    else if (unitcats[unit->utype].combatant)
		agent->suitability
		    += rangefactor (unit, target->location);

	    /* non-mobile non-combatant non-builders excluded */
	    else if (! unitcats[unit->utype].builder)
		agent->suitability = 0;
	}

    /* rank agents according to suitability */
    sortagents (agents->count, index);

    /* allocate agents */
    for (i = 0;
	 i < agents->count &&
	     index[i]->suitability &&
	     target->allocation < target->requirements;
	 ++i) {
	index[i]->target = target;
	unit = battle->units[index[i]->unit];
	utype = ai->data->game->campaign->unittypes[unit->utype];
	target->allocation
	    += utype->power * utype->range;
    }

    /* next target */
    free (index);
    return target->next;
}

/**
 * Allocate a gatherer to a resource unit.
 * @param  target A pointer to the attack target in a linked list.
 * @param  agents A pointer to the linked list of agents.
 * @return        A pointer to the target to allocate against next.
 */
static Target *allocategatherer (Target *target, Agent *agents)
{
    Agent *agent, /* pointer to current agent */
	**index; /* index of agents */
    Battle *battle; /* pointer to the battle */
    Unit *unit; /* pointer to current unit */
    UnitType *utype; /* pointer to the current unit's type */
    UnitCategory *unitcats; /* pointer to unit category data */
    Terrain *terrain; /* terrain of target square */
    int i; /* counter for index */

    /* initialise convenience variables */
    battle = ai->data->game->battle;
    unitcats = ai->data->unitcats;
    index = indexagents (agents);
    terrain = ai->data->game->campaign->terrain
	[battle->map->terrain[target->location]];

    /* score all the agents for suitability */
    for (agent = agents; agent; agent = agent->next) {

	/* get agent unit information */
	unit = battle->units[agent->unit];
	utype = ai->data->game->campaign->unittypes[unit->utype];

	/* agent already has a target - skip it */
	if (agent->target)
	    agent->suitability = 0;

	/* consider building a gatherer unit */
	else if (utype->builds[ai->data->game->campaign->gatherer]) {
	    agent->building = 1;
	    agent->suitability = proximityfactor
		(agent->location, target->location);
	}

	/* reject non-builder non-gatherers */
	else if (unit->utype != ai->data->game->campaign->gatherer)
	    agent->suitability = 0;

	/* reject static units not on the spot */
	else if (agent->location != target->location &&
		 ! ai->data->unitcats[unit->utype].mobile)
	    agent->suitability = 0;

	/* otherwise score by proximity */
	else
	{
	    agent->suitability = proximityfactor
		(agent->location, target->location);
	    agent->building = 0;
	}

    }

    /* rank agents according to suitability */
    sortagents (agents->count, index);

    /* allocate agents */
    for (i = 0;
	 i < agents->count &&
	     index[i]->suitability &&
	     target->allocation < target->requirements;
	 ++i) {
	index[i]->target = target;
	target->allocation = 1;
    }

    /* next target */
    free (index);
    return target->next;
}

/**
 * Allocate an occupier to a victory point.
 * @param  target A pointer to the attack target in a linked list.
 * @param  agents A pointer to the linked list of agents.
 * @return        A pointer to the target to allocate against next.
 */
static Target *allocateoccupier (Target *target, Agent *agents)
{
    Agent *agent, /* pointer to current agent */
	**index; /* index of agents */
    Battle *battle; /* pointer to the battle */
    Unit *unit; /* pointer to current unit */
    UnitCategory *unitcats; /* pointer to unit category data */
    Terrain *terrain; /* terrain of target square */
    int i; /* counter for index */

    /* initialise convenience variables */
    battle = ai->data->game->battle;
    unitcats = ai->data->unitcats;
    index = indexagents (agents);
    terrain = ai->data->game->campaign->terrain
	[battle->map->terrain[target->location]];

    /* score all the agents for suitability */
    for (agent = agents; agent; agent = agent->next)

	/* agent already has a target - skip it */
	if (agent->target)
	    agent->suitability = 0;

	/* agent doesn't yet have a target */
	else {
	    unit = battle->units[agent->unit];

	    /* score builders */
	    if (unitcats[unit->utype].builder) {
		agent->suitability
		    = builderfactor (unit->utype);
		agent->building = 1;
	    }

	    /* score combatants */
	    else {
		agent->suitability = ai->data->maxdist;
		agent->building = 0;
	    }

	    /* score proximity */
	    agent->suitability
		+= proximityfactor (agent->location, target->location);

	    /* give units already occupying the square a max score */
	    if (agent->location == target->location)
		agent->suitability = 2 * ai->data->maxdist;

	    /* exclude units that can't occupy the square */
	    else if (! agent->building && ! terrain->moves[unit->utype])
		agent->suitability = 0;

	}

    /* rank agents according to suitability */
    sortagents (agents->count, index);

    /* allocate agents */
    for (i = 0;
	 i < agents->count &&
	     index[i]->suitability &&
	     target->allocation < target->requirements;
	 ++i) {
	index[i]->target = target;
	target->allocation = target->requirements;
    }

    /* next target */
    free (index);
    return target->next;
}

/**
 * Allocate a repair unit to a damaged colleague.
 * @param  target A pointer to the attack target in a linked list.
 * @param  agents A pointer to the linked list of agents.
 * @return        A pointer to the target to allocate against next.
 */
static Target *allocaterepairer (Target *target, Agent *agents)
{
    Agent *agent, /* pointer to current agent */
	**index; /* index of agents */
    Battle *battle; /* pointer to the battle */
    Unit *unit, /* pointer to current unit */
	*targetunit; /* pointer to the target unit */
    UnitType *utype, /* pointer to the current unit's type */
	*btype; /* pointer to a potential repairer's unit type */
    UnitCategory *unitcats; /* pointer to unit category data */
    Terrain *terrain; /* terrain of target square */
    int i, /* counter for index */
	u; /* counter for potential repairer unit types */

    /* initialise convenience variables */
    battle = ai->data->game->battle;
    unitcats = ai->data->unitcats;
    index = indexagents (agents);
    terrain = ai->data->game->campaign->terrain
	[battle->map->terrain[target->location]];
    targetunit = battle->units[battle->map->units[target->location]];

    /* score all the agents for suitability */
    for (agent = agents; agent; agent = agent->next) {

	/* get agent unit information */
	unit = battle->units[agent->unit];
	utype = ai->data->game->campaign->unittypes[unit->utype];

	/* agent already has a target - skip it */
	if (agent->target)
	    agent->suitability = 0;

	/* consider agents that can repair the target unit */
	else if (utype->builds[targetunit->utype]) {

	    /* we are not building */
	    agent->building = 0;

	    /* unit is adjacent to target - fast track it */
	    if (distance (unit, targetunit) == 1)
		agent->suitability = ai->data->maxdist;

	    /* agent doesn't yet have a target - score it */
	    else if (unitcats[unit->utype].mobile)
		agent->suitability = proximityfactor
		    (agent->location, target->location);

	}

	/* see if the agent can build something that can repair
	   the target unit */
	else {
	    agent->suitability = 0;
	    for (u = 0; u < CWG_UTYPES; ++u) {
		btype = ai->data->game->campaign->unittypes[u];
		if (utype->builds[u] &&
		    btype->builds[targetunit->utype]) {
		    agent->suitability = proximityfactor
			(agent->location, target->location) / 2;
		    agent->building = 1;
		}
	    }
	}
    }

    /* rank agents according to suitability */
    sortagents (agents->count, index);

    /* allocate agents */
    for (i = 0;
	 i < agents->count &&
	     index[i]->suitability &&
	     target->allocation < target->requirements;
	 ++i) {
	index[i]->target = target;
	target->allocation = 1;
    }

    /* next target */
    free (index);
    return target->next;
}

/**
 * Allocate a damaged unit to a repair unit.
 * @param  target A pointer to the attack target in a linked list.
 * @param  agents A pointer to the linked list of agents.
 * @return        A pointer to the target to allocate against next.
 */
static Target *allocatedamagedunit (Target *target, Agent *agents)
{
    Agent *agent, /* pointer to current agent */
	**index; /* index of agents */
    Battle *battle; /* pointer to the battle */
    Unit *unit, /* pointer to current unit */
	*targetunit; /* pointer to the target unit */
    UnitType *utype, /* pointer to the current unit's type */
	*targettype; /* pointer to target's unit type */
    UnitCategory *unitcats; /* pointer to unit category data */
    Terrain *terrain; /* terrain of target square */
    int i; /* counter for index */

    /* don't repair if we have no resources */
    battle = ai->data->game->battle;
    if (battle->resources[battle->side] == 0 ||
	ai->data->gatherers == 0)
	return target->next;

    /* initialise convenience variables */
    unitcats = ai->data->unitcats;
    index = indexagents (agents);
    terrain = ai->data->game->campaign->terrain
	[battle->map->terrain[target->location]];
    targetunit = battle->units[battle->map->units[target->location]];
    targettype = ai->data->game->campaign->unittypes[targetunit->utype];

    /* score all the agents for suitability */
    for (agent = agents; agent; agent = agent->next) {

	/* get agent unit information */
	unit = battle->units[agent->unit];
	utype = ai->data->game->campaign->unittypes[unit->utype];

	/* agent already has a target - skip it */
	if (agent->target)
	    agent->suitability = 0;

    	/* agent is in full repair - skip it */
	else if (unit->hits == utype->hits)
	    agent->suitability = 0;

	/* agent is static - skip it */
	else if (! unitcats[unit->utype].mobile)
	    agent->suitability = 0;

	/* target cannot repair this unit - skip it */
	else if (! targettype->builds[unit->utype])
	    agent->suitability = 0;

	/* unit is adjacent to target - fast track it */
	else if (distance (unit, targetunit) == 1)
	    agent->suitability = 2 * ai->data->maxdist;

	/* agent doesn't yet have a target - score it */
	else {
	    agent->suitability = damagefactor (unit);
	    agent->suitability
		+= proximityfactor (agent->location, target->location);
	    agent->building = 0;
	}
    }

    /* rank agents according to suitability */
    sortagents (agents->count, index);

    /* allocate agents */
    for (i = 0;
	 i < agents->count && index[i]->suitability;
	 ++i) {
	unit = battle->units[index[i]->unit];
	index[i]->target = target;
	++target->allocation;
    }

    /* next target */
    free (index);
    return target->next;
}

/**
 * Allocate a defender to a vulnerable unit.
 * @param  target A pointer to the attack target in a linked list.
 * @param  agents A pointer to the linked list of agents.
 * @return        A pointer to the target to allocate against next.
 */
static Target *allocatedefender (Target *target, Agent *agents)
{
    Agent *agent, /* pointer to current agent */
	**index; /* index of agents */
    Battle *battle; /* pointer to the battle */
    Unit *unit; /* pointer to current unit */
    UnitCategory *unitcats; /* pointer to unit category data */
    UnitType *utype; /* pointer to current unit's type */
    int i; /* counter for index */

    /* initialise convenience variables */
    battle = ai->data->game->battle;
    unitcats = ai->data->unitcats;
    index = indexagents (agents);

    /* score all the agents for suitability */
    for (agent = agents; agent; agent = agent->next)

	/* agent already has a target - skip it */
	if (agent->target)
	    agent->suitability = 0;

	/* agent is target */
	else if (agent->location == target->location)
	    agent->suitability = 0;

	/* agent doesn't yet have a target */
	else {
	    unit = battle->units[agent->unit];

	    /* score builders */
	    if (unitcats[unit->utype].builder) {
		agent->suitability
		    = builderfactor (unit->utype);
		agent->building = 1;
	    }

	    /* score firepower for combatants */
	    else {
		agent->suitability
		    = powerfactor (unit->utype)
		    + ai->data->maxdist - damagefactor (unit);
		agent->suitability /= 2;
		agent->building = 0;
	    }

	    /* score proximity for mobile units */
	    if (unitcats[unit->utype].mobile)
		agent->suitability
		    += proximityfactor (agent->location,
					target->location);

	    /* score range for static units */
	    else if (unitcats[unit->utype].combatant)
		agent->suitability
		    += rangefactor (unit, target->location);

	    /* non-mobile non-combatant non-builders excluded */
	    else if (! unitcats[unit->utype].builder)
		agent->suitability = 0;
	}

    /* rank agents according to suitability */
    sortagents (agents->count, index);

    /* allocate agents */
    for (i = 0;
	 i < agents->count &&
	     index[i]->suitability &&
	     target->allocation < target->requirements;
	 ++i) {
	index[i]->target = target;
	unit = battle->units[index[i]->unit];
	utype = ai->data->game->campaign->unittypes[unit->utype];
	++target->allocation;
    }

    /* next target */
    free (index);
    return target->next;
}

/**
 * Build a unit to attack a target.
 * @param agent The agent to act.
 */
static void buildaction (Agent *agent)
{
    int bpx, /* build at position x */
	bpy, /* build at position y */
	fpx, /* build from position x */
	fpy, /* build from position y */
	utypeid, /* type of unit to build */
	buildpos = -1, /* position to build at */
	frompos = -1; /* position to build from */
    Battle *battle; /* pointer to the battle */
    Unit *unit; /* agent unit */

    /* determine most appropriate unit type to build */
    utypeid = bestbuildunittype (agent);
    if (utypeid == -1)
	return;

    /* determine where to build the new unit */
    battle = ai->data->game->battle;
    bestbuildpositions (agent, utypeid, &buildpos, &frompos);
    if (buildpos < 0 || frompos < 0)
	return;
    bpx = buildpos % battle->map->width;
    bpy = buildpos / battle->map->width;
    fpx = frompos % battle->map->width;
    fpy = frompos / battle->map->width;

    /* attempt to build the unit */
    initreport ();
    unit = battle->units[agent->unit];
    battle->move (battle, unit, fpx, fpy, movehook);
    battle->create (battle, unit, utypeid, bpx, bpy, buildhook);
}

/**
 * Attack a target.
 * @param agent The agent to act.
 */
static void attackaction (Agent *agent)
{
    int position, /* position to attack from */
	apx, /* attack position x */
	apy; /* attack position y */
    Battle *battle; /* pointer to the battle */
    Unit *unit, /* agent unit */
	*target; /* target unit */

    /* find the best location to attack from */
    position = bestattackposition (agent, agent->target);
    battle = ai->data->game->battle;
    apx = position % battle->map->width;
    apy = position / battle->map->width;

    /* move to the attack location and fire */
    initreport ();
    unit = battle->units[agent->unit];
    target = battle->units[battle->map->units[agent->target->location]];
    battle->move (battle, unit, apx, apy, movehook);
    battle->attack (battle, unit, target, attackhook);
}

/**
 * Gather resources.
 * @param agent The agent to act.
 */
static void gatheringaction (Agent *agent)
{
    int x, /* target location x */
	y; /* target location y */
    Battle *battle; /* pointer to the battle */
    Unit *unit; /* agent unit */

    /* get target location as x/y pair */
    battle = ai->data->game->battle;
    x = agent->target->location % battle->map->width;
    y = agent->target->location / battle->map->width;

    /* move to resource location */
    initreport ();
    unit = battle->units[agent->unit];
    battle->move (battle, unit, x, y, movehook);
}

/**
 * Occupy a victory point.
 * @param agent The agent to act.
 */
static void occupationaction (Agent *agent)
{
    int x, /* target location x */
	y; /* target location y */
    Battle *battle; /* pointer to the battle */
    Unit *unit; /* agent unit */

    /* get target location as x/y pair */
    battle = ai->data->game->battle;
    x = agent->target->location % battle->map->width;
    y = agent->target->location / battle->map->width;

    /* move to occupation location */
    initreport ();
    unit = battle->units[agent->unit];
    battle->move (battle, unit, x, y, movehook);
}

/**
 * Repair a target.
 * @param agent The agent to act.
 */
static void repairaction (Agent *agent)
{
    Battle *battle; /* pointer to the battle map */
    Unit *agentunit, /* pointer to the agent unit */
	*targetunit; /* pointer to the target unit */

    /* initialise convenience variables */
    battle = ai->data->game->battle;
    agentunit = battle->units[agent->unit];
    targetunit = battle->units
	[battle->map->units[agent->target->location]];

    /* move towards target unit */
    initreport ();
    if (distance (agentunit, targetunit) > 1)
	battle->move (battle, agentunit, targetunit->x, targetunit->y,
		      movehook);
    battle->restore (battle, agentunit, targetunit, repairhook);
}

/**
 * Rally to a friendly target for repair.
 * @param agent The agent to act.
 */
static void rallyaction (Agent *agent)
{
    Battle *battle; /* pointer to the battle map */
    Unit *agentunit, /* pointer to the agent unit */
	*targetunit; /* pointer to the target unit */

    /* initialise convenience variables */
    battle = ai->data->game->battle;
    agentunit = battle->units[agent->unit];
    targetunit = battle->units
	[battle->map->units[agent->target->location]];

    /* move towards target unit */
    initreport ();
    if (distance (agentunit, targetunit) > 1)
	battle->move (battle, agentunit, targetunit->x, targetunit->y,
		      movehook);
}

/**
 * Defend a friendly target.
 * @param agent The agent to act.
 */
static void defenceaction (Agent *agent)
{
    int position, /* position to defend from */
	apx, /* defence position x */
	apy; /* defence position y */
    Battle *battle; /* pointer to the battle */
    Unit *unit, /* agent unit */
	*target; /* target unit */

    /* find the best location to defend from */
    battle = ai->data->game->battle;
    position = bestdefenceposition (agent, agent->target);
    apx = position % battle->map->width;
    apy = position / battle->map->width;

    /* move to the defence location */
    initreport ();
    unit = battle->units[agent->unit];
    target = battle->units[battle->map->units[agent->target->location]];
    battle->move (battle, unit, apx, apy, movehook);
}

/**
 * Repair any adjacent unit that is damaged.
 * @param agent The agent to act.
 */
static void opportunityrepair (Agent *agent, Target *targets)
{
    Battle *battle; /* pointer to the battle */
    Unit *agentunit, /* the agent unit */
	*targetunit; /* the target unit */
    Target *target; /* pointer to target under examination */
    battle = ai->data->game->battle;
    agentunit = battle->units[agent->unit];
    for (target = targets; target; target = target->next)
	if (target->type == TARGET_REPAIR) {
	    targetunit
		= battle->units[battle->map->units[target->location]];
	    battle->restore (battle, agentunit, targetunit, repairhook);
	}
}

/**
 * Attack any enemy target in range.
 * @param agent The agent to act.
 */
static void opportunityfire (Agent *agent)
{
    Battle *battle; /* pointer to the battle object */
    Unit *aunit, /* pointer to the agent unit */
	*tunit; /* pointer to the target unit */
    UnitType *atype; /* pointer to agent unit's type */
    int u; /* unit counter */

    /* initialise convenience variables */
    battle = ai->data->game->battle;
    aunit = battle->units[agent->unit];
    atype = ai->data->game->campaign->unittypes[aunit->utype];

    /* validate */
    if (! aunit->hits || ! aunit->moves)
	return;

    /* look for a target */
    for (u = 0; u < CWG_UNITS; ++u)
	if ((tunit = ai->data->game->battle->units[u]) &&
	    tunit->hits &&
	    tunit->side != battle->side &&
	    distance (aunit, tunit) <= atype->range)
	{
	    initreport ();
	    battle->attack (battle, aunit, tunit, attackhook);
	}
}

/**
 * Target the nearest enemy for attack.
 * @param agent   The agent to act.
 * @param targets A list of targets.
 */
static void opportunitytarget (Agent *agent, Target *targets)
{
    Target *target, /* pointer to target */
	*nearest; /* pointer to nearest target */
    int best = 0,
	score; /* distance to nearest target */
    Unit *unit; /* pointer to the target unit */
    Battle *battle; /* pointer to the battle */

    /* initialise */
    battle = ai->data->game->battle;

    /* find the nearest enemy */
    for (target = targets; target; target = target->next) {
	if (target->type != TARGET_ATTACK)
	    continue;
	unit = battle->units[battle->map->units[target->location]];
	if (! unit->hits || unit->side == battle->side)
	    continue;
	score = proximityfactor (agent->location, target->location);
	if (score > best) {
	    best = score;
	    nearest = target;
	}
    }

    /* choose the nearest enemy as an attack target */
    agent->target = nearest;
}

/*----------------------------------------------------------------------
 * Level 1 Private Function Definitions.
 */

/**
 * Categorise the different unit types.
 */
static void categorise (void)
{
    int u, /* unit type counter */
	c, /* terrain or secondary unit type counter */
	combatants = 0, /* number of combatant units */
	builders = 0; /* number of builder units */
    Campaign *campaign; /* pointer to the campaign */
    UnitCategory *unitcats; /* pointer ot unit categories */

    /* initialise convenience variables */
    campaign = ai->data->game->campaign;
    unitcats = ai->data->unitcats;

    /* do initial categorisations */
    for (u = 0; u < CWG_UTYPES; ++u)
	if (campaign->unittypes[u]) {

	    /* is this unit a gatherer? */
	    unitcats[u].gatherer = (u == campaign->gatherer);

	    /* is this unit a builder? */
	    unitcats[u].builder = 0;
	    for (c = 0; c < CWG_UTYPES; ++c)
		if (campaign->unittypes[u]->builds[c])
		    builders += unitcats[u].builder = 1;

	    /* is this unit a combatant? */
	    unitcats[u].combatant =
		! unitcats[u].gatherer &&
		! unitcats[u].builder &&
		campaign->unittypes[u]->power;
	    combatants += unitcats[u].combatant;

	    /* is this unit mobile? */
	    unitcats[u].mobile = 0;
	    for (c = 0; c < CWG_TERRAIN; ++c)
		if (campaign->terrain[c]->moves[u])
		    unitcats[u].mobile = 1;
	}

    /* if we are left without combatants, include gatherers/builders
       with offensive capability among the combatants too. */
    if (! combatants)
	for (u = 0; u < CWG_UTYPES; ++u)
	    unitcats[u].combatant
		= (campaign->unittypes[u]->power != 0);
}

/**
 * Work out the maximum attack power on the battlefield.
 * @return The maximum attack power (power * range).
 */
static int maxpower (void)
{
    Battle *battle; /* pointer to battle */
    UnitType *utype, /* pointer to a unit's type */
	**unittypes; /* pointer to the campaign's unit type list */
    int u, /* unit counter */
	max; /* accumulated maximum */

    /* initialise convenience variables */
    battle = ai->data->game->battle;
    unittypes = ai->data->game->campaign->unittypes;

    /* find the most powerful unit on the battlefield ... */
    max = 0;
    for (u = 0; u < CWG_UNITS; ++u)
	if (battle->units[u] &&
	    (utype = unittypes[battle->units[u]->utype]) &&
	    utype->hits &&
	    utype->power * utype->range > max)
	    max = utype->power * utype->range;

    /* ... and return its power */
    return max;
}

/**
 * Count the number of friendly gatherer units.
 * @return The number of friendly gatherers on the map.
 */
static int countgatherers (void)
{
    int u; /* unit counter */
    Unit *unit; /* pointer to a unit in the battle */
    ai->data->gatherers = 0;
    for (u = 0; u < CWG_UNITS; ++u)
	if ((unit = ai->data->game->battle->units[u]) &&
	    unit->side == ai->data->game->battle->side &&
	    unit->hits &&
	    ai->data->unitcats[unit->utype].gatherer)
	    ++ai->data->gatherers;
    return ai->data->gatherers;
}

/**
 * Check if either side can occupy all victory points.
 * @return 1 if yes, 0 if no.
 */
static int checkpoints (void)
{
    Map *map; /* the battle map */
    int s, /* map square counter */
	points = 0, /* total number of victory points */
	units[2]; /* total number of mobile or occupying units */
    Unit *unit; /* current unit under consideration */
    map = ai->data->game->battle->map;
    units[0] = units[1] = 0;
    for (s = 0; s < map->width * map->height; ++s) {
	if (map->points[s])
	    ++points;
	if (map->units[s] != CWG_NO_UNIT) {
	    unit = ai->data->game->battle->units[map->units[s]];
	    if (ai->data->unitcats[unit->utype].mobile ||
		map->points[s])
		++units[unit->side];
	}
    }
    return units[0] >= points || units[1] >= points;
}

/**
 * Determine targets
 * @return A linked list of targets.
 */
static Target *gettargets (void)
{
    Target *targets = NULL; /* the list of targets to return */
    UnitCategory *unitcats; /* pointer to unit category array */
    Campaign *campaign; /* pointer to campaign object */
    Battle *battle; /* pointer to the battle */
    Map *map; /* pointer to the battle map */
    Unit *unit; /* pointer to a unit on the map */
    UnitType *utype; /* pointer to unit's type */
    Terrain *terrain; /* pointer to terrain definition */
    int s; /* map square counter */

    /* initialise convenience variables */
    unitcats = ai->data->unitcats;
    campaign = ai->data->game->campaign;
    battle = ai->data->game->battle;
    map = battle->map;

    /* look at all of the squares on the map */
    for (s = 0; s < map->width * map->height; ++s) {

	/* prompt */
	ai->data->prompthook
	    ("Identifying targets",
	     100 * s / (map->width * map->height - 1));

	/* identify terrain and unit */
	terrain = campaign->terrain[map->terrain[s]];
	unit = (map->units[s] == CWG_NO_UNIT)
	    ? NULL
	    : battle->units[map->units[s]];
	utype = unit
	    ? campaign->unittypes[unit->utype]
	    : NULL;

	/* is this an enemy? */
	if (unit && unit->side != battle->side)
	    targets = targetattack (targets, s);

	/* is this a resource square? */
	if (map->terrain[s] == campaign->resource)
	    targets = targetgathering (targets, s);

	/* is this a victory point? */
	if (map->points[s] && ai->data->points)
	    targets = targetpoint (targets, s);

	/* is this a damaged friendly unit? */
	if (unit && unit->side == battle->side &&
	    unit->hits < utype->hits)
	    targets = targetrepair (targets, s);

	/* is this a friendly builder? */
	if (unit && unit->side == battle->side &&
	    unitcats[unit->utype].builder)
	    targets = targetrally (targets, s);

	/* is this a friendly unit that requires defence? */
	if (unit && unit->side == battle->side &&
	    ((! unitcats[unit->utype].combatant &&
	     ! unitcats[unit->utype].mobile) ||
	    map->terrain[s] == campaign->resource ||
	    map->points[s]))
	    targets = targetdefence (targets, s);

    }

    /* sort and return the list of targets */
    targets = sorttargets (targets);
    ai->data->loghook ("Targets identified.");
    return targets;
}

/**
 * Allocate agents to targets
 * @param  target Pointer into a linked list of identified targets
 * @return        A linked list of agents.
 */
static Agent *allocate (Target *target)
{
    Agent *agents = NULL; /* linked list of agents */

    /* find and count the agents */
    agents = getagents ();

    /* loop through targets */
    while (target) {
	ai->data->prompthook ("Issuing orders", 0);
	switch (target->type) {
	case TARGET_ATTACK:
	    target = allocateattacker (target, agents);
	    break;
	case TARGET_GATHERING:
	    target = allocategatherer (target, agents);
	    break;
	case TARGET_OCCUPATION:
	    target = allocateoccupier (target, agents);
	    break;
	case TARGET_REPAIR:
	    target = allocaterepairer (target, agents);
	    break;
	case TARGET_RALLY:
	    target = allocatedamagedunit (target, agents);
	    break;
	case TARGET_DEFENCE:
	    target = allocatedefender (target, agents);
	    break;
	}
    }

    /* return the agent list */
    ai->data->loghook ("Orders issued");
    return agents;
}

/**
 * Take action with agents
 * @param agents A linked list of allocated agents.
 */
static void action (Agent *agents, Target *targets)
{
    Agent **index, /* index of agents */
	*agent; /* pointer to an individual agent */
    Unit *unit, /* the agent unit */
	*tunit; /* the target unit */
    UnitType *utype; /* pointer to agent unit type */
    Battle *battle; /* pointer to the battle */
    Campaign *campaign; /* pointer to the campaign */
    int i, /* agent index counter */
	tunitid, /* target unit ID */
	stay; /* 1 if unit should stay put */

    /* initialise convenience variables */
    campaign = ai->data->game->campaign;
    battle = ai->data->game->battle;
    
    /* index agents and set suitability by proximity to target */
    index = indexagents (agents);
    for (agent = agents; agent; agent = agent->next)
	if (agent->target)
	    agent->suitability
		= proximityfactor (agent->location,
				   ai->data->theircentre);
	else
	    agent->suitability = 0;
    sortagents (agents->count, index);

    /* process agent actions in order of proximity to enemy */
    for (i = 0; agents && i < agents->count; ++i) {

	/* display progress */
	ai->data->prompthook
	    ("Mobilising units",
	     agents->count - 1
	     ? i * 100 / (agents->count - 1)
	     : 100);

	/* try to build unit to act against target */
	if (! index[i]->target)
	    ; /* skip agents without a target */
	else if (index[i]->building)
	    buildaction (index[i]);

	/* act directly against target */
	if (! index[i]->target)
	    ; /* skip agents without a target */
	else if (index[i]->target->type == TARGET_ATTACK)
	    attackaction (index[i]);
	else if (index[i]->target->type == TARGET_GATHERING)
	    gatheringaction (index[i]);
	else if (index[i]->target->type == TARGET_OCCUPATION)
	    occupationaction (index[i]);
	else if (index[i]->target->type == TARGET_REPAIR)
	    repairaction (index[i]);
	else if (index[i]->target->type == TARGET_RALLY)
	    rallyaction (index[i]);
	else if (index[i]->target->type == TARGET_DEFENCE)
	    defenceaction (index[i]);
    }

    /* opportunity actions for agents with movement remaining */
    for (agent = agents; agent; agent = agent->next) {
	if (! (unit = battle->units[agent->unit]))
	    continue;
	if (! unit->moves)
	    continue;
	opportunityrepair (agent, targets);
	opportunityfire (agent);
    }

    /* pursuit for agents that haven't moved at all */
    for (agent = agents; agent; agent = agent->next) {

	/* skip nonexistent or dead agents */
	if (! (unit = battle->units[agent->unit]))
	    continue;
	if (! unit->hits)
	    continue;

	/* skip agents in the middle of doing something else */
	stay = 0;
	utype = campaign->unittypes[unit->utype];
	tunitid = battle->map->units[agent->target->location];
	tunit = (tunitid == CWG_NO_UNIT) ? NULL : battle->units[tunitid];
	if (unit->moves < utype->moves)
	    continue;
	if (agent->target
	    && (((agent->target->type == TARGET_GATHERING
		 || agent->target->type == TARGET_OCCUPATION)
		&& agent->location == agent->target->location)
		|| (agent->target->type == TARGET_RALLY
		    && distance (unit, tunit) == 1)
		|| agent->target->type == TARGET_DEFENCE))
	    stay = 1;

	/* agents guarding resources may build but not pursue */
	opportunitytarget (agent, targets);
	buildaction (agent);
	if (stay ||
	    pointfactor (agent->location) ||
	    resourcefactor (agent->location))
	    continue;
	attackaction (agent);

    }

    /* clean up */
    ai->data->loghook ("Units mobilised");
    free (index);
}

/**
 * Destroy the targets when finished playing a turn.
 * @param targets The linked list of targets to destroy.
 */
static void destroytargets (Target *targets)
{
    Target *target; /* pointer to current target */
    while (targets) {
	target = targets->next;
	free (targets);
	targets = target;
    }
}

/**
 * Destroy the agents when finished playing a turn.
 * @param agents The linked list of agents to destroy.
 */
static void destroyagents (Agent *agents)
{
    Agent *agent; /* pointer to current agent */
    agent = agents;
    while (agents) {
	agent = agents->next;
	free (agents);
	agents = agent;
    }
}

/*----------------------------------------------------------------------
 * Public Method Level Definitions.
 */

/**
 * Destroy AI when no longer needed.
 * @param ai The AI to destroy.
 */
static void destroy (void)
{
    if (ai) {
	if (ai->data)
	    free (ai->data);
	free (ai);
	ai = NULL;
    }
}

/**
 * Play a computer turn.
 */
static void turn (void)
{
    Game *game; /* pointer ot the game */
    Target *targets; /* a linked list of identified targets */
    Agent *agents; /* a linked list of allocated agents */

    /* initialise */
    game = ai->data->game;
    ai->data->ourcentre = game->centre (game, game->battle->side);
    ai->data->theircentre = game->centre (game, ! game->battle->side);
    ai->data->maxpower = maxpower ();
    ai->data->gatherers = countgatherers ();
    ai->data->points = checkpoints ();

    /* set difficulty levels */
    game->battle->setlevel
	(game->battle->side,
	 game->playertypes[game->battle->side] - PLAYER_COMPUTER);
    game->battle->setlevel (1 - game->battle->side, 0);

    /* turn phases */
    targets = gettargets ();
    agents = allocate (targets);
    action (agents, targets);

    /* clean up */
    destroytargets (targets);
    destroyagents (agents);
}

/*----------------------------------------------------------------------
 * Top Level Function Definitions.
 */

/**
 * Construct a new ai object.
 * @param  game The game object.
 * @return      The new ai object.
 */
AI *get_AI (Game *game, PromptHook prompthook, LogHook loghook)
{
    /* if the AI exists, return it */
    if (ai)
	return ai;

    /* otherwise make a new AI */
    if (! (ai = malloc (sizeof (AI))))
	return NULL;
    if (! (ai->data = malloc (sizeof (AIData)))) {
	free (ai);
	return NULL;
    }

    /* initialise methods */
    ai->destroy = destroy;
    ai->turn = turn;

    /* initialise simple attributes */
    ai->data->game = game;
    ai->data->maxdist
	= game->battle->map->width
	+ game->battle->map->height
	- 1;
    ai->data->prompthook = prompthook;
    ai->data->loghook = loghook;

    /* initialise other AI aspects */
    categorise ();

    /* return the AI instance */
    return ai;
}

