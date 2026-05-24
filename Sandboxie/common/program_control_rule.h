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
// Shared program control matching helpers for DLL/SVC code paths.
// Keep this header C/C++ compatible and header-only to avoid project wiring.
//---------------------------------------------------------------------------

#ifndef _PROGRAM_CONTROL_RULE_H
#define _PROGRAM_CONTROL_RULE_H

#ifndef PROGRAM_CONTROL_RULE_NO_CRT
#include <wchar.h>
#include <wctype.h>
#endif
#include "my_version.h"

typedef int (*BreakoutMatchImageFn)(const WCHAR *pattern, const WCHAR *imageName, void *context);
typedef void (*BreakoutAdjustRuleFn)(WCHAR *value, void *context);

typedef struct _SBIE_NORMALIZED_RULE
{
    WCHAR *base_rule;
    WCHAR *target_box;
    int has_target_box;
    int has_recursive;
    long recursive_depth;   // -1 means unlimited
    int has_priority;
    long priority;
} SBIE_NORMALIZED_RULE;

typedef enum _SBIE_CONTEXT_KIND
{
    SBIE_CTX_UNSANDBOXED_PROCESS_START = 1,
    SBIE_CTX_SANDBOXED_PROCESS_START   = 2,
    SBIE_CTX_SANDBOXED_DOCUMENT_OPEN   = 3
} SBIE_CONTEXT_KIND;

typedef enum _SBIE_POLICY_DECISION
{
    SBIE_DECISION_NO_MATCH = 0,
    SBIE_DECISION_FORCE_SAME_BOX,
    SBIE_DECISION_FORCE_OTHER_BOX,
    SBIE_DECISION_BREAKOUT_UNBOXED,
    SBIE_DECISION_BREAKOUT_TARGET_BOX,
    SBIE_DECISION_DENY_FALLBACK
} SBIE_POLICY_DECISION;

typedef struct _SBIE_RULE_MATCH_SET
{
    int force_process_match;
    int force_folder_match;
    int force_children_match;
    int breakout_process_match;
    int breakout_folder_match;
    int breakout_document_match;
    int breakout_has_target;
} SBIE_RULE_MATCH_SET;

typedef struct _SBIE_POLICY_INPUT
{
    SBIE_CONTEXT_KIND context_kind;
    int prioritize_breakout_over_force;
    int caller_forced_by_children;
    int source_equals_candidate_box;
} SBIE_POLICY_INPUT;

static __inline int ProgramControl_IsRuleExtensionSetting(const WCHAR *setting)
{
    if (!setting)
        return 0;

    return (
        _wcsicmp(setting, L"BreakoutProcess") == 0 ||
        _wcsicmp(setting, L"BreakoutFolder") == 0 ||
        _wcsicmp(setting, L"BreakoutDocument") == 0 ||
        _wcsicmp(setting, L"ForceProcess") == 0 ||
        _wcsicmp(setting, L"ForceFolder") == 0 ||
        _wcsicmp(setting, L"ForceChildren") == 0
    ) ? 1 : 0;
}

static __inline int SbiePolicy_HasForceMatch(const SBIE_RULE_MATCH_SET *m)
{
    return m && (m->force_process_match || m->force_folder_match || m->force_children_match);
}

static __inline int SbiePolicy_HasBreakoutMatch(const SBIE_RULE_MATCH_SET *m)
{
    return m && (m->breakout_process_match || m->breakout_folder_match || m->breakout_document_match);
}

static __inline int SbiePolicy_ShouldPrioritizeBreakout(
    int legacy_prioritize_breakout,
    int force_has_priority,
    long force_priority,
    int breakout_has_priority,
    long breakout_priority)
{
    if (force_has_priority || breakout_has_priority) {
        if (breakout_has_priority && !force_has_priority)
            return 1;
        if (force_has_priority && !breakout_has_priority)
            return 0;
        if (breakout_priority < force_priority)
            return 1;
        if (breakout_priority > force_priority)
            return 0;
        return legacy_prioritize_breakout ? 1 : 0;
    }

    return legacy_prioritize_breakout ? 1 : 0;
}

static __inline SBIE_POLICY_DECISION SbiePolicy_ResolveDecision(
    const SBIE_POLICY_INPUT *in,
    const SBIE_RULE_MATCH_SET *m)
{
    int has_force;
    int has_breakout;

    if (!in || !m)
        return SBIE_DECISION_NO_MATCH;

    has_force = SbiePolicy_HasForceMatch(m);
    has_breakout = SbiePolicy_HasBreakoutMatch(m);

    if (!has_force && !has_breakout)
        return SBIE_DECISION_NO_MATCH;

    if (in->context_kind == SBIE_CTX_SANDBOXED_PROCESS_START &&
        in->caller_forced_by_children &&
        !in->prioritize_breakout_over_force &&
        has_breakout)
        return SBIE_DECISION_FORCE_SAME_BOX;

    if (has_force && !has_breakout)
        return in->source_equals_candidate_box ? SBIE_DECISION_FORCE_SAME_BOX : SBIE_DECISION_FORCE_OTHER_BOX;

    if (!has_force && has_breakout) {
        if (m->breakout_has_target)
            return SBIE_DECISION_BREAKOUT_TARGET_BOX;
        return SBIE_DECISION_BREAKOUT_UNBOXED;
    }

    if (in->prioritize_breakout_over_force) {
        if (m->breakout_has_target)
            return SBIE_DECISION_BREAKOUT_TARGET_BOX;
        return SBIE_DECISION_BREAKOUT_UNBOXED;
    }

    return in->source_equals_candidate_box ? SBIE_DECISION_FORCE_SAME_BOX : SBIE_DECISION_FORCE_OTHER_BOX;
}

