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

#ifdef KERNEL_MODE
#include "core/drv/my_winnt.h"
#else
#include <windows.h>
#include <string.h>
#include <wchar.h>
#endif

#include "list.h"

#ifndef PROGRAM_CONTROL_RULE_NO_CRT
#define PROGRAM_CONTROL_RUNTIME_TEMP_NO_CRT
#define PROGRAM_CONTROL_RULE_NO_CRT
#endif

#include "program_control_runtime.h"

#define PATTERN XPATTERN
#include "pattern.h"
#undef PATTERN

#ifdef PROGRAM_CONTROL_RUNTIME_TEMP_NO_CRT
#undef PROGRAM_CONTROL_RULE_NO_CRT
#undef PROGRAM_CONTROL_RUNTIME_TEMP_NO_CRT
#endif

static const WCHAR *ProgramControl_RuntimeCanonicalSettingName(const WCHAR *setting)
{
    if (!setting)
        return NULL;

    if (_wcsicmp(setting, L"ForceProcess") == 0)
        return L"ForceProcess";
    if (_wcsicmp(setting, L"ForceFolder") == 0)
        return L"ForceFolder";
    if (_wcsicmp(setting, L"ForceChildren") == 0)
        return L"ForceChildren";
    if (_wcsicmp(setting, L"BreakoutProcess") == 0)
        return L"BreakoutProcess";
    if (_wcsicmp(setting, L"BreakoutFolder") == 0)
        return L"BreakoutFolder";
    if (_wcsicmp(setting, L"BreakoutDocument") == 0)
        return L"BreakoutDocument";

    return setting;
}

static WCHAR *ProgramControl_RuntimeDupString(POOL *pool, const WCHAR *text)
{
    size_t len;
    size_t bytes;
    WCHAR *copy;

    if (!pool || !text)
        return NULL;

    len = wcslen(text);
    bytes = (len + 1) * sizeof(WCHAR);

    copy = (WCHAR *)Pool_Alloc(pool, (ULONG)bytes);
    if (!copy)
        return NULL;

    memcpy(copy, text, bytes);
    return copy;
}

static SBIE_RT_RULE_LIST *ProgramControl_RuntimeSelectRuleList(
    SBIE_RT_RULESET *ruleset,
    const WCHAR *setting)
{
    if (!ruleset || !setting)
        return NULL;

    if (_wcsicmp(setting, L"ForceProcess") == 0)
        return &ruleset->force_process;
    if (_wcsicmp(setting, L"ForceFolder") == 0)
        return &ruleset->force_folder;
    if (_wcsicmp(setting, L"ForceChildren") == 0)
        return &ruleset->force_children;
    if (_wcsicmp(setting, L"BreakoutProcess") == 0)
        return &ruleset->breakout_process;
    if (_wcsicmp(setting, L"BreakoutFolder") == 0)
        return &ruleset->breakout_folder;
    if (_wcsicmp(setting, L"BreakoutDocument") == 0)
        return &ruleset->breakout_document;

    return NULL;
}

static int ProgramControl_RuntimeAppendRule(
    SBIE_RT_RULESET *ruleset,
    SBIE_RT_RULE_LIST *list,
    const SBIE_RT_RULE *rule)
{
    SBIE_RT_RULE *newRules;
    ULONG newCapacity;
    size_t bytes;

    if (!ruleset || !ruleset->pool || !list || !rule)
        return 0;

    if (list->count == list->capacity) {
        newCapacity = list->capacity ? (list->capacity * 2) : 4;
        bytes = (size_t)newCapacity * sizeof(SBIE_RT_RULE);

        newRules = (SBIE_RT_RULE *)Pool_Alloc(ruleset->pool, (ULONG)bytes);
        if (!newRules)
            return 0;

        if (list->rules && list->count)
            memcpy(newRules, list->rules, (size_t)list->count * sizeof(SBIE_RT_RULE));

        list->rules = newRules;
        list->capacity = newCapacity;
    }

    list->rules[list->count] = *rule;
    ++list->count;
    return 1;
}

