/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2023 David Xanatos, xanasoft.com
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
// Configuration
//---------------------------------------------------------------------------


#include "conf.h"
#include "process.h"
#include "api.h"
#include "api_flags.h"
#include "obj.h"
#include "util.h"

#define KERNEL_MODE
#include "common/stream.h"

#include "common/my_version.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

#define USE_CONF_MAP

#define CONF_LINE_LEN               2000        // keep in sync with sbieiniwire.h
#define CONF_MAX_LINES              100000      // keep in sync with sbieiniwire.h

#define CONF_TMPL_LINE_BASE         0x01000000


//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

//
// Note: we want to preserve the order of the settings when enumerating
//          hence we can not replace the list with a hash map entirely
//          instead we use both, here the hash map is used only for lookups
//          the keys in the map are only pointers to the name fields in the list entries
//

typedef struct _CONF_DATA {

    POOL *pool;
    LIST sections;      // CONF_SECTION
#ifdef USE_CONF_MAP
    HASH_MAP sections_map;
#endif
    ULONG home;         // 1 if configuration read from Driver_Home_Path
    WCHAR* path;
    ULONG encoding;     // 0 - unicode, 1 - utf8, 2 - unicode (byte swapped)
    volatile ULONG use_count;

} CONF_DATA;


typedef struct _CONF_SECTION {

    LIST_ELEM list_elem;
    WCHAR *name;
    LIST settings;      // CONF_SETTING
#ifdef USE_CONF_MAP
    HASH_MAP settings_map;
#endif
    BOOLEAN from_template;

} CONF_SECTION;


typedef struct _CONF_SETTING {

    LIST_ELEM list_elem;
    WCHAR *name;
    WCHAR *value;
    BOOLEAN from_template;
    BOOLEAN template_handled;

} CONF_SETTING;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static NTSTATUS Conf_Read(ULONG session_id);

static NTSTATUS Conf_Read_Sections(
    STREAM *stream, CONF_DATA *data, int *linenum);

static NTSTATUS Conf_Read_Settings(
    STREAM *stream, CONF_DATA *data, CONF_SECTION *section,
    WCHAR *line, int *linenum);

NTSTATUS Conf_Read_Line(STREAM *stream, WCHAR *line, int *linenum);

static NTSTATUS Conf_Merge_Templates(CONF_DATA *data, ULONG session_id);

static NTSTATUS Conf_Merge_Global(
    CONF_DATA *data, ULONG session_id,
    CONF_SECTION *global);

static NTSTATUS Conf_Merge_Template(
    CONF_DATA *data, ULONG session_id,
    const WCHAR *tmpl_name, CONF_SECTION *section, const WCHAR* name);

static const WCHAR *Conf_Get_Helper(
    const WCHAR *section_name, const WCHAR *setting_name,
    ULONG *index, BOOLEAN skip_tmpl);

static const WCHAR *Conf_Get_Section_Name(ULONG index, BOOLEAN skip_tmpl);

static const WCHAR *Conf_Get_Setting_Name(
    const WCHAR *section_name, ULONG index, BOOLEAN skip_tmpl);


//---------------------------------------------------------------------------



//---------------------------------------------------------------------------

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Conf_Init)
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static CONF_DATA Conf_Data;
static PERESOURCE Conf_Lock = NULL;

static const WCHAR *Conf_GlobalSettings   = L"GlobalSettings";
static const WCHAR *Conf_UserSettings_    = L"UserSettings_";
static const WCHAR *Conf_Template_        = L"Template_";
static const WCHAR *Conf_DefaultTemplates = L"DefaultTemplates";
       const WCHAR *Conf_TemplateSettings = L"TemplateSettings";

static const WCHAR *Conf_Template = L"Template";
       const WCHAR *Conf_Tmpl     = L"Tmpl.";

static const WCHAR *Conf_H = L"H";
static const WCHAR *Conf_W = L"W";

static const WCHAR* Conf_Unicode = L"U";
static const WCHAR* Conf_UTF8 = L"8";


//---------------------------------------------------------------------------
// Conf_AdjustUseCount
//---------------------------------------------------------------------------


_FX void Conf_AdjustUseCount(BOOLEAN increase)
{
    KIRQL irql;
    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceExclusiveLite(Conf_Lock, TRUE);

    if (increase)
        InterlockedIncrement(&Conf_Data.use_count);
    else
        InterlockedDecrement(&Conf_Data.use_count);

    ExReleaseResourceLite(Conf_Lock);
    KeLowerIrql(irql);
}


//---------------------------------------------------------------------------
// Conf_Read
//---------------------------------------------------------------------------