static __inline SBIE_POLICY_DECISION SbiePolicy_ResolveWithPriorities(
    const SBIE_POLICY_INPUT *baseInput,
    const SBIE_RULE_MATCH_SET *matches,
    int legacy_prioritize_breakout,
    int force_has_priority,
    long force_priority,
    int breakout_has_priority,
    long breakout_priority)
{
    SBIE_POLICY_INPUT in;

    if (!baseInput || !matches)
        return SBIE_DECISION_NO_MATCH;

    in = *baseInput;
    in.prioritize_breakout_over_force = SbiePolicy_ShouldPrioritizeBreakout(
        legacy_prioritize_breakout,
        force_has_priority ? 1 : 0,
        force_priority,
        breakout_has_priority ? 1 : 0,
        breakout_priority) ? 1 : 0;

    return SbiePolicy_ResolveDecision(&in, matches);
}

static __inline int ProgramControl_MatchFolderRule(
    const WCHAR *rule, const WCHAR *appPath, unsigned long appDirLen);

static __inline int ProgramControl_MatchFolderRuleNormalized(
    const SBIE_NORMALIZED_RULE *rule, const WCHAR *appPath, unsigned long appDirLen);

static __inline int ProgramControl_WildcardMatchI(const WCHAR *pattern, const WCHAR *text)
{
    if (!pattern || !text)
        return 0;

    while (*pattern) {
        if (*pattern == L'*') {
            while (*pattern == L'*')
                ++pattern;
            if (!*pattern)
                return 1;
            while (*text) {
                if (ProgramControl_WildcardMatchI(pattern, text))
                    return 1;
                ++text;
            }
            return 0;
        }

        if (!*text)
            return 0;

        if (*pattern != L'?' &&
#ifdef PROGRAM_CONTROL_RULE_NO_CRT
            ((*pattern >= L'A' && *pattern <= L'Z') ? (*pattern + (L'a' - L'A')) : *pattern)
                != ((*text >= L'A' && *text <= L'Z') ? (*text + (L'a' - L'A')) : *text)
#else
            towlower(*pattern) != towlower(*text)
#endif
            )
            return 0;

        ++pattern;
        ++text;
    }

    return (*text == L'\0');
}

static __inline int ProgramControl_RuleLooksLikePath(const WCHAR *rule)
{
    return (rule && (wcschr(rule, L'\\') || wcschr(rule, L'/')));
}

static __inline int ProgramControl_ParseLong(const WCHAR *value, long *outValue)
{
    long result = 0;
    int sign = 1;

    if (!value || !*value || !outValue)
        return 0;

    if (*value == L'-') {
        sign = -1;
        ++value;
    }
    else if (*value == L'+') {
        ++value;
    }

    if (!*value)
        return 0;

    while (*value) {
        if (*value < L'0' || *value > L'9')
            return 0;
        result = (result * 10) + (long)(*value - L'0');
        ++value;
    }

    *outValue = result * sign;
    return 1;
}

static __inline int ProgramControl_ParseRuleExtensionsInPlace(
    WCHAR *value,
    SBIE_NORMALIZED_RULE *outRule,
    int useRuleExtensions)
{
    WCHAR *sep;
    WCHAR *token;

    if (!value || !*value || !outRule)
        return 0;

    outRule->base_rule = value;
    outRule->target_box = NULL;
    outRule->has_target_box = 0;
    outRule->has_recursive = 0;
    outRule->recursive_depth = -1;
    // -1 is an internal sentinel meaning "no explicit priority set".
    outRule->has_priority = 0;
    outRule->priority = -1;

    // When extensions are disabled, keep only the base rule and ignore
    // any metadata suffixes (Priority/Recursive/TargetBox).
    if (!useRuleExtensions) {
        sep = wcschr(value, L'|');
        if (sep)
            *sep = L'\0';
        return 1;
    }

    sep = wcschr(value, L'|');
    if (!sep)
        return 1;

    // Split at '|' only when rule extensions are enabled.

    *sep = L'\0';

    token = sep + 1;

    while (token && *token) {
        WCHAR *next = wcschr(token, L'|');
        WCHAR *eq;

        if (next)
            *next = L'\0';

        eq = wcschr(token, L'=');
        if (eq && eq > token && eq[1]) {
            *eq = L'\0';
            if (_wcsicmp(token, L"TargetBox") == 0) {
                outRule->target_box = eq + 1;
                outRule->has_target_box = (*outRule->target_box != L'\0') ? 1 : 0;
            }
            else if (_wcsicmp(token, L"Recursive") == 0) {
                const WCHAR *rv = eq + 1;
                long depth = -1;

                if (*rv == L'*' || *rv == L'y' || *rv == L'Y') {
                    outRule->has_recursive = 1;
                    outRule->recursive_depth = -1;
                }
                else if (*rv == L'n' || *rv == L'N') {
                    outRule->has_recursive = 1;
                    outRule->recursive_depth = 0;
                }
                else if (ProgramControl_ParseLong(rv, &depth) && depth >= 0) {
                    outRule->has_recursive = 1;
                    outRule->recursive_depth = depth;
                }
            }
            else if (_wcsicmp(token, L"Priority") == 0) {
                long prio = -1;
                // User-configured Priority must be >= 0. Negative values are invalid.
                // Keep -1 reserved as the internal "unspecified" sentinel.
                if (ProgramControl_ParseLong(eq + 1, &prio) && prio >= 0) {
                    outRule->has_priority = 1;
                    outRule->priority = prio;
                }
            }

            *eq = L'=';
        }
        else if (_wcsicmp(token, L"Recursive") == 0) {
            outRule->has_recursive = 1;
            outRule->recursive_depth = -1;
        }

        if (!next)
            break;

        token = next + 1;
    }

    return 1;
}

