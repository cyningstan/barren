/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2022.
 * Turn report module.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* project-specific headers */
#include "cwg.h"
#include "barren.h"
#include "fatal.h"
#include "report.h"

/*----------------------------------------------------------------------
 * Public Method Function Definitions.
 */

/**
 * Destroy a report when it is no longer needed.
 * @param report The report to destroy.
 */
static void destroy (Report *report)
{
    if (report) {
	if (report->battle)
	    report->battle->destroy (report->battle);
	if (report->count)
	    free (report->entries);
	free (report);
    }
}

/**
 * Create a clone of this report.
 * @param report The report to clone.
 * @result       The cloned report.
 */
static Report *clone (Report *report)
{
    Report *newreport; /* newly cloned report */
    int size; /* size of report array in bytes */

    /* reserve memory for the report */
    if (! (newreport = new_Report ()))
	return NULL;

    /* copy the attributes to the new report */
    newreport->count = report->count;
    newreport->battle = report->battle ?
	report->battle->clone (report->battle) :
	NULL;
    size = report->count * sizeof (long int);
    newreport->entries = malloc (size);
    memcpy (newreport->entries, report->entries, size);

    /* return the new report */
    return newreport;
}

/**
 * Write a report to an open file.
 * @param  report The report to write.
 * @param  output The output file handle.
 * @return        1 if successful, 0 on failure.
 */
static int write (Report *report, FILE *output)
{
    int c, /* entry counter */
	zero = 0; /* constant zero for writing */

    /* if the report is empty, write a 0 action count and be done */
    if (! report->battle || ! report->count) {
	fwrite(&zero, 2, 1, output);
	return 1;
    }

    /* write action count, battle state and actions */
    if (! fwrite (&report->count, 2, 1, output))
	return 0;
    if (! report->battle->write (report->battle, output))
	return 0;
    for (c = 0; c < report->count; ++c)
	if (! fwrite (&report->entries[c], 4, 1, output))
	    return 0;

    /* all done */
    return 1;
}

/**
 * Read a report from an open file.
 * @param  report The report to read.
 * @param  input  The input file handle.
 * @return        1 if successful, 0 on failure.
 */
static int read (Report *report, FILE *input)
{
    int c; /* entry counter */

    /* clear any current report data */
    report->clear (report);

    /* read the report count word */
    if (! fread (&report->count, 2, 1, input))
	return 0;

    /* return now if there are zero entries */
    if (! report->count)
	return 1;

    /* read the initial battle state */
    if (! (report->battle = new_Battle (NULL, NULL)))
	fatalerror (FATAL_MEMORY);
    if (! report->battle->read (report->battle, input))
	return 0;

    /* read the report entries */
    if (! (report->entries = malloc (report->count *
				     sizeof (long int))))
	fatalerror (FATAL_MEMORY);
    for (c = 0; c < report->count; ++c)
	if (! fread (&report->entries[c], 4, 1, input))
	    return 0;

    /* return success */
    return 1;
}

/**
 * Clear the entries from a report.
 * @param report The report to clear.
 */
static void clear (Report *report)
{
    if (report->entries) {
	free (report->entries);
	report->entries = NULL;
    }
    if (report->battle) {
	report->battle->destroy (report->battle);
	report->battle = NULL;
    }
    report->count = 0;
}

/**
 * Add an action to the report.
 * @param  report The report to add to.
 * @param  action The action type and result.
 * @param  utype  The unit type that performed the action.
 * @param  ttype  The target type (for attacks only).
 * @param  origin The origin position.
 * @param  target The target position.
 * @return        1 on success, 0 on failure.
 */
static int add (Report *report, int action, int utype, int ttype,
	    int origin, int target)
{
    /* make room for new entry */
    if (report->count)
	report->entries = realloc (report->entries,
				   4 * (report->count + 1));
    else
	report->entries = malloc (4);
    if (! report->entries)
	fatalerror (FATAL_MEMORY);
    ++report->count;

    /* store entry */
    report->entries[report->count - 1] =
	action +
	(utype << 3) +
	(ttype << 6) +
	(((long int) origin) << 9) +
	(((long int) target) << 17);

    /* return success code */
    return 1;
}

/**
 * Extract the action from a report.
 * @param  report The report to extract from.
 * @param  index  The index of the report entry.
 (report, report->count - 1) * @return        The action code.
 */
static int action (Report *report, int index)
{
    if (index >= report->count)
	return ACTION_NONE;
    return report->entries[index] & 0x7;
}

/**
 * Extract the active unit type from a report.
 * @param  report The report to extract from.
 * @param  index  The index of the report entry.
 * @return        The active unit type.
 */
static int utype (Report *report, int index)
{
    return (report->entries[index] >> 3) & 0x7;
}

/**
 * Extract the target unit type from a report.
 * @param  report The report to extract from.
 * @param  index  The index of the report entry.
 * @return        The target unit type.
 */
static int ttype (Report *report, int index)
{
    return (report->entries[index] >> 6) & 0x7;
}

/**
 * Extract the origin position from a report.
 * @param  report The report to extract from.
 * @param  index  The index of the report entry.
 * @return        The origin position.
 */
static int origin (Report *report, int index)
{
    return (report->entries[index] >> 9) & 0xff;
}

/**
 * Extract the target position from a report.
 * @param  report The report to extract from.
 * @param  index  The index of the report entry.
 * @return        The origin position.
 */
static int target (Report *report, int index)
{
    return (report->entries[index] >> 17) & 0xff;
}

/*----------------------------------------------------------------------
 * Constructor Definitions.
 */

/**
 * Campaign constructor.
 * @return the new campaign.
 */
Report *new_Report (void)
{
    Report *report; /* report to return */

    /* reserve memory for the report */
    if (! (report = malloc (sizeof (Report))))
	return NULL;

    /* initialise methods */
    report->destroy = destroy;
    report->clone = clone;
    report->write = write;
    report->read = read;
    report->clear = clear;
    report->add = add;
    report->action = action;
    report->utype = utype;
    report->ttype = ttype;
    report->origin = origin;
    report->target = target;

    /* initialise attributes */
    report->count = 0;
    report->battle = NULL;
    report->entries = NULL;

    /* return the new report */
    return report;
}
