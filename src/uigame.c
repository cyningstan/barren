/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * New Game Screen Module.
 */

/*----------------------------------------------------------------------
 * Headers
 */

/* ANSI C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <time.h>
#include <io.h>

/* project-specific headers */
#include "barren.h"
#include "fatal.h"
#include "uiscreen.h"
#include "display.h"
#include "controls.h"
#include "config.h"
#include "game.h"
#include "turn.h"
#include "campaign.h"
#include "scenario.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/**
 * @struct uiscreendata
 * Private data for this UI Screen.
 */
struct uiscreendata {

    /** @var game The game to set up. */
    Game *game;

    /** @var gameindex The index of the game file in the list. */
    int gameindex;

    /** @var campaignindex The index of the campaign in the list. */
    int campaignindex;

    /** @var highlight The highlighted setting. */
    int highlight;

};

/**
 * @struct campaign_ref is a reference to an unloaded campaign.
 */
typedef struct campaign_ref CampaignRef;
struct campaign_ref {

    /** @var filename The campaign filename. */
    char filename[13];

    /** @var name The campaign display name. */
    char name[33];

    /** @var singleplayer Single player setting. */
    int singleplayer;

    /** @var corpnames The corporation names. */
    char corpnames[2][9];

};

/**
 * @struct game_ref A reference to an unloaded game.
 */
typedef struct game_ref GameRef;
struct game_ref {

    /** @var filename The game filename */
    char filename[13];

    /** @var name The game display name */
    char name[33];

    /** @var campaignindex Which campaign the game plays. */
    int campaignindex;

    /** @var playertypes The player types in the game. */
    int playertypes[2];

};

/** @var display A pointer to the display module. */
static Display *display;

/** @var controls A pointer to the game controls module. */
static Controls *controls;

/** @var config A pointer to the configuration. */
static Config *config;

/** @var gamerefs References to in-progress games found on disk. */
static GameRef gamerefs[16];

/** @var campaignrefs References to campaigns found on disk. */
static CampaignRef campaignrefs[16];

/** @var newgamemenu The menu for a new game. */
static char *newgamemenu[] = {
    "Cancel menu",
    "Start game",
    "Delete game",
    "Exit game"
};

/** @var instructions The instruction text. */
static char *instructions =
    "Set up and start a new game, or load an old one. Use left and "
    "right controls to change the currently higlighted setting. Use up "
    "and down controls to access the other settings on the screen. "
    "Hold the FIRE control to access the menu.";

/** @var playerdescs Player type text array. */
static char *playerdescs[] = {
    "Human",
    "Computer (Easy)",
    "Computer (Fair)",
    "Computer (Hard)",
    "PBM"
};

/*----------------------------------------------------------------------
 * Level 2 Private Function Definitions.
 */

/**
 * Identify a campaign file in the list by its filename.
 * @param  filename The filename of the campaign.
 * @return          Its index in the list, or -1 if not found.
 */
static int identifycampaign (char *filename)
{
    int c = 0; /* campaign index counter */
    for (c = 0; c < 16; ++c)
	if (! strcmp (campaignrefs[c].filename, filename))
	    return c;
    return -1;
}

/**
 * Adjust the game.
 * @param direction The direction to move along the list.
 */
static void adjustgame (UIScreen *uiscreen, int direction)
{
    char text[128]; /* a line of text */
    int c; /* field counter */
    
    /* adjust the game entry itself */
    uiscreen->data->gameindex += direction;
    if (uiscreen->data->gameindex >= 16 ||
	uiscreen->data->gameindex < 0 ||
	gamerefs[uiscreen->data->gameindex].name[0] == 0) {
	uiscreen->data->gameindex -= direction;
	return;
    }

    /* show a report message */
    if (uiscreen->data->gameindex)
	sprintf (text, "Selecting game %s.",
		 gamerefs[uiscreen->data->gameindex].name);
    else
	strcpy (text, "Selecting a new game.");
    display->linetext (text, 18);

    /* update the other fields internally */
    uiscreen->data->campaignindex =
	gamerefs[uiscreen->data->gameindex].campaignindex;

    /* update the other fields on the screen */
    for (c = 1; c < 4; ++c)
	display->showgameoption
	    (gamerefs[uiscreen->data->gameindex].name,
	     campaignrefs[uiscreen->data->campaignindex].name,
	     campaignrefs[uiscreen->data->campaignindex].corpnames,
	     gamerefs[uiscreen->data->gameindex].playertypes,
	     c,
	     -(uiscreen->data->gameindex != 0));
}