static int ProgramControl_RuntimeShouldIgnoreBroadBreakoutProcessRule(
    const WCHAR *setting,
    const SBIE_NORMALIZED_RULE *rule)
{
    if (!setting || !rule || !rule->base_rule)
        return 0;

    if (_wcsicmp(setting, L"BreakoutProcess") != 0)
        return 0;

    if (!(wcschr(rule->base_rule, L'*') || wcschr(rule->base_rule, L'?')))
        return 0;

    if (ProgramControl_RuleLooksLikePath(rule->base_rule))
        return 0;

    return ProgramControl_IsBroadWildcardImageRule(rule->base_rule) ? 1 : 0;
}

static int ProgramControl_RuntimeCompileBackend(
    POOL *pool,
    SBIE_RT_RULE *rule)
{
    const WCHAR *baseRule;
    int hasWildcard;

    if (!pool || !rule || !rule->normalized.base_rule || !*rule->normalized.base_rule)
        return 0;

    baseRule = rule->normalized.base_rule;
    hasWildcard = (wcschr(baseRule, L'*') || wcschr(baseRule, L'?')) ? 1 : 0;

    rule->backend_kind = SBIE_RT_BACKEND_NONE;
    rule->backend_state = NULL;

    if (rule->rule_kind == SBIE_RULE_KIND_PROCESS) {
        if (!ProgramControl_RuleLooksLikePath(baseRule)) {
            rule->backend_kind = hasWildcard ? SBIE_RT_BACKEND_IMAGE_WILDCARD : SBIE_RT_BACKEND_EXACT_STRING;
            return 1;
        }
    }

    if (hasWildcard) {
        rule->backend_kind = SBIE_RT_BACKEND_PATH_PATTERN;
        rule->backend_state = Pattern_Create(pool, baseRule, TRUE, 0);
        return (rule->backend_state != NULL) ? 1 : 0;
    }

    rule->backend_kind = SBIE_RT_BACKEND_EXACT_STRING;
    return 1;
}

static int ProgramControl_RuntimeMatchPatternLower(
    const SBIE_RT_RULESET *ruleset,
    XPATTERN *pattern,
    const WCHAR *text,
    unsigned long textLen)
{
    WCHAR stackText[512];
    size_t bytes;
    WCHAR *lowerText;
    unsigned long i;
    int matched;
    int usedPool = 0;

    if (!ruleset || !pattern || !text)
        return 0;

    if (!textLen)
        textLen = (unsigned long)wcslen(text);

    bytes = ((size_t)textLen + 1) * sizeof(WCHAR);
    if ((textLen + 1) <= (sizeof(stackText) / sizeof(stackText[0])))
        lowerText = stackText;
    else {
        lowerText = (WCHAR *)Pool_Alloc(ruleset->pool, (ULONG)bytes);
        if (!lowerText)
            return 0;
        usedPool = 1;
    }

    for (i = 0; i < textLen; ++i)
        lowerText[i] = text[i];
    lowerText[textLen] = L'\0';
    _wcslwr(lowerText);

    matched = Pattern_Match(pattern, lowerText, (int)textLen) ? 1 : 0;

    if (usedPool)
        Pool_Free(lowerText, (ULONG)bytes);

    return matched;
}

static int ProgramControl_RuntimeMatchProcessRule(
    const SBIE_RT_RULESET *ruleset,
    const SBIE_RT_RULE *rule,
    const WCHAR *imageName,
    const WCHAR *imagePath,
    unsigned long imagePathLen)
{
    if (!rule || !rule->normalized.base_rule || !imageName || !*imageName || !imagePath || !*imagePath)
        return 0;

    if (!imagePathLen)
        imagePathLen = (unsigned long)wcslen(imagePath);

    switch (rule->backend_kind) {
    case SBIE_RT_BACKEND_EXACT_STRING:
    case SBIE_RT_BACKEND_IMAGE_WILDCARD:
        return ProgramControl_MatchProcessRule(rule->normalized.base_rule, imageName, imagePath, imagePathLen);
    case SBIE_RT_BACKEND_PATH_PATTERN:
        return ProgramControl_RuntimeMatchPatternLower(ruleset, (XPATTERN *)rule->backend_state, imagePath, imagePathLen);
    default:
        return 0;
    }
}

