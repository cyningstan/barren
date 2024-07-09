/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2022.
 * Turn report header.
 */

/* types defined in this file */
typedef struct report Report;

#ifndef __REPORT_H__
#define __REPORT_H__

/* header dependencies */
#include "cwg.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/**
 * @enum ReportAction
 * A list of actions for turn report entries.
 */
typedef enum {
    ACTION_NONE, /* no action (invalid entry) */
    ACTION_MOVE, /* move a unit */
    ACTION_ATTACK, /* attack a target */
    ACTION_BATTLE, /* attack a target that returns fire */
    ACTION_DESTROY, /* attack a target and destroy it */
    ACTION_ATTACK_FAIL, /* attack a unit and be destroyed */
    ACTION_BUILD, /* build a unit */
    ACTION_REPAIR /* repair a unit */
} ReportAction;

/**
 * @struct report
 * The turn report structure.
 */
struct report {

    /** @var count is the number of actions. */
    int count;

    /** @var battle The initial battle state. */
    Battle *battle;

    /** @var entries is the list of entries in the report. */
    long int *entries;

    /**
     * Destroy a report when it is no longer needed.
     * @param report The report to destroy.
     */
    void (*destroy) (Report *report);

    /**
     * Create a clone of this report.
     * @param report The report to clone.
     * @result       The cloned report.
     */
    Report *(*clone) (Report *report);

    /**
     * Write a report to an open file.
     * @param  report The report to write.
     * @param  output The output file handle.
     * @return        1 if successful, 0 on failure.
     */
    int (*write) (Report *report, FILE *output);

    /**
     * Read a report from an open file.
     * @param  report The report to read.
     * @param  input  The input file handle.
     * @return        1 if successful, 0 on failure.
     */
    int (*read) (Report *report, FILE *input);

    /**
     * Clear the entries from a report.
     * @param report The report to clear.
     */
    void (*clear) (Report *report);

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
    int (*add) (Report *report, int action, int utype, int ttype,
		int origin, int target);

    /**
     * Extract the action from a report.
     * @param  report The report to extract from.
     * @param  index  The index of the report entry.
     * @return        The action code.
     */
    int (*action) (Report *report, int index);

    /**
     * Extract the active unit type from a report.
     * @param  report The report to extract from.
     * @param  index  The index of the report entry.
     * @return        The active unit type.
     */
    int (*utype) (Report *report, int index);

    /**
     * Extract the target unit type from a report.
     * @param  report The report to extract from.
     * @param  index  The index of the report entry.
     * @return        The target unit type.
     */
    int (*ttype) (Report *report, int index);

    /**
     * Extract the origin position from a report.
     * @param  report The report to extract from.
     * @param  index  The index of the report entry.
     * @return        The origin position.
     */
    int (*origin) (Report *report, int index);

    /**
     * Extract the target position from a report.
     * @param  report The report to extract from.
     * @param  index  The index of the report entry.
     * @return        The origin position.
     */
    int (*target) (Report *report, int index);

};

/*----------------------------------------------------------------------
 * Constructor Declarations.
 */

/**
 * Report constructor.
 * @return the new campaign.
 */
Report *new_Report (void);

void debug_Report (Report *report);
#endif