/**
 * Adjust the campaign.
 * @param direction is the direction on the list.
 */
static void adjustcampaign (UIScreen *uiscreen, int direction)
{
    char text[128]; /* a line of text */
    int c, /* field counter */
	sp, /* single-player setting */
	*playertypes; /* pointer to player types array */

    /* adjust the campaign name itself */
    uiscreen->data->campaignindex += direction;
    if (uiscreen->data->campaignindex >= 16 ||
	uiscreen->data->campaignindex < 0 ||
	campaignrefs[uiscreen->data->campaignindex].filename[0] == 0) {
	uiscreen->data->campaignindex -= direction;
	return;
    }
    gamerefs[uiscreen->data->gameindex].campaignindex
	= uiscreen->data->campaignindex;

    /* show a report message */
    sprintf (text, "Selecting campaign %s.",
	     campaignrefs[uiscreen->data->campaignindex].name);
    display->linetext (text, 18);

    /* adjust the players if necessary */
    sp = campaignrefs[uiscreen->data->campaignindex].singleplayer;
    if (sp) {
	playertypes = gamerefs[uiscreen->data->gameindex].playertypes;
	playertypes[sp - 1] = PLAYER_HUMAN;
	playertypes[2 - sp] = PLAYER_COMPUTER;
    }

    /* adjust the corporation names */
    for (c = 2; c < 4; ++c)
	display->showgameoption
	    (gamerefs[uiscreen->data->gameindex].name,
	     campaignrefs[uiscreen->data->campaignindex].name,
	     campaignrefs[uiscreen->data->campaignindex].corpnames,
	     gamerefs[uiscreen->data->gameindex].playertypes,
	     c,
	     -(uiscreen->data->gameindex != 0));

    /* adjust the saved config setting */
    strcpy (config->campaignfile,
	    campaignrefs[uiscreen->data->campaignindex].filename);
}

/**
 * Adjust a player control.
 * @param direction is the direction to move along the list.
 * @param player is the player number.
 */
static void adjustplayer (UIScreen *uiscreen, int direction)
{
    char text[128]; /* a line of text */
    int player, /* player number selected */
	*playertypes, /* pointer to player types for game */
	sp, /* copy of single-player setting */
	lowplayer, /* lowest player option allowed */
	highplayer; /* highest player option allowed */

    /* initialise convenience variables */
    playertypes = gamerefs[uiscreen->data->gameindex].playertypes;
    player = uiscreen->data->highlight - 2;

    /* work out lowest and highest player types allowed */
    sp = campaignrefs[uiscreen->data->campaignindex].singleplayer;
    if (sp) {
	lowplayer = (sp == player + 1) ? PLAYER_HUMAN : PLAYER_COMPUTER;
	highplayer = (sp == player + 1) ? PLAYER_HUMAN : PLAYER_HARD;
    } else {
	lowplayer = PLAYER_HUMAN;
	highplayer = PLAYER_PBM;
    }

    /* cycles through the player types */
    if (playertypes[player] <= lowplayer &&
	direction == -1)
	playertypes[player] = highplayer;
    else if (playertypes[player] >= highplayer &&
	     direction == 1)
	playertypes[player] = lowplayer;
    else
	playertypes[player] += direction;

    /* ensure that at least one player is human */
    if (playertypes[player] != PLAYER_HUMAN &&
	playertypes[1 - player] != PLAYER_HUMAN) {
	playertypes[1 - player] = PLAYER_HUMAN;
	sprintf
	    (text, "Setting %s to %s control.",
	     campaignrefs[uiscreen->data->campaignindex]
	     .corpnames[1 - player],
	     playerdescs[playertypes[1 - player]]);
	display->linetext (text, 18);
    }

    /* show a report message */
    sprintf
	(text, "Setting %s to %s control.",
	 campaignrefs[uiscreen->data->campaignindex].corpnames[player],
	 playerdescs[playertypes[player]]);
    display->linetext (text, 18);

}