static int ProgramControl_RuntimeMatchFolderRule(
    const SBIE_RT_RULESET *ruleset,
    const SBIE_RT_RULE *rule,
    const WCHAR *path,
    unsigned long pathLen)
{
    if (!rule || !rule->normalized.base_rule || !path || !*path)
        return 0;

    if (!pathLen)
        pathLen = (unsigned long)wcslen(path);

    switch (rule->backend_kind) {
    case SBIE_RT_BACKEND_EXACT_STRING:
        return ProgramControl_MatchFolderRuleNormalized(&rule->normalized, path, pathLen);
    case SBIE_RT_BACKEND_PATH_PATTERN:
        if (!ProgramControl_RuntimeMatchPatternLower(
                ruleset,
                (XPATTERN *)rule->backend_state,
                path,
                (path[pathLen] == L'\\') ? (unsigned long)wcslen(path) : pathLen) &&
            !ProgramControl_RuntimeMatchPatternLower(ruleset, (XPATTERN *)rule->backend_state, path, pathLen))
            return 0;
        if (rule->normalized.has_recursive)
            return ProgramControl_MatchFolderRuleNormalized(&rule->normalized, path, pathLen);
        return 1;
    default:
        return 0;
    }
}

static int ProgramControl_RuntimeMatchDocumentRule(
    const SBIE_RT_RULESET *ruleset,
    const SBIE_RT_RULE *rule,
    const WCHAR *path,
    unsigned long pathLen)
{
    if (!rule || !rule->normalized.base_rule || !path || !*path)
        return 0;

    if (!pathLen)
        pathLen = (unsigned long)wcslen(path);

    switch (rule->backend_kind) {
    case SBIE_RT_BACKEND_EXACT_STRING:
    case SBIE_RT_BACKEND_IMAGE_WILDCARD:
        return ProgramControl_MatchDocumentRule(rule->normalized.base_rule, path, pathLen);
    case SBIE_RT_BACKEND_PATH_PATTERN:
        return ProgramControl_RuntimeMatchPatternLower(ruleset, (XPATTERN *)rule->backend_state, path, pathLen);
    default:
        return 0;
    }
}

static int ProgramControl_RuntimeMatchScope(
    const SBIE_RT_RULE *rule,
    const WCHAR *imageName,
    BreakoutMatchImageFn matchImage,
    void *matchContext,
    unsigned long *outLevel)
{
    int matched;

    if (outLevel)
        *outLevel = 2;

    if (!rule)
        return 0;

    if (!rule->has_scope)
        return 1;

    if (!imageName || !*imageName || !matchImage || !rule->scope_rule || !*rule->scope_rule)
        return 0;

    matched = matchImage(rule->scope_rule, imageName, matchContext) ? 1 : 0;
    if (rule->scope_invert)
        matched = !matched;

    if (matched && outLevel)
        *outLevel = (unsigned long)rule->scope_level;

    return matched;
}

static void ProgramControl_RuntimeStoreMatch(
    SBIE_RT_MATCH *match,
    const SBIE_RT_RULE *rule,
    unsigned long level)
{
    if (!match || !rule)
        return;

    match->matched = 1;
    match->rule = rule;
    match->base_rule = rule->normalized.base_rule;
    match->target_box = rule->normalized.target_box;
    match->has_target_box = rule->normalized.has_target_box;
    match->has_priority = rule->normalized.has_priority;
    match->priority = rule->normalized.has_priority ? rule->normalized.priority : -1;
    match->level = (long)level;
}

