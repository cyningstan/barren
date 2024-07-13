/*======================================================================
 * SPKLIB
 * A PC Speaker Sound Library
 *
 * Copyright (C) Damian Gareth Walker 2021.
 *
 * Demonstration Program for sound effects.
 */

/*----------------------------------------------------------------------
 * Included headers.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* project headers */
#include "effect.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/** @var patterns Names of the patterns. */
static char *patterns[] = {
    "Noise",
    "Falling",
    "Rising"
};

/*----------------------------------------------------------------------
 * Level 1 Routines.
 */

/**
 * Display the menu and get an option.
 * @return the option.
 */
static int getmenuoption (Effect *effect)
{
    char textoption[81]; /* text value of option */
    int option; /* option chosen */
    printf ("Pat:%s Rep:%d Lo:%d Hi:%d Dur:%d Pau:%d\n",
	    patterns[effect->pattern],
	    effect->repetitions,
	    effect->low,
	    effect->high,
	    effect->duration,
	    effect->pause);
    printf ("0:Quit 1:Play"
	    " 2:Pattern 3:Repetitions 4:Low"
	    " 5:High 6:Duration 7:Pause\n");
    do {
	printf ("Option: ");
	scanf ("%s", textoption);
	option = atoi (textoption);
    } while (option < 0 || option > 7);
    return option;
}

/**
 * Ask for a new pattern.
 * @param effect The effect to modify.
 * @return The new pattern.
 */
static int getpattern (Effect *effect)
{
    int i, /* iterator */
	pattern; /* pattern input */
    char patterntext[81]; /* text input */

    /* show patterns available */
    for (i = 0; i < 3; ++i)
	printf ("%d:%s ", i, patterns[i]);
    printf ("\n");

    /* ask for a value */
    do {
	printf ("Pattern [%d]: ", effect->pattern);
	scanf ("%s", patterntext);
	pattern = atoi (patterntext);
    } while (pattern < 0 || pattern > 2);

    /* set the value */
    return pattern;
}

/**
 * Ask for a new repetitions.
 * @param effect The effect to modify.
 * @return The new repetitions.
 */
static int getrepetitions (Effect *effect)
{
    int repetitions; /* repetitions input */
    char text[81]; /* text input */

    /* ask for a value */
    do {
	printf ("Repetitions [%d]: ", effect->repetitions);
	scanf ("%s", text);
	repetitions = atoi (text);
    } while (repetitions < 0);

    /* set the value */
    return repetitions;
}

/**
 * Ask for a new low note.
 * @param effect The effect to modify.
 * @return The new low note.
 */
static int getlow (Effect *effect)
{
    int low; /* low input */
    char text[81]; /* text input */

    /* ask for a value */
    do {
	printf ("Low [%d]: ", effect->low);
	scanf ("%s", text);
	low = atoi (text);
    } while (low < 0);

    /* set the value */
    return low;
}

/**
 * Ask for a new high note.
 * @param effect The effect to modify.
 * @return The new high note.
 */
static int gethigh (Effect *effect)
{
    int high; /* high input */
    char text[81]; /* text input */

    /* ask for a value */
    do {
	printf ("High [%d]: ", effect->high);
	scanf ("%s", text);
	high = atoi (text);
    } while (high < 0);

    /* set the value */
    return high;
}

/**
 * Ask for a new duration.
 * @param effect The effect to modify.
 * @return The new duration.
 */
static int getduration (Effect *effect)
{
    int duration; /* duration input */
    char text[81]; /* text input */

    /* ask for a value */
    do {
	printf ("Duration [%d]: ", effect->duration);
	scanf ("%s", text);
	duration = atoi (text);
    } while (duration < 0);

    /* set the value */
    return duration;
}

/**
 * Ask for a new pause.
 * @param effect The effect to modify.
 * @return The new pause.
 */
static int getpause (Effect *effect)
{
    int pause; /* pause input */
    char text[81]; /* text input */

    /* ask for a value */
    do {
	printf ("Pause [%d]: ", effect->pause);
	scanf ("%s", text);
	pause = atoi (text);
    } while (pause < 0);

    /* set the value */
    return pause;
}

/*----------------------------------------------------------------------
 * Top level routines.
 */

/**
 * Main program.
 * @param argc is the argument count.
 * @param argv is the argument array.
 * @return 0 on success, >0 on error.
 */
int main (int argc, char **argv)
{
    Effect *effect; /* sound effect */
    int option; /* menu option */

    /* create sound effect */
    if (! (effect = new_Effect())) {
	printf ("Cannot create sound effect!\n");
	exit (1);
    }

    /* main menu loop */
    do {
	option = getmenuoption (effect);
	switch (option) {
	case 1:
	    effect->play (effect);
	    break;
	case 2:
	    effect->pattern = getpattern (effect);
	    break;
	case 3:
	    effect->repetitions = getrepetitions (effect);
	    break;
	case 4:
	    effect->low = getlow (effect);
	    break;
	case 5:
	    effect->high = gethigh (effect);
	    break;
	case 6:
	    effect->duration = getduration (effect);
	    break;
	case 7:
	    effect->pause = getpause (effect);
	    break;
	}
    } while (option);

    /* clean up */
    effect->destroy (effect);
    return 0;
}