/*----------------------------------------------------------------------
 * Level 1 Private Function Definitions.
 */

/**
 * Initialise the list of campaigns available.
 */
static void initialisecampaignfiles (void)
{
    DIR *dir; /* directory handle */
    struct dirent *direntry; /* a directory entry */
    char *ext; /* pointer to filename extension */
    int c = 0; /* campaign counter */
    Campaign *campaign; /* temporary campaign variable */

    /* Initialise the temporary campaign variable */
    if (! (campaign = new_Campaign ()))
	fatalerror (FATAL_MEMORY);

    /* attempt to open the directory */
    if (! (dir = opendir (".")))
	fatalerror (FATAL_INVALIDDATA);

    /* read each entry and see if it is a campaign */
    while (c < 16 && (direntry = readdir (dir))) {

	/* skip past files that are not *.CAM files */
	if (! (ext = strchr (direntry->d_name, '.')))
	    continue;
	if (strcmp (ext, ".cam") && strcmp (ext, ".CAM"))
	    continue;

	/* attempt to load the campaign and add it to the list */
	strcpy (campaign->filename, direntry->d_name);
	if (! campaign->load (campaign, 1))
	    continue;
	strcpy (campaignrefs[c].filename, campaign->filename);
	strcpy (campaignrefs[c].name, campaign->name);
	campaignrefs[c].singleplayer = campaign->singleplayer;
	strcpy (campaignrefs[c].corpnames[0], campaign->corpnames[0]);
	strcpy (campaignrefs[c].corpnames[1], campaign->corpnames[1]);
	++c;
    }

    /* close the directory when done */
    closedir (dir);
    campaign->destroy (campaign);
}

/**
 * Process any inbound turn files for new games.
 */
static void processturnfiles (void)
{
    DIR *dir; /* directory handle */
    struct dirent *direntry; /* a directory entry */
    char *ext; /* pointer to filename extension */
    Game *game; /* temporary game buffer */
    Turn *turn; /* temporary turn buffer */

    /* Initialise the temporary variables */
    if (! (game = new_Game ()))
	fatalerror (FATAL_MEMORY);
    if (! (turn = new_Turn ()))
	fatalerror (FATAL_MEMORY);

    /* attempt to open the directory */
    if (! (dir = opendir (".")))
	fatalerror (FATAL_INVALIDDATA);

    /* read each entry and see if it is a game */
    while ((direntry = readdir (dir))) {

	/* skip past files that are not *.TRN files */
	if (! (ext = strchr (direntry->d_name, '.')))
	    continue;
	if (strcmp (ext, ".trn") && strcmp (ext, ".TRN"))
	    continue;

	/* skip past files for which a *.GAM file exists */
	strcpy (game->filename, direntry->d_name);
	ext = strchr (game->filename, '.');
	strcpy (ext, ".gam");
	if (game->load (game, 1))
	    continue;

	/* skip past invalid turn files */
	strcpy (turn->filename, direntry->d_name);
	if (! turn->load (turn, 0))
	    continue; /* invalid/not a turn file */
	if (turn->scenid != 0)
	    continue; /* turn file for a game in progress */
	if (turn->turnno != 0)
	    continue; /* turn file for a game in progress */

	/* create a game file */
	if (turn->turntogame (turn, game)) {
	    game->save (game);
	    unlink (turn->filename);
	}
    }

    /* close the directory when done */
    game->destroy (game);
    turn->destroy (turn);
    closedir (dir);
}

/**
 * Initialise the list if in-progress games.
 */