static void ProgramControl_RuntimeStoreTargetMatch(
    SBIE_RT_MATCH *match,
    const SBIE_RT_RULE *rule,
    unsigned long level)
{
    if (!match || !rule || !rule->normalized.has_target_box)
        return;

    match->target_matched = 1;
    match->target_rule = rule;
    match->target_candidate_box = rule->normalized.target_box;
    match->target_has_priority = rule->normalized.has_priority;
    match->target_priority = rule->normalized.has_priority ? rule->normalized.priority : -1;
    match->target_level = (long)level;
}

static int ProgramControl_RuntimeFindBestProcessMatch(
    const SBIE_RT_RULESET *ruleset,
    const SBIE_RT_RULE_LIST *list,
    const WCHAR *imageName,
    const WCHAR *imagePath,
    BreakoutMatchImageFn matchImage,
    void *matchContext,
    SBIE_RT_MATCH *outMatch)
{
    ULONG i;
    int hasMatch = 0;
    int bestHasPriority = 0;
    long bestPriority = -1;
    int hasTargetMatch = 0;
    int bestTargetHasPriority = 0;
    long bestTargetPriority = -1;
    unsigned long bestLevel = (unsigned long)-1;
    unsigned long bestTargetLevel = (unsigned long)-1;

    if (outMatch)
        ProgramControl_RuntimeInitMatch(outMatch);

    if (!ruleset || !list || !imageName || !*imageName || !imagePath || !*imagePath)
        return 0;

    for (i = 0; i < list->count; ++i) {
        const SBIE_RT_RULE *rule = &list->rules[i];
        unsigned long level = 2;

        if (!ProgramControl_RuntimeMatchProcessRule(ruleset, rule, imageName, imagePath, 0))
            continue;

        if (!ProgramControl_RuntimeMatchScope(
                rule, imageName, matchImage, matchContext, &level))
            continue;

        if (rule->normalized.has_target_box &&
            ProgramControl_ShouldReplaceTargetMatch(
                hasTargetMatch,
                bestTargetHasPriority,
                bestTargetPriority,
                bestTargetLevel,
                rule->normalized.has_priority,
                rule->normalized.priority,
                level)) {
            hasTargetMatch = 1;
            bestTargetHasPriority = rule->normalized.has_priority;
            bestTargetPriority = rule->normalized.has_priority ? rule->normalized.priority : -1;
            bestTargetLevel = level;
            if (outMatch)
                ProgramControl_RuntimeStoreTargetMatch(outMatch, rule, level);
        }

        if (!ProgramControl_ShouldReplaceTargetMatch(
                hasMatch,
                bestHasPriority,
                bestPriority,
                bestLevel,
                rule->normalized.has_priority,
                rule->normalized.priority,
                level)) {
            continue;
        }

        hasMatch = 1;
        bestHasPriority = rule->normalized.has_priority;
        bestPriority = rule->normalized.has_priority ? rule->normalized.priority : -1;
        bestLevel = level;

        if (outMatch)
            ProgramControl_RuntimeStoreMatch(outMatch, rule, level);
    }

    return hasMatch;
}

