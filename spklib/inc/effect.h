/*======================================================================
 * SPKLIB
 * A PC Speaker Sound Library
 *
 * Copyright (C) Damian Gareth Walker 2022.
 *
 * Sound Effect Handler Header.
 */

#ifndef __EFFECT_H__
#define __EFFECT_H__

/* required headers */
#include "speaker.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/* effect pattern enumeration */
typedef enum {
    EFFECT_NOISE,
    EFFECT_FALL,
    EFFECT_RISE
} EffectPattern;

/* forward declarations of types. */
typedef struct effect Effect;

/** @struct effect is a piece of music. */
struct effect {

    /** @var pattern The pattern of the sound effect. */
    int pattern;

    /** @var repetitions The number of repetitions. */
    int repetitions;

    /** @var low The lowest frequency. */
    int low;

    /** @var high The highest frequency. */
    int high;

    /** @var duration The number of 1-tick beeps per iteration. */
    int duration;

    /** @var pause Number of ticks delay after each iteration. */
    int pause;

    /**
     * Destroy a effect when no longer needed.
     * @param effect is the effect to destroy.
     */
    void (*destroy) (Effect *effect);

    /**
     * Read the effect in from an already open file.
     * @param effect is the effect to read.
     * @param input is the input file.
     * @return 1 if successful, 0 if failed.
     */
    int (*read) (Effect *effect, FILE *input);

    /**
     * Write the present effect to an already open file.
     * @param effect is the effect to write.
     * @param output is the output file.
     * @return 1 if successful, 0 if failed.
     */
    int (*write) (Effect *effect, FILE *output);

    /**
     * Play the present effect till a key is pressed.
     * If no keyhandler is supplied, the whole effect will play.
     * Playback will stop if a key is pressed or the effect ends.
     * Playback will resume after interruption on the next call.
     * @param effect is the effect to play.
     */
    void (*play) (Effect *effect);

};

/*----------------------------------------------------------------------
 * Top Level Function Declarations.
 */

/**
 * Construct a new effect.
 * @returns the new effect, or NULL if unsuccessful.
 */
Effect *new_Effect (void);

#endif