static void initialisegamefiles (void)
{
    DIR *dir; /* directory handle */
    struct dirent *direntry; /* a directory entry */
    char *ext, /* pointer to filename extension */
	*ptr; /* pointer that strtol needs */
    Game *game; /* temporary game buffer */
    int g = 0; /* game counter */
    time_t t; /* time in seconds since epoch */

    /* the first entry is always "New Game" */
    *gamerefs[g].filename = '\0';
    strcpy (gamerefs[g].name, "New game");
    gamerefs[g].campaignindex = 0;
    gamerefs[g].playertypes[0] = config->playertypes[0];
    gamerefs[g].playertypes[1] = config->playertypes[1];
    ++g;

    /* Initialise the temporary game variable */
    if (! (game = new_Game ()))
	fatalerror (FATAL_MEMORY);

    /* attempt to open the directory */
    if (! (dir = opendir (".")))
	fatalerror (FATAL_INVALIDDATA);

    /* read each entry and see if it is a campaign */
    while (g < 16 && (direntry = readdir (dir))) {

	/* skip past files that are not *.CAM files */
	if (! (ext = strchr (direntry->d_name, '.')))
	    continue;
	if (strcmp (ext, ".gam") && strcmp (ext, ".GAM"))
	    continue;

	/* add the game to the list */
	strcpy (game->filename, direntry->d_name);
	if (! game->load (game, 1))
	    continue; /* invalid/not a game file */
	strcpy (gamerefs[g].filename, direntry->d_name);
	t = strtol (direntry->d_name, &ptr, 16);
	strftime (gamerefs[g].name, 32, "%Y-%m-%d %H:%M:%S",
		  localtime(&t));
	gamerefs[g].campaignindex =
	    identifycampaign (game->campaignfile);
	if (gamerefs[g].campaignindex == -1)
	    continue; /* invalid/deleted campaign file */
	gamerefs[g].playertypes[0] = game->playertypes[0];
	gamerefs[g].playertypes[1] = game->playertypes[1];
	++g;
    }

    /* close the directory when done */
    game->destroy (game);
    closedir (dir);
}

/**
 * Get the game options until FIRE is pressed.
 */
static void getgameoptions (UIScreen *uiscreen)
{
    int step; /* direction of change: left -1, right +1 */
    char message[49]; /* prompt message */

    /* main game option loop */
    do {

	/* display an appropriate prompt */
	switch (uiscreen->data->highlight) {
	case 0: /* Game */
	    display->prompt
		("New game or saved game?");
	    break;
	case 1: /* Campaign */
	    display->prompt
		("Which campaign do you want to play?");
	    break;
	case 2: case 3: /* Player control */
	    sprintf (message, "Who controls the %s forces?",
		     campaignrefs[uiscreen->data->campaignindex]
		     .corpnames[uiscreen->data->highlight - 2]);
	    display->prompt (message);
	    break;
	}

	/* highlight option and await control press */
	display->showgameoption
	    (gamerefs[uiscreen->data->gameindex].name,
	     campaignrefs[uiscreen->data->campaignindex].name,
	     campaignrefs[uiscreen->data->campaignindex].corpnames,
	     gamerefs[uiscreen->data->gameindex].playertypes,
	     uiscreen->data->highlight,
	     1);
	if (uiscreen->data->highlight == 2 ||
	    uiscreen->data->highlight == 3)
	    display->showgameoption
		(gamerefs[uiscreen->data->gameindex].name,
		 campaignrefs[uiscreen->data->campaignindex].name,
		 campaignrefs[uiscreen->data->campaignindex].corpnames,
		 gamerefs[uiscreen->data->gameindex].playertypes,
		 5 - uiscreen->data->highlight,
		 -(uiscreen->data->gameindex != 0));
	display->update ();
	controls->release (0);
	controls->wait ();
	display->showgameoption
	    (gamerefs[uiscreen->data->gameindex].name,
	     campaignrefs[uiscreen->data->campaignindex].name,
	     campaignrefs[uiscreen->data->campaignindex].corpnames,
	     gamerefs[uiscreen->data->gameindex].playertypes,
	     uiscreen->data->highlight,
	     -(uiscreen->data->gameindex != 0));

	/* up/down controls */
	if (controls->up () && uiscreen->data->highlight > 0)
	    --uiscreen->data->highlight;
	else if (controls->down () &&
		 uiscreen->data->highlight < 3 &&
		 uiscreen->data->gameindex == 0)
	    ++uiscreen->data->highlight;

	/* left/right controls */
	else if ((step = controls->right () - controls->left ())) {
	    switch (uiscreen->data->highlight) {
	    case 0: /* game */
		adjustgame (uiscreen, step);
		break;
	    case 1: /* campaign */
		adjustcampaign (uiscreen, step);
		break;
	    case 2: case 3: /* player 1 or 2 */
		adjustplayer (uiscreen, step);
		break;
	    }
	}

    } while (! controls->fire ());
    display->prompt ("");

    /* do a final screen update, as there may be a delay now */
    display->update ();

    /* save "New Game" settings to configuration */
    strcpy (config->campaignfile,
	    campaignrefs[gamerefs[0].campaignindex].filename);
    config->playertypes[0] = gamerefs[0].playertypes[0];
    config->playertypes[1] = gamerefs[0].playertypes[1];
}

