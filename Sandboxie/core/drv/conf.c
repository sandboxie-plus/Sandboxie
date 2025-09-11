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
    HASH_MAP sections_map;
    ULONG home;         // 1 if configuration read from Driver_Home_Path
    WCHAR* path;
    ULONG encoding;     // 0 - unicode, 1 - utf8, 2 - unicode (byte swapped)
    volatile ULONG use_count;

} CONF_DATA;


typedef struct _CONF_SECTION {

    LIST_ELEM list_elem;
    WCHAR *name;
    LIST settings;      // CONF_SETTING
    HASH_MAP settings_map;
    BOOLEAN from_template;
    BOOLEAN is_virtual;
    WCHAR* include_path;

} CONF_SECTION;


typedef struct _CONF_SETTING {

    LIST_ELEM list_elem;
    WCHAR *name;
    WCHAR *value;
    BOOLEAN from_template;

} CONF_SETTING;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static NTSTATUS Conf_Read(ULONG session_id);

static NTSTATUS Conf_Read_Sections(
    STREAM *stream, CONF_DATA *data, int *linenum, BOOLEAN from_template);

static NTSTATUS Conf_Read_Settings(
    STREAM *stream, CONF_DATA *data, CONF_SECTION *section,
    WCHAR *line, int *linenum);

NTSTATUS Conf_Read_Line(STREAM *stream, WCHAR *line, int *linenum);

static CONF_SECTION* Conf_Find_Sections(CONF_DATA* data, const WCHAR* section_name);

static CONF_SECTION* Conf_Add_Sections(
    CONF_DATA* data, const WCHAR* section_name, BOOLEAN insert);

static CONF_SETTING* Conf_Add_Setting(
    CONF_DATA *data, CONF_SECTION *section, 
    const WCHAR* setting_name, const WCHAR* value, BOOLEAN insert);

static NTSTATUS Conf_Import_AllIncludes(CONF_DATA *data, ULONG session_id);

static NTSTATUS Conf_Import_Includes(CONF_DATA *data, ULONG session_id, const WCHAR* include_path);

static NTSTATUS Conf_Import_Include(CONF_DATA *data, ULONG session_id, const WCHAR* include_path, BOOLEAN fromWildCard);

static NTSTATUS Conf_Merge_AllTemplates(CONF_DATA *data, ULONG session_id);

static NTSTATUS Conf_Merge_Templates(
    CONF_DATA *data, ULONG session_id, CONF_SECTION* sandbox, 
    CONF_SECTION* section, const WCHAR* name);

static NTSTATUS Conf_Merge_Template(
    CONF_DATA *data, ULONG session_id,
    const WCHAR *tmpl_name, CONF_SECTION *section, const WCHAR* name);

static const WCHAR *Conf_Get_Helper(
    const WCHAR *section_name, const WCHAR *setting_name,
    ULONG *index, BOOLEAN skip_tmpl);

static const WCHAR *Conf_Get_Prop(
    const WCHAR *section_name, const WCHAR *setting_name);

static const WCHAR *Conf_Get_Section_Name(ULONG index, BOOLEAN skip_tmpl);

static const WCHAR *Conf_Get_Setting_Name(
    const WCHAR *section_name, ULONG index, BOOLEAN skip_tmpl);

static NTSTATUS Conf_Drop_Section(CONF_DATA *data, CONF_SECTION *section);

static NTSTATUS Conf_Update(CONF_DATA *data, 
    const WCHAR* section_name, const WCHAR* setting_name, const WCHAR* SettingValue, ULONG uMode);


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

static const WCHAR *Conf_IniLocation = L"IniLocation";
static const WCHAR *Conf_IsVirtual   = L"IsVirtual";

static const WCHAR *Conf_ImportBox = L"ImportBox";

static const WCHAR *Conf_H = L"H";
static const WCHAR *Conf_W = L"W";

