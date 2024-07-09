/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker 2021
 * Game Controls Module
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>

/* project headers */
#include "keylib.h"
#include "controls.h"
#include "fatal.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/** @var controls A pointer to the one game controls handler. */
static Controls *controls = NULL;

/** @var keys The keyboard handler. */
static KeyHandler *keys = NULL;

/*----------------------------------------------------------------------
 * Public Method Function Definitions.
 */

/**
 * Destroy the keyboard handler when it is no longer needed.
 */
static void destroy (void)
{
    if (controls) {
	free (controls);
	controls = NULL;
	if (keys)
	    keys->destroy ();
	fatalcontrols (NULL);
    }
}

/**
 * Expose the KeyHandler for other libraries that need it.
 * @return A pointer to the KeyHandler object.
 */
static KeyHandler *getkeyhandler (void)
{
    return keys;
}

/**
 * Key left indicator.
 * @return 1 if the left control is activated, 0 if not.
 */
static int left (void)
{
    return keys->key (KEY_KP4);
}

/**
 * Key right indicator.
 * @return 1 if the right control is activated, 0 if not.
 */
static int right (void)
{
    return keys->key (KEY_KP6);
}

/**
 * Up key indicator.
 * @return 1 if the up control is activated, 0 if not.
 */
static int up (void)
{
    return keys->key (KEY_KP8);
}

/**
 * Down key indicator.
 * @return 1 if the down control is activated, 0 if not.
 */
static int down (void)
{
    return keys->key (KEY_KP2);
}

/**
 * Fire key indicator.
 * @return 1 if the fire control is activated, 0 if not.
 */
static int fire (void)
{
    return keys->key (KEY_CTRL) ||
	keys->key (KEY_ENTER) ||
	keys->key (KEY_SPACE);
}

/**
 * Wait for a key and return its ASCII value.
 * @return The ASCII value of the key.
 */
static int key (void)
{
    keys->wait ();
    return keys->ascii ();
}

/**
 * Wait for a key but leave its value to be checked later.
 */
static void wait (void)
{
    keys->wait ();
}

/**
 * Wait for a key release or repeat delay.
 * @param msecs Number of milliseconds to wait, 0 forever.
 */
static void release (int msecs)
{
    int keydown, /* 1 if a key is down */
	k, /* key scancode counter */
	remaining; /* time remaining */
    struct timeb end, /* time we will finish waiting */
	now; /* time now */

    /* forget about the last key pressed */
    keys->scancode ();

    /* initialise the timer */
    ftime (&end);
    end.millitm += msecs;
    while (end.millitm > 1000) {
	++end.time;
	end.millitm -= 1000;
    }
    
    /* main wait loop */
    do {

	/* check all the keys */
	keydown = 0;
	for (k = 0x01; k <= 0x54; ++k)
	    if (keys->key (k))
		keydown = 1;

	/* check the timer */
	ftime (&now);
	remaining = msecs ?
	    (end.time - now.time) * 1000 + end.millitm - now.millitm :
	    1;

    } while (keydown && remaining > 0);
}

/*----------------------------------------------------------------------
 * Top Level Function Declarations.
 */

/**
 * Construct new game controls handler.
 * @return The new game controls handler.
 */
Controls *new_Controls (void)
{
    /* ensure only one control hander exists */
    if (controls)
	return controls;
    if (! (controls = malloc (sizeof (Controls))))
	return NULL;

    /* initialise methods */
    controls->destroy = destroy;
    controls->getkeyhandler = getkeyhandler;
    controls->left = left;
    controls->right = right;
    controls->up = up;
    controls->down = down;
    controls->fire = fire;
    controls->key = key;
    controls->wait = wait;
    controls->release = release;

    /* initialise the keyhandler */
    keys = new_KeyHandler ();

    /* return the new control handler */
    fatalcontrols (controls);
    return controls;
}