/**
 * Load in a chosen game and initialise it.
 * @param loadgame (UIScreen *uiscreen)
 */
static int loadgame (UIScreen *uiscreen)
{
    Game *game; /* pointer to the game */

    /* display a "Please wait..." prompt */
    display->prompt ("Please wait...");

    /* load the game */
    game = uiscreen->data->game;
    strcpy (game->filename,
	    gamerefs[uiscreen->data->gameindex].filename);
    game->load (game, 0);
    strcpy (config->gamefile, game->filename);
    display->setgame (game);

    /* return the initial game state */
    return game->state;
}

/**
 * Prepare a new game when options have been chosen.
 * @param uiscreen A pointer to the screen data.
 */
static int preparegame (UIScreen *uiscreen)
{
    /* local variables */
    Game *game; /* pointer to the game */
    Campaign *campaign; /* the campaign */

    /* display a "Please wait..." prompt */
    display->prompt ("Please wait...");
    
    /* initialise the basic game data */
    game = uiscreen->data->game;
    game->clear (game);
    sprintf (game->filename, "%08lx.gam", time (NULL));
    strcpy (game->campaignfile,
	    campaignrefs[uiscreen->data->campaignindex].filename);

    /* initialise the campaign and first scenario */
    if (game->campaign)
	game->campaign->clear (game->campaign);
    else if (! (game->campaign = new_Campaign ()))
	fatalerror (FATAL_MEMORY);
    campaign = game->campaign;
    strcpy (campaign->filename, game->campaignfile);
    if (! campaign->load (campaign, 0))
	fatalerror (FATAL_INVALIDDATA);
    game->battle = NULL;
    game->setscenario (game);

    /* save the game information in the configuration */
    strcpy (config->campaignfile, game->campaignfile);
    strcpy (config->gamefile, game->filename);

    /* give the display access to the game data */
    display->setgame (game);

    /* return the initial game state */
    return game->state;
}

/**
 * Delete an old game.
 * @param screen The user interface screen and its data.
 */
static void deletegame (UIScreen *uiscreen)
{
    int c; /* display line counter */
    char text[128]; /* "Game deleted" message */

    /* remove the game file */
    unlink (gamerefs[uiscreen->data->gameindex].filename);
    sprintf (text, "Game %s deleted.",
	     gamerefs[uiscreen->data->gameindex].name);

    /* make sure the game won't be re-saved on exit */
    if (! stricmp (gamerefs[uiscreen->data->gameindex].filename,
		  uiscreen->data->game->filename))
	*uiscreen->data->game->filename = '\0';

    /* remove the game from the game refs */
    for (c = uiscreen->data->gameindex; c < 15; ++c)
	gamerefs[c] = gamerefs[c + 1];
    *gamerefs[15].filename = '\0';
    *gamerefs[15].name = '\0';
    gamerefs[15].campaignindex = 0;
    gamerefs[15].playertypes[0] = config->playertypes[0];
    gamerefs[15].playertypes[1] = config->playertypes[1];

    /* ensure we're not pointing at an empty game */
    if (! *gamerefs[uiscreen->data->gameindex].filename &&
	uiscreen->data->gameindex > 0)
	--uiscreen->data->gameindex;

    /* update the display */
    for (c = 0; c < 4; ++c)
	display->showgameoption
	    (gamerefs[uiscreen->data->gameindex].name,
	     campaignrefs[uiscreen->data->campaignindex].name,
	     campaignrefs[uiscreen->data->campaignindex].corpnames,
	     gamerefs[uiscreen->data->gameindex].playertypes,
	     c, 0);
    display->linetext (text, 18);
    display->update ();
}