static __inline WCHAR *ProgramControl_FindTargetSeparator(WCHAR *value)
{
    SBIE_NORMALIZED_RULE rule;

    if (!value)
        return NULL;

    if (!ProgramControl_ParseRuleExtensionsInPlace(value, &rule, 1))
        return NULL;

    return rule.has_target_box ? rule.target_box : NULL;
}

static __inline int ProgramControl_ShouldReplaceTargetMatch(
    int hasCurrentMatch,
    int currentHasPriority,
    long currentPriority,
    unsigned long currentLevel,
    int candidateHasPriority,
    long candidatePriority,
    unsigned long candidateLevel)
{
    if (!hasCurrentMatch)
        return 1;

    if (candidateHasPriority != currentHasPriority)
        return candidateHasPriority;

    if (candidateHasPriority && candidatePriority != currentPriority)
        return (candidatePriority < currentPriority) ? 1 : 0;

    if (candidateLevel != currentLevel)
        return (candidateLevel < currentLevel) ? 1 : 0;

    return 0;
}

static __inline int ProgramControl_ShouldReplacePriorityWinner(
    int hasCurrentWinner,
    int currentHasPriority,
    long currentPriority,
    int candidateHasPriority,
    long candidatePriority)
{
    return ProgramControl_ShouldReplaceTargetMatch(
        hasCurrentWinner,
        currentHasPriority,
        currentPriority,
        0,
        candidateHasPriority,
        candidatePriority,
        0);
}

static __inline int ProgramControl_IsPrimaryPreferredByPriority(
    long primaryPriority,
    long secondaryPriority,
    int preferPrimaryOnTie)
{
    int primaryHasPriority = (primaryPriority >= 0) ? 1 : 0;
    int secondaryHasPriority = (secondaryPriority >= 0) ? 1 : 0;

    if (primaryHasPriority && secondaryHasPriority) {
        if (primaryPriority < secondaryPriority)
            return 1;
        if (primaryPriority > secondaryPriority)
            return 0;
        return preferPrimaryOnTie ? 1 : 0;
    }

    if (primaryHasPriority != secondaryHasPriority)
        return primaryHasPriority ? 1 : 0;

    return preferPrimaryOnTie ? 1 : 0;
}

static __inline int ProgramControl_ShouldAcceptTargetChoice(
    int overallHasMatch,
    int overallHasPriority,
    long overallBestPriority,
    int targetHasMatch,
    int targetHasPriority,
    long targetBestPriority)
{
    if (overallHasMatch && overallHasPriority) {
        if (!targetHasMatch)
            return 0;
        if (!targetHasPriority)
            return 0;
        if (targetBestPriority > overallBestPriority)
            return 0;
    }

    return targetHasMatch ? 1 : 0;
}

static __inline WCHAR *ProgramControl_MatchImageScopeAndGetValueEx(
    WCHAR *value, const WCHAR *imageName, BreakoutMatchImageFn matchImage, void *context,
    unsigned long *outLevel)
{
    WCHAR *tmp = wcschr(value, L',');

    if (tmp) {
        int inv;
        int match;
        size_t len;

        if (!imageName || !matchImage)
            return NULL;

        if (*value == L'!') {
            inv = 1;
            ++value;
        }
        else
            inv = 0;

        len = (size_t)(tmp - value);
        if (len) {
            WCHAR saved = value[len];
            value[len] = L'\0';
            match = matchImage(value, imageName, context);
            value[len] = saved;
            if (inv)
                match = !match;
            if (!match)
                return NULL;

            if (outLevel) {
                if (len == 1 && *value == L'*')
                    *outLevel = 2;
                else
                    *outLevel = inv ? 1 : 0;
            }
        }

        value = tmp + 1;
    }
    else if (outLevel) {
        *outLevel = 2;
    }

    if (!*value)
        return NULL;

    return value;
}