_FX NTSTATUS Conf_Read(ULONG session_id)
{
    static const WCHAR *path_sandboxie = L"%s\\" SANDBOXIE_INI;
    static const WCHAR *path_templates = L"%s\\Templates.ini";
    static const WCHAR *SystemRoot = L"\\SystemRoot";
    NTSTATUS status;
    CONF_DATA data;
    int linenum;
    WCHAR linenum_str[32];
    ULONG path_len;
    WCHAR *path = NULL;
    ULONG path_home;
    STREAM *stream = NULL;
    POOL *pool;

    //
    // allocate a buffer large enough for \SystemRoot\Sandboxie.ini
    // or (Home Path)\Sandboxie.ini
    //

    path_len = 260 * sizeof(WCHAR);
    //path_len = 32;      // room for \SystemRoot
    if (path_len < wcslen(Driver_HomePathDos) * sizeof(WCHAR) + 64)
        path_len = wcslen(Driver_HomePathDos) * sizeof(WCHAR) + 64;
    //path_len += 64;     // room for \Sandboxie.ini
    

    pool = Pool_Create();
    if (! pool)
        return STATUS_INSUFFICIENT_RESOURCES;

    path = Mem_Alloc(pool, path_len);
    if (! path) {
        Pool_Delete(pool);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // try open a custom configuration file, if set
    //

    UNICODE_STRING IniPath = { 0, (USHORT)path_len - (4 * sizeof(WCHAR)), path };
    status = GetRegString(RTL_REGISTRY_ABSOLUTE, Driver_RegistryPath, L"IniPath", &IniPath);
    if (NT_SUCCESS(status)) {

        if (path[0] != L'\\') {
            wmemmove(path + 4, path, (IniPath.Length / sizeof(WCHAR)) + 1);
            wmemcpy(path, L"\\??\\", 4);
        }

        path_home = 2;
        status = Stream_Open(
            &stream, path,
            FILE_GENERIC_READ, 0, FILE_SHARE_READ, FILE_OPEN, 0);
    }

    //
    // open the configuration file, try both places, home first
    //

    if (!NT_SUCCESS(status)) {

        path_home = 1;
        RtlStringCbPrintfW(path, path_len, path_sandboxie, Driver_HomePathDos);

        status = Stream_Open(
            &stream, path, FILE_GENERIC_READ, 0, FILE_SHARE_READ, FILE_OPEN, 0);
    }

    if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

		path_home = 0;
		RtlStringCbPrintfW(path, path_len, path_sandboxie, SystemRoot);

        status = Stream_Open(
            &stream, path,
            FILE_GENERIC_READ, 0, FILE_SHARE_READ, FILE_OPEN, 0);
    }

    if (! NT_SUCCESS(status)) {

        if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
            status == STATUS_OBJECT_PATH_NOT_FOUND)
        {
            Log_Msg_Session(MSG_CONF_NO_FILE, NULL, NULL, session_id);
            status = STATUS_SUCCESS; // we need to continue and load the Templates.ini with the defaults
        } else {
            wcscpy(linenum_str, L"(none)");
            Log_Status_Ex_Session(
                MSG_CONF_READ, 0, status, linenum_str, session_id);
        }
    }

    if (! NT_SUCCESS(status)) {
        Pool_Delete(pool);
        return status;
    }

    //
    // read data from the file
    //

    data.pool = pool;
    List_Init(&data.sections);
#ifdef USE_CONF_MAP
    map_init(&data.sections_map, data.pool);
    data.sections_map.func_key_size = NULL;
    data.sections_map.func_match_key = &str_map_match;
    data.sections_map.func_hash_key = &str_map_hash;
    map_resize(&data.sections_map, 16); // prepare some buckets for better performance
#endif
    data.home = path_home;
    if (path_home == 2)
        data.path = Mem_AllocStringEx(data.pool, path, TRUE);
    else
        data.path = NULL;
    data.use_count = 0;

    if (stream) {

        status = Stream_Read_BOM(stream, &data.encoding);

        linenum = 1;
        while (NT_SUCCESS(status))
            status = Conf_Read_Sections(stream, &data, &linenum);
        if (status == STATUS_END_OF_FILE)
            status = STATUS_SUCCESS;
    }

    if (stream) Stream_Close(stream);

    //
    // read (Home Path)\Templates.ini
    //

    if (NT_SUCCESS(status)) {

        RtlStringCbPrintfW(path, path_len, path_templates, Driver_HomePathDos);

        status = Stream_Open(
            &stream, path,
            FILE_GENERIC_READ, 0, FILE_SHARE_READ, FILE_OPEN, 0);

        if (! NT_SUCCESS(status)) {

            Log_Status_Ex_Session(
                MSG_CONF_NO_TMPL_FILE, 0, status, NULL, session_id);

        } else {

            status = Stream_Read_BOM(stream, NULL);

            linenum = 1 + CONF_TMPL_LINE_BASE;

            while (NT_SUCCESS(status))
                status = Conf_Read_Sections(stream, &data, &linenum);
            if (status == STATUS_END_OF_FILE)
                status = STATUS_SUCCESS;

            Stream_Close(stream);

            linenum -= CONF_TMPL_LINE_BASE;
            if (! NT_SUCCESS(status))
                Log_Msg_Session(MSG_CONF_BAD_TMPL_FILE, 0, NULL, session_id);
        }
    }

    //
    // merge templates
    //

    if (NT_SUCCESS(status)) {
        status = Conf_Merge_Templates(&data, session_id);
        linenum = 0;
    }

    //
    // if read successfully, replace existing configuration
    //

    if (NT_SUCCESS(status)) {

        BOOLEAN done = FALSE;
        while (! done) {

            KIRQL irql;
            KeRaiseIrql(APC_LEVEL, &irql);
            ExAcquireResourceExclusiveLite(Conf_Lock, TRUE);

            if (Conf_Data.use_count == 0) {

                pool = Conf_Data.pool;
                memcpy(&Conf_Data, &data, sizeof(CONF_DATA));

                done = TRUE;
            }

            ExReleaseResourceLite(Conf_Lock);
            KeLowerIrql(irql);

            if (! done)
                ZwYieldExecution();
        }
    }

    Mem_Free(path, path_len);
    if (pool)
        Pool_Delete(pool);  // may be either data.pool or old Conf_Data.pool

    //
    // Possible error values through Conf_Read_* functions:
    //
    // STATUS_BUFFER_OVERFLOW   (80000005) line too long
    // STATUS_TOO_MANY_COMMANDS (C00000C1) too many lines in file
    // STATUS_INVALID_PARAMETER (C000000D) syntax error
    //

    if (! NT_SUCCESS(status)) {
        RtlStringCbPrintfW(linenum_str, sizeof(linenum_str), L"%d", linenum);
        //DbgPrint("Conf error %X at line %d (%S)\n", status, linenum, linenum_str);
        if (status == STATUS_BUFFER_OVERFLOW) {
            Log_Msg_Session(
                MSG_CONF_LINE_TOO_LONG, linenum_str, NULL, session_id);
        } else if (status == STATUS_TOO_MANY_COMMANDS) {
            Log_Msg_Session(
                MSG_CONF_FILE_TOO_LONG, linenum_str, NULL, session_id);
        } else if (status == STATUS_INVALID_PARAMETER) {
            Log_Msg_Session(
                MSG_CONF_SYNTAX_ERROR, linenum_str, NULL, session_id);
        } else {
            Log_Status_Ex_Session(
                MSG_CONF_READ, 0, status, linenum_str, session_id);
        }
    }

    //
    // cache some config
    //

    Log_LogMessageEvents = Conf_Get_Boolean(NULL, L"LogMessageEvents", 0, FALSE);

    return status;
}


