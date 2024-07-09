/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * Created: 01-Jan-2021.
 *
 * Scenario Handler Header.
 */

/* typedefs needed */
typedef struct scenario Scenario;

#ifndef __SCENARIO_H__
#define __SCENARIO_H__

/* header dependencies */
#include "cwg.h"
#include "campaign.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/**
 * @enum scenario_text_type is the slot for scenario text.
 */
typedef enum scenario_text_type {
    SCENARIO_BRIEFING_1, /* player 1 briefing */
    SCENARIO_BRIEFING_2, /* player 2 briefing */
    SCENARIO_VICTORY_1, /* player 1 victory debriefing */
    SCENARIO_VICTORY_2, /* player 2 victory debriefing */
    SCENARIO_DEFEAT_1, /* player 1 defeat debriefing */
    SCENARIO_DEFEAT_2 /* player 2 defeat debriefing */
} ScenarioTextType;

/**
 * @struct scenario is a scenario in a campaign.
 */
struct scenario {

    /** @var campaign is the campaign with unit/terrain types. */
    Campaign *campaign;

    /** @var battle is the battle on which the scenario is based. */
    Battle *battle;

    /** @var text is the brief/debrief text for the scenario. */
    char text[6][577];

    /**
     * @var next is an array of IDs of the next scenarios.
     * Element 0 is the ID of the next scenario if player 1 wins.
     * Element 1 is the ID of the next scenario if player 2 wins.
     * A value of 0 for either means the campaign ends there.
     */
    int next[2];

    /**
     * Destroy scenario when it is no longer needed.
     * @param scenario is the scenario to destroy.
     */
    void (*destroy) (Scenario *scenario);

    /**
     * Write scenario details to an already open file.
     * @param scenario is the scenario to save.
     * @param output is the output file.
     * @return 1 for success, 0 for failure.
     */
    int (*write) (Scenario *scenario, FILE *output);

    /**
     * Read scenario details from an already open file.
     * @param scenario is the scenario to load.
     * @param input is the input file.
     * @return 1 for success, 0 for failure.
     */
    int (*read) (Scenario *scenario, FILE *input);

};

/*----------------------------------------------------------------------
 * Constructor Prototypes.
 */

/**
 * Construct a scenario.
 * @return the new scenario.
 */
Scenario *new_Scenario (Campaign *campaign);

#endif