static __inline WCHAR *ProgramControl_MatchImageScopeAndGetValue(
    WCHAR *value, const WCHAR *imageName, BreakoutMatchImageFn matchImage, void *context)
{
    return ProgramControl_MatchImageScopeAndGetValueEx(
        value, imageName, matchImage, context, NULL);
}

static __inline void ProgramControl_TrimTrailingBackslashes(WCHAR *value)
{
    size_t len;

    if (!value)
        return;

    len = wcslen(value);
    while (len && value[len - 1] == L'\\')
        value[--len] = L'\0';
}

#ifndef PROGRAM_CONTROL_RULE_NO_QUERY_HELPERS

static __inline int ProgramControl_CheckFolderSettingMatchFromConf(
    const WCHAR *boxname, const WCHAR *setting, const WCHAR *imageName, const WCHAR *appPath, unsigned long appDirLen,
    int allowTargeted, BOOLEAN *outHasPriority, LONG *outPriority, int useRuleExtensions,
    BreakoutMatchImageFn matchImage, void *matchContext,
    BreakoutAdjustRuleFn adjustRule, void *adjustContext)
{
    WCHAR buf[CONF_LINE_LEN];
    unsigned long index = 0;
    int hasMatch = 0;
    int bestHasPriority = 0;
    long bestPriority = -1;
    unsigned long bestLevel = (unsigned long)-1;

    if (outHasPriority)
        *outHasPriority = FALSE;
    if (outPriority)
        *outPriority = -1;

    if (!setting || !*setting || !appPath || !*appPath)
        return 0;

    while (1) {
        NTSTATUS status = SbieApi_QueryConf(boxname, setting, index, buf, sizeof(buf) - 16 * sizeof(WCHAR));
        WCHAR *value;
        SBIE_NORMALIZED_RULE rule;
        unsigned long level = (unsigned long)-1;

        ++index;
        if (!NT_SUCCESS(status)) {
            if (status == STATUS_BUFFER_TOO_SMALL)
                continue;
            break;
        }

        value = ProgramControl_MatchImageScopeAndGetValueEx(buf, imageName, matchImage, matchContext, &level);
        if (!value)
            continue;

        if (!ProgramControl_ParseRuleExtensionsInPlace(value, &rule, useRuleExtensions))
            continue;

        if (rule.has_target_box && !allowTargeted)
            continue;

        if (adjustRule)
            adjustRule(rule.base_rule, adjustContext);

        if (ProgramControl_MatchFolderRuleNormalized(&rule, appPath, appDirLen)
            || ProgramControl_MatchFolderRuleNormalized(&rule, appPath, (unsigned long)wcslen(appPath))) {
            if (!outHasPriority && !outPriority)
                return 1;

            if (!hasMatch || level < bestLevel) {
                hasMatch = 1;
                bestLevel = level;
                bestHasPriority = rule.has_priority ? 1 : 0;
                bestPriority = rule.has_priority ? rule.priority : -1;
            }
            else if (level == bestLevel && rule.has_priority) {
                if (!bestHasPriority || rule.priority < bestPriority) {
                    bestHasPriority = 1;
                    bestPriority = rule.priority;
                }
            }
        }
    }

    if (hasMatch) {
        if (outHasPriority)
            *outHasPriority = bestHasPriority ? TRUE : FALSE;
        if (outPriority)
            *outPriority = bestHasPriority ? bestPriority : -1;
    }

    return hasMatch;
}

static __inline int ProgramControl_CheckFolderMatchFromConf(
    const WCHAR *boxname, const WCHAR *imageName, const WCHAR *appPath, unsigned long appDirLen,
    int allowTargeted, int useRuleExtensions, BreakoutMatchImageFn matchImage, void *matchContext,
    BreakoutAdjustRuleFn adjustRule, void *adjustContext)
{
    return ProgramControl_CheckFolderSettingMatchFromConf(
        boxname,
        L"BreakoutFolder",
        imageName,
        appPath,
        appDirLen,
        allowTargeted,
        NULL,
        NULL,
        useRuleExtensions,
        matchImage,
        matchContext,
        adjustRule,
        adjustContext);
}