static const WCHAR *Conf_Y = L"Y";
static const WCHAR *Conf_N = L"N";


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
    map_init(&data.sections_map, data.pool);
    data.sections_map.func_key_size = NULL;
    data.sections_map.func_match_key = &str_map_match;
    data.sections_map.func_hash_key = &str_map_hash;
    map_resize(&data.sections_map, 16); // prepare some buckets for better performance
    data.home = path_home;
    if (path_home == 2)
        data.path = Mem_AllocStringEx(data.pool, path, TRUE);
    else
        data.path = NULL;
    data.use_count = 0;

    if (stream) {

        status = Stream_Read_BOM(stream, &data.encoding);

        linenum = 1;
        if (NT_SUCCESS(status))
            status = Conf_Read_Sections(stream, &data, &linenum, FALSE);
        if (status == STATUS_END_OF_FILE)
            status = STATUS_SUCCESS;
    }

    if (stream) Stream_Close(stream);

    //
    // read Includes
    //
    
    if (NT_SUCCESS(status)) {
        Conf_Import_AllIncludes(&data, session_id);
    }

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

            linenum = 1;
            if (NT_SUCCESS(status))
                status = Conf_Read_Sections(stream, &data, &linenum, TRUE);
            if (status == STATUS_END_OF_FILE)
                status = STATUS_SUCCESS;

            Stream_Close(stream);

            if (! NT_SUCCESS(status))
                Log_Msg_Session(MSG_CONF_BAD_TMPL_FILE, 0, NULL, session_id);
        }
    }

    //
    // merge templates
    //

    if (NT_SUCCESS(status)) {
        status = Conf_Merge_AllTemplates(&data, session_id);
        linenum = 0;
    }

    //
    // import existing virtual sections
    //

    Conf_AdjustUseCount(TRUE);

    if (Conf_Data.pool)  {
        
		CONF_SECTION* section = List_Head(&Conf_Data.sections);
        while (section) {
            if (section->is_virtual) {

                //
				// check if the new settings contains a section matching an existing virtual section
				// and fail with STATUS_OBJECT_NAME_EXISTS if it does
                //

                CONF_SECTION* new_section = map_get(&data.sections_map, section->name);
                if (new_section) {
                    status = STATUS_OBJECT_NAME_EXISTS;
                    break;
                }

                //
				// add the virtual section to the new settings
                //

				for (CONF_SETTING* setting = List_Head(&section->settings); setting; setting = List_Next(setting)) {

                    if (setting->from_template)
                        continue;

					Conf_Update(&data, section->name, setting->name, setting->value, CONF_APPEND_VALUE);
				}

                Conf_Update(&data, section->name, NULL, NULL, CONF_UPDATE_TEMPLATES);
            }
			section = List_Next(section);
        }
    }

    Conf_AdjustUseCount(FALSE);

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
// Conf_Find_Sections
//---------------------------------------------------------------------------


_FX CONF_SECTION* Conf_Find_Sections(CONF_DATA* data, const WCHAR* section_name)
{
    CONF_SECTION *section;
    section = map_get(&data->sections_map, section_name);
	return section;
}


//---------------------------------------------------------------------------
// Conf_Add_Sections
//---------------------------------------------------------------------------


_FX CONF_SECTION* Conf_Add_Sections(
    CONF_DATA* data, const WCHAR* section_name, BOOLEAN insert)
{
	CONF_SECTION *section;

    section = Mem_Alloc(data->pool, sizeof(CONF_SECTION));
    if (! section)
        return NULL;

    section->from_template = FALSE;
    section->is_virtual = FALSE;
    section->include_path = NULL;

    section->name = Mem_AllocString(data->pool, section_name);
    if (! section->name) 
        return NULL;

    List_Init(&section->settings);
    map_init(&section->settings_map, data->pool);
    section->settings_map.func_key_size = NULL;
    section->settings_map.func_match_key = &str_map_match;
    section->settings_map.func_hash_key = &str_map_hash;
    map_resize(&section->settings_map, 16); // prepare some buckets for better performance

    if(insert) // insert at the top so it is not after the templates
        List_Insert_Before(&data->sections, NULL, section);
    else
        List_Insert_After(&data->sections, NULL, section);
    if(map_insert(&data->sections_map, section->name, section, 0) == NULL) 
        return NULL;

    return section;
}


//---------------------------------------------------------------------------
// Conf_Read_Header
//---------------------------------------------------------------------------