/*----------------------------------------------------------------------
 * Public Method Function Definitions.
 */

/**
 * Destroy the current UI screen when no longer needed.
 * @param uiscreen The screen to destroy.
 */
static void destroy (UIScreen *uiscreen)
{
    if (uiscreen) {
	if (uiscreen->data)
	    free (uiscreen->data);
	free (uiscreen);
    }
}

/**
 * Initialise the UI state when first encountered.
 * @param uiscreen The screen to affect.
 */
static void init (UIScreen *uiscreen)
{
    int c; /* campaign counter */

    /* save the current game */
    if (*uiscreen->data->game->filename)
	uiscreen->data->game->save (uiscreen->data->game);

    /* initialise the campaign and game lists */
    initialisecampaignfiles ();
    processturnfiles ();
    initialisegamefiles ();

    /* initialise the list indexes */
    for (c = 0; c < 16; ++c)
	if (! strcmp (config->campaignfile, campaignrefs[c].filename))
	    uiscreen->data->campaignindex
		= gamerefs[0].campaignindex
		= c;
    uiscreen->data->gameindex = 0;
}

/**
 * Show the UI screen.
 * @param uiscreen The screen to show.
 * @return         The ID of the screen to show next.
 */
static UIState show (UIScreen *uiscreen)
{
    int option; /* option chosen from the menu */

    /* initialise the display */
    display->shownewgame
	(gamerefs[uiscreen->data->gameindex].name,
	 campaignrefs[uiscreen->data->campaignindex].name,
	 campaignrefs[uiscreen->data->campaignindex].corpnames,
	 config->playertypes);
    display->linetext (instructions, 18);
    display->linetext (" ", 18);
    display->update ();

    /* main loop */
    while (1) {

	/* allow navigation of the game options */
	getgameoptions (uiscreen);

	/* get a choice from the menu */
	option = display->menu (4, newgamemenu, 1);
	switch (option) {

	case 0: /* cancel menu */
	    break;

	case 1: /* start game */
	    if (uiscreen->data->gameindex)
		return loadgame (uiscreen);
	    else
		return preparegame (uiscreen);

	case 2: /* delete game */
	    if (uiscreen->data->gameindex &&
		uiscreen->confirm ("Really delete this game?"))
		deletegame (uiscreen);
	    break;

	case 3: /* exit game */
	    strcpy (config->gamefile,
		    gamerefs[uiscreen->data->gameindex].filename);
	    return STATE_QUIT;
	}
    }
}

/*----------------------------------------------------------------------
 * Top Level Function Definitions.
 */

/**
 * Construct the New Game Screen.
 * @param  game The game object.
 * @return      The new New Game Screen.
 */
UIScreen *new_NewGameScreen (Game *game)
{
    UIScreen *uiscreen; /* the screen to return */

    /* reserve memory for the screen and its data */
    uiscreen = new_UIScreen ();
    if (! (uiscreen->data = malloc (sizeof (UIScreenData)))) {
	uiscreen->destroy (uiscreen);
	return NULL;
    }

    /* initialise methods */
    uiscreen->destroy = destroy;
    uiscreen->init = init;
    uiscreen->show = show;

    /* get pointers to necessary modules */
    display = getdisplay ();
    controls = getcontrols ();
    config = getconfig ();

    /* initialise attributes */
    uiscreen->data->game = game;
    uiscreen->data->gameindex = 0;
    uiscreen->data->campaignindex = 0;
    uiscreen->data->highlight = 0;

    /* return the screen */
    return uiscreen;
}