static __inline int ProgramControl_GetFolderTargetFromConf(
    const WCHAR *boxname, const WCHAR *imageName, const WCHAR *appPath, unsigned long appDirLen,
    WCHAR *outTarget, size_t outTargetCch,
    BOOLEAN *outHasPriority, LONG *outPriority,
    int useRuleExtensions, BreakoutMatchImageFn matchImage, void *matchContext,
    BreakoutAdjustRuleFn adjustRule, void *adjustContext)
{
    WCHAR buf[CONF_LINE_LEN];
    unsigned long index = 0;
    unsigned long appPathLen;
    int hasMatch = 0;
    int bestHasPriority = 0;
    long bestPriority = -1;
    unsigned long bestLevel = (unsigned long)-1;
    int overallHasMatch = 0;
    int overallHasPriority = 0;
    long overallBestPriority = -1;

    if (!outTarget || !outTargetCch)
        return 0;

    outTarget[0] = L'\0';
    if (outHasPriority)
        *outHasPriority = FALSE;
    if (outPriority)
        *outPriority = -1;

    appPathLen = (unsigned long)wcslen(appPath);

    while (1) {
        NTSTATUS status = SbieApi_QueryConf(boxname, L"BreakoutFolder", index, buf, sizeof(buf) - 16 * sizeof(WCHAR));
        WCHAR *value;
        SBIE_NORMALIZED_RULE rule;
        unsigned long level = (unsigned long)-1;

        ++index;
        if (!NT_SUCCESS(status)) {
            if (status == STATUS_BUFFER_TOO_SMALL)
                continue;
            break;
        }

        value = ProgramControl_MatchImageScopeAndGetValueEx(buf, imageName, matchImage, matchContext, &level);
        if (!value)
            continue;

        if (!ProgramControl_ParseRuleExtensionsInPlace(value, &rule, useRuleExtensions))
            continue;

        if (adjustRule)
            adjustRule(rule.base_rule, adjustContext);

        if (ProgramControl_MatchFolderRuleNormalized(&rule, appPath, appDirLen)
            || ProgramControl_MatchFolderRuleNormalized(&rule, appPath, appPathLen)) {
            overallHasMatch = 1;
            if (rule.has_priority && (!overallHasPriority || rule.priority < overallBestPriority)) {
                overallHasPriority = 1;
                overallBestPriority = rule.priority;
            }

            if (!rule.has_target_box || !rule.target_box || !*rule.target_box)
                continue;

            if (ProgramControl_ShouldReplaceTargetMatch(
                    hasMatch,
                    bestHasPriority,
                    bestPriority,
                    bestLevel,
                    rule.has_priority,
                    rule.priority,
                    level)) {
                wcscpy_s(outTarget, outTargetCch, rule.target_box);
                hasMatch = 1;
                bestHasPriority = rule.has_priority;
                bestPriority = rule.priority;
                bestLevel = level;
            }
        }

    }

    if (!ProgramControl_ShouldAcceptTargetChoice(
            overallHasMatch,
            overallHasPriority,
            overallBestPriority,
            hasMatch,
            bestHasPriority,
            bestPriority)) {
        return 0;
    }

    if (hasMatch) {
        if (outHasPriority)
            *outHasPriority = bestHasPriority ? TRUE : FALSE;
        if (outPriority)
            *outPriority = bestHasPriority ? bestPriority : -1;
    }

    return hasMatch;
}

static __inline int ProgramControl_GetFolderPriorityFromConfEx(
    const WCHAR *boxname, const WCHAR *setting, const WCHAR *imageName, const WCHAR *appPath, unsigned long appDirLen,
    BOOLEAN *outHasPriority, LONG *outPriority, int useRuleExtensions, BreakoutMatchImageFn matchImage, void *matchContext,
    BreakoutAdjustRuleFn adjustRule, void *adjustContext)
{
    WCHAR buf[CONF_LINE_LEN];
    unsigned long index = 0;
    BOOLEAN hasPriority = FALSE;
    LONG bestPriority = -1;

    if (outHasPriority) *outHasPriority = FALSE;
    if (outPriority) *outPriority = -1;

    while (1) {
        NTSTATUS status = SbieApi_QueryConf(boxname, setting, index, buf, sizeof(buf) - 16 * sizeof(WCHAR));
        WCHAR *value;
        SBIE_NORMALIZED_RULE rule;

        ++index;
        if (!NT_SUCCESS(status)) {
            if (status == STATUS_BUFFER_TOO_SMALL)
                continue;
            break;
        }

        value = ProgramControl_MatchImageScopeAndGetValue(buf, imageName, matchImage, matchContext);
        if (!value)
            continue;

        if (!ProgramControl_ParseRuleExtensionsInPlace(value, &rule, useRuleExtensions))
            continue;

        if (!rule.has_priority)
            continue;

        if (adjustRule)
            adjustRule(rule.base_rule, adjustContext);

        if (ProgramControl_MatchFolderRuleNormalized(&rule, appPath, appDirLen)
            || ProgramControl_MatchFolderRuleNormalized(&rule, appPath, (unsigned long)wcslen(appPath))) {
            if (!hasPriority || rule.priority < bestPriority) {
                hasPriority = TRUE;
                bestPriority = rule.priority;
            }
        }
    }

    if (hasPriority) {
        if (outHasPriority) *outHasPriority = TRUE;
        if (outPriority) *outPriority = bestPriority;
    }

    return hasPriority ? 1 : 0;
}

static __inline int ProgramControl_GetFolderPriorityFromConf(
    const WCHAR *boxname, const WCHAR *imageName, const WCHAR *appPath, unsigned long appDirLen,
    BOOLEAN *outHasPriority, LONG *outPriority, int useRuleExtensions, BreakoutMatchImageFn matchImage, void *matchContext,
    BreakoutAdjustRuleFn adjustRule, void *adjustContext)
{
    return ProgramControl_GetFolderPriorityFromConfEx(
        boxname, L"BreakoutFolder", imageName, appPath, appDirLen,
        outHasPriority, outPriority, useRuleExtensions, matchImage, matchContext,
        adjustRule, adjustContext);
}