static int ProgramControl_RuntimeFindBestFolderMatch(
    const SBIE_RT_RULESET *ruleset,
    const SBIE_RT_RULE_LIST *list,
    const WCHAR *imageName,
    BreakoutMatchImageFn matchImage,
    void *matchContext,
    const WCHAR *path,
    unsigned long pathLen,
    SBIE_RT_MATCH *outMatch)
{
    ULONG i;
    int hasMatch = 0;
    int bestHasPriority = 0;
    long bestPriority = -1;
    unsigned long bestLevel = (unsigned long)-1;
    int hasTargetMatch = 0;
    int bestTargetHasPriority = 0;
    long bestTargetPriority = -1;
    unsigned long bestTargetLevel = (unsigned long)-1;

    if (outMatch)
        ProgramControl_RuntimeInitMatch(outMatch);

    if (!ruleset || !list || !path || !*path)
        return 0;

    for (i = 0; i < list->count; ++i) {
        const SBIE_RT_RULE *rule = &list->rules[i];
        unsigned long level = 2;

        if (!ProgramControl_RuntimeMatchScope(rule, imageName, matchImage, matchContext, &level))
            continue;

        if (!ProgramControl_RuntimeMatchFolderRule(ruleset, rule, path, pathLen))
            continue;

        if (rule->normalized.has_target_box &&
            ProgramControl_ShouldReplaceTargetMatch(
                hasTargetMatch,
                bestTargetHasPriority,
                bestTargetPriority,
                bestTargetLevel,
                rule->normalized.has_priority,
                rule->normalized.priority,
                level)) {
            hasTargetMatch = 1;
            bestTargetHasPriority = rule->normalized.has_priority;
            bestTargetPriority = rule->normalized.has_priority ? rule->normalized.priority : -1;
            bestTargetLevel = level;
            if (outMatch)
                ProgramControl_RuntimeStoreTargetMatch(outMatch, rule, level);
        }

        if (!ProgramControl_ShouldReplaceTargetMatch(
                hasMatch,
                bestHasPriority,
                bestPriority,
                bestLevel,
                rule->normalized.has_priority,
                rule->normalized.priority,
                level)) {
            continue;
        }

        hasMatch = 1;
        bestHasPriority = rule->normalized.has_priority;
        bestPriority = rule->normalized.has_priority ? rule->normalized.priority : -1;
        bestLevel = level;

        if (outMatch)
            ProgramControl_RuntimeStoreMatch(outMatch, rule, level);
    }

    return hasMatch;
}

static int ProgramControl_RuntimeFindBestDocumentMatch(
    const SBIE_RT_RULESET *ruleset,
    const SBIE_RT_RULE_LIST *list,
    const WCHAR *imageName,
    BreakoutMatchImageFn matchImage,
    void *matchContext,
    const WCHAR *path,
    unsigned long pathLen,
    SBIE_RT_MATCH *outMatch)
{
    ULONG i;
    int hasMatch = 0;
    int bestHasPriority = 0;
    long bestPriority = -1;
    unsigned long bestLevel = (unsigned long)-1;
    int hasTargetMatch = 0;
    int bestTargetHasPriority = 0;
    long bestTargetPriority = -1;
    unsigned long bestTargetLevel = (unsigned long)-1;

    if (outMatch)
        ProgramControl_RuntimeInitMatch(outMatch);

    if (!ruleset || !list || !path || !*path)
        return 0;

    for (i = 0; i < list->count; ++i) {
        const SBIE_RT_RULE *rule = &list->rules[i];
        unsigned long level = 2;

        if (!ProgramControl_RuntimeMatchScope(rule, imageName, matchImage, matchContext, &level))
            continue;

        if (!ProgramControl_RuntimeMatchDocumentRule(ruleset, rule, path, pathLen))
            continue;

        if (rule->normalized.has_target_box &&
            ProgramControl_ShouldReplaceTargetMatch(
                hasTargetMatch,
                bestTargetHasPriority,
                bestTargetPriority,
                bestTargetLevel,
                rule->normalized.has_priority,
                rule->normalized.priority,
                level)) {
            hasTargetMatch = 1;
            bestTargetHasPriority = rule->normalized.has_priority;
            bestTargetPriority = rule->normalized.has_priority ? rule->normalized.priority : -1;
            bestTargetLevel = level;
            if (outMatch)
                ProgramControl_RuntimeStoreTargetMatch(outMatch, rule, level);
        }

        if (!ProgramControl_ShouldReplaceTargetMatch(
                hasMatch,
                bestHasPriority,
                bestPriority,
                bestLevel,
                rule->normalized.has_priority,
                rule->normalized.priority,
                level)) {
            continue;
        }

        hasMatch = 1;
        bestHasPriority = rule->normalized.has_priority;
        bestPriority = rule->normalized.has_priority ? rule->normalized.priority : -1;
        bestLevel = level;

        if (outMatch)
            ProgramControl_RuntimeStoreMatch(outMatch, rule, level);
    }

    return hasMatch;
}