_FX NTSTATUS Conf_Read_Header(
    STREAM *stream, CONF_DATA *data, WCHAR *line, WCHAR** name)
{
    WCHAR *ptr;

    //
    // extract the section name from the section name
    //

    if (line[0] != L'[') {
        return STATUS_INVALID_PARAMETER;
    }
    ptr = &line[1];
    while (*ptr && *ptr != L']')
        ++ptr;
    if (*ptr != L']') {
        return STATUS_INVALID_PARAMETER;
    }
    *ptr = L'\0';

    if (_wcsnicmp(&line[1], Conf_UserSettings_, 13) == 0) {
        if (! line[14]) {
            return STATUS_INVALID_PARAMETER;
        }
    } else if (_wcsnicmp(&line[1], Conf_Template_, 9) == 0) {
        if (! line[10]) {
            return STATUS_INVALID_PARAMETER;
        }
    } else if (! Box_IsValidName(&line[1])) {
        return STATUS_INVALID_PARAMETER;
    }

    *name = &line[1];
	return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Conf_Read_Sections
//---------------------------------------------------------------------------


_FX NTSTATUS Conf_Read_Sections(
    STREAM *stream, CONF_DATA *data, int *linenum, BOOLEAN from_template)
{
    const int line_len = (CONF_LINE_LEN + 2) * sizeof(WCHAR);
    NTSTATUS status;
    WCHAR *line;
    CONF_SECTION *section;
    WCHAR* name;

    line = Mem_Alloc(data->pool, line_len);
    if (! line)
        return STATUS_INSUFFICIENT_RESOURCES;

    status = Conf_Read_Line(stream, line, linenum);
    //DbgPrint("Conf_Read_Line (%d/%X) --> %S\n", *linenum, status, line);
    while (NT_SUCCESS(status)) {

        status = Conf_Read_Header(stream, data, line, &name);
        if (!NT_SUCCESS(status))
            break;

        //
        // find an existing section by that name or create a new one
        //

        section = Conf_Find_Sections(data, name);
        if (!section) {

            section = Conf_Add_Sections(data, name, FALSE);
            if (!section) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            section->from_template = from_template;
        }

        //
        // read settings for this section
        //

        status = Conf_Read_Settings(stream, data, section, line, linenum);
    }

    Mem_Free(line, line_len);

    return status;
}


//---------------------------------------------------------------------------
// Conf_Add_Setting
//---------------------------------------------------------------------------


_FX CONF_SETTING* Conf_Add_Setting(
    CONF_DATA *data, CONF_SECTION *section, 
    const WCHAR* setting_name, const WCHAR* value, BOOLEAN insert)
{
    CONF_SETTING* setting;

    setting = Mem_Alloc(data->pool, sizeof(CONF_SETTING));
    if (! setting) 
        return NULL;

    setting->from_template = FALSE;

    setting->name = Mem_AllocString(data->pool, setting_name);
    if (! setting->name) 
        return NULL;

    setting->value = Mem_AllocString(data->pool, value);
    if (! setting->value) return NULL;

    if(insert)
		List_Insert_Before(&section->settings, NULL, setting);
	else
        List_Insert_After(&section->settings, NULL, setting);
    if(map_append(&section->settings_map, setting->name, setting, 0) == NULL) 
        return NULL;

    return setting;
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

		setting = Conf_Add_Setting(data, section, line, value, FALSE);
        if (! setting) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
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
                if ((++(*linenum)) > CONF_MAX_LINES) {
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
    //
    // lookup the template section in the hash map
    //

    return map_get(&data->sections_map, section_name);
}


//---------------------------------------------------------------------------
// Conf_Import_Includes
//---------------------------------------------------------------------------


_FX NTSTATUS Conf_Import_Includes(CONF_DATA *data, ULONG session_id, const WCHAR* include_path)
{
    NTSTATUS status;
    ULONG path_len = 0;
    const WCHAR *path = NULL;
    ULONG path_offset = 0;
    UNICODE_STRING DirectoryPath;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE DirectoryHandle = NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG BufferSize = 1024;
    PVOID Buffer = NULL;
	
	path_len = (wcslen(include_path) + 6 + BOXNAME_COUNT + 4) * sizeof(WCHAR); // \??\C:\...\RootPath\BoxName.ini
    path = Mem_Alloc(data->pool, path_len);
    if (!path) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto finish;
    }

    if (*include_path == L'\\')
        wcscpy((WCHAR*)path, include_path);
    else {
        wcscpy((WCHAR*)path, L"\\??\\");
        wcscat((WCHAR*)path, include_path);
        path_offset = 4;
    }
 
	DirectoryPath.Buffer = (WCHAR*)path;
	DirectoryPath.Length = (USHORT)wcslen((WCHAR*)path) * sizeof(WCHAR);
	DirectoryPath.MaximumLength = (USHORT)path_len;
	while (DirectoryPath.Length > (8*sizeof(WCHAR)) && (DirectoryPath.Buffer[DirectoryPath.Length / sizeof(WCHAR) - 1] == L'\\' 
                                                     || DirectoryPath.Buffer[DirectoryPath.Length / sizeof(WCHAR) - 1] == L'*'))
		DirectoryPath.Length -= sizeof(WCHAR); // remove trailing backslashes and wildcards
    InitializeObjectAttributes(&ObjectAttributes, &DirectoryPath, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = ZwCreateFile(&DirectoryHandle, FILE_LIST_DIRECTORY | SYNCHRONIZE, &ObjectAttributes, &IoStatusBlock,
        NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, FILE_OPEN,
        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

    if (!NT_SUCCESS(status))
        goto finish;

	Buffer = Mem_Alloc(data->pool, BufferSize);
    if (!Buffer) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto finish;
    }

    for(;;) {

        status = ZwQueryDirectoryFile(DirectoryHandle, NULL, NULL, NULL, &IoStatusBlock,
            Buffer, BufferSize, FileDirectoryInformation, FALSE, NULL, FALSE );
        if(!NT_SUCCESS(status))
            break;

        PFILE_DIRECTORY_INFORMATION FileInfo = (PFILE_DIRECTORY_INFORMATION)Buffer;
        for(;;) {

            if(FileInfo->FileNameLength/sizeof(WCHAR) > BOXNAME_COUNT + 4 || FileInfo->FileNameLength/sizeof(WCHAR) < 5)
                goto next; // the filename is to long or to short, skip this file

			DirectoryPath.Buffer[DirectoryPath.Length / sizeof(WCHAR)] = L'\\';
			memcpy(&DirectoryPath.Buffer[DirectoryPath.Length / sizeof(WCHAR) + 1], FileInfo->FileName, FileInfo->FileNameLength);
			DirectoryPath.Buffer[(DirectoryPath.Length + sizeof(WCHAR) + FileInfo->FileNameLength) / sizeof(WCHAR)] = 0;

            //DbgPrint("File: %.*S\n", FileInfo->FileNameLength/sizeof(WCHAR), FileInfo->FileName);
            //DbgPrint("File: %S\n", DirectoryPath.Buffer + path_offset);
            Conf_Import_Include(data, session_id, DirectoryPath.Buffer + path_offset, TRUE);

        next:
            if(!FileInfo->NextEntryOffset)
                break;
            FileInfo = (PFILE_DIRECTORY_INFORMATION)((PUCHAR)FileInfo + FileInfo->NextEntryOffset);
        }
    }

    if (status == STATUS_NO_MORE_FILES)
        status = STATUS_SUCCESS;

finish:

    if(path_len)
        Mem_Free((WCHAR*)path, path_len);

    if(DirectoryHandle)
        ZwClose(DirectoryHandle);

    if(Buffer)
        Mem_Free((WCHAR*)Buffer, BufferSize);

    if (!NT_SUCCESS(status))
        Log_Status_Ex_Session(MSG_1413, 0, status, include_path, session_id);

    return status;
}


//---------------------------------------------------------------------------
// Conf_Import_Include
//---------------------------------------------------------------------------


_FX NTSTATUS Conf_Import_Include(CONF_DATA *data, ULONG session_id, const WCHAR* include_path, BOOLEAN fromWildCard)
{
    const int line_len = (CONF_LINE_LEN + 2) * sizeof(WCHAR);
    NTSTATUS status;
    int msg = MSG_1414;
    WCHAR *line;
    ULONG path_len = 0;
    const WCHAR *path = NULL;
    STREAM *stream = NULL;
    int linenum = 0;
    CONF_SECTION *section = NULL;
    WCHAR* name = NULL;
    CONF_SETTING *setting;

    if(*include_path == L'\\')
        path = include_path;
    else {
		path_len = (wcslen(include_path) + 5) * sizeof(WCHAR);
        path = Mem_Alloc(data->pool, path_len);
        if (!path) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto finish;
        }

        wcscpy((WCHAR*)path, L"\\??\\");
        wcscat((WCHAR*)path, include_path);
    }

	//DbgPrint("Conf_Import_Include: %S\n", path);
	WCHAR* file_name = wcsrchr(path, L'\\');
    if (!file_name) { // must be a full path
        status = STATUS_OBJECT_NAME_INVALID;
		goto finish;
    }
	WCHAR* dot = wcschr(++file_name, L'.'); // ++ skip L'\\'
    if (!dot) { // an extension must be present, no matter what but something must be there
        if(fromWildCard) status = STATUS_SUCCESS; // fail silently for folders
        else status = STATUS_OBJECT_NAME_INVALID;
        goto finish;
    }
    if (_wcsicmp(dot, L".ini") != 0) {
        if(fromWildCard) status = STATUS_SUCCESS; // not an ini fail silen
        else status = STATUS_OBJECT_NAME_INVALID;
        goto finish;
    }
	SIZE_T file_name_len = dot - file_name;
	//DbgPrint("Conf_Import_Include: %.*S\n", file_name_len, file_name);

    status = Stream_Open(
        &stream, path,
        FILE_GENERIC_READ, 0, FILE_SHARE_READ, FILE_OPEN, 0);

    if (!NT_SUCCESS(status))
        goto finish;
        
    line = Mem_Alloc(data->pool, line_len);
    if (! line)
        status = STATUS_INSUFFICIENT_RESOURCES;

    if (NT_SUCCESS(status))
        status = Stream_Read_BOM(stream, NULL);

    if (NT_SUCCESS(status))
        status = Conf_Read_Line(stream, line, &linenum);

    if (NT_SUCCESS(status)) 
        status = Conf_Read_Header(stream, data, line, &name);
        
    //
	// An imported box must contain one section with the same name as the file
    //

    if (NT_SUCCESS(status)) {
        if (_wcsnicmp(name, file_name, file_name_len) != 0 || name[file_name_len] != 0)
        {
            msg = MSG_1415;
            status = STATUS_OBJECT_NAME_INVALID;
        }
    }

    if (NT_SUCCESS(status)) {

        section = Conf_Find_Sections(data, name);

        if (section) {
            msg = MSG_1416;
            status = STATUS_OBJECT_NAME_COLLISION;
        }
        else {

            section = Conf_Add_Sections(data, name, FALSE);
            if (!section)
                status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
        
    if (NT_SUCCESS(status)) {

		section->include_path =  Mem_AllocString(data->pool, include_path);

        status = Conf_Read_Settings(stream, data, section, line, &linenum);

        //
        // An include must contain exactly one section, those Conf_Read_Settings must end on STATUS_END_OF_FILE
        //

        if (status == STATUS_END_OF_FILE)
            status = STATUS_SUCCESS;
        else if(NT_SUCCESS(status))
            status = STATUS_TOO_MANY_SESSIONS;

        //
        // An imported box has its FileRootPath set to the path of the include file
        //

        if (NT_SUCCESS(status) || status == STATUS_TOO_MANY_SESSIONS) {
            *dot = 0;
            setting = Conf_Add_Setting(data, section, L"FileRootPath", path, TRUE);
            *dot = L'.';
            if(!setting)
                status = STATUS_INSUFFICIENT_RESOURCES;
        }

        //
        // Drop the section if something went wrong, except if there were additional sections,
		// for future extensibility and forwards compatibility we tolerate that case.
        //

        if (!NT_SUCCESS(status) && status != STATUS_TOO_MANY_SESSIONS)
            Conf_Drop_Section(data, section);
    }

    Mem_Free(line, line_len);

    Stream_Close(stream);

finish:

    if(path_len)
        Mem_Free((WCHAR*)path, path_len);

    if (!NT_SUCCESS(status))
        Log_Status_Ex_Session(msg, 0, status, include_path, session_id);

    return status;
}


//---------------------------------------------------------------------------
// Conf_Import_AllIncludes
//---------------------------------------------------------------------------


_FX NTSTATUS Conf_Import_AllIncludes(CONF_DATA *data, ULONG session_id)
{
    CONF_SETTING *setting;

    CONF_SECTION* section = Conf_Get_Section(data, Conf_GlobalSettings);
	if (!section)
		return STATUS_OBJECT_NAME_NOT_FOUND;

    //
    // use a keyed iterator to quickly go through all IncludeBox=Xxx settings
    //

    map_iter_t iter2 = map_key_iter(&section->settings_map, Conf_ImportBox);
    while (map_next(&section->settings_map, &iter2)) {
        setting = iter2.value;

        //
		// import include, don't break on error, continue with next include
        //

		SIZE_T len = wcslen(setting->value);
		if (len > 2 && (setting->value[len - 1] == L'\\' || setting->value[len - 1] == L'*')) {
		
            Conf_Import_Includes(data, session_id, setting->value);
		}
        else {

            Conf_Import_Include(data, session_id, setting->value, FALSE);
        }
    }

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Conf_Merge_AllTemplates
//---------------------------------------------------------------------------


_FX NTSTATUS Conf_Merge_AllTemplates(CONF_DATA *data, ULONG session_id)
{
    NTSTATUS status;
    CONF_SECTION *sandbox;
    CONF_SETTING *setting;

    CONF_SECTION* global = Conf_Get_Section(data, Conf_GlobalSettings);
    if (global) {
        status = Conf_Merge_Templates(data, session_id, global, global, Conf_GlobalSettings);

        CONF_SECTION* common = Conf_Get_Section(data, Conf_DefaultTemplates);
        if (common)
            status = Conf_Merge_Templates(data, session_id, global, common, Conf_DefaultTemplates);
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

        if (_wcsicmp(sandbox->name, Conf_GlobalSettings)     == 0 ||
            _wcsnicmp(sandbox->name, Conf_Template_, 9)      == 0 || // Template_ or Template_Local_
            _wcsnicmp(sandbox->name, Conf_UserSettings_, 13) == 0) {

            sandbox = next_sandbox;
            continue;
        }

        //
        // scan the section for a Template=Xxx setting
        //

        status = Conf_Merge_Templates(data, session_id, sandbox, sandbox, sandbox->name);

        if (! NT_SUCCESS(status))
            return status;

        //
        // advance to next sandbox section
        //

        sandbox = next_sandbox;
    }

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Conf_Merge_Templates
//---------------------------------------------------------------------------


_FX NTSTATUS Conf_Merge_Templates(
    CONF_DATA *data, ULONG session_id, CONF_SECTION* sandbox, 
    CONF_SECTION* section, const WCHAR* name)
{
    NTSTATUS status = STATUS_SUCCESS;
    CONF_SETTING *setting;

    //
    // use a keyed iterator to quickly go through all Template=Xxx settings
    //

    map_iter_t iter2 = map_key_iter(&section->settings_map, Conf_Template);
    while (map_next(&section->settings_map, &iter2)) {
        setting = iter2.value;

        //
        // merge the template into the sandbox section
        //

        status = Conf_Merge_Template(
            data, session_id, setting->value, sandbox, name);

        if (! NT_SUCCESS(status))
            return status;
    }

    return status;
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

            nset = Conf_Add_Setting(data, section, oset->name, oset->value, FALSE);
            if (! nset)
                return STATUS_INSUFFICIENT_RESOURCES;
            nset->from_template = TRUE;

            oset = List_Next(oset);
        }

    } else {

        Log_Msg_Session(MSG_CONF_MISSING_TMPL, name, tmpl_name, session_id);
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

    *index &= CONF_INDEX_MASK;

	section = Conf_Find_Sections(&Conf_Data, section_name);
    if (skip_tmpl && section && section->from_template)
        section = NULL;

    if (section) {

        //
        // use a keyed iterator to quickly go through all matching settings
        //

        map_iter_t iter2 = map_key_iter(&section->settings_map, setting_name);
	    while (map_next(&section->settings_map, &iter2)) {
            setting = iter2.value;
            if (skip_tmpl && setting->from_template) {
                // we can break because template settings come after
                // all non-template settings
                break;
            }
            if (*index == 0) {
                value = setting->value;
				if (setting->from_template)
					*index = CONF_GET_NO_TEMPLS;
                break;
            }
            --(*index);
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

		//DbgPrint("Examining section at %X name %S\n", section, section->name);
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

    //
    // lookup the section in the hash map
    //

    section = map_get(&Conf_Data.sections_map, section_name);
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
// Conf_Get_Helper
//---------------------------------------------------------------------------


_FX const WCHAR *Conf_Get_Prop(
    const WCHAR *section_name, const WCHAR *setting_name)
{
    const WCHAR *value;
    CONF_SECTION *section;
    CONF_SETTING *setting;

    value = NULL;

    //
    // lookup the section in the hash map
    //

    section = map_get(&Conf_Data.sections_map, section_name);

    if (!section)
        return NULL;
    
    if (_wcsicmp(setting_name, Conf_IsVirtual) == 0) {

		value = section->is_virtual ? Conf_Y : Conf_N;

    } else if (_wcsicmp(setting_name, Conf_IniLocation) == 0) {

        value = section->include_path;
    }

    return value;
}


//---------------------------------------------------------------------------
// Conf_GetEx
//---------------------------------------------------------------------------


_FX const WCHAR *Conf_GetEx(
    const WCHAR *section, const WCHAR *setting, ULONG* index)
{
    const WCHAR *value;
    BOOLEAN have_section;
    ULONG section_length;
    BOOLEAN have_setting;
    BOOLEAN check_global;
    BOOLEAN skip_tmpl;
    KIRQL irql;

    value = NULL;
    have_section = (section && section[0]);
    have_setting = (setting && setting[0]);
    skip_tmpl = ((*index & CONF_GET_NO_TEMPLS) != 0);

    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceSharedLite(Conf_Lock, TRUE);

    if ((! have_section) && have_setting &&
            _wcsicmp(setting, Conf_IniLocation) == 0) {

        // return "H" if configuration file was found in the Sandboxie
        // home directory, or "W" if it was found in Windows directory

        if (Conf_Data.path) value = Conf_Data.path; // special case when custom path set in registry
        else value = (Conf_Data.home) ? Conf_H : Conf_W; // regular case

    } else if ((*index & CONF_GET_PROPERTY) != 0) { // Get Property

        if ((*index & CONF_INDEX_MASK) == 0) // properties are never lists
            value = Conf_Get_Prop(section, setting);

    } else if (have_setting) {

        check_global = ((*index & CONF_GET_NO_GLOBAL) == 0);

        if (section)
            value = Conf_Get_Helper(section, setting, index, skip_tmpl);

        //
        // when no value has been found for the given section
        // try getting it from the global section
        //

        if ((! value) && check_global && (!section || _wcsicmp(section, Conf_GlobalSettings) != 0)) {
            value = Conf_Get_Helper(Conf_GlobalSettings, setting, index, skip_tmpl);
			if (value) *index |= CONF_GET_NO_GLOBAL;
        }

    } else if (have_section && (! have_setting)) { // Enum Settings

        value = Conf_Get_Setting_Name(section, *index & CONF_INDEX_MASK, skip_tmpl);

    } else if ((! have_section) && (! have_setting)) { // Enum Sections

        value = Conf_Get_Section_Name(*index & CONF_INDEX_MASK, skip_tmpl);
    }

    ExReleaseResourceLite(Conf_Lock);
    KeLowerIrql(irql);

    return value;
}


//---------------------------------------------------------------------------
// Conf_Get
//---------------------------------------------------------------------------


_FX const WCHAR *Conf_Get(
    const WCHAR *section, const WCHAR *setting, ULONG index)
{
    return Conf_GetEx(section, setting, &index);
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

    //DbgPrint("Conf_Api_Reload\n");

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
    	map_init(&Conf_Data.sections_map, NULL);
    	Conf_Data.sections_map.func_key_size = NULL;
		Conf_Data.sections_map.func_match_key = &str_map_match;
		Conf_Data.sections_map.func_hash_key = &str_map_hash;
        map_resize(&Conf_Data.sections_map, 16); // prepare some buckets for better performance  

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
    WCHAR section_name[70];
    WCHAR setting_name[70];
    ULONG index;
	BOOLEAN no_expand;
    const WCHAR *value1;
    WCHAR *value2;

    //
    // prepare parameters
    // 

    // parms[1] --> WCHAR [66] SectionName

    memzero(section_name, sizeof(section_name));
    parm = (WCHAR *)parms[1];
    if (parm) {
        ProbeForRead(parm, sizeof(WCHAR) * 64, sizeof(WCHAR));
        if (parm[0])
            wcsncpy(section_name, parm, 64);
        else
            parm = NULL;
    }
    if (!parm && proc)
        wcscpy(section_name, proc->box->name);

    // parms[2] --> WCHAR [66] SettingName

    memzero(setting_name, sizeof(setting_name));
    parm = (WCHAR *)parms[2];
    if (parm) {
        ProbeForRead(parm, sizeof(WCHAR) * 64, sizeof(WCHAR));
        if (parm[0])
            wcsncpy(setting_name, parm, 64);
    }

    // parms[3] --> ULONG SettingIndex

    index = 0;
    parm2 = (ULONG *)parms[3];
    if (parm2) {
        ProbeForRead(parm2, sizeof(ULONG), sizeof(ULONG));
        index = *parm2;
        if ((index & CONF_INDEX_MASK) > CONF_MAX_LINES)
            return STATUS_INVALID_PARAMETER;
    } else
        return STATUS_INVALID_PARAMETER;

    no_expand = (index & CONF_GET_NO_EXPAND) != 0;

    //
    // get value
    //

    Conf_AdjustUseCount(TRUE);

    //if(index & CONF_FLAG_DEBUG)
	//    DbgPrint("Conf_Api_Query: %S %S 0x%08X\n", section_name, setting_name, index);
    if (/*(setting_name && setting_name[0] == L'%') ||*/ (index & CONF_JUST_EXPAND))
        value1 = setting_name; // shortcut to expand a variable
    else
        value1 = Conf_GetEx(section_name, setting_name, &index);
    if (! value1) {
        status = STATUS_RESOURCE_NAME_NOT_FOUND;
        goto release_and_return;
    }

    if (no_expand)
        value2 = (WCHAR *)value1;
    else {

        // expand value.  if caller is sandboxed, use its BOX (with its
        // expand_args) for that.  otherwise, create a temporary BOX

        if (proc)
            value2 = Conf_Expand(proc->box->expand_args, value1, setting_name);
        else {

            CONF_EXPAND_ARGS *expand_args = Mem_Alloc(Driver_Pool, sizeof(CONF_EXPAND_ARGS));
            if (! expand_args) {
                status = STATUS_UNSUCCESSFUL;
                goto release_and_return;
            }

            expand_args->pool = Driver_Pool;
            expand_args->sandbox = section_name;

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

            value2 = Conf_Expand(expand_args, value1, setting_name);

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

    parm2 = (ULONG *)parms[5];
    if (parm2) {
        ProbeForWrite(parm2, sizeof(ULONG), sizeof(ULONG));
        *parm2 = (index & CONF_FLAG_MASK);
    }

    return status;
}


//---------------------------------------------------------------------------
// Conf_Drop_Section
//---------------------------------------------------------------------------


_FX NTSTATUS Conf_Drop_Section(CONF_DATA *data, CONF_SECTION *section)
{
    CONF_SETTING *setting;

    List_Remove(&data->sections, section);
    map_remove(&data->sections_map, section->name);

    setting = List_Head(&section->settings);
    while (setting) {

        CONF_SETTING *next_setting = List_Next(setting);

        if(setting->name)
            Mem_FreeString(setting->name);
        if(setting->value)
            Mem_FreeString(setting->value);
        Mem_Free(setting, sizeof(CONF_SETTING));

        setting = next_setting;
    }

    if(section->include_path)
        Mem_FreeString(section->include_path);
    if(section->name)
        Mem_FreeString(section->name);
    Mem_Free(section, sizeof(CONF_SECTION));

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Conf_Update
//---------------------------------------------------------------------------


_FX NTSTATUS Conf_Update(
    CONF_DATA *data, const WCHAR* section_name, 
    const WCHAR* setting_name, const WCHAR* value_ptr, ULONG uMode)
{
	NTSTATUS status = STATUS_SUCCESS;

    CONF_SECTION *section;
    CONF_SETTING *setting;

    //
    // find an existing section by that name or create a new one
    //

    section = Conf_Find_Sections(data, section_name);

    if (! section) {
        
        if (uMode == CONF_REMOVE_VALUE || uMode == CONF_REMOVE_SECTION)
			return STATUS_SUCCESS; // nothing to do

        section = Conf_Add_Sections(data, section_name, TRUE);
        if (! section)
            return STATUS_INSUFFICIENT_RESOURCES;

        section->is_virtual = TRUE;
    }

    if (uMode == CONF_REMOVE_SECTION)
    {
        //
        // remove the section
        //

		return Conf_Drop_Section(data, section);
    }

	if ((uMode == CONF_UPDATE_VALUE || uMode == CONF_APPEND_VALUE /*|| uMode == CONF_INSERT_VALUE*/ || uMode == CONF_REMOVE_VALUE) && setting_name)
    {
        if (uMode != CONF_APPEND_VALUE /*&& uMode != CONF_INSERT_VALUE*/)
        {
            //
			// remove all old values
            //

            map_iter_t iter = map_key_iter(&section->settings_map, setting_name); // keyed iterator
            for(map_next(&section->settings_map, &iter); iter.node; ) {
                setting = iter.value;
                if (setting->from_template) 
                    break;

                if (!(uMode == CONF_REMOVE_VALUE && value_ptr && _wcsicmp(setting->value, value_ptr) != 0)) { // we are not looking for one specific value
                    map_erase(&section->settings_map, &iter);
                    List_Remove(&section->settings, setting);

                    if (setting->name)
                        Mem_FreeString(setting->name);
                    if (setting->value)
                        Mem_FreeString(setting->value);
                    Mem_Free(setting, sizeof(CONF_SETTING));
                }
                else
                    map_next(&section->settings_map, &iter);
            }
        }

        if (uMode != CONF_REMOVE_VALUE && value_ptr)
        {
            //
            // add the new value
            //

            setting = Conf_Add_Setting(data, section, setting_name, value_ptr, TRUE);

            if (!setting)
                return STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    //
	// Update templates if needed
    //

    if (uMode == CONF_UPDATE_TEMPLATES || (setting_name && _wcsicmp(setting_name, Conf_Template) == 0))
    {
        ULONG session_id = 0;
        MyGetSessionId(&session_id);

        if (uMode != CONF_APPEND_VALUE /*&& uMode != CONF_INSERT_VALUE*/)
        {
            map_iter_t iter = map_iter();
            for(map_next(&section->settings_map, &iter); iter.node; ) {
                setting = iter.value;
                if (setting->from_template) {
                    map_erase(&section->settings_map, &iter);
                    List_Remove(&section->settings, setting);

                    if (setting->name)
                        Mem_FreeString(setting->name);
                    if (setting->value)
                        Mem_FreeString(setting->value);
                    Mem_Free(setting, sizeof(CONF_SETTING));
                }
                else
                    map_next(&section->settings_map, &iter);                    
            }
        }

		if (uMode == CONF_UPDATE_TEMPLATES || (uMode == CONF_REMOVE_VALUE && value_ptr)) // remove one specific template, read all others
        {
            status = Conf_Merge_Templates(data, session_id, section, section, section->name); // rebuild all templates
        }
		else if (uMode != CONF_REMOVE_VALUE) // add new template
		{
			status = Conf_Merge_Template(data, session_id, value_ptr, section, section->name); // add one template
		}   
    }

    return status;
}


//---------------------------------------------------------------------------
// Conf_Api_Update
//---------------------------------------------------------------------------


_FX NTSTATUS Conf_Api_Update(PROCESS *proc, ULONG64 *parms)
{
    NTSTATUS status;
    WCHAR *parm;
	ULONG *parm2;
    WCHAR section_name[70];
    WCHAR setting_name[70];
    size_t value_len;
    WCHAR *value_ptr;
    ULONG uMode;

    if (proc)
        return STATUS_ACCESS_DENIED;

    // todo: uncomment
    //if (PsGetCurrentProcessId() != Api_ServiceProcessId) {
    //    if (Session_GetLeadSession(PsGetCurrentProcessId()) == 0)
    //        return STATUS_ACCESS_DENIED;
    //}

    //
    // prepare parameters
    // 

    // parms[1] --> ULONG uMode

    uMode = (ULONG)parms[1];
    if (uMode != CONF_UPDATE_VALUE && uMode != CONF_APPEND_VALUE /*&& uMode != CONF_INSERT_VALUE*/ && uMode != CONF_REMOVE_VALUE 
     && uMode != CONF_REMOVE_SECTION && uMode != CONF_UPDATE_TEMPLATES)
		return STATUS_INVALID_PARAMETER_1;

    // parms[2] --> WCHAR [66] SectionName

    memzero(section_name, sizeof(section_name));
    parm = (WCHAR *)parms[2];
    if (parm) {
        ProbeForRead(parm, sizeof(WCHAR) * 64, sizeof(WCHAR));
        if (parm[0])
            wcsncpy(section_name, parm, 64);
    }
    if(!*section_name)
        return STATUS_INVALID_PARAMETER_2;

    // parms[3] --> WCHAR [66] SettingName

    memzero(setting_name, sizeof(setting_name));
    parm = (WCHAR *)parms[3];
    if (parm) {
        ProbeForRead(parm, sizeof(WCHAR) * 64, sizeof(WCHAR));
        if (parm[0])
            wcsncpy(setting_name, parm, 64);
    }

    // parms[4] --> UNICODE_STRING64 SettingValue

    value_len = 0;
    value_ptr = NULL;
    if (parms[4]) {
        __try {

            status = Api_CopyStringFromUser(&value_ptr, &value_len, (UNICODE_STRING64*)parms[4]);

        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
            if (value_ptr)
                Mem_Free(value_ptr, value_len);
        }
        if (!NT_SUCCESS(status))
            return status;
    }

    KIRQL irql;
    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceExclusiveLite(Conf_Lock, TRUE);

	status = Conf_Update(&Conf_Data, section_name, setting_name, value_ptr, uMode);

    ExReleaseResourceLite(Conf_Lock);
    KeLowerIrql(irql);

    if(value_ptr)
        Mem_Free(value_ptr, value_len);

    return status;
}


//---------------------------------------------------------------------------
// Conf_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Conf_Init(void)
{
    Conf_Data.pool = NULL; //= Pool_Create();
    List_Init(&Conf_Data.sections);
    map_init(&Conf_Data.sections_map, NULL);
    Conf_Data.sections_map.func_key_size = NULL;
	Conf_Data.sections_map.func_match_key = &str_map_match;
	Conf_Data.sections_map.func_hash_key = &str_map_hash;

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
    Api_SetFunction(API_UPDATE_CONF,        Conf_Api_Update);

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