static __inline int ProgramControl_MatchDocumentRule(
    const WCHAR *rule,
    const WCHAR *docPath,
    unsigned long docPathLen)
{
    size_t ruleLen;

    if (!rule || !*rule || !docPath || !*docPath)
        return 0;

    if (!docPathLen)
        docPathLen = (unsigned long)wcslen(docPath);

    if (wcschr(rule, L'*') || wcschr(rule, L'?'))
        return ProgramControl_WildcardMatchI(rule, docPath);

    ruleLen = wcslen(rule);
    if ((unsigned long)ruleLen != docPathLen)
        return 0;

    return (_wcsnicmp(rule, docPath, ruleLen) == 0);
}

static __inline int ProgramControl_FindDocumentSettingMatch(
    const WCHAR *boxname,
    const WCHAR *setting,
    const WCHAR *imageName,
    const WCHAR *docPath,
    unsigned long docPathLen,
    int allowTargeted,
    int useRuleExtensions,
    BreakoutMatchImageFn matchImage,
    void *matchContext,
    BreakoutAdjustRuleFn adjustRule,
    void *adjustContext,
    int *outHasTarget,
    WCHAR *outTarget,
    size_t outTargetCch,
    BOOLEAN *outHasPriority,
    LONG *outPriority,
    unsigned long *outLevel)
{
    WCHAR buf[CONF_LINE_LEN];
    unsigned long index = 0;
    int hasMatch = 0;
    int bestHasPriority = 0;
    long bestPriority = -1;
    unsigned long bestLevel = (unsigned long)-1;
    int bestHasTarget = 0;

    if (outHasTarget)
        *outHasTarget = 0;
    if (outTarget && outTargetCch)
        outTarget[0] = L'\0';
    if (outHasPriority)
        *outHasPriority = FALSE;
    if (outPriority)
        *outPriority = -1;
    if (outLevel)
        *outLevel = (unsigned long)-1;

    if (!boxname || !setting || !imageName || !*imageName || !docPath || !*docPath)
        return 0;

    if (!docPathLen)
        docPathLen = (unsigned long)wcslen(docPath);

    while (1) {
        NTSTATUS status = SbieApi_QueryConfAsIs(boxname, setting, index, buf, sizeof(buf) - sizeof(WCHAR));
        WCHAR *value;
        SBIE_NORMALIZED_RULE rule;
        unsigned long level = 2;

        ++index;
        if (!NT_SUCCESS(status)) {
            if (status == STATUS_BUFFER_TOO_SMALL)
                continue;
            break;
        }

        value = ProgramControl_MatchImageScopeAndGetValueEx(buf, imageName, matchImage, matchContext, &level);
        if (!value)
            continue;

        if (!ProgramControl_ParseRuleExtensionsInPlace(value, &rule, useRuleExtensions))
            continue;

        if (rule.has_target_box && !allowTargeted)
            continue;

        if (adjustRule)
            adjustRule(rule.base_rule, adjustContext);

        if (!ProgramControl_MatchDocumentRule(rule.base_rule, docPath, docPathLen))
            continue;

        if (ProgramControl_ShouldReplaceTargetMatch(
                hasMatch,
                bestHasPriority,
                bestPriority,
                bestLevel,
                rule.has_priority,
                rule.priority,
                level)) {
            hasMatch = 1;
            bestHasPriority = rule.has_priority;
            bestPriority = rule.has_priority ? rule.priority : -1;
            bestLevel = level;
            bestHasTarget = (rule.has_target_box && rule.target_box && *rule.target_box) ? 1 : 0;

            if (bestHasTarget) {
                if (outTarget && outTargetCch)
                    wcscpy_s(outTarget, outTargetCch, rule.target_box);
            }
            else if (outTarget && outTargetCch) {
                outTarget[0] = L'\0';
            }
        }
    }

    if (hasMatch) {
        if (outHasTarget)
            *outHasTarget = bestHasTarget;
        if (outHasPriority)
            *outHasPriority = bestHasPriority ? TRUE : FALSE;
        if (outPriority)
            *outPriority = bestHasPriority ? bestPriority : -1;
        if (outLevel)
            *outLevel = bestLevel;
    }

    return hasMatch;
}

#endif


static __inline int ProgramControl_MatchProcessRule(
    const WCHAR *rule, const WCHAR *imageName, const WCHAR *appPath, unsigned long appPathLen)
{
    size_t ruleLen;

    if (!rule || !*rule || !imageName || !*imageName || !appPath || !appPathLen)
        return 0;

    if (_wcsicmp(rule, imageName) == 0)
        return 1;

    if (!ProgramControl_RuleLooksLikePath(rule))
        return 0;

    ruleLen = wcslen(rule);
    if (!ruleLen)
        return 0;

    if (wcschr(rule, L'*') || wcschr(rule, L'?'))
        return ProgramControl_WildcardMatchI(rule, appPath);

    if (ruleLen != appPathLen)
        return 0;

    return (_wcsnicmp(rule, appPath, ruleLen) == 0);
}

