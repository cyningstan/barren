/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * AI Header.
 */

/* types defined in this header */
typedef struct ai AI;
typedef struct aidata AIData;

#ifndef __AI_H__
#define __AI_H__

/* required headers */
#include "game.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/**
 * @struct ai
 * The structure for the AI module.
 */
struct ai {

    /*
     * Attributes
     */

    /** @var aidata Private data for the AI. */
    AIData *data;

    /*
     * Methods
     */

    /**
     * Destroy AI when no longer needed.
     * @param ai The AI to destroy.
     */
    void (*destroy) (void);

    /**
     * Play a computer turn.
     * @param ai The AI object.
     * @param game The game object.
     */
    void (*turn) (void);

};

/* display hooks */
typedef void (*PromptHook) (char *, int);
typedef void (*LogHook) (char *);

/*----------------------------------------------------------------------
 * Top Level Function Declarations.
 */

/**
 * Construct a new ai object, or return the existing instance.
 * @return The new or existing ai object.
 */
AI *get_AI (Game *game, PromptHook prompthook, LogHook loghook);

#endif