//---------------------------------------------------------------------------
// Conf_Read_Sections
//---------------------------------------------------------------------------


_FX NTSTATUS Conf_Read_Sections(
    STREAM *stream, CONF_DATA *data, int *linenum)
{
    const int line_len = (CONF_LINE_LEN + 2) * sizeof(WCHAR);
    NTSTATUS status;
    WCHAR *line;
    WCHAR *ptr;
    CONF_SECTION *section;

    line = Mem_Alloc(data->pool, line_len);
    if (! line)
        return STATUS_INSUFFICIENT_RESOURCES;

    status = Conf_Read_Line(stream, line, linenum);
    //DbgPrint("Conf_Read_Line (%d/%X) --> %S\n", *linenum, status, line);
    while (NT_SUCCESS(status)) {

        //
        // extract the section name from the section name
        //

        if (line[0] != L'[') {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        ptr = &line[1];
        while (*ptr && *ptr != L']')
            ++ptr;
        if (*ptr != L']') {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        *ptr = L'\0';

        if (_wcsnicmp(&line[1], Conf_UserSettings_, 13) == 0) {
            if (! line[14]) {
                status = STATUS_INVALID_PARAMETER;
                break;
            }
        } else if (_wcsnicmp(&line[1], Conf_Template_, 9) == 0) {
            if (! line[10]) {
                status = STATUS_INVALID_PARAMETER;
                break;
            }
        } else if (! Box_IsValidName(&line[1])) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        //
        // find an existing section by that name or create a new one
        //
#ifdef USE_CONF_MAP
        section = map_get(&data->sections_map, &line[1]);
#else
        section = List_Head(&data->sections);
        while (section) {
            if (_wcsicmp(section->name, &line[1]) == 0)
                break;
            section = List_Next(section);
        }
#endif

        if (! section) {

            section = Mem_Alloc(data->pool, sizeof(CONF_SECTION));
            if (! section) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            if ((*linenum) >= CONF_TMPL_LINE_BASE)
                section->from_template = TRUE;
            else
                section->from_template = FALSE;

            section->name = Mem_AllocString(data->pool, &line[1]);
            if (! section->name) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            List_Init(&section->settings);
#ifdef USE_CONF_MAP
            map_init(&section->settings_map, data->pool);
            section->settings_map.func_key_size = NULL;
            section->settings_map.func_match_key = &str_map_match;
            section->settings_map.func_hash_key = &str_map_hash;
            map_resize(&section->settings_map, 16); // prepare some buckets for better performance
#endif			

            List_Insert_After(&data->sections, NULL, section);
#ifdef USE_CONF_MAP
            if(map_insert(&data->sections_map, section->name, section, 0) == NULL) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
#endif
        }

        // read settings for this section

        status = Conf_Read_Settings(stream, data, section, line, linenum);
    }

    Mem_Free(line, line_len);

    return status;
}


//---------------------------------------------------------------------------
// Conf_Read_Settings
//---------------------------------------------------------------------------


_FX NTSTATUS Conf_Read_Settings(
    STREAM *stream, CONF_DATA *data, CONF_SECTION *section,
    WCHAR *line, int *linenum)
{
    NTSTATUS status;
    WCHAR *ptr;
    WCHAR *value;
    CONF_SETTING *setting;

    while (1) {

        status = Conf_Read_Line(stream, line, linenum);
        if (! NT_SUCCESS(status))
            break;

        if (line[0] == L'[' || line[0] == L']')
            break;

        // parse setting name=value

        ptr = wcschr(line, L'=');
        if ((! ptr) || ptr == line) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        value = &ptr[1];

        // eliminate trailing whitespace in the setting name

        while (ptr > line) {
            --ptr;
            if (*ptr > 32) {
                ++ptr;
                break;
            }
        }
        *ptr = L'\0';

        // eliminate leading and trailing whitespace in value

        while (*value <= 32) {
            if (! (*value))
                break;
            ++value;
        }

        if (*value == L'\0') {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        ptr = value + wcslen(value);
        while (ptr > value) {
            --ptr;
            if (*ptr > 32) {
                ++ptr;
                break;
            }
        }
        *ptr = L'\0';

        //
        // add the new setting
        //

        setting = Mem_Alloc(data->pool, sizeof(CONF_SETTING));
        if (! setting) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        if ((*linenum) >= CONF_TMPL_LINE_BASE)
            setting->from_template = TRUE;
        else
            setting->from_template = FALSE;

        setting->template_handled = FALSE;

        setting->name = Mem_AllocString(data->pool, line);
        if (! setting->name) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        setting->value = Mem_AllocString(data->pool, value);
        if (! setting->value) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        List_Insert_After(&section->settings, NULL, setting);
#ifdef USE_CONF_MAP
        if(map_append(&section->settings_map, setting->name, setting, 0) == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
#endif
    }

    return status;
}


//---------------------------------------------------------------------------
// Conf_Read_Line
//---------------------------------------------------------------------------


_FX NTSTATUS Conf_Read_Line(STREAM *stream, WCHAR *line, int *linenum)
{
    NTSTATUS status;
    WCHAR *ptr;
    USHORT ch;

    while (1) {

        // skip leading control and whitespace characters
        while (1) {
            status = Stream_Read_Wchar(stream, &ch);
            if ((! NT_SUCCESS(status)) || (ch > 32 && ch < 0xFE00))
                break;
            if (ch == L'\r')
                continue;
            if (ch == L'\n') {
                ULONG numlines = (++(*linenum));
                if (numlines >= CONF_TMPL_LINE_BASE)
                    numlines -= CONF_TMPL_LINE_BASE;
                if (numlines > CONF_MAX_LINES) {
                    status = STATUS_TOO_MANY_COMMANDS;
                    break;
                }
            }
        }
        if (! NT_SUCCESS(status)) {
            *line = L'\0';
            break;
        }

        // read characters until hitting the newline mark
        ptr = line;
        while (1) {
            *ptr = ch;
            ++ptr;
            if (ptr - line == CONF_LINE_LEN)
                status = STATUS_BUFFER_OVERFLOW;
            else
                status = Stream_Read_Wchar(stream, &ch);
            if ((! NT_SUCCESS(status)) || ch == L'\n' || ch == L'\r')
                break;
        }

        // remove all trailing control and whitespace characters
        while (ptr > line) {
            --ptr;
            if (*ptr > 32) {
                ++ptr;
                break;
            }
        }
        *ptr = L'\0';

        // don't report end-of-file if we have data to return
        if (ptr > line && status == STATUS_END_OF_FILE)
            status = STATUS_SUCCESS;

        // if we are about to successfully return a comment line,
        // then discard the line and restart from the top
        if (status == STATUS_SUCCESS && *line == L'#')
            continue;

        break;
    }

    return status;
}


//---------------------------------------------------------------------------
// Conf_Get_Section
//---------------------------------------------------------------------------


_FX CONF_SECTION* Conf_Get_Section(
    CONF_DATA* data, const WCHAR* section_name)
{
#ifdef USE_CONF_MAP
    //
    // lookup the template section in the hash map
    //

    return map_get(&data->sections_map, section_name);
#else
    //
    // scan for a matching template section
    //

    CONF_SECTION* section = List_Head(&data->sections);
    while (section) {

        if (_wcsicmp(section->name, section_name) == 0) {

            break;
        }

        section = List_Next(section);
    }
    return section;
#endif
}


//---------------------------------------------------------------------------
// Conf_Merge_Templates
//---------------------------------------------------------------------------


_FX NTSTATUS Conf_Merge_Templates(CONF_DATA *data, ULONG session_id)
{
    NTSTATUS status;
    CONF_SECTION *sandbox;
    CONF_SETTING *setting;

    //
    // first handle the global section
    //

    CONF_SECTION* global = Conf_Get_Section(data, Conf_GlobalSettings);
    if (global) {
        status = Conf_Merge_Global(data, session_id, global);
        if (!NT_SUCCESS(status))
            return status;
    }

    //
    // second handle the default templates
    //

    global = Conf_Get_Section(data, Conf_DefaultTemplates);
    if (global) {
        status = Conf_Merge_Global(data, session_id, global);
        if (!NT_SUCCESS(status))
            return status;
    }

    //
    // scan sections to find a sandbox section
    //

    sandbox = List_Head(&data->sections);
    while (sandbox) {

        CONF_SECTION *next_sandbox = List_Next(sandbox);

        //
        // break once the template section starts
        //

        if (sandbox->from_template) {
            // we can break because template sections come after
            // all non-template sections
            break;
        }

        //
        // skip the global section, skip any local template sections and user settings sections
        //

        if (_wcsicmp(sandbox->name, Conf_GlobalSettings) == 0     ||
            _wcsnicmp(sandbox->name, Conf_Template_, 9)      == 0 || // Template_ or Template_Local_
            _wcsnicmp(sandbox->name, Conf_UserSettings_, 13) == 0) {

            sandbox = next_sandbox;
            continue;
        }

#ifdef USE_CONF_MAP

        //
        // use a keyed iterator to quickly go through all Template=Xxx settings
        //

        map_iter_t iter2 = map_key_iter(&sandbox->settings_map, Conf_Template);
	    while (map_next(&sandbox->settings_map, &iter2)) {
            setting = iter2.value;
#else

        //
        // scan the section for a Template=Xxx setting
        //

        setting = List_Head(&sandbox->settings);
        while (setting) {

            if (_wcsicmp(setting->name, Conf_Template) != 0) {

                setting = List_Next(setting);
                continue;
            }
#endif

            if (setting->template_handled) {

#ifndef USE_CONF_MAP
                setting = List_Next(setting);
#endif
                continue;
            }

            //
            // merge the template into the sandbox section
            //

            status = Conf_Merge_Template(
                data, session_id, setting->value, sandbox, NULL);

            if (! NT_SUCCESS(status))
                return status;

            setting->template_handled = TRUE;

#ifndef USE_CONF_MAP
            //
            // advance to next setting
            //

            setting = List_Head(&sandbox->settings);
#endif
        }

        //
        // advance to next section
        //

        sandbox = next_sandbox;
    }

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Conf_Merge_Global
//---------------------------------------------------------------------------


_FX NTSTATUS Conf_Merge_Global(
    CONF_DATA *data, ULONG session_id,
    CONF_SECTION *global)
{
    NTSTATUS status;
    CONF_SECTION *sandbox;
    CONF_SETTING *setting;

    //
    // scan the section for a Template=Xxx setting
    //

    setting = List_Head(&global->settings);
    while (setting) {

        if (_wcsicmp(setting->name, Conf_Template) != 0) {

            setting = List_Next(setting);
            continue;
        }

        //
        // scan sections to find a sandbox section
        //

        sandbox = List_Head(&data->sections);
        while (sandbox) {

            CONF_SECTION *next_sandbox = List_Next(sandbox);

            //
            // break once the template section starts
            //

            if (sandbox->from_template) {
                // we can break because template sections come after
                // all non-template sections
                break;
            }

            //
            // skip the global section, any template sections and user settings sections
            //

            if (_wcsicmp(sandbox->name, Conf_GlobalSettings)     == 0 ||
                _wcsnicmp(sandbox->name, Conf_Template_, 9)      == 0 ||
                _wcsnicmp(sandbox->name, Conf_UserSettings_, 13) == 0) {

                sandbox = next_sandbox;
                continue;
            }

            //
            // merge the template into the sandbox section
            //

            status = Conf_Merge_Template(
                data, session_id, setting->value, sandbox, L"GlobalSettings");

            if (! NT_SUCCESS(status))
                return status;

            //
            // advance to next section
            //

            sandbox = next_sandbox;
        }

        //
        // advance to next setting
        //

        setting = List_Next(setting);
    }

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Conf_Merge_Template
//---------------------------------------------------------------------------


_FX NTSTATUS Conf_Merge_Template(
    CONF_DATA *data, ULONG session_id,
    const WCHAR *tmpl_name, CONF_SECTION *section, const WCHAR* name)
{
    CONF_SECTION *tmpl = NULL;

    WCHAR section_name[130]; // 128 + 2 // max regular section length is 64
    if (wcslen(tmpl_name) < 119) { // 128 - wcslen(Conf_Template_)
        wcscpy(section_name, Conf_Template_);
        wcscat(section_name, tmpl_name);
        tmpl = Conf_Get_Section(data, section_name);
    }

    //
    // copy settings from template section into sandbox section
    //

    if (tmpl) {

        CONF_SETTING *oset, *nset;

        oset = List_Head(&tmpl->settings);
        while (oset) {

            if (_wcsnicmp(oset->name, Conf_Tmpl, 5) == 0) {
                oset = List_Next(oset);
                continue;
            }

            nset = Mem_Alloc(data->pool, sizeof(CONF_SETTING));
            if (! nset)
                return STATUS_INSUFFICIENT_RESOURCES;
            nset->from_template = TRUE;
            nset->template_handled = FALSE;
            nset->name = Mem_AllocString(data->pool, oset->name);
            if (! nset->name)
                return STATUS_INSUFFICIENT_RESOURCES;
            nset->value = Mem_AllocString(data->pool, oset->value);
            if (! nset->value)
                return STATUS_INSUFFICIENT_RESOURCES;

            List_Insert_After(&section->settings, NULL, nset);
#ifdef USE_CONF_MAP
            if(map_append(&section->settings_map, nset->name, nset, 0) == NULL)
                return STATUS_INSUFFICIENT_RESOURCES;
#endif

            oset = List_Next(oset);
        }

    } else {

        Log_Msg_Session(MSG_CONF_MISSING_TMPL,
                        name ? name : section->name, tmpl_name, session_id);
    }

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Conf_Get_Helper
//---------------------------------------------------------------------------


_FX const WCHAR *Conf_Get_Helper(
    const WCHAR *section_name, const WCHAR *setting_name,
    ULONG *index, BOOLEAN skip_tmpl)
{
    WCHAR *value;
    CONF_SECTION *section;
    CONF_SETTING *setting;

    value = NULL;

#ifdef USE_CONF_MAP
    //
    // lookup the section in the hash map
    //

    section = map_get(&Conf_Data.sections_map, section_name);
#else
    section = List_Head(&Conf_Data.sections);
    while (section) {
        //DbgPrint("        Examining section at %X name %S (looking for %S)\n", section, section->name, section_name);
        if (_wcsicmp(section->name, section_name) == 0) 
            break;
        section = List_Next(section);
    }
#endif
    if (skip_tmpl && section && section->from_template)
        section = NULL;

    if (section) {
#ifdef USE_CONF_MAP
        //
        // use a keyed iterator to quickly go through all matching settings
        //

        map_iter_t iter2 = map_key_iter(&section->settings_map, setting_name);
	    while (map_next(&section->settings_map, &iter2)) {
            setting = iter2.value;
#else
        setting = List_Head(&section->settings);
        while (setting) {
            //DbgPrint("        Examining setting at %X name %S (looking for %S)\n", setting, setting->name, setting_name);
#endif
            if (skip_tmpl && setting->from_template) {
                // we can break because template settings come after
                // all non-template settings
                break;
            }
#ifndef USE_CONF_MAP
            if (_wcsicmp(setting->name, setting_name) == 0) {
#endif
                if (*index == 0) {
                    value = setting->value;
                    break;
                }
                --(*index);
#ifndef USE_CONF_MAP
            }
            setting = List_Next(setting);
#endif
        }
    }

    return value;
}


//---------------------------------------------------------------------------
// Conf_Get_Section_Name
//---------------------------------------------------------------------------


_FX const WCHAR *Conf_Get_Section_Name(ULONG index, BOOLEAN skip_tmpl)
{
    WCHAR *value;
    CONF_SECTION *section;

    value = NULL;

    section = List_Head(&Conf_Data.sections);
    while (section) {
        CONF_SECTION *next_section = List_Next(section);

        if (_wcsicmp(section->name, Conf_GlobalSettings) == 0) {
            section = next_section;
            continue;
        }
        if (skip_tmpl && section->from_template) {
            // we can break because template sections come after
            // all non-template sections
            break;
        }
        if (index == 0) {
            value = section->name;
            break;
        }

        --index;
        section = next_section;
    }

    return value;
}


//---------------------------------------------------------------------------
// Conf_Get_Setting_Name
//---------------------------------------------------------------------------


_FX const WCHAR *Conf_Get_Setting_Name(
    const WCHAR *section_name, ULONG index, BOOLEAN skip_tmpl)
{
    WCHAR *value;
    CONF_SECTION *section;
    CONF_SETTING *setting, *setting2;
    BOOLEAN dup;

    value = NULL;

#ifdef USE_CONF_MAP
    //
    // lookup the section in the hash map
    //

    section = map_get(&Conf_Data.sections_map, section_name);
#else
    section = List_Head(&Conf_Data.sections);
    while (section) {
        if (_wcsicmp(section->name, section_name) == 0)
            break;
        section = List_Next(section);
    }
#endif
    if (skip_tmpl && section && section->from_template)
        section = NULL;

    if (section) {
        setting = List_Head(&section->settings);
        while (setting) {

            if (skip_tmpl && setting->from_template) {
                // we can break because template settings come after
                // all non-template settings
                break;
            }

            //
            // check if we already processed this name
            //

            dup = FALSE;
            setting2 = List_Head(&section->settings);
            while (setting2 && setting2 != setting) {
                if (_wcsicmp(setting2->name, setting->name) == 0) {
                    dup = TRUE;
                    break;
                } else
                    setting2 = List_Next(setting2);
            }

            if (! dup) {
                if (index == 0) {
                    value = setting->name;
                    break;
                } else
                    --index;
            }

            setting = List_Next(setting);
        }
    }

    return value;
}


//---------------------------------------------------------------------------
// Conf_Get
//---------------------------------------------------------------------------


_FX const WCHAR *Conf_Get(
    const WCHAR *section, const WCHAR *setting, ULONG index)
{
    const WCHAR *value;
    BOOLEAN have_section;
    BOOLEAN have_setting;
    BOOLEAN check_global;
    BOOLEAN skip_tmpl;
    KIRQL irql;

    value = NULL;
    have_section = (section && section[0]);
    have_setting = (setting && setting[0]);
    skip_tmpl = ((index & CONF_GET_NO_TEMPLS) != 0);

    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceSharedLite(Conf_Lock, TRUE);

    if ((! have_section) && have_setting &&
            _wcsicmp(setting, L"IniLocation") == 0) {

        // return "H" if configuration file was found in the Sandboxie
        // home directory, or "W" if it was found in Windows directory

        if (Conf_Data.path) value = Conf_Data.path;
        else value = (Conf_Data.home) ? Conf_H : Conf_W;

    } else if ((!have_section) && have_setting &&
        _wcsicmp(setting, L"IniEncoding") == 0) {

        // return "U" if configuration file was Unicode encoded,
        // or "8" if it was UTF-8 encoded

        value = (Conf_Data.encoding == 1) ? Conf_UTF8 : Conf_Unicode;

    }
    else if (have_setting) {

        check_global = ((index & CONF_GET_NO_GLOBAL) == 0);
        index &= 0xFFFF;

        if (section)
            value = Conf_Get_Helper(section, setting, &index, skip_tmpl);

        //
        // when no value has been found for the given section
        // try getting it from the global section
        //

        if ((! value) && check_global) {
            value = Conf_Get_Helper(
                Conf_GlobalSettings, setting, &index, skip_tmpl);
        }

    } else if (have_section && (! have_setting)) {

        value = Conf_Get_Setting_Name(section, index & 0xFFFF, skip_tmpl);

    } else if ((! have_section) && (! have_setting)) {

        value = Conf_Get_Section_Name(index & 0xFFFF, skip_tmpl);
    }

    ExReleaseResourceLite(Conf_Lock);
    KeLowerIrql(irql);

    return value;
}


//---------------------------------------------------------------------------
// Conf_Get_Boolean
//---------------------------------------------------------------------------


_FX BOOLEAN Conf_Get_Boolean(
    const WCHAR *section, const WCHAR *setting, ULONG index, BOOLEAN def)
{
    const WCHAR *value;
    BOOLEAN retval;

    Conf_AdjustUseCount(TRUE);

    value = Conf_Get(section, setting, index);

    retval = def;
    if (value) {
        if (*value == 'y' || *value == 'Y')
            retval = TRUE;
        else if (*value == 'n' || *value == 'N')
            retval = FALSE;
    }

    Conf_AdjustUseCount(FALSE);

    return retval;
}


//---------------------------------------------------------------------------
// Conf_Get_Number
//---------------------------------------------------------------------------


_FX ULONG Conf_Get_Number(
    const WCHAR *section, const WCHAR *setting, ULONG index, ULONG def)
{
    const WCHAR *value;
    ULONG retval;

    Conf_AdjustUseCount(TRUE);

    value = Conf_Get(section, setting, index);

    retval = def;
    if (value) {

        NTSTATUS status;
        UNICODE_STRING uni;
        RtlInitUnicodeString(&uni, value);
        status = RtlUnicodeStringToInteger(&uni, 10, &retval);
        if (! NT_SUCCESS(status))
            retval = def;
    }

    Conf_AdjustUseCount(FALSE);

    return retval;
}


//---------------------------------------------------------------------------
// Conf_IsValidBox
//---------------------------------------------------------------------------


_FX NTSTATUS Conf_IsValidBox(const WCHAR *section_name)
{
    CONF_SECTION *section;
    NTSTATUS status;
    KIRQL irql;

    if (     _wcsicmp(section_name, Conf_GlobalSettings)    == 0
        ||   _wcsicmp(section_name, Conf_TemplateSettings)  == 0
        ||  _wcsnicmp(section_name, Conf_Template_, 9)      == 0
        ||  _wcsnicmp(section_name, Conf_UserSettings_, 13) == 0) {

        status = STATUS_OBJECT_TYPE_MISMATCH;

    } else {

        KeRaiseIrql(APC_LEVEL, &irql);
        ExAcquireResourceSharedLite(Conf_Lock, TRUE);

        section = List_Head(&Conf_Data.sections);
        while (section) {
            if (_wcsicmp(section->name, section_name) == 0)
                break;
            section = List_Next(section);
        }

        if (! section)
            status = STATUS_OBJECT_NAME_NOT_FOUND;

        else if (section->from_template)
            status = STATUS_OBJECT_TYPE_MISMATCH;

        else
            status = STATUS_SUCCESS;

        ExReleaseResourceLite(Conf_Lock);
        KeLowerIrql(irql);
    }

    return status;
}


//---------------------------------------------------------------------------
// Conf_Api_Reload
//---------------------------------------------------------------------------


_FX NTSTATUS Conf_Api_Reload(PROCESS *proc, ULONG64 *parms)
{
    NTSTATUS status;
    ULONG flags;

    if (proc)
        return STATUS_NOT_IMPLEMENTED;

    flags = (ULONG)parms[2];

    if (flags & SBIE_CONF_FLAG_RELOAD_CERT) {
        status = MyValidateCertificate();
        goto finish;
    }

    status = Conf_Read((ULONG)parms[1]);

    if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
        status == STATUS_OBJECT_PATH_NOT_FOUND) {

        //
        // if configuration file was removed, reset configuration
        //

        POOL *pool;

        KIRQL irql;
        KeRaiseIrql(APC_LEVEL, &irql);
        ExAcquireResourceExclusiveLite(Conf_Lock, TRUE);

        pool = Conf_Data.pool;

        Conf_Data.pool = NULL;
        List_Init(&Conf_Data.sections);
#ifdef USE_CONF_MAP
    	map_init(&Conf_Data.sections_map, NULL);
    	Conf_Data.sections_map.func_key_size = NULL;
		Conf_Data.sections_map.func_match_key = &str_map_match;
		Conf_Data.sections_map.func_hash_key = &str_map_hash;
        map_resize(&Conf_Data.sections_map, 16); // prepare some buckets for better performance  
#endif

		Conf_Data.home = FALSE;
        Conf_Data.path = NULL;
        Conf_Data.encoding = 0;

        ExReleaseResourceLite(Conf_Lock);
        KeLowerIrql(irql);

        if (pool)
            Pool_Delete(pool);

        status = STATUS_SUCCESS;
    }

    //
    // Check the reconfigure drier flag and if its set, load/unload the components accordingly
    //

    if (flags & SBIE_CONF_FLAG_RECONFIGURE) {

        static volatile ULONG reconf_lock = 0;
        if (InterlockedCompareExchange(&reconf_lock, 1, 0) != 0) {
            status = STATUS_OPERATION_IN_PROGRESS;
            goto finish; // don't do anything is a reconfiguration is already in progress
        }

        BOOLEAN wpf_enabled = Conf_Get_Boolean(NULL, L"NetworkEnableWFP", 0, FALSE);
        extern BOOLEAN WFP_Enabled;
        if (WFP_Enabled != wpf_enabled) {
            if (wpf_enabled) {
                extern BOOLEAN WFP_Load(void);
                WFP_Load();
            }
            else {
                extern void WFP_Unload(void);
                WFP_Unload();
            }
        }

        BOOLEAN obj_filter_enabled = Conf_Get_Boolean(NULL, L"EnableObjectFiltering", 0, TRUE);
        if (Obj_CallbackInstalled != obj_filter_enabled && Driver_OsVersion > DRIVER_WINDOWS_VISTA) {
            if (obj_filter_enabled) {
                Obj_Load_Filter();
            }
            else {
                Obj_Unload_Filter();
            }
        }

        void Syscall_Update_Config();
        Syscall_Update_Config();

        /*
#ifdef HOOK_WIN32K
        // must be windows 10 or later
        if (Driver_OsBuild >= 14393) {
            extern ULONG Syscall_MaxIndex32;
            if (Conf_Get_Boolean(NULL, L"EnableWin32kHooks", 0, FALSE) && Syscall_MaxIndex32 == 0) {
                if(Syscall_Init_List32()){
                    Syscall_Init_Table32();
                }
            }
        }
#endif
        */

        InterlockedExchange(&reconf_lock, 0);
    }

    //
    // notify service about setting change
    //

    ULONG process_id = (ULONG)PsGetCurrentProcessId();

    Api_SendServiceMessage(SVC_CONFIG_UPDATED, sizeof(process_id), &process_id);

finish:
    return status;
}


//---------------------------------------------------------------------------
// Conf_Api_Query
//---------------------------------------------------------------------------


_FX NTSTATUS Conf_Api_Query(PROCESS *proc, ULONG64 *parms)
{
    NTSTATUS status;
    WCHAR *parm;
    ULONG *parm2;
    WCHAR boxname[70];
    WCHAR setting[70];
    ULONG index;
    const WCHAR *value1;
    WCHAR *value2;

    //
    // prepare parameters
    // 

    // parms[1] --> WCHAR [66] SectionName

    memzero(boxname, sizeof(boxname));
    parm = (WCHAR *)parms[1];
    if (parm) {
        ProbeForRead(parm, sizeof(WCHAR) * 64, sizeof(WCHAR));
        if (parm[0])
            wcsncpy(boxname, parm, 64);
        else
            parm = NULL;
    }
    if (!parm && proc)
        wcscpy(boxname, proc->box->name);

    // parms[2] --> WCHAR [66] SettingName

    memzero(setting, sizeof(setting));
    parm = (WCHAR *)parms[2];
    if (parm) {
        ProbeForRead(parm, sizeof(WCHAR) * 64, sizeof(WCHAR));
        if (parm[0])
            wcsncpy(setting, parm, 64);
    }

    // parms[3] --> ULONG SettingIndex

    index = 0;
    parm2 = (ULONG *)parms[3];
    if (parm2) {
        ProbeForRead(parm2, sizeof(ULONG), sizeof(ULONG));
        index = *parm2;
        if ((index & 0xFFFF) > 1000)
            return STATUS_INVALID_PARAMETER;
    } else
        return STATUS_INVALID_PARAMETER;

    //
    // get value
    //

    Conf_AdjustUseCount(TRUE);

    if ((setting && setting[0] == L'%') || (index & CONF_JUST_EXPAND))
        value1 = setting; // shortcut to expand a variable
    else
        value1 = Conf_Get(boxname, setting, index);
    if (! value1) {
        status = STATUS_RESOURCE_NAME_NOT_FOUND;
        goto release_and_return;
    }

    if (index & CONF_GET_NO_EXPAND)
        value2 = (WCHAR *)value1;
    else {

        // expand value.  if caller is sandboxed, use its BOX (with its
        // expand_args) for that.  otherwise, create a temporary BOX

        if (proc)
            value2 = Conf_Expand(proc->box->expand_args, value1, setting);
        else {

            CONF_EXPAND_ARGS *expand_args = Mem_Alloc(Driver_Pool, sizeof(CONF_EXPAND_ARGS));
            if (! expand_args) {
                status = STATUS_UNSUCCESSFUL;
                goto release_and_return;
            }

            expand_args->pool = Driver_Pool;
            expand_args->sandbox = boxname;

            UNICODE_STRING SidString;
            ULONG SessionId;
            status = Process_GetSidStringAndSessionId(NtCurrentProcess(), NULL, &SidString, &SessionId);
            if (!NT_SUCCESS(status)) {
                Mem_Free(expand_args, sizeof(CONF_EXPAND_ARGS));
                status = STATUS_UNSUCCESSFUL;
                goto release_and_return;
            }

            expand_args->sid = SidString.Buffer;
            expand_args->session = &SessionId;

            value2 = Conf_Expand(expand_args, value1, setting);

            RtlFreeUnicodeString(&SidString);

            Mem_Free(expand_args, sizeof(CONF_EXPAND_ARGS));
        }

        if (! value2) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto release_and_return;
        }
    }

    // write value into user buffer Output
    // parms[4] --> user buffer Output

    __try {

        UNICODE_STRING64 *user_uni = (UNICODE_STRING64 *)parms[4];
        ULONG len = (wcslen(value2) + 1) * sizeof(WCHAR);
        Api_CopyStringToUser(user_uni, value2, len);

        status = STATUS_SUCCESS;

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (value2 != value1)
        Mem_FreeString(value2);

release_and_return:

    Conf_AdjustUseCount(FALSE);

    return status;
}


//---------------------------------------------------------------------------
// Conf_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Conf_Init(void)
{
    Conf_Data.pool = NULL;
    List_Init(&Conf_Data.sections);
#ifdef USE_CONF_MAP
    map_init(&Conf_Data.sections_map, NULL);
    Conf_Data.sections_map.func_key_size = NULL;
	Conf_Data.sections_map.func_match_key = &str_map_match;
	Conf_Data.sections_map.func_hash_key = &str_map_hash;
#endif

    Conf_Data.home = FALSE;
    Conf_Data.path = NULL;
    Conf_Data.encoding = 0;

    if (! Mem_GetLockResource(&Conf_Lock, TRUE))
        return FALSE;

    if (! Conf_Init_User())
        return FALSE;

    Conf_Read(-1);

    //
    // set API functions
    //

    Api_SetFunction(API_RELOAD_CONF,        Conf_Api_Reload);
    Api_SetFunction(API_QUERY_CONF,         Conf_Api_Query);

    return TRUE;
}


//---------------------------------------------------------------------------
// Conf_Unload
//---------------------------------------------------------------------------


_FX void Conf_Unload(void)
{
    Conf_Unload_User();

    if (Conf_Data.pool) {
        Pool_Delete(Conf_Data.pool);
        Conf_Data.pool = NULL;
    }

    Mem_FreeLockResource(&Conf_Lock);
}
