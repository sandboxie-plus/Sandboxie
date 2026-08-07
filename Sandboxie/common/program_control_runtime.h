/*
 * Copyright 2026 David Xanatos, xanasoft.com
 *
 * This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

//---------------------------------------------------------------------------
// Shared Force/Breakout runtime API for compiled rulesets.
// Keep this header C/C++ compatible and backend-light for consumers.
//---------------------------------------------------------------------------

#ifndef _PROGRAM_CONTROL_RUNTIME_H
#define _PROGRAM_CONTROL_RUNTIME_H

#ifndef PROGRAM_CONTROL_RULE_NO_QUERY_HELPERS
#define PROGRAM_CONTROL_RUNTIME_TEMP_NO_QUERY_HELPERS
#define PROGRAM_CONTROL_RULE_NO_QUERY_HELPERS
#endif

#if defined(KERNEL_MODE) && !defined(PROGRAM_CONTROL_RULE_NO_CRT)
#define PROGRAM_CONTROL_RUNTIME_TEMP_NO_CRT
#define PROGRAM_CONTROL_RULE_NO_CRT
#endif

#include "program_control_rule.h"
#include "pool.h"

#ifdef PROGRAM_CONTROL_RUNTIME_TEMP_NO_CRT
#undef PROGRAM_CONTROL_RULE_NO_CRT
#undef PROGRAM_CONTROL_RUNTIME_TEMP_NO_CRT
#endif

#ifdef PROGRAM_CONTROL_RUNTIME_TEMP_NO_QUERY_HELPERS
#undef PROGRAM_CONTROL_RULE_NO_QUERY_HELPERS
#undef PROGRAM_CONTROL_RUNTIME_TEMP_NO_QUERY_HELPERS
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _SBIE_RT_BACKEND_KIND
{
    SBIE_RT_BACKEND_NONE = 0,
    SBIE_RT_BACKEND_EXACT_STRING,
    SBIE_RT_BACKEND_IMAGE_WILDCARD,
    SBIE_RT_BACKEND_PATH_PATTERN
} SBIE_RT_BACKEND_KIND;

typedef struct _SBIE_RT_RULE
{
    SBIE_PROGRAM_RULE_KIND rule_kind;
    const WCHAR *setting_name;
    WCHAR *raw_rule;
    WCHAR *scope_rule;
    int has_scope;
    int scope_invert;
    long scope_level;
    SBIE_NORMALIZED_RULE normalized;
    SBIE_RT_BACKEND_KIND backend_kind;
    void *backend_state;
} SBIE_RT_RULE;

typedef struct _SBIE_RT_RULE_LIST
{
    SBIE_RT_RULE *rules;
    ULONG count;
    ULONG capacity;
} SBIE_RT_RULE_LIST;

typedef struct _SBIE_RT_RULESET
{
    POOL *pool;
    SBIE_RT_RULE_LIST force_process;
    SBIE_RT_RULE_LIST force_folder;
    SBIE_RT_RULE_LIST force_children;
    SBIE_RT_RULE_LIST breakout_process;
    SBIE_RT_RULE_LIST breakout_folder;
    SBIE_RT_RULE_LIST breakout_document;
} SBIE_RT_RULESET;

typedef struct _SBIE_RT_MATCH
{
    int matched;
    const SBIE_RT_RULE *rule;
    const WCHAR *base_rule;
    const WCHAR *target_box;
    int has_target_box;
    int has_priority;
    long priority;
    long level;
    int target_matched;
    const SBIE_RT_RULE *target_rule;
    const WCHAR *target_candidate_box;
    int target_has_priority;
    long target_priority;
    long target_level;
} SBIE_RT_MATCH;

void ProgramControl_RuntimeInitMatch(SBIE_RT_MATCH *match);
void ProgramControl_RuntimeInitRuleset(SBIE_RT_RULESET *ruleset, POOL *pool);
void ProgramControl_RuntimeFreeRuleset(SBIE_RT_RULESET *ruleset);

int ProgramControl_RuntimeCompileSetting(
    SBIE_RT_RULESET *ruleset,
    const WCHAR *setting,
    const WCHAR *value,
    int use_rule_extensions);

int ProgramControl_RuntimeGetApplicableTargetBox(
    const SBIE_RT_MATCH *match,
    const WCHAR **out_target_box);

int ProgramControl_RuntimeMatchProcess(
    const SBIE_RT_RULESET *ruleset,
    const WCHAR *image_name,
    const WCHAR *image_path,
    BreakoutMatchImageFn match_image,
    void *match_context,
    SBIE_RT_MATCH *force_process_match,
    SBIE_RT_MATCH *force_children_match,
    SBIE_RT_MATCH *breakout_process_match);

int ProgramControl_RuntimeMatchFolder(
    const SBIE_RT_RULESET *ruleset,
    const WCHAR *image_name,
    BreakoutMatchImageFn match_image,
    void *match_context,
    const WCHAR *path,
    unsigned long path_len,
    SBIE_RT_MATCH *force_folder_match,
    SBIE_RT_MATCH *breakout_folder_match);

int ProgramControl_RuntimeMatchDocument(
    const SBIE_RT_RULESET *ruleset,
    const WCHAR *image_name,
    BreakoutMatchImageFn match_image,
    void *match_context,
    const WCHAR *path,
    unsigned long path_len,
    SBIE_RT_MATCH *breakout_document_match);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _PROGRAM_CONTROL_RUNTIME_H
