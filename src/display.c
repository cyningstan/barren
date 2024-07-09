/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 29-Dec-2020.
 *
 * Graphical Display Handler Module.
 */

/*----------------------------------------------------------------------
 * Headers.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* compiler specific headers */
#include <conio.h>
#include <dos.h>

/* project-specific headers */
#include "cgalib.h"
#include "speaker.h"
#include "cwg.h"
#include "barren.h"
#include "fatal.h"
#include "display.h"
#include "controls.h"
#include "game.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/**
 * @struct displaylist
 * A linked list containing coordinates to be updated.
 */
typedef struct displaylist DisplayList;
struct displaylist {
    int x, /* x coordinate to update */
	y, /* y coordinate to update */
	w, /* width to update */
	h; /* height to update */
    DisplayList *next; /* next entry in display list */
};

/**
 * @enum PanelImageIDs The IDs for the panel images.
 */
typedef enum {
    PANEL_UNIT, /* a panel for displaying unit stats */
    PANEL_TERRAIN, /* a panel for displaying terrain stats */
    PANEL_BUILD, /* a panel for selecting units to build */
    PANEL_LAST /* placeholder */
} PanelImageIDs;

/**
 * @enum MenuImageIDs
 * IDs for the menu scroll arrows.
 */
typedef enum {
    MENU_BLANK, /* no arrow */
    MENU_UP, /* up arrow */
    MENU_DOWN /* down arrow */
} MenuImageIDs;

/**
 * @enum BuildImageIDs
 * IDs for the build panel scroll arrows.
 */
typedef enum {
    BUILD_BLANK, /* no arrow */
    BUILD_LEFT, /* left arrow */
    BUILD_RIGHT /* right arrow */
} BuildImageIDs;

/** @var display The display object */
static Display *display = NULL;

/** @var displaylist A list of areas to update. */
DisplayList *displaylist = NULL;

/** @var screen is the CGALIB screen data. */
static Screen *screen;

/** @var controls A pointer to the game control handler. */
static Controls *controls;

/** @var scrbuf The screen buffer. */
static Bitmap *scrbuf;

/** @var title is the title screen image. */
static Bitmap *title;

/** @var panels are the game screen panel images. */
static Bitmap *panels[PANEL_LAST];

/** @var points are the victory point graphics. */
static Bitmap *points[4];

/** @var cursor is the map cursor. */
static Bitmap *cursor;

/** @var cursormask is the map cursor mask. */
static Bitmap *cursormask;

/** @var flash is the unit firing bitmap. */
static Bitmap *flash;

/** @var flashmask is the unit firing mask. */
static Bitmap *flashmask;

/** @var blast is the unit exploding bitmap. */
static Bitmap *blast;

/** @var blastmask is the unit exploding mask. */
static Bitmap *blastmask;

/** @var spanner is the spanner bitmap for building & repair. */
static Bitmap *spanner;

/** @var spannermask is the mask for the spanner bitmap. */
static Bitmap *spannermask;

/** @var menuarrows The scroll arrows for the menu. */
static Bitmap *menuarrows[3];

/** @var buildarrows The scroll arrows for the build panel. */
static Bitmap *buildarrows[3];

/** @var back is the screen background. */
static Bitmap *back;

/** @var mapimg The map image bitmap. */
static Bitmap *mapimg = NULL;

/** @var font is the font in standard colours. */
static Font *font;

/** @var highfont is the font in highlighted colours. */
static Font *highfont;

/** @var lowfont is the font in lowlighted colours. */
static Font *lowfont;

/** @var soundenabled 1 if sound enabled, 0 if not. */
static int soundenabled;

/** @var tune The intro tune. */
static Tune *tune = NULL;

/** @var noises A selection of sound effect noises. */
static Effect *noises[4];

/** @var playerdescs Player type text array. */
static char *playerdescs[] = {
    "Human",
    "Computer (Easy)",
    "Computer (Fair)",
    "Computer (Hard)",
    "PBM"
};

/** @var scrtitle The title of the current screen. */
static char scrtitle[4][13];

/** @var game The game being played. */
static Game *game = NULL;

/*----------------------------------------------------------------------
 * Service Level Private Functions.
 */

/**
 * Add a screen area to the display list (areas to be updated).
 * @param x The x coordinate of the area.
 * @param y The y coordinate of the area.
 * @param w The width of the area.
 * @param h The height of the area.
 */
static void queueupdate (int x, int y, int w, int h)
{
    DisplayList *entry; /* new entry for display list */

    /* reserve memory for new display list entry */
    if (! (entry = malloc (sizeof (DisplayList))))
	fatalerror (FATAL_MEMORY);

    /* fill in the display list entry */
    entry->x = x;
    entry->y = y;
    entry->w = w;
    entry->h = h;

    /* add to the display list */
    entry->next = displaylist;
    displaylist = entry;
}

/*----------------------------------------------------------------------
 * Level 1 Private Function Definitions.
 */

/**
 * Load the graphical assets.
 */