static __inline int ProgramControl_MatchFolderRule(
    const WCHAR *rule, const WCHAR *appPath, unsigned long appDirLen)
{
    size_t ruleLen;

    if (!rule || !*rule || !appPath || !appDirLen)
        return 0;

    ruleLen = wcslen(rule);
    while (ruleLen && rule[ruleLen - 1] == L'\\')
        --ruleLen;

    if (!ruleLen || !appDirLen)
        return 0;

    if (wcschr(rule, L'*') || wcschr(rule, L'?'))
        return ProgramControl_WildcardMatchI(rule, appPath);

    // Allow prefix matching: rule "C:\Folder" must match "C:\Folder"
    // (direct child) as well as "C:\Folder\sub" (subdirectory child).
    // Require a backslash separator at rule length to avoid partial
    // directory name matches (e.g. "C:\Prog" must not match "C:\Program Files").
    if (ruleLen > appDirLen)
        return 0;

    if (ruleLen < appDirLen && appPath[ruleLen] != L'\\')
        return 0;

    return (_wcsnicmp(rule, appPath, ruleLen) == 0);
}

static __inline int ProgramControl_MatchFolderRuleNormalized(
    const SBIE_NORMALIZED_RULE *rule, const WCHAR *appPath, unsigned long appDirLen)
{
    size_t baseLen;

    if (!rule || !rule->base_rule)
        return 0;

    if (!ProgramControl_MatchFolderRule(rule->base_rule, appPath, appDirLen))
        return 0;

    if (!rule->has_recursive || rule->recursive_depth < 0)
        return 1;

    if (wcschr(rule->base_rule, L'*') || wcschr(rule->base_rule, L'?')) {
        // For wildcard rules, find the base folder: last backslash before
        // any wildcard character, then count depth from there.
        const WCHAR *wc_scan = rule->base_rule;
        const WCHAR *wc_last_sep = NULL;
        size_t wc_base_len;

        while (*wc_scan && *wc_scan != L'*' && *wc_scan != L'?') {
            if (*wc_scan == L'\\')
                wc_last_sep = wc_scan;
            ++wc_scan;
        }

        if (!wc_last_sep)
            return 1;

        wc_base_len = (size_t)(wc_last_sep - rule->base_rule);

        if (wc_base_len >= appDirLen)
            return 1;

        {
            const WCHAR *dir_begin = appPath + wc_base_len;
            const WCHAR *dir_end = appPath + appDirLen;
            long depth = 0;

            if (*dir_begin == L'\\')
                ++dir_begin;

            if (dir_begin < dir_end) {
                const WCHAR *p = dir_begin;
                depth = 1;
                while (p < dir_end) {
                    if (*p == L'\\')
                        ++depth;
                    ++p;
                }
            }

            return (depth <= rule->recursive_depth) ? 1 : 0;
        }
    }

    baseLen = wcslen(rule->base_rule);
    while (baseLen && rule->base_rule[baseLen - 1] == L'\\')
        --baseLen;

    if ((unsigned long)baseLen >= appDirLen)
        return 1;

    {
        const WCHAR *dir_begin = appPath + baseLen;
        const WCHAR *dir_end = appPath + appDirLen;
        long depth = 0;

        if (*dir_begin == L'\\')
            ++dir_begin;

        if (dir_begin < dir_end) {
            const WCHAR *p = dir_begin;
            depth = 1;
            while (p < dir_end) {
                if (*p == L'\\')
                    ++depth;
                ++p;
            }
        }

        return (depth <= rule->recursive_depth) ? 1 : 0;
    }
}

#ifndef PROGRAM_CONTROL_RULE_NO_QUERY_HELPERS

static __inline int ProgramControl_FindProcessSettingMatch(
    const WCHAR *boxname, const WCHAR *setting, const WCHAR *imageName, const WCHAR *appPath,
    unsigned long appPathLen, int allowTargeted, int useRuleExtensions, BreakoutAdjustRuleFn adjustRule, void *adjustContext,
    WCHAR *outTarget, size_t outTargetCch, int *outHasTarget,
    BOOLEAN *outHasPriority, LONG *outPriority)
{
    WCHAR buf[CONF_LINE_LEN];
    unsigned long index = 0;
    int hasMatch = 0;
    int bestHasPriority = 0;
    long bestPriority = -1;

    if (outTarget && outTargetCch)
        outTarget[0] = L'\0';
    if (outHasTarget)
        *outHasTarget = 0;
    if (outHasPriority)
        *outHasPriority = FALSE;
    if (outPriority)
        *outPriority = -1;

    while (1) {
        NTSTATUS status = SbieApi_QueryConfAsIs(boxname, setting, index, buf, sizeof(buf) - sizeof(WCHAR));
        SBIE_NORMALIZED_RULE rule;

        ++index;
        if (!NT_SUCCESS(status)) {
            if (status == STATUS_BUFFER_TOO_SMALL)
                continue;
            break;
        }

        if (!ProgramControl_ParseRuleExtensionsInPlace(buf, &rule, useRuleExtensions))
            continue;

        if (rule.has_target_box && !allowTargeted)
            continue;

        if (adjustRule)
            adjustRule(rule.base_rule, adjustContext);

        if (!ProgramControl_MatchProcessRule(rule.base_rule, imageName, appPath, appPathLen))
            continue;

        if (ProgramControl_ShouldReplaceTargetMatch(
                hasMatch,
                bestHasPriority,
                bestPriority,
                2,
                rule.has_priority,
                rule.priority,
                2)) {
            hasMatch = 1;
            bestHasPriority = rule.has_priority;
            bestPriority = rule.priority;

            if (rule.has_target_box && rule.target_box && *rule.target_box) {
                if (outHasTarget)
                    *outHasTarget = 1;

                if (outTarget && outTargetCch)
                    wcscpy_s(outTarget, outTargetCch, rule.target_box);
            }
            else {
                if (outHasTarget)
                    *outHasTarget = 0;

                if (outTarget && outTargetCch)
                    outTarget[0] = L'\0';
            }
        }
    }

    if (hasMatch) {
        if (outHasPriority)
            *outHasPriority = bestHasPriority ? TRUE : FALSE;
        if (outPriority)
            *outPriority = bestHasPriority ? bestPriority : -1;
    }

    return hasMatch;
}