void ProgramControl_RuntimeInitMatch(SBIE_RT_MATCH *match)
{
    if (!match)
        return;

    memset(match, 0, sizeof(*match));
    match->priority = -1;
    match->level = -1;
    match->target_priority = -1;
    match->target_level = -1;
}

int ProgramControl_RuntimeGetApplicableTargetBox(
    const SBIE_RT_MATCH *match,
    const WCHAR **outTargetBox)
{
    if (outTargetBox)
        *outTargetBox = NULL;

    if (!match || !match->matched || !match->target_matched ||
        !match->target_candidate_box || !*match->target_candidate_box)
        return 0;

    if (!ProgramControl_ShouldAcceptTargetChoice(
            match->matched,
            match->has_priority,
            match->priority,
            (unsigned long)match->level,
            match->target_matched,
            match->target_has_priority,
            match->target_priority,
            (unsigned long)match->target_level))
        return 0;

    if (outTargetBox)
        *outTargetBox = match->target_candidate_box;
    return 1;
}

void ProgramControl_RuntimeInitRuleset(SBIE_RT_RULESET *ruleset, POOL *pool)
{
    if (!ruleset)
        return;

    memset(ruleset, 0, sizeof(*ruleset));
    ruleset->pool = pool;
}

void ProgramControl_RuntimeFreeRuleset(SBIE_RT_RULESET *ruleset)
{
    SBIE_RT_RULE_LIST *lists[6];
    int i;
    ULONG j;

    if (!ruleset)
        return;

    lists[0] = &ruleset->force_process;
    lists[1] = &ruleset->force_folder;
    lists[2] = &ruleset->force_children;
    lists[3] = &ruleset->breakout_process;
    lists[4] = &ruleset->breakout_folder;
    lists[5] = &ruleset->breakout_document;

    for (i = 0; i < 6; ++i) {
        SBIE_RT_RULE_LIST *list = lists[i];

        if (!list || !list->rules)
            continue;

        for (j = 0; j < list->count; ++j) {
            SBIE_RT_RULE *rule = &list->rules[j];
            if (rule->backend_kind == SBIE_RT_BACKEND_PATH_PATTERN && rule->backend_state) {
                Pattern_Free((XPATTERN *)rule->backend_state);
                rule->backend_state = NULL;
            }
        }

        list->rules = NULL;
        list->count = 0;
        list->capacity = 0;
    }

    ruleset->pool = NULL;
}

int ProgramControl_RuntimeCompileSetting(
    SBIE_RT_RULESET *ruleset,
    const WCHAR *setting,
    const WCHAR *value,
    int useRuleExtensions)
{
    SBIE_PROGRAM_RULE_KIND ruleKind;
    SBIE_RT_RULE_LIST *list;
    SBIE_RT_RULE rule;
    WCHAR *storage;
    WCHAR *valuePart;
    unsigned long scopeLevel = 2;

    if (!ruleset || !ruleset->pool || !setting || !value || !*value)
        return 0;

    if (!ProgramControl_GetRuleKindForSetting(setting, &ruleKind))
        return 0;

    list = ProgramControl_RuntimeSelectRuleList(ruleset, setting);
    if (!list)
        return 0;

    memset(&rule, 0, sizeof(rule));
    rule.rule_kind = ruleKind;
    rule.setting_name = ProgramControl_RuntimeCanonicalSettingName(setting);
    rule.scope_level = 2;

    storage = ProgramControl_RuntimeDupString(ruleset->pool, value);
    if (!storage)
        return 0;

    rule.raw_rule = storage;
    valuePart = storage;
    if (ruleKind != SBIE_RULE_KIND_PROCESS || _wcsicmp(setting, L"BreakoutProcess") == 0) {
        valuePart = ProgramControl_ParseImageScopeInPlace(
            storage,
            &rule.scope_rule,
            &rule.has_scope,
            &rule.scope_invert,
            &scopeLevel);
        rule.scope_level = (long)scopeLevel;
    }

    if (!valuePart || !*valuePart)
        return 0;

    if (!ProgramControl_ParseRuleExtensionsInPlace(valuePart, &rule.normalized, useRuleExtensions))
        return 0;

    if (ProgramControl_RuntimeShouldIgnoreBroadBreakoutProcessRule(setting, &rule.normalized))
        return 1;

    if (!ProgramControl_RuntimeCompileBackend(ruleset->pool, &rule))
        return 0;

    return ProgramControl_RuntimeAppendRule(ruleset, list, &rule);
}