static void loadassets (void)
{
    /* local variables */
    FILE *input; /* input file */
    char header[8]; /* the input file header */
    Bitmap *logo; /* the Cyningstan logo */
    time_t start; /* time the Cyningstan logo was displayed */
    int c; /* counter */

    /* open the input file and check the header */
    if (! (input = fopen ("barren.dat", "rb")))
	fatalerror (FATAL_INVALIDDATA);
    if (! fread (header, 8, 1, input))
	fatalerror (FATAL_INVALIDDATA);
    if (strcmp (header, "BAR100D"))
	fatalerror (FATAL_INVALIDDATA);

    /* load and display the Cyningstan logo */
    start = time (NULL);
    if (! (logo = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    scr_put (screen, logo, 96, 92, DRAW_PSET);
    bit_destroy (logo);

    /* load the screen components */
    if (! (title = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    if (! (panels[PANEL_UNIT] = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    if (! (panels[PANEL_TERRAIN] = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    if (! (panels[PANEL_BUILD] = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    if (! (points[0] = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    if (! (points[1] = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    if (! (points[2] = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    if (! (points[3] = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    if (! (cursor = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    if (! (cursormask = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    if (! (flash = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    if (! (flashmask = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    if (! (blast = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    if (! (blastmask = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    if (! (spanner = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    if (! (spannermask = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    if (! (menuarrows[MENU_BLANK] = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    if (! (menuarrows[MENU_UP] = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    if (! (menuarrows[MENU_DOWN] = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    if (! (buildarrows[BUILD_BLANK] = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    if (! (buildarrows[BUILD_LEFT] = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    if (! (buildarrows[BUILD_RIGHT] = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    if (! (back = bit_read (input)))
	fatalerror (FATAL_INVALIDDATA);

    /* load the font and initialise the screen with it */
    if (! (font = fnt_read (input)))
	fatalerror (FATAL_INVALIDDATA);
    scr_font (screen, font);

    /* create the highlighted and lowlighted fonts */
    highfont = fnt_copy (font);
    fnt_colours (highfont, 3, 2);
    lowfont = fnt_copy (font);
    fnt_colours (lowfont, 1, 0);
    
    /* clear the logo after at least three seconds */
    while (time (NULL) < start + 4);
    scr_ink (screen, 0);
    scr_box (screen, 96, 92, 128, 16);
    sleep (1);

    /* load the music */
    if (! (tune = new_Tune ()))
	fatalerror (FATAL_MEMORY);
    if (! tune->read (tune, input))
	fatalerror (FATAL_INVALIDDATA);

    /* load the sound effects */
    for (c = 0; c <= DISPLAY_COMMS; ++c) {	
	if (! (noises[c] = new_Effect ()))
	    fatalerror (FATAL_MEMORY);
	if (! (noises[c]->read (noises[c], input)))
	    fatalerror (FATAL_INVALIDDATA);
    }

    /* close the input file and return */
    fclose (input);
}

/**
 * Centre a line of text.
 * @var dest The destination string.
 * @var src  The source string.
 * @var len  The length of the string.
 */
static void centreline (char *dest, char *src, int len)
{
    int srclen, /* length of source string */
	padlen; /* length of padding */

    /* return the source string as is if it fills the space */
    if (len == (srclen = strlen (src)))
	strcpy (dest, src);

    /* truncate the source string if it overfills the space */
    else if (len < srclen) {
	strncpy (dest, src, len);
	dest[len] = '\0';
    }

    /* otherwise centre the source string in the space */
    else {
	padlen = (len - strlen (src)) / 2;
	sprintf (dest, "%*s%-*s%*s", padlen, "", len - 2 * padlen, src,
		 padlen + (srclen % 1), "");
    }
}

/**
 * Set the four-line screen title.
 * This occupies the menu window when the menu is not present.
 * @param line1 The first line.
 * @param line2 The second line.
 * @param line3 The third line.
 * @param line4 The fourth line.
 */
static void setscreentitle (char *line1, char *line2, char *line3,
			    char *line4)
{
    centreline (scrtitle[0], line1, 12);
    centreline (scrtitle[1], line2, 12);
    centreline (scrtitle[2], line3, 12);
    centreline (scrtitle[3], line4, 12);
}

/**
 * Show the screen title.
 */
static void showscreentitle (void)
{
    int l; /* line counter */
    bit_ink (scrbuf, 3);
    bit_font (scrbuf, font);
    for (l = 0; l < 4; ++l)
	bit_print (scrbuf, 8, 160 + 8 * l, scrtitle[l]);
    queueupdate (8, 160, 48, 32);
}

/**
 * Display the menu.
 * @param count The number of options in the menu.
 * @param options The menu options.
 * @param option The selected option.
 * @param top The topmost option.
 */
static void displaymenu (int count, char **options, int option, int top)
{
    int c; /* row counter */
    char optstring[13]; /* formatted option string */

    /* display the visible menu options */
    for (c = 0; c < 4; ++c)
	if (top + c < count) {
	    sprintf (optstring, "%-12.12s", options[top + c]);
	    if (top + c == option)
		bit_font (scrbuf, highfont);
	    else
		bit_font (scrbuf, font);
	    bit_ink (scrbuf, 3);
	    bit_print (scrbuf, 8, 160 + 8 * c, optstring);
	} else {
	    bit_ink (scrbuf, 0);
	    bit_box (scrbuf, 8, 160 + 8 * c, 48, 8);
	}

    /* display any scroll arrows */
    bit_put (scrbuf,
	     top == 0
	     ? menuarrows[MENU_BLANK]
	     : menuarrows[MENU_UP],
	     0, 160, DRAW_PSET);
    bit_put (scrbuf,
	     top >= count - 4
	     ? menuarrows[MENU_BLANK]
	     : menuarrows[MENU_DOWN],
	     0, 184, DRAW_PSET);

    /* update the screen */
    scr_putpart (screen, scrbuf, 0, 160, 0, 160, 56, 32, DRAW_PSET);
}

/**
 * Clean up after the menu.
 */
static void cleanupmenu (void)
{
    bit_putpart (scrbuf, back, 0, 160, 0, 160, 8, 32, DRAW_PSET);
    scr_putpart (screen, scrbuf, 0, 160, 0, 160, 8, 32, DRAW_PSET);
}

/**
 * Wrap some text into 32 character lines by replacing spaces with
 * newlines.
 * @param text The text to wrap.
 */
static void wraptext (char *text)
{
    char *cptr, /* pointer to character in token */
	*spc; /* pointer to last space */
    int l; /* current line length */

    /* wrap it */
    spc = NULL;
    l = 0;
    for (cptr = text; *cptr; ++cptr) {

	/* increment line length */
	++l;

	/* perform the wrap if the line is long enough */
	if (l > 32) {
	    if (*cptr == ' ' || ! spc)
		spc = cptr;
	    *spc = '\n';
	    l = cptr - spc;
	    spc = NULL;
	}

	/* check for hard line breaks */
	if (*cptr == '\n') {
	    l = 0;
	    spc = NULL;
	}

	/* check for spaces */
	else if (*cptr == ' ')
	    spc = cptr;

    }
}

/*----------------------------------------------------------------------
 * Public Methods.
 */

/**
 * Destroy the display when no longer needed.
 * @param display is the display to destroy.
 */
static void destroy ()
{
    int c; /* generic counter variable */

    /* clean up the display */
    if (display) {

	/* back to text mode */
	scr_destroy (screen);

	/* destroy any assets that were created */
	if (title)
	    bit_destroy (title);
	if (panels[PANEL_UNIT])
	    bit_destroy (panels[PANEL_UNIT]);
	if (panels[PANEL_TERRAIN])
	    bit_destroy (panels[PANEL_TERRAIN]);
	if (panels[PANEL_BUILD])
	    bit_destroy (panels[PANEL_BUILD]);
	if (points[0])
	    bit_destroy (points[0]);
	if (points[1])
	    bit_destroy (points[1]);
	if (points[2])
	    bit_destroy (points[2]);
	if (points[3])
	    bit_destroy (points[3]);
	if (cursor)
	    bit_destroy (cursor);
	if (cursormask)
	    bit_destroy (cursormask);
	if (back)
	    bit_destroy (back);
	if (font)
	    fnt_destroy (font);
	if (highfont)
	    fnt_destroy (highfont);
	if (lowfont)
	    fnt_destroy (lowfont);
	if (tune)
	    tune->destroy (tune);
	for (c = 0; c < 4; ++c)
	    if (noises[c])
		noises[c]->destroy (noises[c]);

	/* destroy a map, if created */
	if (mapimg)
	    bit_destroy (mapimg);

    	free (display);
    }
}

/**
 * Give display routines access to the game data.
 * @param ingame The game being played.
 */
static void setgame (Game *ingame)
{
    game = ingame;
}

/**
 * Update the screen from the buffer.
 */
static void update (void)
{
    DisplayList *curr, /* current display list entry */
	*next; /* next display list entry */

    /* loop through all display list entries */
    curr = displaylist;
    while (curr) {

	/* update the screen */
	scr_putpart (screen, scrbuf, curr->x, curr->y, curr->x, curr->y,
		     curr->w, curr->h, DRAW_PSET);

	/* look at the next entry */
	next = curr->next;
	free (curr);
	curr = next;
    }

    /* clear the display list */
    displaylist = NULL;
}

/**
 * Display a message in the prompt box.
 * @param message The message to display.
 */
static void prompt (char *message)
{
    bit_ink (scrbuf, 0);
    bit_box (scrbuf, 64, 160, 192, 8);
    bit_ink (scrbuf, 3);
    bit_font (scrbuf, font);
    bit_print (scrbuf, (160 - (strlen (message) / 2 * 4)) & 0x1fc, 160,
	       message);
    scr_putpart (screen, scrbuf, 64, 160, 64, 160, 192, 8, DRAW_PSET);
}

/**
 * Display a menu and get an option from it.
 * @param count   The number of options in the menu.
 * @param options The option names in an array.
 * @param initial The initial option selected.
 * @return        The index of the option selected, or -1.
 */
static int menu (int count, char **options, int initial)
{
    int top, /* index of top row option */
	option, /* option currently selected */
	pressed; /* 1 if fire is pressed initially */

    /* initialise the menu */
    pressed = controls->fire ();
    top = initial > 3 ? initial - 3 : 0;
    option = initial;
    displaymenu (count, options, option, top);

    /* allow option selection till fire pressed/released */
    while (controls->fire () == pressed) {

	/* up key pressed */
	if (controls->up () && option > 0) {
	    --option;
	    if (option < top)
		top = option;
	    displaymenu (count, options, option, top);
	    while (controls->up ());
	}

	/* down key pressed */
	else if (controls->down () && option < count - 1) {
	    ++option;
	    if (option > top + 3)
		top = option - 3;
	    displaymenu (count, options, option, top);
	    while (controls->down ());
	}
    }

    /* clean up and return */
    showscreentitle ();
    cleanupmenu ();
    update ();
    while (controls->fire ());
    return option;
}

/**
 * Display a screenful of text in the right-hand window.
 * @param text is the text to display.
 */
static void fulltext (char *text)
{
    /* local variables */
    int line; /* line we are currently printing */
    char *textcopy, /* copy of the text */
	*linestart; /* start of line */

    /* prepare the text copy */
    if (! (textcopy = malloc (strlen (text) + 1)))
	fatalerror (FATAL_MEMORY);
    strcpy (textcopy, text);

    /* prepare a window */
    bit_ink (scrbuf, 0);
    bit_box (scrbuf, 184, 8, 128, 144);
    bit_ink (scrbuf, 3);
    bit_font (scrbuf, font);

    /* print up to 18 lines of text */
    linestart = strtok (textcopy, "\n");
    for (line = 0; line < 18 && linestart; ++line) {
	bit_print (scrbuf, 184, 8 + 8 * line, linestart);
	linestart = strtok (NULL, "\n");
    }

    /* queue a window update and clean up */
    queueupdate (184, 8, 128, 144);
    free (textcopy);
}

/**
 * Scroll one or more lines of text into the right-hand window.
 * @param text  The text to add.
 * @param lines The number of lines to scroll.
 */
static void linetext (char *text, int lines)
{
    char textcopy[577], /* buffer for wrapped text */
	*linestart; /* pointer to the start of the line */
    int line; /* line counter */

    /* wrap the lines */
    strcpy (textcopy, text);
    wraptext (textcopy);

    /* initialise font */
    bit_font (scrbuf, font);

    /* print the text a line at a time. */
    linestart = strtok (textcopy, "\n");
    for (line = 0; line < lines && linestart; ++line) {
	bit_putpart (scrbuf, scrbuf,
		     184, 152 - 8 * lines,
		     184, 152 - 8 * (lines - 1),
		     128, 8 * (lines - 1),
		     DRAW_PSET);
	bit_ink (scrbuf, 0);
	bit_box (scrbuf, 184, 144, 128, 8);
	bit_ink (scrbuf, 3);
	bit_print (scrbuf, 184, 144, linestart);
	linestart = strtok (NULL, "\n");
    }

    /* schedule a screen update */
    queueupdate (184, 152 - 8 * lines, 128, 8 * lines);
}

/**
 * Show the title screen.
 */
static void showtitlescreen (void)
{
    scr_font (screen, font);
    scr_ink (screen, 3);
    scr_put (screen, title, 0, 0, DRAW_PSET);
    scr_print (screen, 128, 184, " Please wait... ");
}

/**
 * Await the fire key at the title screen.
 */
static void titlekey ()
{
    int key; /* value of key pressed */

    /* show the"Press FIRE" message */
    scr_font (screen, font);
    scr_ink (screen, 3);
    scr_print (screen, 128, 184, "   Press FIRE   ");

    /* play the tune or wait for a key until FIRE is pressed */
    do {
	if (soundenabled) {
	    tune->play (tune, controls->getkeyhandler ());
	    while (! controls->getkeyhandler ()->anykey ());
	} else
	    while (! controls->getkeyhandler ()->anykey ());
	key = controls->getkeyhandler ()->scancode ();
    } while (key != KEY_CTRL && key != KEY_ENTER && key != KEY_SPACE);

    /* erase the "Press FIRE" message */
    scr_ink (screen, 0);
    scr_box (screen, 128, 184, 64, 8);
    while (controls->fire ());
}

/**
 * Make a sound.
 * @param id The ID of the sound.
 */
static void playsound (int id)
{
    if (soundenabled)
	noises[id]->play (noises[id]);
}

/**
 * Update a single square on the display map.
 * @param campaign The campaign containing the bitmaps.
 * @param battle   The battle to display.
 * @param x        The x coordinate of the map square to update.
 * @param y        The y coordinate of the map square to update.
 */
static void updatemap (Campaign *campaign, Battle *battle, int x, int y)
{
    int t, /* terrain type id */
	u, /* unit id */
	ut, /* unit type id */
	s, /* unit side */
	connects; /* connections */
    Map *map; /* convenience pointer to battle map */

    /* ascertain the terrain */
    map = battle->map;
    t = map->terrain[x + map->width * y];

    /* work out connects for connected terrain */
    connects = x + 1 == map->width ||
	map->terrain[x + 1 + map->width * y] == t;
    connects |= 2 *
	(y + 1 == map->height ||
	 map->terrain[x + map->width * (y + 1)]
	 == t);
    connects |= 4 *
	(x == 0 ||
	 map->terrain[x - 1 + map->width * y]
	 == t);
    connects |= 8 *
	(y == 0 ||
	 map->terrain[x + map->width * (y - 1)]
	 == t);

    /* get the terrain tile and put it on the map */
    bit_put (mapimg,
	     campaign->terrainbitmaps[t * 16 + connects],
	     16 * x, 16 * y, DRAW_PSET);

    /* check for a unit */
    s = -1;
    if ((u = map->units[x + map->width * y]) != CWG_NO_UNIT) {

	/* work out unit type and side */
	s = battle->units[u]->side;
	ut = battle->units[u]->utype;

	/* put it on the map */
	bit_put (mapimg,
		 campaign->unitbitmaps[4 * ut + 1 + 2 * s],
		 16 * x, 16 * y, DRAW_AND);
	bit_put (mapimg,
		 campaign->unitbitmaps[4 * ut + 2 * s],
		 16 * x, 16 * y, DRAW_OR);
    }

    /* check for a victory position */
    if (map->points[x + map->width * y]) {
	bit_put (mapimg, points[3], 16 * x, 16 * y, DRAW_AND);
	bit_put (mapimg, points[1 + s], 16 * x, 16 * y,
		 DRAW_OR);
    }
}

/**
 * Prepare a battle map.
 * @param campaign The campaign containing the bitmaps.
 * @param battle   The battle to display.
 */
static void preparemap (Campaign *campaign, Battle *battle)
{
    int x, /* x location counter */
	y; /* y location counter */
    
    /* initialise the map image */
    if (mapimg)
	bit_destroy (mapimg);
    mapimg = bit_create (16 * battle->map->width,
			 16 * battle->map->height);

    /* scan across the whole map generating the terrain */
    for (x = 0; x < battle->map->width; ++x)
	for (y = 0; y < battle->map->height; ++y)
	    updatemap (campaign, battle, x, y);
}

/**
 * Show the battle map.
 * @param x The x coordinate shown at the top left.
 * @param y The y coordinate shown at the top left.
 */
static void showmap (int x, int y)
{
    bit_putpart (scrbuf, mapimg, 8, 8, 16 * x, 16 * y, 144, 144,
		 DRAW_PSET);
    queueupdate (8, 8, 144, 144);
}

/**
 * Show the battle map cursor.
 * @param xcursor The x coordinate of the cursor.
 * @param ycursor The y coordinate of the cursor.
 * @param xview The x coordinate of the map view.
 * @param yview The y coordinate of the map view.
 */
static void showmapcursor (int xcursor, int ycursor, int xview,
			   int yview)
{
    int x, /* scratch pad for x location */
	y; /* scratch pad for y location */

    /* return if the cursor is outside the map view */
    if (xcursor < xview || xcursor > xview + 8 ||
	ycursor < yview || ycursor > yview + 8)
	return;

    /* work out x and y location */
    x = 8 + 16 * (xcursor - xview);
    y = 8 + 16 * (ycursor - yview);

    /* update the off-screen buffer */
    bit_putpart (scrbuf, mapimg, x, y, 16 * xcursor, 16 * ycursor,
		 16, 16, DRAW_PSET);
    bit_put (scrbuf, cursormask, x, y, DRAW_AND);
    bit_put (scrbuf, cursor, x, y, DRAW_OR);
    queueupdate (x, y, 16, 16);
}

/**
 * Hide the battle map cursor.
 * @param xcursor The x coordinate of the cursor.
 * @param ycursor The y coordinate of the cursor.
 * @param xview The x coordinate of the map view.
 * @param yview The y coordinate of the map view.
 */
static void hidemapcursor (int xcursor, int ycursor, int xview,
			   int yview)
{
    int x, /* scratch pad for x location */
	y; /* scratch pad for y location */

    /* return if the cursor is outside the map view */
    if (xcursor < xview || xcursor > xview + 8 ||
	ycursor < yview || ycursor > yview + 8)
	return;

    /* work out x and y location */
    x = 8 + 16 * (xcursor - xview);
    y = 8 + 16 * (ycursor - yview);

    /* update the off-screen buffer */
    bit_putpart (scrbuf, mapimg, x, y, 16 * xcursor, 16 * ycursor,
		 16, 16, DRAW_PSET);
    queueupdate (x, y, 16, 16);
}

/**
 * Show information on empty terrain.
 * @param terrainid The id of the terrain to show.
 * @param unitid    The ID of the last selected friendly unit.
 */
static void showterraininfo (int terrainid, int unitid)
{
    Terrain *terrain; /* convenience pointer to terrain */
    Unit *unit; /* convenience pointer to unit */
    UnitType *utype; /* convenience pointer to unit type */
    int ubid; /* unit bitmap id */
    char nstr[3]; /* number rendered as a string */

    /* initialise convenience variables */
    terrain = game->campaign->terrain[terrainid];

    /* draw the terrain panel */
    bit_put (scrbuf, panels[PANEL_TERRAIN], 184, 8, DRAW_PSET);

    /* icon and name */
    bit_put (scrbuf, game->campaign->terrainbitmaps[16 * terrainid],
	     188, 12, DRAW_PSET);
    bit_ink (scrbuf, 3);
    bit_font (scrbuf, font);
    bit_print (scrbuf, 208, 16, terrain->name);

    /* unit defence and movement */
    if (unitid != CWG_NO_UNIT) {
	unit = game->battle->units[unitid];
	ubid = 4 * unit->utype + 2 * unit->side;
	utype = game->campaign->unittypes[unit->utype];
	bit_put (scrbuf, game->campaign->unitbitmaps[ubid],
		 216, 32, DRAW_PSET);

	/* show defence */
	if (terrain->defence[unit->utype]) {
	    sprintf (nstr, "%02d", terrain->defence[unit->utype]);
	} else
	    strcpy (nstr, "--");
	bit_print (scrbuf, 248, 36, nstr);

	/* show movement cost */
	if (terrain->moves[unit->utype] > utype->moves)
	    strcpy (nstr, "--");
	else if (terrain->moves[unit->utype])
	    sprintf (nstr, "%02d", terrain->moves[unit->utype]);
	else
	    strcpy (nstr, "--");
	bit_print (scrbuf, 272, 36, nstr);
    }

    /* schedule an update */
    queueupdate (184, 8, 128, 48);
}

/**
 * Show information on a friendly unit.
 * @param unit The unit to show.
 */
static void showunitinfo (Unit *unit)
{
    int bitmapid; /* calculated bitmap id */
    UnitType *utype; /* unit type */
    char nstr[3]; /* numeric string */

    /* set convenience variables */
    bitmapid = 4 * unit->utype + 2 * unit->side;
    utype = game->campaign->unittypes[unit->utype];

    /* draw the unit panel */
    bit_put (scrbuf, panels[PANEL_UNIT], 184, 8, DRAW_PSET);

    /* unit icon and name */
    bit_put (scrbuf, game->campaign->unitbitmaps[bitmapid],
	     188, 12, DRAW_PSET);
    bit_ink (scrbuf, 3);
    bit_font (scrbuf, font);
    bit_print (scrbuf, 208, 12, unit->name);
    bit_font (scrbuf, lowfont);
    bit_print (scrbuf, 208, 20, game->campaign->corpnames[unit->side]);

    /* individual unit stats */ 
    bit_font (scrbuf, font);
    sprintf (nstr, "%02d", unit->hits);
    bit_print (scrbuf, 200, 32, nstr);
    sprintf (nstr, "%02d", unit->moves);
    bit_print (scrbuf, 296, 32, nstr);

    /* unit class stats */
    bit_font (scrbuf, lowfont);
    sprintf (nstr, "%02d", utype->hits);
    bit_print (scrbuf, 200, 40, nstr);
    sprintf (nstr, "%02d", utype->power);
    bit_print (scrbuf, 224, 36, nstr);
    sprintf (nstr, "%02d", utype->range);
    bit_print (scrbuf, 248, 36, nstr);
    sprintf (nstr, "%02d", utype->armour);
    bit_print (scrbuf, 272, 36, nstr);
    sprintf (nstr, "%02d", utype->moves);
    bit_print (scrbuf, 296, 40, nstr);

    /* schedule an update */
    queueupdate (184, 8, 128, 48);
}

/**
 * Show information on an enemy unit.
 * @param unit The unit to show.
 */
static void showenemyinfo (Unit *unit)
{
    int bitmapid; /* calculated bitmap id */
    UnitType *utype; /* unit type */
    char nstr[3]; /* numeric string */

    /* set convenience variables */
    bitmapid = 4 * unit->utype + 2 * unit->side;
    utype = game->campaign->unittypes[unit->utype];

    /* draw the unit panel */
    bit_put (scrbuf, panels[PANEL_UNIT], 184, 8, DRAW_PSET);

    /* unit icon and name */
    bit_put (scrbuf, game->campaign->unitbitmaps[bitmapid],
	     188, 12, DRAW_PSET);
    bit_ink (scrbuf, 3);
    bit_font (scrbuf, font);
    bit_print (scrbuf, 208, 12, unit->name);
    bit_font (scrbuf, lowfont);
    bit_print (scrbuf, 208, 20, game->campaign->corpnames[unit->side]);

    /* unit class stats */
    sprintf (nstr, "%02d", utype->hits);
    bit_print (scrbuf, 200, 36, nstr);
    sprintf (nstr, "%02d", utype->power);
    bit_print (scrbuf, 224, 36, nstr);
    sprintf (nstr, "%02d", utype->range);
    bit_print (scrbuf, 248, 36, nstr);
    sprintf (nstr, "%02d", utype->armour);
    bit_print (scrbuf, 272, 36, nstr);
    sprintf (nstr, "%02d", utype->moves);
    bit_print (scrbuf, 296, 36, nstr);

    /* schedule an update */
    queueupdate (184, 8, 128, 48);
}

/**
 * Show the map square information panel.
 * @param x The x coordinate to show.
 * @param y The y coordinate to show.
 * @param u The ID of the last selected unit.
 */
static void showmapinfo (int x, int y, int u)
{
    Map *map; /* convenience pointer to the map */
    int loc; /* coordinates as a single location number */

    /* set convenience variable(s) */
    map = game->battle->map;
    loc = x + map->width * y;

    /* show unit or terrain as appropriate */
    if (map->units[loc] == CWG_NO_UNIT)
	showterraininfo (map->terrain[loc], u);
    else if (game->battle->units[map->units[loc]]->side
	     == game->battle->side)
	showunitinfo (game->battle->units[map->units[loc]]);
    else
	showenemyinfo (game->battle->units[map->units[loc]]);
}

/**
 * Display the main menu window.
 * @param gamename The game name to display.
 * @param campaignname The campaign name to display.
 * @param playertypes An array of playertypes to display.
 */
static void shownewgame (char *gamename, char *campaignname,
			 char corpnames[2][9], int *playertypes)
{
    int l; /* display line counter */

    /* prepare the background */
    bit_put (scrbuf, back, 0, 0, DRAW_PSET);

    /* show the window title */
    bit_font (scrbuf, lowfont);
    bit_print (scrbuf, 16, 16, "NEW GAME");
    setscreentitle ("", "SET UP", "GAME", "");
    showscreentitle ();

    /* show the current options */
    for (l = 0; l < 4; ++l)
    	display->showgameoption (gamename, campaignname, corpnames,
				 playertypes, l, 0);

    /* queue a full screen update */
    queueupdate (0, 0, 320, 200);
}

/**
 * Show a single line of the new game screen.
 * @param gamename     The game name to display.
 * @param campaignname The campaign name to display.
 * @param playertypes  An array of playertypes to display.
 * @param line         The line to show.
 * @param highlight    1 if line should be highlighted.
 */
static void showgameoption (char *gamename, char *campaignname,
			    char corpnames[2][9], int *playertypes,
			    int line, int highlight)
{
    char text[33]; /* formatted text line */

    /* choose the correct font */
    if (highlight > 0)
	bit_font (scrbuf, highfont);
    else if (highlight < 0)
	bit_font (scrbuf, lowfont);
    else
	bit_font (scrbuf, font);

    /* format the line text */
    switch (line) {
    case 0:
	sprintf (text, "Game:     %-22.22s", gamename);
	break;
    case 1: /* campaign name */
	sprintf (text, "Campaign: %-22.22s", campaignname);
	break;
    case 2: case 3: /* player type */
	sprintf (text, "%8s: %-22.22s", corpnames[line - 2],
		 playerdescs[playertypes[line - 2]]);
	break;
    }

    /* output the line text */
    bit_ink (scrbuf, 3);
    bit_print (scrbuf, 16, 48 + 24 * line, text);
    queueupdate (16, 48 + 24 * line, 128, 8);
}

/**
 * Show the briefing screen.
 * @param text The briefing text.
 * @param side The side to brief.
 * @param x    The location shown in the map window - x coordinate.
 * @param y    The location shown in the map window - y coordinate.
 */
static void showbriefing (char *text, int side, int x, int y)
{
    /* prepare the background */
    bit_put (scrbuf, back, 0, 0, DRAW_PSET);
    setscreentitle ("", "MISSION", "BRIEFING", "");
    showscreentitle ();

    /* show the briefing text */
    fulltext (text);

    /* show the map */
    showmap (x, y);

    /* show the player icon and name */
    bit_put (scrbuf, game->campaign->corpbitmaps[side], 280, 160,
	     DRAW_PSET);
    bit_font (scrbuf, font);
    bit_print (scrbuf, 280, 184, game->campaign->corpnames[side]);

    /* queue a full screen update */
    queueupdate (0, 0, 320, 200);
}

/**
 * Show the Start Human Turn Screen (for 2-player games).
 * @param side The side who is playing.
 */
static void showstartturn (int side)
{
    /* set font and colour */
    bit_ink (scrbuf, 3);
    bit_font (scrbuf, font);

    /* prepare the background */
    bit_put (scrbuf, back, 0, 0, DRAW_PSET);

    /* show the player icon and name in the map window */
    bit_put (scrbuf, game->campaign->corpbitmaps[side], 64, 64,
	     DRAW_PSET);
    bit_print (scrbuf, 64, 88, game->campaign->corpnames[side]);

    /* show the same in the dialogue window */
    bit_put (scrbuf, game->campaign->corpbitmaps[side], 232, 64,
	     DRAW_PSET);
    bit_print (scrbuf, 232, 88, game->campaign->corpnames[side]);

    /* show the same in the usual player icon panel */
    bit_put (scrbuf, game->campaign->corpbitmaps[game->battle->side],
	     280, 160, DRAW_PSET);
    bit_print (scrbuf, 280, 184,
	       game->campaign->corpnames[game->battle->side]);

    /* set and show the screen title */
    setscreentitle ("", "", "", "");
    showscreentitle ();

    /* queue a screen update */
    queueupdate (0, 0, 320, 200);
}

/**
 * Show the Human Turn Screen.
 * @param xcursor The x position of the map cursor.
 * @param ycursor The y position of the map cursor.
 * @param xview   The x position of the map view (top left).
 * @param yview   The y position of the map view (top left).
 */
static void showhumanturn (int xcursor, int ycursor, int xview,
			   int yview)
{
    /* prepare the background */
    bit_put (scrbuf, back, 0, 0, DRAW_PSET);

    /* set and show the screen title */
    setscreentitle ("", "PLAYER", "TURN", "");
    showscreentitle ();

    /* show the map and cursor */
    showmap (xview, yview);
    showmapcursor (xcursor, ycursor, xview, yview);

    /* initialise the unit/terrain info */
    showmapinfo (xcursor, ycursor, CWG_NO_UNIT);

    /* show the player icon and name */
    bit_put (scrbuf, game->campaign->corpbitmaps[game->battle->side],
	     280, 160, DRAW_PSET);
    bit_font (scrbuf, font);
    bit_ink (scrbuf, 3);
    bit_print (scrbuf, 280, 184,
	       game->campaign->corpnames[game->battle->side]);

    /* queue a full screen update */
    queueupdate (0, 0, 320, 200);
}

/**
 * Show a unit firing.
 * @param xunit The x location of the unit.
 * @param yunit The y location of the unit.
 * @param xview The x location of the top left of the map view.
 * @param yview The y location of the top left of the map view.
 */
static void showunitfire (int xunit, int yunit, int xview, int yview)
{
    /* don't show anything if the unit is off screen */
    if (xunit < xview || xunit > 8 + xview ||
	yunit < yview || yunit > 8 + yview)
	return;

    /* show the unit fire */
    bit_put (scrbuf, flashmask, 8 + 16 * (xunit - xview),
	     8 + 16 * (yunit - yview), DRAW_AND);
    bit_put (scrbuf, flash, 8 + 16 * (xunit - xview),
	     8 + 16 * (yunit - yview), DRAW_OR);

    /* schedule an update */
    queueupdate (8 + 16 * (xunit - xview), 8 + 16 * (yunit - yview),
		 16, 16);
}

/**
 * Show a unit exploding.
 * @param xunit The x location of the unit.
 * @param yunit The y location of the unit.
 * @param xview The x location of the top left of the map view.
 * @param yview The y location of the top left of the map view.
 */
static void showunitblast (int xunit, int yunit, int xview, int yview)
{
    /* don't show anything if the unit is off screen */
    if (xunit < xview || xunit > 8 + xview ||
	yunit < yview || yunit > 8 + yview)
	return;

    /* show the blast */
    bit_put (scrbuf, blastmask, 8 + 16 * (xunit - xview),
	     8 + 16 * (yunit - yview), DRAW_AND);
    bit_put (scrbuf, blast, 8 + 16 * (xunit - xview),
	     8 + 16 * (yunit - yview), DRAW_OR);

    /* schedule an update */
    queueupdate (8 + 16 * (xunit - xview), 8 + 16 * (yunit - yview),
		 16, 16);
}

/**
 * Show a unit being built or repaired.
 * @param xunit the X location of the unit.
 * @param yunit The y location of the unit.
 * @param xview The x location of the top left of the map view.
 * @param yview The y location of the top left of the map view.
 */
static void showspanner (int xunit, int yunit, int xview, int yview)
{
    /* don't show anything if the unit is off screen */
    if (xunit < xview || xunit > 8 + xview ||
	yunit < yview || yunit > 8 + yview)
	return;

    /* show the buid/repair icon */
    bit_put (scrbuf, spannermask, 8 + 16 * (xunit - xview),
	     8 + 16 * (yunit - yview), DRAW_AND);
    bit_put (scrbuf, spanner, 8 + 16 * (xunit - xview),
	     8 + 16 * (yunit - yview), DRAW_OR);

    /* schedule an update */
    queueupdate (8 + 16 * (xunit - xview), 8 + 16 * (yunit - yview),
		 16, 16);
}

/**
 * Show the unit types interface.
 * @param bcount The number of types that can be built.
 * @param btypes An array of types that can be built by ID.
 * @param first  The first type displayed.
 */
static void showunittypes (int bcount, int *btypes, int first)
{
    int b; /* build type count */

    /* draw the unit type panel */
    bit_put (scrbuf, panels[PANEL_BUILD], 184, 8, DRAW_PSET);
    for (b = 0; b < 5; ++b)
	display->hideunittypecursor (bcount, btypes, first, b + first);

    /* display any scroll arrows */
    bit_put (scrbuf,
	     first == 0
	     ? buildarrows[BUILD_BLANK]
	     : buildarrows[BUILD_LEFT],
	     192, 48, DRAW_PSET);
    bit_put (scrbuf,
	     first >= bcount - 5
	     ? buildarrows[BUILD_BLANK]
	     : buildarrows[BUILD_RIGHT],
	     288, 48, DRAW_PSET);

    /* schedule an update */
    queueupdate (184, 8, 128, 48);
}

/**
 * Show a cursor on the unit types interface.
 * @param bcount  The number of types that can be built.
 * @param first   The first type displayed.
 * @param current The current type highlighted.
 */
static void showunittypecursor (int bcount, int *btypes, int first, int current)
{
    int x; /* x location of cursor */
    UnitType *utype; /* unit type */

    /* blank out the unit name */
    bit_ink (scrbuf, 0);
    bit_box (scrbuf, 208, 12, 96, 8);

    /* calculate X coordinate */
    x = 184 + 8 + 24 * (current - first);

    /* display the cursor */
    bit_put (scrbuf, cursormask, x, 24, DRAW_AND);
    bit_put (scrbuf, cursor, x, 24, DRAW_OR);

    /* display the unit type name */
    if (current < bcount) {
	utype = game->campaign->unittypes[btypes[current]];
	bit_font (scrbuf, font);
	bit_ink (scrbuf, 3);
	bit_print (scrbuf, 208, 12, utype->name);
    }

    /* screen updates */
    queueupdate (208, 12, 96, 8);
    queueupdate (x, 24, 16, 16);
}

/**
 * Hide the cursor on the unit types interface.
 * @param bcount  The number of types that can be built.
 * @param btypes  An array of types that can be built by ID.
 * @param first   The first type displayed.
 * @param current The current type highlighted.
 */
static void hideunittypecursor (int bcount, int *btypes, int first,
				int current)
{
    int x, /* x coordinate of unit type */
	bitmapid; /* bitmap ID of unit type */
    UnitType *utype; /* unit type */
    char nstr[3]; /* buffer for cost */

    /* calculate x coordinate of current unit */
    x = 184 + 8 + 24 * (current - first);

    /* blank out the space if appropriate */
    if (current >= bcount) {
	bit_ink (scrbuf, 0);
	bit_box (scrbuf, x, 24, 16, 24);
    }

    /* otherwise show the unit type and cost */
    else {
	bitmapid = 4 * btypes[current] + 2 * game->battle->side;
	utype = game->campaign->unittypes[btypes[current]];
	bit_put (scrbuf, game->campaign->unitbitmaps[bitmapid],
		 x, 24, DRAW_PSET);
	bit_font (scrbuf, font);
	bit_ink (scrbuf, 3);
	sprintf (nstr, "%02d", utype->cost);
	bit_print (scrbuf, x + 4, 40, nstr);
    }

    /* schedule an update */
    queueupdate (x, 24, 16, 24);
}

/**
 * Update the unit types interface on scrolling.
 * @param bcount The number of types that can be built.
 * @param btypes An array of types taht can be built by ID.
 * @param first  The first type displayed.
 */
static void updateunittypes (int bcount, int *btypes, int first)
{
    int b; /* build type counter */

    /* refresh the unit types */
    for (b = 0; b < 5; ++b)
	display->hideunittypecursor (bcount, btypes, first, b + first);

    /* show/hide any scroll arrows */
    bit_put (scrbuf,
	     first == 0
	     ? buildarrows[BUILD_BLANK]
	     : buildarrows[BUILD_LEFT],
	     192, 48, DRAW_PSET);
    bit_put (scrbuf,
	     first >= bcount - 5
	     ? buildarrows[BUILD_BLANK]
	     : buildarrows[BUILD_RIGHT],
	     288, 48, DRAW_PSET);

    /* queue the screen updates */
    queueupdate (192, 48, 16, 8);
    queueupdate (288, 48, 16, 8);
}

/**
 * Show the current resources.
 * @param resources The resource figure.
 */
static void showresources (int resources)
{
    char num[5]; /* resources as a string */
    if (resources < 100)
	sprintf (num, " %02d ", resources);
    else
	sprintf (num, "%03d ", resources);
    bit_font (scrbuf, font);
    bit_ink (scrbuf, 3);
    bit_print (scrbuf, 160, 84, num);
    queueupdate (160, 84, 16, 8);
}

/**
 * Show the Turn Report Screen.
 * @param xview   The x position of the map view (top left).
 * @param yview   The y position of the map view (top left).
 */
static void showturnreport (int xview, int yview)
{
    /* prepare the background */
    bit_put (scrbuf, back, 0, 0, DRAW_PSET);

    /* set and show the screen title */
    setscreentitle ("", "TURN", "REPORT", "");
    showscreentitle ();

    /* show the map */
    showmap (xview, yview);

    /* show the player icon and name */
    bit_put (scrbuf, game->campaign->corpbitmaps[game->battle->side],
	     280, 160, DRAW_PSET);
    bit_font (scrbuf, font);
    bit_ink (scrbuf, 3);
    bit_print (scrbuf, 280, 184,
	       game->campaign->corpnames[game->battle->side]);

    /* queue a full screen update */
    queueupdate (0, 0, 320, 200);
}

/**
 * Show the debriefing screen.
 * @param text The debriefing text.
 * @param side The side to brief.
 * @param x    The location shown in the map window - x coordinate.
 * @param y    The location shown in the map window - y coordinate.
 */
static void showdebriefing (char *text, int side, int x, int y)
{
    /* prepare the background */
    bit_put (scrbuf, back, 0, 0, DRAW_PSET);
    setscreentitle ("", "MISSION", "DEBRIEFING", "");
    showscreentitle ();

    /* show the briefing text */
    fulltext (text);

    /* show the map */
    showmap (x, y);

    /* show the player icon and name */
    bit_put (scrbuf, game->campaign->corpbitmaps[side], 280, 160,
	     DRAW_PSET);
    bit_font (scrbuf, font);
    bit_print (scrbuf, 280, 184, game->campaign->corpnames[side]);

    /* queue a full screen update */
    queueupdate (0, 0, 320, 200);
}

/**
 * Show the Computer Turn screen.
 * @param side The side to brief.
 * @param x    The location shown in the map window - x coordinate.
 * @param y    The location shown in the map window - y coordinate.
 */
static void showcomputerturn (int side, int x, int y)
{
    /* set font and colour */
    bit_ink (scrbuf, 3);
    bit_font (scrbuf, font);

    /* prepare the background */
    bit_put (scrbuf, back, 0, 0, DRAW_PSET);

    /* show the map */
    showmap (x, y);

    /* show the player icon in the player icon panel */
    bit_put (scrbuf, game->campaign->corpbitmaps[game->battle->side],
	     280, 160, DRAW_PSET);
    bit_print (scrbuf, 280, 184,
	       game->campaign->corpnames[game->battle->side]);

    /* set and show the screen title */
    setscreentitle ("", "COMPUTER", "TURN", "");
    showscreentitle ();

    /* queue a screen update */
    queueupdate (0, 0, 320, 200);
}

/**
 * Show the PBM Turn screen.
 */
static void showpbmturn (int side, int x, int y)
{
    /* set font and colour */
    bit_ink (scrbuf, 3);
    bit_font (scrbuf, font);

    /* prepare the background */
    bit_put (scrbuf, back, 0, 0, DRAW_PSET);

    /* show the map */
    showmap (x, y);

    /* show the player icon in the player icon panel */
    bit_put (scrbuf, game->campaign->corpbitmaps[game->battle->side],
	     280, 160, DRAW_PSET);
    bit_print (scrbuf, 280, 184,
	       game->campaign->corpnames[game->battle->side]);

    /* set and show the screen title */
    setscreentitle ("", "REMOTE", "TURN", "");
    showscreentitle ();

    /* queue a screen update */
    queueupdate (0, 0, 320, 200);
}
    
/**
 * Show the end game screen.
 * @param victor The winner of the game.
 * @param x      The location shown in the map window - x coordinate
 * @param y      The location shown in the map window - y coordinate
 */
static void showendgame (int victor, int x, int y)
{
    /* prepare the background */
    bit_put (scrbuf, back, 0, 0, DRAW_PSET);
    setscreentitle ("", "GAME", "OVER", "");
    showscreentitle ();

    /* show the victor in the dialogue window */
    bit_ink (scrbuf, 3);
    bit_font (scrbuf, font);
    bit_print (scrbuf, 224, 48, "Game won by:");
    bit_put (scrbuf, game->campaign->corpbitmaps[victor], 232, 64,
	     DRAW_PSET);
    bit_print (scrbuf, 232, 88, game->campaign->corpnames[victor]);

    /* show the map */
    showmap (x, y);

    /* show the player icon and name */
    bit_put (scrbuf, game->campaign->corpbitmaps[victor], 280, 160,
	     DRAW_PSET);
    bit_font (scrbuf, font);
    bit_print (scrbuf, 280, 184, game->campaign->corpnames[victor]);

    /* queue a full screen update */
    queueupdate (0, 0, 320, 200);
}

/*----------------------------------------------------------------------
 * Constructor Functions.
 */

/**
 * Display constructor.
 * @param colourset = 0 for mono, 1 for colour, 2 for nice colour.
 * @param quiet = 0 for sound and music, 1 for silence.
 * @return the new display.
 */
Display *new_Display (int colourset, int quiet)
{
    /* local variables */
    int mode; /* the screen mode */

    /* allocate memory */
    if (display)
	return display;
    if (! (display = malloc (sizeof (Display))))
	return NULL;

    /* initialise methods */
    display->destroy = destroy;
    display->setgame = setgame;
    display->update = update;
    display->prompt = prompt;
    display->menu = menu;
    display->fulltext = fulltext;
    display->linetext = linetext;
    display->showtitlescreen = showtitlescreen;
    display->titlekey = titlekey;
    display->playsound = playsound;
    display->updatemap = updatemap;
    display->preparemap = preparemap;
    display->showmap = showmap;
    display->showmapcursor = showmapcursor;
    display->hidemapcursor = hidemapcursor;
    display->showterraininfo = showterraininfo;
    display->showunitinfo = showunitinfo;
    display->showenemyinfo = showenemyinfo;
    display->showmapinfo = showmapinfo;
    display->shownewgame = shownewgame;
    display->showgameoption = showgameoption;
    display->showbriefing = showbriefing;
    display->showstartturn = showstartturn;
    display->showhumanturn = showhumanturn;
    display->showunitfire = showunitfire;
    display->showunitblast = showunitblast;
    display->showspanner = showspanner;
    display->showunittypes = showunittypes;
    display->showunittypecursor = showunittypecursor;
    display->hideunittypecursor = hideunittypecursor;
    display->updateunittypes = updateunittypes;
    display->showresources = showresources;
    display->showturnreport = showturnreport;
    display->showdebriefing = showdebriefing;
    display->showcomputerturn = showcomputerturn;
    display->showpbmturn = showpbmturn;
    display->showendgame = showendgame;

    /* initialise the game controls handler */
    controls = getcontrols ();

    /* store sound settings */
    soundenabled = ! quiet;

    /* prepare to initialise the CGALIB screen */
    if (colourset == 0)
	mode = 6;
    else if (colourset == 1)
	mode = 4;
    else if (colourset == 2)
	mode = 5;

    /* initialise the CGALIB screen */
    if (! (screen = scr_create (mode))) {
	free (display);
	return NULL;
    }
    fataldisplay (display);
    if (colourset == 2)
	scr_palette (screen, 5, 8);

    /* create the off-screen buffer */
    scrbuf = bit_create (320, 200);

    /* initialise the assets */
    loadassets ();

    /* initialise the screen title */
    setscreentitle ("", "", "", "");

    /* return the screen */
    return display;
}
