#ifndef _MY_CMDLINE_H
#define _MY_CMDLINE_H

BOOLEAN CmdLine_Build(
    RTL_USER_PROCESS_PARAMETERS* process_parms,
    UNICODE_STRING* command_line_w,
    ANSI_STRING* command_line_a,
    UNICODE_STRING* command_line_peb);

#endif