int ProgramControl_RuntimeMatchProcess(
    const SBIE_RT_RULESET *ruleset,
    const WCHAR *image_name,
    const WCHAR *image_path,
    BreakoutMatchImageFn match_image,
    void *match_context,
    SBIE_RT_MATCH *force_process_match,
    SBIE_RT_MATCH *force_children_match,
    SBIE_RT_MATCH *breakout_process_match)
{
    int anyMatch = 0;

    if (force_process_match)
        ProgramControl_RuntimeInitMatch(force_process_match);
    if (force_children_match)
        ProgramControl_RuntimeInitMatch(force_children_match);
    if (breakout_process_match)
        ProgramControl_RuntimeInitMatch(breakout_process_match);

    if (!ruleset || !image_name || !*image_name || !image_path || !*image_path)
        return 0;

    if (ProgramControl_RuntimeFindBestProcessMatch(ruleset, &ruleset->force_process, image_name, image_path, match_image, match_context, force_process_match))
        anyMatch = 1;
    if (ProgramControl_RuntimeFindBestProcessMatch(ruleset, &ruleset->force_children, image_name, image_path, match_image, match_context, force_children_match))
        anyMatch = 1;
    if (ProgramControl_RuntimeFindBestProcessMatch(ruleset, &ruleset->breakout_process, image_name, image_path, match_image, match_context, breakout_process_match))
        anyMatch = 1;

    return anyMatch;
}

int ProgramControl_RuntimeMatchFolder(
    const SBIE_RT_RULESET *ruleset,
    const WCHAR *image_name,
    BreakoutMatchImageFn match_image,
    void *match_context,
    const WCHAR *path,
    unsigned long path_len,
    SBIE_RT_MATCH *force_folder_match,
    SBIE_RT_MATCH *breakout_folder_match)
{
    int anyMatch = 0;

    if (force_folder_match)
        ProgramControl_RuntimeInitMatch(force_folder_match);
    if (breakout_folder_match)
        ProgramControl_RuntimeInitMatch(breakout_folder_match);

    if (!ruleset || !path || !*path)
        return 0;

    if (ProgramControl_RuntimeFindBestFolderMatch(
            ruleset,
            &ruleset->force_folder,
            image_name,
            match_image,
            match_context,
            path,
            path_len,
            force_folder_match)) {
        anyMatch = 1;
    }

    if (ProgramControl_RuntimeFindBestFolderMatch(
            ruleset,
            &ruleset->breakout_folder,
            image_name,
            match_image,
            match_context,
            path,
            path_len,
            breakout_folder_match)) {
        anyMatch = 1;
    }

    return anyMatch;
}

int ProgramControl_RuntimeMatchDocument(
    const SBIE_RT_RULESET *ruleset,
    const WCHAR *image_name,
    BreakoutMatchImageFn match_image,
    void *match_context,
    const WCHAR *path,
    unsigned long path_len,
    SBIE_RT_MATCH *breakout_document_match)
{
    if (breakout_document_match)
        ProgramControl_RuntimeInitMatch(breakout_document_match);

    if (!ruleset || !path || !*path)
        return 0;

    return ProgramControl_RuntimeFindBestDocumentMatch(
        ruleset,
        &ruleset->breakout_document,
        image_name,
        match_image,
        match_context,
        path,
        path_len,
        breakout_document_match);
}
