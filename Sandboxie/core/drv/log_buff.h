/*
 * Copyright 2020 DavidXanatos, xanasoft.com
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

#ifndef _MY_LOG_BUFFER_H
#define _MY_LOG_BUFFER_H

#define LOG_BUFFER_SIZE_T ULONG
#define LOG_BUFFER_SEQ_T ULONG

typedef struct _LOG_BUFFER
{
	LOG_BUFFER_SEQ_T seq_counter;
	SIZE_T buffer_size;
	SIZE_T buffer_used;
	CHAR* buffer_start_ptr;
	CHAR buffer_data[0]; // [[SIZE 4][DATA n][SEQ 4][SITE 4]][...] // Note 2nd size tags allows to traverse the ring in both directions
} LOG_BUFFER;

LOG_BUFFER* log_buffer_init(SIZE_T buffer_size);
void log_buffer_free(LOG_BUFFER* ptr_buffer);

CHAR* log_buffer_push_entry(LOG_BUFFER_SIZE_T size, LOG_BUFFER* ptr_buffer, BOOLEAN can_pop);
void log_buffer_pop_entry(LOG_BUFFER* ptr_buffer);
CHAR* log_buffer_byte_at(CHAR** data_ptr, LOG_BUFFER* ptr_buffer);
BOOLEAN log_buffer_push_bytes(CHAR* data, SIZE_T size, CHAR** write_ptr, LOG_BUFFER* ptr_buffer);
BOOLEAN log_buffer_get_bytes(CHAR* data, SIZE_T size, CHAR** read_ptr, LOG_BUFFER* ptr_buffer);
LOG_BUFFER_SIZE_T log_buffer_get_size(CHAR** read_ptr, LOG_BUFFER* ptr_buffer);
LOG_BUFFER_SEQ_T log_buffer_get_seq_num(CHAR** read_ptr, LOG_BUFFER* ptr_buffer);
CHAR* log_buffer_get_next(LOG_BUFFER_SEQ_T seq_number, LOG_BUFFER* ptr_buffer);

#endif // _MY_LOG_BUFFER_H
