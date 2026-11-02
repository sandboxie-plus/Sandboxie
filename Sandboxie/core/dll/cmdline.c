/*
 * Copyright 2021-2026 David Xanatos, xanasoft.com
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

#define NOGDI
#include "dll.h"
#include "cmdline.h"
#include "common/pattern.h"

#define CMDLINE_SETTING                 L"CustomProcessCommandLine"
#define CMDLINE_CHROMIUM_SETTING        L"CustomChromiumFlags"
#define CMDLINE_GLOBAL_SECTION          L"GlobalSettings"
#define CMDLINE_MAX_RULES               128
#define CMDLINE_MAX_CONDITIONS          8
#define CMDLINE_MAX_ARGUMENTS           512

#define CMD_SEEN_DISABLED               0x0001
#define CMD_SEEN_STOP                   0x0002
#define CMD_SEEN_MATCH                  0x0004
#define CMD_SEEN_OCCURRENCES            0x0008
#define CMD_SEEN_DUPLICATE              0x0010
#define CMD_SEEN_CASE_SENSITIVE         0x0020
#define CMD_SEEN_CONDITION_SOURCE       0x0040
#define CMD_SEEN_UPDATE_PEB             0x0080

typedef enum {
    CMD_ACTION_NONE,
    CMD_ACTION_CLEAR,
    CMD_ACTION_SET,
    CMD_ACTION_REMOVE,
    CMD_ACTION_REMOVE_SEQUENCE,
    CMD_ACTION_REPLACE,
    CMD_ACTION_REPLACE_SEQUENCE,
    CMD_ACTION_ADD
} CMD_ACTION;

typedef enum {
    CMD_MATCH_EXACT,
    CMD_MATCH_PATTERN,
    CMD_MATCH_TEXT
} CMD_MATCH;

typedef enum {
    CMD_OCCURRENCES_ALL,
    CMD_OCCURRENCES_FIRST,
    CMD_OCCURRENCES_LAST
} CMD_OCCURRENCES;

typedef enum {
    CMD_ADD_END,
    CMD_ADD_START,
    CMD_ADD_BEFORE,
    CMD_ADD_AFTER
} CMD_ADD_POSITION;

typedef enum {
    CMD_PEB_UPDATE,
    CMD_PEB_SKIP,
    CMD_PEB_KEEP_ORIGINAL
} CMD_PEB_MODE;

typedef struct {
    ULONG prefix_start;
    ULONG raw_start;
    ULONG raw_end;
    BOOLEAN unterminated_quote;
    WCHAR* value;
} CMD_ARGUMENT;

typedef enum {
    CMD_CONDITION_GENERIC,
    CMD_CONDITION_ARGUMENT,
    CMD_CONDITION_SEQUENCE
} CMD_CONDITION_KIND;

typedef struct {
    WCHAR* value;
    CMD_CONDITION_KIND kind;
} CMD_CONDITION;

typedef struct {
    WCHAR* storage;
    WCHAR* selector;
    WCHAR* id;
    WCHAR* value;
    WCHAR* replacement;
    WCHAR* relative;
    CMD_CONDITION conditions[CMDLINE_MAX_CONDITIONS];
    CMD_CONDITION skips[CMDLINE_MAX_CONDITIONS];
    ULONG condition_count;
    ULONG skip_count;
    LONG order;
    CMD_ACTION action;
    CMD_MATCH match;
    CMD_ADD_POSITION position;
    CMD_OCCURRENCES occurrences;
    CMD_PEB_MODE peb_mode;
    BOOLEAN explicit_order;
    BOOLEAN disabled;
    BOOLEAN stop;
    BOOLEAN duplicate_allow;
    BOOLEAN selector_match;
    BOOLEAN case_sensitive;
    BOOLEAN condition_input;
    BOOLEAN applied;
} CMD_RULE;

typedef struct {
    WCHAR* text;
    CMD_ARGUMENT* args;
    ULONG count;
    ULONG argv0_end;
    ULONG input_count;
} CMD_STATE;

extern POOL* Dll_PoolTemp;

static BOOLEAN CmdLine_ParseCountCondition(
    const WCHAR* condition, const WCHAR* name, ULONG count,
    BOOLEAN* result);

static BOOLEAN CmdLine_IdSeen(
    const CMD_RULE* rules, ULONG count, const WCHAR* id, BOOLEAN applied_only);

static WCHAR* CmdLine_DuplicateLength(const WCHAR* text, size_t length);

static void CmdLine_Trim(WCHAR* text)
{
    WCHAR* start;
    size_t len;

    if (!text)
        return;

    start = text;
    while (*start == L' ' || *start == L'\t')
        ++start;

    if (start != text)
        memmove(text, start, (wcslen(start) + 1) * sizeof(WCHAR));

    len = wcslen(text);
    while (len && (text[len - 1] == L' ' || text[len - 1] == L'\t'))
        text[--len] = L'\0';
}

static WCHAR* CmdLine_Duplicate(const WCHAR* text)
{
    return text ? CmdLine_DuplicateLength(text, wcslen(text)) : NULL;
}

static WCHAR* CmdLine_DuplicateLength(const WCHAR* text, size_t length)
{
    WCHAR* copy;

    if (!text)
        return NULL;

    copy = LocalAlloc(LMEM_FIXED, (length + 1) * sizeof(WCHAR));
    if (copy) {
        wmemcpy(copy, text, length);
        copy[length] = L'\0';
    }
    return copy;
}

static WCHAR* CmdLine_NextField(WCHAR** cursor)
{
    WCHAR* field;
    WCHAR* read;
    WCHAR* write;

    if (!cursor || !*cursor)
        return NULL;

    field = *cursor;
    read = field;
    write = field;

    while (*read) {
        if (*read == L';') {
            if (read[1] == L';') {
                *write++ = L';';
                read += 2;
                continue;
            }
            ++read;
            break;
        }
        *write++ = *read++;
    }

    *write = L'\0';
    *cursor = *read ? read : NULL;
    CmdLine_Trim(field);
    return field;
}

static BOOLEAN CmdLine_ParseLong(const WCHAR* text, LONG* value)
{
    WCHAR* end;
    LONG parsed;

    if (!text || !*text || !value)
        return FALSE;

    parsed = wcstol(text, &end, 10);
    if (end == text || *end)
        return FALSE;

    *value = parsed;
    return TRUE;
}

static BOOLEAN CmdLine_ParseBoolean(const WCHAR* text, BOOLEAN* value)
{
    if (!text || !value)
        return FALSE;

    if (_wcsicmp(text, L"y") == 0 || _wcsicmp(text, L"yes") == 0 ||
            _wcsicmp(text, L"true") == 0 || wcscmp(text, L"1") == 0) {
        *value = TRUE;
        return TRUE;
    }

    if (_wcsicmp(text, L"n") == 0 || _wcsicmp(text, L"no") == 0 ||
            _wcsicmp(text, L"false") == 0 || wcscmp(text, L"0") == 0) {
        *value = FALSE;
        return TRUE;
    }

    return FALSE;
}

static BOOLEAN CmdLine_MatchSelector(const WCHAR* selector)
{
    BOOLEAN invert = FALSE;
    BOOLEAN match;
    ULONG len;

    if (!selector || !*selector)
        return FALSE;

    if (*selector == L'!') {
        invert = TRUE;
        ++selector;
    }

    len = (ULONG)wcslen(selector);
    if (!len)
        return FALSE;

    match = Config_MatchImageEx(
        selector, len, Dll_ImageName, 1, Dll_ImageType);

    return invert ? !match : match;
}

static LONG CmdLine_DefaultOrder(CMD_ACTION action)
{
    if (action == CMD_ACTION_CLEAR || action == CMD_ACTION_SET)
        return 100;
    if (action == CMD_ACTION_REMOVE || action == CMD_ACTION_REMOVE_SEQUENCE)
        return 200;
    if (action == CMD_ACTION_REPLACE || action == CMD_ACTION_REPLACE_SEQUENCE)
        return 300;
    return 400;
}

static BOOLEAN CmdLine_SetAction(
    CMD_RULE* rule, CMD_ACTION action, WCHAR* value)
{
    if (rule->action != CMD_ACTION_NONE)
        return FALSE;

    rule->action = action;
    rule->value = value;
    return TRUE;
}

static BOOLEAN CmdLine_ParseRule(WCHAR* text, CMD_RULE* rule)
{
    WCHAR* comma;
    WCHAR* cursor;
    WCHAR* field;
    ULONG seen = 0;
    ULONG index;
    BOOLEAN unused;

    memset(rule, 0, sizeof(*rule));
    rule->storage = text;
    rule->match = CMD_MATCH_EXACT;
    rule->position = CMD_ADD_END;
    rule->case_sensitive = TRUE;

    comma = wcschr(text, L',');
    if (!comma)
        return FALSE;

    *comma = L'\0';
    rule->selector = text;
    CmdLine_Trim(rule->selector);
    rule->selector_match = CmdLine_MatchSelector(rule->selector);
    cursor = comma + 1;

    while ((field = CmdLine_NextField(&cursor)) != NULL) {
        WCHAR* equal;
        WCHAR* key;
        WCHAR* value;

        if (!*field)
            continue;

        equal = wcschr(field, L'=');
        if (!equal)
            return FALSE;

        *equal = L'\0';
        key = field;
        value = equal + 1;
        CmdLine_Trim(key);
        CmdLine_Trim(value);

        if (!*key || !*value)
            return FALSE;

        if (_wcsicmp(key, L"Id") == 0) {
            if (rule->id)
                return FALSE;
            rule->id = value;
        }
        else if (_wcsicmp(key, L"Order") == 0) {
            if (rule->explicit_order ||
                    !CmdLine_ParseLong(value, &rule->order) ||
                    rule->order < 0 || rule->order > 9999)
                return FALSE;
            rule->explicit_order = TRUE;
        }
        else if (_wcsicmp(key, L"Disabled") == 0) {
            if ((seen & CMD_SEEN_DISABLED) ||
                    !CmdLine_ParseBoolean(value, &rule->disabled))
                return FALSE;
            seen |= CMD_SEEN_DISABLED;
        }
        else if (_wcsicmp(key, L"Stop") == 0) {
            if ((seen & CMD_SEEN_STOP) ||
                    !CmdLine_ParseBoolean(value, &rule->stop))
                return FALSE;
            seen |= CMD_SEEN_STOP;
        }
        else if (_wcsicmp(key, L"Clear") == 0) {
            if (_wcsicmp(value, L"Arguments") != 0 ||
                    !CmdLine_SetAction(rule, CMD_ACTION_CLEAR, value))
                return FALSE;
        }
        else if (_wcsicmp(key, L"Set") == 0) {
            if (!CmdLine_SetAction(rule, CMD_ACTION_SET, value))
                return FALSE;
        }
        else if (_wcsicmp(key, L"Remove") == 0) {
            if (!CmdLine_SetAction(rule, CMD_ACTION_REMOVE, value))
                return FALSE;
        }
        else if (_wcsicmp(key, L"RemoveSequence") == 0) {
            if (!CmdLine_SetAction(rule, CMD_ACTION_REMOVE_SEQUENCE, value))
                return FALSE;
        }
        else if (_wcsicmp(key, L"Replace") == 0) {
            if (!CmdLine_SetAction(rule, CMD_ACTION_REPLACE, value))
                return FALSE;
        }
        else if (_wcsicmp(key, L"ReplaceSequence") == 0) {
            if (!CmdLine_SetAction(rule, CMD_ACTION_REPLACE_SEQUENCE, value))
                return FALSE;
        }
        else if (_wcsicmp(key, L"With") == 0) {
            if (rule->replacement)
                return FALSE;
            rule->replacement = value;
        }
        else if (_wcsicmp(key, L"Add") == 0) {
            if (!CmdLine_SetAction(rule, CMD_ACTION_ADD, value))
                return FALSE;
        }
        else if (_wcsicmp(key, L"If") == 0 ||
                _wcsicmp(key, L"IfArg") == 0 ||
                _wcsicmp(key, L"IfSequence") == 0) {
            CMD_CONDITION* condition;

            if (rule->condition_count >= CMDLINE_MAX_CONDITIONS)
                return FALSE;
            condition = &rule->conditions[rule->condition_count++];
            condition->value = value;
            condition->kind = _wcsicmp(key, L"IfArg") == 0
                ? CMD_CONDITION_ARGUMENT : _wcsicmp(key, L"IfSequence") == 0
                ? CMD_CONDITION_SEQUENCE : CMD_CONDITION_GENERIC;
        }
        else if (_wcsicmp(key, L"Skip") == 0 ||
                _wcsicmp(key, L"SkipArg") == 0 ||
                _wcsicmp(key, L"SkipSequence") == 0) {
            CMD_CONDITION* condition;

            if (rule->skip_count >= CMDLINE_MAX_CONDITIONS)
                return FALSE;
            condition = &rule->skips[rule->skip_count++];
            condition->value = value;
            condition->kind = _wcsicmp(key, L"SkipArg") == 0
                ? CMD_CONDITION_ARGUMENT : _wcsicmp(key, L"SkipSequence") == 0
                ? CMD_CONDITION_SEQUENCE : CMD_CONDITION_GENERIC;
        }
        else if (_wcsicmp(key, L"ConditionSource") == 0) {
            if (seen & CMD_SEEN_CONDITION_SOURCE)
                return FALSE;
            if (_wcsicmp(value, L"Input") == 0)
                rule->condition_input = TRUE;
            else if (_wcsicmp(value, L"Current") != 0)
                return FALSE;
            seen |= CMD_SEEN_CONDITION_SOURCE;
        }
        else if (_wcsicmp(key, L"UpdatePEB") == 0) {
            if (seen & CMD_SEEN_UPDATE_PEB)
                return FALSE;
            if (_wcsicmp(value, L"y") == 0)
                rule->peb_mode = CMD_PEB_UPDATE;
            else if (_wcsicmp(value, L"n") == 0)
                rule->peb_mode = CMD_PEB_SKIP;
            else if (_wcsicmp(value, L"KeepOriginal") == 0)
                rule->peb_mode = CMD_PEB_KEEP_ORIGINAL;
            else
                return FALSE;
            seen |= CMD_SEEN_UPDATE_PEB;
        }
        else if (_wcsicmp(key, L"Match") == 0) {
            if (seen & CMD_SEEN_MATCH)
                return FALSE;
            if (_wcsicmp(value, L"Exact") == 0)
                rule->match = CMD_MATCH_EXACT;
            else if (_wcsicmp(value, L"Pattern") == 0)
                rule->match = CMD_MATCH_PATTERN;
            else if (_wcsicmp(value, L"Text") == 0)
                rule->match = CMD_MATCH_TEXT;
            else
                return FALSE;
            seen |= CMD_SEEN_MATCH;
        }
        else if (_wcsicmp(key, L"CaseSensitive") == 0) {
            if ((seen & CMD_SEEN_CASE_SENSITIVE) ||
                    !CmdLine_ParseBoolean(value, &rule->case_sensitive))
                return FALSE;
            seen |= CMD_SEEN_CASE_SENSITIVE;
        }
        else if (_wcsicmp(key, L"At") == 0) {
            if (rule->relative)
                return FALSE;
            if (_wcsicmp(value, L"Start") == 0)
                rule->position = CMD_ADD_START;
            else if (_wcsicmp(value, L"End") == 0)
                rule->position = CMD_ADD_END;
            else
                return FALSE;
            rule->relative = value;
        }
        else if (_wcsicmp(key, L"Before") == 0) {
            if (rule->relative)
                return FALSE;
            rule->position = CMD_ADD_BEFORE;
            rule->relative = value;
        }
        else if (_wcsicmp(key, L"After") == 0) {
            if (rule->relative)
                return FALSE;
            rule->position = CMD_ADD_AFTER;
            rule->relative = value;
        }
        else if (_wcsicmp(key, L"Occurrences") == 0) {
            if (seen & CMD_SEEN_OCCURRENCES)
                return FALSE;
            if (_wcsicmp(value, L"First") == 0)
                rule->occurrences = CMD_OCCURRENCES_FIRST;
            else if (_wcsicmp(value, L"Last") == 0)
                rule->occurrences = CMD_OCCURRENCES_LAST;
            else if (_wcsicmp(value, L"All") != 0)
                return FALSE;
            seen |= CMD_SEEN_OCCURRENCES;
        }
        else if (_wcsicmp(key, L"Duplicate") == 0) {
            if (seen & CMD_SEEN_DUPLICATE)
                return FALSE;
            if (_wcsicmp(value, L"Allow") == 0)
                rule->duplicate_allow = TRUE;
            else if (_wcsicmp(value, L"Skip") != 0)
                return FALSE;
            seen |= CMD_SEEN_DUPLICATE;
        }
        else {
            return FALSE;
        }
    }

    if (rule->disabled)
        return rule->id && rule->action == CMD_ACTION_NONE;

    if (rule->action == CMD_ACTION_NONE)
        return FALSE;
    if ((rule->action == CMD_ACTION_REPLACE ||
            rule->action == CMD_ACTION_REPLACE_SEQUENCE) && !rule->replacement)
        return FALSE;
    if (rule->action != CMD_ACTION_REPLACE &&
            rule->action != CMD_ACTION_REPLACE_SEQUENCE && rule->replacement)
        return FALSE;
    if (rule->action != CMD_ACTION_ADD && rule->relative)
        return FALSE;
    if ((seen & CMD_SEEN_OCCURRENCES) &&
            rule->action != CMD_ACTION_REMOVE &&
            rule->action != CMD_ACTION_REMOVE_SEQUENCE &&
            rule->action != CMD_ACTION_REPLACE &&
            rule->action != CMD_ACTION_REPLACE_SEQUENCE)
        return FALSE;
    if ((seen & CMD_SEEN_DUPLICATE) && rule->action != CMD_ACTION_ADD)
        return FALSE;

    for (index = 0; index < rule->condition_count; ++index) {
        WCHAR* condition = rule->conditions[index].value;
        if (rule->conditions[index].kind != CMD_CONDITION_GENERIC)
            continue;
        if (_wcsnicmp(condition, L"Id:", 3) == 0 && !condition[3])
            return FALSE;
        if ((_wcsnicmp(condition, L"InputArgCount", 13) == 0 &&
                !CmdLine_ParseCountCondition(
                    condition, L"InputArgCount", 0, &unused)) ||
            (_wcsnicmp(condition, L"ArgCount", 8) == 0 &&
                !CmdLine_ParseCountCondition(
                    condition, L"ArgCount", 0, &unused)))
            return FALSE;
    }
    for (index = 0; index < rule->skip_count; ++index) {
        WCHAR* condition = rule->skips[index].value;
        if (rule->skips[index].kind != CMD_CONDITION_GENERIC)
            continue;
        if (_wcsnicmp(condition, L"Id:", 3) == 0 && !condition[3])
            return FALSE;
        if ((_wcsnicmp(condition, L"InputArgCount", 13) == 0 &&
                !CmdLine_ParseCountCondition(
                    condition, L"InputArgCount", 0, &unused)) ||
            (_wcsnicmp(condition, L"ArgCount", 8) == 0 &&
                !CmdLine_ParseCountCondition(
                    condition, L"ArgCount", 0, &unused)))
            return FALSE;
    }

    if (!rule->explicit_order)
        rule->order = CmdLine_DefaultOrder(rule->action);

    return TRUE;
}

static WCHAR* CmdLine_DecodeArgument(
    const WCHAR* text, ULONG start, ULONG end)
{
    WCHAR* decoded;
    ULONG read = start;
    ULONG write = 0;
    BOOLEAN quoted = FALSE;

    decoded = LocalAlloc(LMEM_FIXED, (end - start + 1) * sizeof(WCHAR));
    if (!decoded)
        return NULL;

    while (read < end) {
        ULONG slashes = 0;

        while (read < end && text[read] == L'\\') {
            ++slashes;
            ++read;
        }

        if (read < end && text[read] == L'"') {
            ULONG i;
            for (i = 0; i < slashes / 2; ++i)
                decoded[write++] = L'\\';

            if (slashes & 1) {
                decoded[write++] = L'"';
            }
            else if (quoted && read + 1 < end && text[read + 1] == L'"') {
                decoded[write++] = L'"';
                ++read;
            }
            else {
                quoted = !quoted;
            }
            ++read;
            continue;
        }

        while (slashes--)
            decoded[write++] = L'\\';

        if (read < end)
            decoded[write++] = text[read++];
    }

    decoded[write] = L'\0';
    return decoded;
}

static void CmdLine_FreeArguments(CMD_STATE* state)
{
    ULONG i;

    if (!state || !state->args)
        return;

    for (i = 0; i < state->count; ++i) {
        if (state->args[i].value)
            LocalFree(state->args[i].value);
    }

    LocalFree(state->args);
    state->args = NULL;
    state->count = 0;
}

static BOOLEAN CmdLine_ParseArguments(CMD_STATE* state)
{
    ULONG length;
    ULONG pos;
    ULONG capacity;

    CmdLine_FreeArguments(state);

    length = (ULONG)wcslen(state->text);
    capacity = length / 2 + 2;
    if (capacity > CMDLINE_MAX_ARGUMENTS)
        capacity = CMDLINE_MAX_ARGUMENTS;
    state->args = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
        capacity * sizeof(CMD_ARGUMENT));
    if (!state->args)
        return FALSE;
    pos = 0;
    while (pos < length) {
        ULONG prefix;
        ULONG start;
        BOOLEAN quoted = FALSE;
        ULONG slashes = 0;
        CMD_ARGUMENT* arg;

        prefix = pos;
        while (pos < length && (state->text[pos] == L' ' ||
                state->text[pos] == L'\t'))
            ++pos;
        if (pos == length)
            break;
        if (state->count >= capacity) {
            SbieApi_MonitorPutMsg(MONITOR_OTHER | MONITOR_TRACE,
                L"CustomProcessCommandLine: argument limit exceeded");
            return FALSE;
        }

        start = pos;
        while (pos < length) {
            WCHAR ch = state->text[pos];

            if (ch == L'\\') {
                ++slashes;
                ++pos;
                continue;
            }

            if (ch == L'"') {
                if (!(slashes & 1))
                    quoted = !quoted;
                slashes = 0;
                ++pos;
                continue;
            }

            slashes = 0;
            if (!quoted && (ch == L' ' || ch == L'\t'))
                break;
            ++pos;
        }

        arg = &state->args[state->count++];
        arg->prefix_start = prefix;
        arg->raw_start = start;
        arg->raw_end = pos;
        arg->unterminated_quote = quoted;
        arg->value = CmdLine_DecodeArgument(state->text, start, pos);
        if (!arg->value)
            return FALSE;
    }

    if (!state->count)
        return FALSE;

    state->argv0_end = state->args[0].raw_end;
    return TRUE;
}

static BOOLEAN CmdLine_MatchPattern(
    const WCHAR* value, const WCHAR* pattern, CMD_MATCH mode,
    BOOLEAN case_sensitive, BOOLEAN* error)
{
    PATTERN* compiled;
    BOOLEAN match = FALSE;
    WCHAR* value_lwr = NULL;

    if (error)
        *error = FALSE;
    if (!value || !pattern)
        return FALSE;

    if (mode == CMD_MATCH_EXACT)
        return case_sensitive
            ? wcscmp(value, pattern) == 0
            : _wcsicmp(value, pattern) == 0;

    if (mode == CMD_MATCH_TEXT) {
        if (case_sensitive)
            return wcsstr(value, pattern) != NULL;
        while (*value) {
            if (_wcsnicmp(value, pattern, wcslen(pattern)) == 0)
                return TRUE;
            ++value;
        }
        return FALSE;
    }

    if (!case_sensitive) {
        value_lwr = CmdLine_Duplicate(value);
        if (!value_lwr) {
            if (error)
                *error = TRUE;
            return FALSE;
        }
        _wcslwr(value_lwr);
        value = value_lwr;
    }

    compiled = Pattern_Create(Dll_PoolTemp, pattern, !case_sensitive, 0);
    if (!compiled) {
        if (error)
            *error = TRUE;
        goto finish;
    }

    match = Pattern_Match(compiled, value, (int)wcslen(value));
    Pattern_Free(compiled);
finish:
    if (value_lwr)
        LocalFree(value_lwr);
    return match;
}

static LONG CmdLine_FindArgument(
    const CMD_STATE* state, const WCHAR* pattern, CMD_MATCH mode,
    BOOLEAN case_sensitive, ULONG start_index, BOOLEAN* error)
{
    ULONG i;

    if (error)
        *error = FALSE;
    if (start_index < 1)
        start_index = 1;

    for (i = start_index; i < state->count; ++i) {
        if (CmdLine_MatchPattern(
                state->args[i].value, pattern, mode, case_sensitive, error))
            return (LONG)i;
        if (error && *error)
            return -1;
    }

    return -1;
}

static BOOLEAN CmdLine_FindArgumentSequence(
    const CMD_STATE* state, const CMD_STATE* sequence, CMD_MATCH mode,
    BOOLEAN case_sensitive, ULONG search, CMD_OCCURRENCES occurrences,
    LONG* found)
{
    ULONG start;
    ULONG sequence_count = sequence->count - 1;

    *found = -1;
    for (start = search; start + sequence_count <= state->count; ++start) {
        ULONG part;

        for (part = 0; part < sequence_count; ++part) {
            BOOLEAN error;
            BOOLEAN match = CmdLine_MatchPattern(
                state->args[start + part].value,
                sequence->args[part + 1].value, mode, case_sensitive, &error);

            if (error)
                return FALSE;
            if (!match)
                break;
        }
        if (part == sequence_count) {
            *found = (LONG)start;
            if (occurrences != CMD_OCCURRENCES_LAST)
                break;
        }
    }
    return TRUE;
}

static BOOLEAN CmdLine_ArgumentSequencePresent(
    const CMD_STATE* state, const WCHAR* fragment, CMD_MATCH mode,
    BOOLEAN case_sensitive, BOOLEAN* present)
{
    CMD_STATE parsed;
    size_t fragment_len;
    LONG found;
    BOOLEAN result = FALSE;

    *present = FALSE;
    memset(&parsed, 0, sizeof(parsed));
    fragment_len = wcslen(fragment);
    parsed.text = LocalAlloc(
        LMEM_FIXED, (fragment_len + 3) * sizeof(WCHAR));
    if (!parsed.text)
        return FALSE;

    wcscpy(parsed.text, L"x ");
    wcscat(parsed.text, fragment);
    if (!CmdLine_ParseArguments(&parsed) || parsed.count < 2)
        goto finish;

    result = CmdLine_FindArgumentSequence(state, &parsed, mode,
        case_sensitive, 1, CMD_OCCURRENCES_FIRST, &found);
    if (result)
        *present = found >= 0;

finish:
    CmdLine_FreeArguments(&parsed);
    if (parsed.text)
        LocalFree(parsed.text);
    return result;
}

static ULONG CmdLine_FragmentArgumentCount(const WCHAR* fragment)
{
    CMD_STATE parsed;
    size_t fragment_len;
    ULONG count = 0;

    memset(&parsed, 0, sizeof(parsed));
    fragment_len = wcslen(fragment);
    parsed.text = LocalAlloc(
        LMEM_FIXED, (fragment_len + 3) * sizeof(WCHAR));
    if (!parsed.text)
        return 0;

    wcscpy(parsed.text, L"x ");
    wcscat(parsed.text, fragment);
    if (CmdLine_ParseArguments(&parsed) && parsed.count > 1)
        count = parsed.count - 1;

    CmdLine_FreeArguments(&parsed);
    LocalFree(parsed.text);
    return count;
}

static BOOLEAN CmdLine_ParseCountCondition(
    const WCHAR* condition, const WCHAR* name, ULONG count,
    BOOLEAN* result)
{
    const WCHAR* op;
    const WCHAR* number;
    WCHAR* end;
    ULONG value;
    size_t name_len = wcslen(name);

    if (_wcsnicmp(condition, name, name_len) != 0)
        return FALSE;

    op = condition + name_len;
    if (*op != L'=' && *op != L'<' && *op != L'>')
        return FALSE;

    number = ((op[0] == L'<' || op[0] == L'>') && op[1] == L'=')
        ? op + 2 : op + 1;
    if (*number < L'0' || *number > L'9')
        return FALSE;
    value = wcstoul(number, &end, 10);

    if (end == number || *end)
        return FALSE;

    if (op[0] == L'=')
        *result = count == value;
    else if (op[0] == L'<' && op[1] == L'=')
        *result = count <= value;
    else if (op[0] == L'>' && op[1] == L'=')
        *result = count >= value;
    else if (op[0] == L'<')
        *result = count < value;
    else
        *result = count > value;

    return TRUE;
}

static BOOLEAN CmdLine_EvaluateCondition(
    const CMD_STATE* state, const CMD_CONDITION* predicate,
    const CMD_RULE* rules, ULONG index, BOOLEAN* error)
{
    const WCHAR* condition = predicate->value;
    CMD_MATCH mode = rules[index].match;
    BOOLEAN case_sensitive = rules[index].case_sensitive;
    BOOLEAN result;
    ULONG arg_count = state->count ? state->count - 1 : 0;

    *error = FALSE;
    if (predicate->kind == CMD_CONDITION_ARGUMENT)
        return CmdLine_FindArgument(state, condition, mode,
            case_sensitive, 1, error) >= 0;
    if (predicate->kind == CMD_CONDITION_SEQUENCE) {
        if (!CmdLine_ArgumentSequencePresent(
                state, condition, mode, case_sensitive, &result))
            *error = TRUE;
        return result;
    }

    if (_wcsnicmp(condition, L"Id:", 3) == 0)
        return CmdLine_IdSeen(rules, index, condition + 3, TRUE);
    if (_wcsicmp(condition, L"Empty") == 0)
        return arg_count == 0;
    if (_wcsicmp(condition, L"NotEmpty") == 0)
        return arg_count != 0;
    if (_wcsicmp(condition, L"Forced") == 0)
        return (Dll_ProcessFlags & SBIE_FLAG_FORCED_PROCESS) != 0;
    if (_wcsicmp(condition, L"ForcedChild") == 0) {
        // Requires the unmerged Force/Breakout process flag.
        // return (Dll_ProcessFlags & SBIE_FLAG_FORCED_CHILD_PROCESS) != 0;
        return FALSE;
    }
    if (_wcsicmp(condition, L"ParentWasStartExe") == 0)
        return (Dll_ProcessFlags & SBIE_FLAG_PARENT_WAS_START_EXE) != 0;
    if (_wcsicmp(condition, L"DropRights") == 0)
        return (Dll_ProcessFlags & SBIE_FLAG_DROP_RIGHTS) != 0;
    if (_wcsicmp(condition, L"FakeAdmin") == 0)
        return (Dll_ProcessFlags & SBIE_FLAG_FAKE_ADMIN) != 0;
    if (_wcsicmp(condition, L"OpenAllWinClass") == 0)
        return (Dll_ProcessFlags & SBIE_FLAG_OPEN_ALL_WIN_CLASS) != 0;
    if (_wcsicmp(condition, L"AppCompartment") == 0)
        return (Dll_ProcessFlags & SBIE_FLAG_APP_COMPARTMENT) != 0;
    if (_wcsicmp(condition, L"ProcessInAppPkg") == 0)
        return (Dll_ProcessFlags & SBIE_FLAG_PROCESS_IN_APP_PKG) != 0;
    if (_wcsicmp(condition, L"PCA") == 0)
        return (Dll_ProcessFlags & SBIE_FLAG_PROCESS_IN_PCA_JOB) != 0;

    if (CmdLine_ParseCountCondition(
            condition, L"InputArgCount", state->input_count, &result))
        return result;
    if (CmdLine_ParseCountCondition(
            condition, L"ArgCount", arg_count, &result))
        return result;

    return CmdLine_FindArgument(
        state, condition, mode, case_sensitive, 1, error) >= 0;
}

static BOOLEAN CmdLine_RuleConditionsMatch(
    const CMD_STATE* state, const CMD_STATE* input, const CMD_RULE* rules,
    ULONG index, BOOLEAN* matched)
{
    const CMD_RULE* rule = &rules[index];
    const CMD_STATE* source = rule->condition_input ? input : state;
    ULONG i;

    *matched = FALSE;
    for (i = 0; i < rule->condition_count; ++i) {
        BOOLEAN error;
        BOOLEAN match = CmdLine_EvaluateCondition(
            source, &rule->conditions[i], rules, index, &error);

        if (error)
            return FALSE;
        if (!match)
            return TRUE;
    }

    for (i = 0; i < rule->skip_count; ++i) {
        BOOLEAN error;
        BOOLEAN match = CmdLine_EvaluateCondition(
            source, &rule->skips[i], rules, index, &error);

        if (error)
            return FALSE;
        if (match)
            return TRUE;
    }

    *matched = TRUE;
    return TRUE;
}

static BOOLEAN CmdLine_ReplaceText(
    CMD_STATE* state, ULONG start, ULONG end, const WCHAR* replacement)
{
    size_t old_len;
    size_t replacement_len;
    size_t new_len;
    WCHAR* output;

    old_len = wcslen(state->text);
    replacement_len = replacement ? wcslen(replacement) : 0;
    new_len = old_len - (end - start) + replacement_len;
    if ((new_len + 1) * sizeof(WCHAR) > 0xFFFEu)
        return FALSE;

    output = LocalAlloc(LMEM_FIXED, (new_len + 1) * sizeof(WCHAR));
    if (!output)
        return FALSE;

    wmemcpy(output, state->text, start);
    if (replacement_len)
        wmemcpy(output + start, replacement, replacement_len);
    wcscpy(output + start + replacement_len, state->text + end);

    LocalFree(state->text);
    state->text = output;
    return CmdLine_ParseArguments(state);
}

static BOOLEAN CmdLine_Clear(CMD_STATE* state)
{
    return CmdLine_ReplaceText(
        state, state->argv0_end, (ULONG)wcslen(state->text), NULL);
}

static BOOLEAN CmdLine_Set(CMD_STATE* state, const WCHAR* value)
{
    size_t value_len = wcslen(value);
    WCHAR* replacement = LocalAlloc(
        LMEM_FIXED, (value_len + 2) * sizeof(WCHAR));
    BOOLEAN result;

    if (!replacement)
        return FALSE;

    replacement[0] = L' ';
    wcscpy(replacement + 1, value);
    result = CmdLine_ReplaceText(
        state, state->argv0_end, (ULONG)wcslen(state->text), replacement);
    LocalFree(replacement);
    return result;
}

static BOOLEAN CmdLine_Add(
    CMD_STATE* state, const CMD_RULE* rule, BOOLEAN* applied)
{
    BOOLEAN present;
    BOOLEAN error;
    ULONG insert_at;
    LONG relative;
    size_t value_len;
    WCHAR* insertion;
    BOOLEAN result;

    *applied = FALSE;
    if (!rule->duplicate_allow) {
        if (!CmdLine_ArgumentSequencePresent(
                state, rule->value, rule->match,
                rule->case_sensitive, &present))
            return FALSE;
        if (present)
            return TRUE;
    }

    if (rule->position == CMD_ADD_START) {
        insert_at = state->argv0_end;
    }
    else if (rule->position == CMD_ADD_END) {
        insert_at = state->args[state->count - 1].raw_end;
    }
    else {
        relative = CmdLine_FindArgument(
            state, rule->relative, rule->match,
            rule->case_sensitive, 1, &error);
        if (error)
            return FALSE;
        if (relative < 0)
            return TRUE;
        insert_at = rule->position == CMD_ADD_BEFORE
            ? state->args[relative].prefix_start
            : state->args[relative].raw_end;
    }

    // Do not append inside an unterminated quoted argument.
    if (state->args[state->count - 1].unterminated_quote &&
            insert_at == state->args[state->count - 1].raw_end)
        return FALSE;

    value_len = wcslen(rule->value);
    insertion = LocalAlloc(LMEM_FIXED, (value_len + 2) * sizeof(WCHAR));
    if (!insertion)
        return FALSE;

    insertion[0] = L' ';
    wcscpy(insertion + 1, rule->value);
    result = CmdLine_ReplaceText(state, insert_at, insert_at, insertion);
    LocalFree(insertion);
    if (result)
        *applied = TRUE;
    return result;
}

static BOOLEAN CmdLine_RemoveOrReplace(
    CMD_STATE* state, const CMD_RULE* rule, BOOLEAN* applied)
{
    ULONG search = 1;
    ULONG replacement_count = rule->action == CMD_ACTION_REPLACE
        ? CmdLine_FragmentArgumentCount(rule->replacement) : 0;
    LONG found;
    BOOLEAN error = FALSE;

    *applied = FALSE;
    if (rule->action == CMD_ACTION_REPLACE && !replacement_count)
        return FALSE;

    while ((found = CmdLine_FindArgument(
            state, rule->value, rule->match,
            rule->case_sensitive, search, &error)) >= 0) {
        ULONG start;
        ULONG end;
        const WCHAR* replacement =
            rule->action == CMD_ACTION_REPLACE ? rule->replacement : NULL;

        if (rule->occurrences == CMD_OCCURRENCES_LAST) {
            LONG next;

            while ((next = CmdLine_FindArgument(
                    state, rule->value, rule->match,
                    rule->case_sensitive, (ULONG)found + 1, &error)) >= 0)
                found = next;
            if (error)
                return FALSE;
        }

        start = rule->action == CMD_ACTION_REPLACE
            ? state->args[found].raw_start
            : state->args[found].prefix_start;
        end = state->args[found].raw_end;

        if (!CmdLine_ReplaceText(state, start, end, replacement))
            return FALSE;
        *applied = TRUE;

        if (rule->occurrences != CMD_OCCURRENCES_ALL)
            break;

        search = replacement
            ? (ULONG)found + replacement_count
            : (ULONG)found;
    }

    return !error;
}

static BOOLEAN CmdLine_RemoveOrReplaceSequence(
    CMD_STATE* state, const CMD_RULE* rule, BOOLEAN* applied)
{
    CMD_STATE sequence;
    const WCHAR* replacement = rule->action == CMD_ACTION_REPLACE_SEQUENCE
        ? rule->replacement : NULL;
    ULONG replacement_count = replacement
        ? CmdLine_FragmentArgumentCount(replacement) : 0;
    size_t value_len;
    ULONG search = 1;
    ULONG sequence_count;

    *applied = FALSE;
    if (replacement && !replacement_count)
        return FALSE;
    memset(&sequence, 0, sizeof(sequence));
    value_len = wcslen(rule->value);
    sequence.text = LocalAlloc(
        LMEM_FIXED, (value_len + 3) * sizeof(WCHAR));
    if (!sequence.text)
        return FALSE;

    wcscpy(sequence.text, L"x ");
    wcscat(sequence.text, rule->value);
    if (!CmdLine_ParseArguments(&sequence) || sequence.count < 2)
        goto failure;
    sequence_count = sequence.count - 1;

    while (search + sequence_count <= state->count) {
        LONG found;
        ULONG begin;
        ULONG end;

        if (!CmdLine_FindArgumentSequence(state, &sequence, rule->match,
                rule->case_sensitive, search, rule->occurrences, &found))
            goto failure;
        if (found < 0)
            break;
        begin = replacement ? state->args[found].raw_start
            : state->args[found].prefix_start;
        end = state->args[found + sequence_count - 1].raw_end;
        if (!CmdLine_ReplaceText(state, begin, end, replacement))
            goto failure;
        *applied = TRUE;
        search = replacement ? (ULONG)found + replacement_count : (ULONG)found;

        if (rule->occurrences != CMD_OCCURRENCES_ALL)
            break;
    }

    CmdLine_FreeArguments(&sequence);
    LocalFree(sequence.text);
    return TRUE;

failure:
    CmdLine_FreeArguments(&sequence);
    LocalFree(sequence.text);
    return FALSE;
}

static BOOLEAN CmdLine_ApplyRule(
    CMD_STATE* state, const CMD_RULE* rule, BOOLEAN* applied)
{
    if (rule->action == CMD_ACTION_CLEAR) {
        *applied = TRUE;
        return CmdLine_Clear(state);
    }
    if (rule->action == CMD_ACTION_SET) {
        *applied = TRUE;
        return CmdLine_Set(state, rule->value);
    }
    if (rule->action == CMD_ACTION_ADD)
        return CmdLine_Add(state, rule, applied);
    if (rule->action == CMD_ACTION_REMOVE_SEQUENCE ||
            rule->action == CMD_ACTION_REPLACE_SEQUENCE)
        return CmdLine_RemoveOrReplaceSequence(state, rule, applied);
    return CmdLine_RemoveOrReplace(state, rule, applied);
}

static BOOLEAN CmdLine_IdSeen(
    const CMD_RULE* rules, ULONG count, const WCHAR* id, BOOLEAN applied_only)
{
    ULONG i;

    for (i = 0; i < count; ++i) {
        if (rules[i].id && (!applied_only || rules[i].applied) &&
                _wcsicmp(rules[i].id, id) == 0)
            return TRUE;
    }

    return FALSE;
}

static void CmdLine_SortRules(CMD_RULE* rules, ULONG count)
{
    ULONG i;

    for (i = 1; i < count; ++i) {
        CMD_RULE current = rules[i];
        ULONG pos = i;

        while (pos && rules[pos - 1].order > current.order) {
            rules[pos] = rules[pos - 1];
            --pos;
        }
        rules[pos] = current;
    }
}

static BOOLEAN CmdLine_AddChromiumFlags(
    CMD_RULE* rules, ULONG* count, const WCHAR* command_line)
{
    WCHAR flags[CONF_LINE_LEN];
    WCHAR* storage;
    CMD_RULE* rule;
    NTSTATUS status;

    if (Dll_ImageType != DLL_IMAGE_GOOGLE_CHROME ||
            wcsstr(command_line, L" --type=") ||
            *count >= CMDLINE_MAX_RULES)
        return TRUE;

    status = SbieApi_QueryConfAsIs(
        NULL, CMDLINE_CHROMIUM_SETTING, 0, flags, sizeof(flags));
    if (!NT_SUCCESS(status) || !*flags)
        return TRUE;

    storage = CmdLine_Duplicate(flags);
    if (!storage)
        return FALSE;

    rule = &rules[(*count)++];
    memset(rule, 0, sizeof(*rule));
    rule->storage = storage;
    rule->selector_match = TRUE;
    rule->action = CMD_ACTION_ADD;
    rule->value = storage;
    rule->match = CMD_MATCH_PATTERN;
    rule->case_sensitive = TRUE;
    rule->position = CMD_ADD_START;
    rule->order = 400;
    rule->duplicate_allow = TRUE;
    return TRUE;
}

static NTSTATUS CmdLine_QuerySetting(
    const WCHAR* setting, ULONG* source, ULONG* index,
    WCHAR* buffer, ULONG buffer_len)
{
    NTSTATUS status = STATUS_OBJECT_NAME_NOT_FOUND;

    while (*source < 2) {
        status = SbieApi_QueryConfAsIs(
            *source ? CMDLINE_GLOBAL_SECTION : NULL, setting,
            (*index)++ | (*source ? 0 : CONF_GET_NO_GLOBAL),
            buffer, buffer_len);
        if (NT_SUCCESS(status) || status == STATUS_BUFFER_TOO_SMALL)
            break;

        ++*source;
        *index = 0;
    }

    return status;
}

static void CmdLine_FreeRules(CMD_RULE* rules, ULONG count)
{
    ULONG i;
    for (i = 0; i < count; ++i) {
        if (rules[i].storage)
            LocalFree(rules[i].storage);
    }
}

BOOLEAN CmdLine_Build(
    RTL_USER_PROCESS_PARAMETERS* process_parms,
    UNICODE_STRING* command_line_w,
    ANSI_STRING* command_line_a,
    UNICODE_STRING* command_line_peb)
{
    CMD_RULE rules[CMDLINE_MAX_RULES];
    CMD_RULE planned[CMDLINE_MAX_RULES];
    CMD_STATE state;
    CMD_STATE input;
    CMD_STATE peb;
    ULONG count = 0;
    ULONG planned_count = 0;
    ULONG filtered_count = 0;
    ULONG source = 0;
    ULONG index = 0;
    const WCHAR* failure = L"CustomProcessCommandLine: preparation failed; changes discarded";
    BOOLEAN changed = FALSE;
    BOOLEAN api_changed = FALSE;
    BOOLEAN peb_changed = FALSE;
    BOOLEAN selective_peb = FALSE;
    BOOLEAN keep_original_peb = FALSE;
    BOOLEAN result = FALSE;
    UNICODE_STRING unicode;
    UNICODE_STRING peb_unicode = { 0 };
    ANSI_STRING ansi = { 0 };
    WCHAR* original_text = NULL;

    if (!process_parms || !process_parms->CommandLine.Buffer ||
            !process_parms->CommandLine.Length ||
            (process_parms->CommandLine.Length % sizeof(WCHAR)) != 0 ||
            !command_line_w || !command_line_a || !command_line_peb)
        return FALSE;

    memset(rules, 0, sizeof(rules));
    memset(planned, 0, sizeof(planned));
    memset(&state, 0, sizeof(state));
    memset(&input, 0, sizeof(input));
    memset(&peb, 0, sizeof(peb));

    if (!CmdLine_AddChromiumFlags(rules, &count, process_parms->CommandLine.Buffer))
        goto finish;

    while (source < 2) {
        WCHAR buffer[CONF_LINE_LEN];
        WCHAR* storage;
        CMD_RULE candidate;
        NTSTATUS status;

        status = CmdLine_QuerySetting(
            CMDLINE_SETTING, &source, &index, buffer, sizeof(buffer));

        if (!NT_SUCCESS(status)) {
            if (status == STATUS_BUFFER_TOO_SMALL) {
                SbieApi_MonitorPutMsg(MONITOR_OTHER | MONITOR_TRACE,
                    L"CustomProcessCommandLine: oversized rule ignored");
                continue;
            }
            break;
        }

        storage = CmdLine_Duplicate(buffer);
        if (!storage)
            goto finish;

        if (!CmdLine_ParseRule(storage, &candidate)) {
            SbieApi_MonitorPutMsg(MONITOR_OTHER | MONITOR_TRACE,
                L"CustomProcessCommandLine: invalid rule ignored");
            LocalFree(storage);
            continue;
        }
        if (!candidate.selector_match) {
            LocalFree(storage);
            continue;
        }
        if (count >= CMDLINE_MAX_RULES) {
            SbieApi_MonitorPutMsg(MONITOR_OTHER | MONITOR_TRACE,
                L"CustomProcessCommandLine: relevant rule limit reached; later rules ignored");
            LocalFree(storage);
            break;
        }

        rules[count++] = candidate;
    }

    for (index = 0; index < count; ++index) {
        CMD_RULE* rule = &rules[index];

        if (!rule->selector_match)
            continue;
        if (rule->id && CmdLine_IdSeen(planned, planned_count, rule->id, FALSE))
            continue;
        if (planned_count >= CMDLINE_MAX_RULES)
            goto finish;

        planned[planned_count++] = *rule;
        rule->storage = NULL;
    }

    for (index = 0; index < planned_count; ++index) {
        if (!planned[index].disabled)
            planned[filtered_count++] = planned[index];
        else if (planned[index].storage)
            LocalFree(planned[index].storage);
    }
    planned_count = filtered_count;
    CmdLine_SortRules(planned, planned_count);

    if (!planned_count) {
        failure = NULL;
        goto finish;
    }

    original_text = CmdLine_DuplicateLength(
        process_parms->CommandLine.Buffer,
        process_parms->CommandLine.Length / sizeof(WCHAR));
    state.text = CmdLine_Duplicate(original_text);
    if (!state.text || !CmdLine_ParseArguments(&state))
        goto finish;
    state.input_count = state.count - 1;

    for (index = 0; index < planned_count; ++index) {
        if (planned[index].condition_input) {
            input.text = original_text;
            if (!CmdLine_ParseArguments(&input))
                goto finish;
            input.input_count = state.input_count;
            break;
        }
    }

    for (index = 0; index < planned_count; ++index) {
        BOOLEAN applied;
        BOOLEAN matched;

        failure = L"CustomProcessCommandLine: condition evaluation failed; changes discarded";
        if (!CmdLine_RuleConditionsMatch(&state, &input, planned, index, &matched))
            goto finish;
        if (!matched)
            continue;
        failure = L"CustomProcessCommandLine: action failed; changes discarded";
        if (!CmdLine_ApplyRule(&state, &planned[index], &applied))
            goto finish;
        planned[index].applied = applied;
        if (applied) {
            changed = TRUE;
            if (planned[index].peb_mode == CMD_PEB_SKIP)
                selective_peb = TRUE;
            else if (planned[index].peb_mode == CMD_PEB_KEEP_ORIGINAL)
                keep_original_peb = TRUE;
        }
        if (applied && planned[index].stop)
            break;
    }

    if (!changed) {
        failure = NULL;
        goto finish;
    }

    api_changed = wcscmp(state.text, original_text) != 0;
    if (!keep_original_peb) {
        if (selective_peb) {
            failure = L"CustomProcessCommandLine: PEB preparation failed; changes discarded";
            peb.text = CmdLine_Duplicate(original_text);
            if (!peb.text || !CmdLine_ParseArguments(&peb))
                goto finish;
            peb.input_count = state.input_count;

            // Replay actions only. Eligibility and control flow use the full API state.
            failure = L"CustomProcessCommandLine: PEB action failed; changes discarded";
            for (index = 0; index < planned_count; ++index) {
                BOOLEAN peb_applied;
                if (!planned[index].applied || planned[index].peb_mode != CMD_PEB_UPDATE)
                    continue;
                if (!CmdLine_ApplyRule(&peb, &planned[index], &peb_applied))
                    goto finish;
            }
            peb_changed = wcscmp(peb.text, original_text) != 0;
        }
        else
            peb_changed = api_changed;
    }

    if (!api_changed && !peb_changed) {
        failure = NULL;
        goto finish;
    }

    if ((wcslen(state.text) + 1) * sizeof(WCHAR) > 0xFFFEu) {
        failure = L"CustomProcessCommandLine: final length limit exceeded; changes discarded";
        goto finish;
    }

    unicode.Buffer = state.text;
    unicode.Length = (USHORT)(wcslen(state.text) * sizeof(WCHAR));
    unicode.MaximumLength = unicode.Length + sizeof(WCHAR);

    if (peb_changed) {
        if (selective_peb) {
            if ((wcslen(peb.text) + 1) * sizeof(WCHAR) > 0xFFFEu) {
                failure = L"CustomProcessCommandLine: PEB length limit exceeded; changes discarded";
                goto finish;
            }
            peb_unicode.Buffer = peb.text;
            peb_unicode.Length = (USHORT)(wcslen(peb.text) * sizeof(WCHAR));
            peb_unicode.MaximumLength = peb_unicode.Length + sizeof(WCHAR);
        }
        else
            peb_unicode = unicode;
    }

    if (!NT_SUCCESS(RtlUnicodeStringToAnsiString(&ansi, &unicode, TRUE))) {
        failure = L"CustomProcessCommandLine: ANSI conversion failed; changes discarded";
        goto finish;
    }

    *command_line_w = unicode;
    *command_line_a = ansi;
    *command_line_peb = peb_unicode;
    state.text = NULL;
    if (peb_changed && selective_peb)
        peb.text = NULL;
    result = TRUE;

finish:
    if (!result && failure)
        SbieApi_MonitorPutMsg(MONITOR_OTHER | MONITOR_TRACE, failure);
    if (state.text)
        LocalFree(state.text);
    if (peb.text)
        LocalFree(peb.text);
    CmdLine_FreeArguments(&state);
    CmdLine_FreeArguments(&input);
    CmdLine_FreeArguments(&peb);
    CmdLine_FreeRules(rules, count);
    CmdLine_FreeRules(planned, planned_count);
    if (original_text)
        LocalFree(original_text);
    if (!result && ansi.Buffer)
        RtlFreeAnsiString(&ansi);
    return result;
}
