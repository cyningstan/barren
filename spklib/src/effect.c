/*======================================================================
 * SPKLIB
 * A PC Speaker Sound Library
 *
 * Copyright (C) Damian Gareth Walker 2022.
 *
 * Sound Effect Handler Module.
 */

/*----------------------------------------------------------------------
 * Included Headers.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* compiler-specific headers */
#include <dos.h>
#include <sys/timeb.h>

/* project headers */
#include "effect.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/* constants */
#define EFFECT_TICK (1000.0 / 18.2)

/** @var speaker A pointer to the Speaker object. */
static Speaker *speaker;

/*----------------------------------------------------------------------
 * Level 1 Private Methods.
 */

/**
 * Noise effect: random tones.
 * @param effect The effect parameters.
 */
static void noiseeffect (Effect *effect)
{
    int d, /* duration counter */
	pitch; /* pitch of sound to play */
    for (d = 0; d < effect->duration; ++d) {
	if (speaker->frequencies[effect->high] ==
	    speaker->frequencies[effect->low])
	    pitch = speaker->frequencies[effect->low];
	else
	    pitch = speaker->frequencies[effect->low]
		+ rand ()
		% (speaker->frequencies[effect->high]
		   - speaker->frequencies[effect->low]);
	sound (pitch);
	delay (EFFECT_TICK);
    }
    nosound ();
}

/**
 * Falling tone effect: also good for pew-pew noises.
 * @param effect The effect parameters.
 */
static void falleffect (Effect *effect)
{
    int d, /* duration counter */
	pitch, /* pitch of sound to play */
	steps; /* number of steps */
    steps = (effect->duration < 2) ? 1 : effect->duration - 1;
    for (d = 0; d < effect->duration; ++d) {
	pitch = speaker->frequencies[effect->high]
	    - (speaker->frequencies[effect->high]
	       - speaker->frequencies[effect->low])
	    * ((float) d / steps);
	sound (pitch);
	delay (EFFECT_TICK);
    }
    nosound ();
}

/**
 * Rising tone effect: maybe for jumping?
 * @param effect The effect parameters.
 */
static void riseeffect (Effect *effect)
{
    int d, /* duration counter */
	pitch, /* pitch of sound to play */
	steps; /* number of steps */
    steps = (effect->duration < 2) ? 1 : effect->duration - 1;
    for (d = 0; d < effect->duration; ++d) {
	pitch = speaker->frequencies[effect->low]
	    + (speaker->frequencies[effect->high]
	       - speaker->frequencies[effect->low])
	    * ((float) d / steps);
	sound (pitch);
	delay (EFFECT_TICK);
    }
    nosound ();
}

/*----------------------------------------------------------------------
 * Public Method Functions.
 */

/**
 * Destroy a effect when no longer needed.
 * @param effect is the effect to destroy.
 */
static void destroy (Effect *effect)
{
    if (effect) {
	free (effect);
    }
}

/**
 * Read the effect in from an already open file.
 * @param effect is the effect to read.
 * @param input is the input file.
 * @return 1 if successful, 0 if failed.
 */
static int read (Effect *effect, FILE *input)
{
    return speaker->readint (&effect->pattern, input) &&
	speaker->readint (&effect->repetitions, input) &&
	speaker->readint (&effect->low, input) &&
	speaker->readint (&effect->high, input) &&
	speaker->readint (&effect->duration, input) &&
	speaker->readint (&effect->pause, input);
}

/**
 * Write the present effect to an already open file.
 * @param effect is the effect to write.
 * @param output is the output file.
 * @return 1 if successful, 0 if failed.
 */
static int write (Effect *effect, FILE *output)
{
    return speaker->writeint (&effect->pattern, output) &&
	speaker->writeint (&effect->repetitions, output) &&
	speaker->writeint (&effect->low, output) &&
	speaker->writeint (&effect->high, output) &&
	speaker->writeint (&effect->duration, output) &&
	speaker->writeint (&effect->pause, output);
}

/**
 * Play the present effect till a key is pressed.
 * If no keyhandler is supplied, the whole effect will play.
 * Playback will stop if a key is pressed or the effect ends.
 * Playback will resume after interruption on the next call.
 * @param effect is the effect to play.
 * @param keys is a keyboard handler or NULL.
 */
static void play (Effect *effect)
{
    int i; /* iteration counter */
    for (i = 0; i < effect->repetitions; ++i) {
	switch (effect->pattern) {
	case EFFECT_NOISE:
	    noiseeffect (effect);
	    break;
	case EFFECT_FALL:
	    falleffect (effect);
	    break;
	case EFFECT_RISE:
	    riseeffect (effect);
	    break;
	}
	delay (effect->pause * EFFECT_TICK);
    }
}

/*----------------------------------------------------------------------
 * Top Level Functions.
 */

/**
 * Construct a new effect.
 * @returns the new effect, or NULL if unsuccessful.
 */
Effect *new_Effect (void)
{
    Effect *effect; /* the effect to return */

    /* attempt to reserve memory */
    if (! (effect = malloc (sizeof (Effect))))
	return NULL;

    /* get the speaker object */
    speaker = get_Speaker ();

    /* initialise methods */
    effect->destroy = destroy;
    effect->read = read;
    effect->write = write;
    effect->play = play;

    /* initialise attributes */
    effect->pattern = EFFECT_NOISE;
    effect->repetitions = 0;
    effect->low = 0;
    effect->high = 0;
    effect->duration = 0;
    effect->pause = 0;

    /* return the new effect */
    return effect;
}