static __inline int ProgramControl_FindProcessMatch(
    const WCHAR *boxname, const WCHAR *imageName, const WCHAR *appPath,
    unsigned long appPathLen, int useRuleExtensions, WCHAR *outTarget, size_t outTargetCch, int *outHasTarget)
{
    return ProgramControl_FindProcessSettingMatch(
        boxname, L"BreakoutProcess", imageName, appPath, appPathLen,
        1, useRuleExtensions, NULL, NULL, outTarget, outTargetCch, outHasTarget, NULL, NULL);
}

static __inline int ProgramControl_GetExactStringSettingPriority(
    const WCHAR *boxname, const WCHAR *setting, const WCHAR *string,
    int useRuleExtensions, BOOLEAN *outHasPriority, LONG *outPriority)
{
    WCHAR buf[CONF_LINE_LEN];
    unsigned long index = 0;
    BOOLEAN matched = FALSE;
    BOOLEAN hasPriority = FALSE;
    LONG bestPriority = -1;

    if (outHasPriority)
        *outHasPriority = FALSE;
    if (outPriority)
        *outPriority = -1;

    if (!boxname || !setting || !string || !*string)
        return FALSE;

    while (1) {
        NTSTATUS status = SbieApi_QueryConfAsIs(boxname, setting, index, buf, sizeof(buf) - sizeof(WCHAR));
        SBIE_NORMALIZED_RULE rule;

        ++index;
        if (!NT_SUCCESS(status)) {
            if (status == STATUS_BUFFER_TOO_SMALL)
                continue;
            break;
        }

        if (!ProgramControl_ParseRuleExtensionsInPlace(buf, &rule, useRuleExtensions))
            continue;

        if (_wcsicmp(rule.base_rule, string) != 0)
            continue;

        matched = TRUE;
        if (rule.has_priority && (!hasPriority || rule.priority < bestPriority)) {
            hasPriority = TRUE;
            bestPriority = rule.priority;
        }
    }

    if (hasPriority) {
        if (outHasPriority)
            *outHasPriority = TRUE;
        if (outPriority)
            *outPriority = bestPriority;
    }

    return matched;
}

static __inline void ProgramControl_GetSettingMinPriority(
    const WCHAR *boxname, const WCHAR *setting,
    int useRuleExtensions, BOOLEAN *outHasPriority, LONG *outPriority)
{
    WCHAR buf[CONF_LINE_LEN];
    unsigned long index = 0;
    BOOLEAN hasPriority = FALSE;
    LONG bestPriority = -1;

    if (outHasPriority)
        *outHasPriority = FALSE;
    if (outPriority)
        *outPriority = -1;

    if (!boxname || !setting)
        return;

    while (1) {
        NTSTATUS status = SbieApi_QueryConfAsIs(boxname, setting, index, buf, sizeof(buf) - sizeof(WCHAR));
        SBIE_NORMALIZED_RULE rule;

        ++index;
        if (!NT_SUCCESS(status)) {
            if (status == STATUS_BUFFER_TOO_SMALL)
                continue;
            break;
        }

        if (!ProgramControl_ParseRuleExtensionsInPlace(buf, &rule, useRuleExtensions))
            continue;

        if (rule.has_priority && (!hasPriority || rule.priority < bestPriority)) {
            hasPriority = TRUE;
            bestPriority = rule.priority;
        }
    }

    if (hasPriority) {
        if (outHasPriority)
            *outHasPriority = TRUE;
        if (outPriority)
            *outPriority = bestPriority;
    }
}

static __inline int ProgramControl_IsPathInHome(const WCHAR *appPath)
{
    WCHAR homePath[MAX_PATH];
    ULONG homeLen;

    if (!appPath || !*appPath)
        return 0;

    if (!NT_SUCCESS(SbieApi_GetHomePath(NULL, 0, homePath, MAX_PATH)))
        return 0;

    homeLen = (ULONG)wcslen(homePath);
    if (!homeLen)
        return 0;

    if (_wcsnicmp(appPath, homePath, homeLen) != 0)
        return 0;

    return (appPath[homeLen] == L'\0' || appPath[homeLen] == L'\\' || appPath[homeLen] == L'/');
}

#endif

#endif

