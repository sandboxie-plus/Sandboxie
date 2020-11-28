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

#include "driver.h"
#include "log_buff.h"

LOG_BUFFER* log_buffer_init(SIZE_T buffer_size)
{
	//LOG_BUFFER* ptr_buffer = (LOG_BUFFER*)malloc(sizeof(LOG_BUFFER) + buffer_size);
	LOG_BUFFER* ptr_buffer = (LOG_BUFFER*)ExAllocatePoolWithTag(PagedPool, sizeof(LOG_BUFFER) + buffer_size, tzuk);
	if (ptr_buffer != NULL)
	{
		ptr_buffer->seq_counter = 0;
		ptr_buffer->buffer_used = 0;
		ptr_buffer->buffer_size = buffer_size;
		ptr_buffer->buffer_start_ptr = ptr_buffer->buffer_data;
	}
	return ptr_buffer;
}

void log_buffer_free(LOG_BUFFER* ptr_buffer)
{
	//free(ptr_buffer);
	ExFreePoolWithTag(ptr_buffer, tzuk);
}

CHAR* log_buffer_push_entry(LOG_BUFFER_SIZE_T size, LOG_BUFFER* ptr_buffer)
{
	SIZE_T total_size = size + sizeof(LOG_BUFFER_SIZE_T) * 2 + sizeof(LOG_BUFFER_SEQ_T);
	if (total_size > ptr_buffer->buffer_size)
		return NULL;

	ptr_buffer->seq_counter++;

	while (ptr_buffer->buffer_size - ptr_buffer->buffer_used < total_size)
		log_buffer_pop_entry(ptr_buffer);

	CHAR* write_ptr = ptr_buffer->buffer_start_ptr + ptr_buffer->buffer_used;
	ptr_buffer->buffer_used += total_size;
	log_buffer_push_bytes((CHAR*)&size, sizeof(LOG_BUFFER_SIZE_T), &write_ptr, ptr_buffer);
	log_buffer_push_bytes((CHAR*)&ptr_buffer->seq_counter, sizeof(LOG_BUFFER_SEQ_T), &write_ptr, ptr_buffer);

	CHAR* end_ptr = write_ptr + size;
	log_buffer_push_bytes((CHAR*)&size, sizeof(LOG_BUFFER_SIZE_T), &end_ptr, ptr_buffer);

	return write_ptr;
}

void log_buffer_pop_entry(LOG_BUFFER* ptr_buffer)
{
	if (ptr_buffer->buffer_used)
	{
		CHAR* start_ptr = ptr_buffer->buffer_start_ptr;
		LOG_BUFFER_SIZE_T size = log_buffer_get_size(&start_ptr, ptr_buffer);
		SIZE_T total_size = size + sizeof(LOG_BUFFER_SIZE_T) * 2 + sizeof(LOG_BUFFER_SEQ_T);

		ptr_buffer->buffer_start_ptr += total_size;
		if (ptr_buffer->buffer_start_ptr >= ptr_buffer->buffer_data + ptr_buffer->buffer_size) // wrap around
			ptr_buffer->buffer_start_ptr -= ptr_buffer->buffer_size;
		ptr_buffer->buffer_used -= total_size;
	}
}

CHAR* log_buffer_byte_at(CHAR** data_ptr, LOG_BUFFER* ptr_buffer)
{
	if (*data_ptr >= ptr_buffer->buffer_data + ptr_buffer->buffer_size) // wrap around
		*data_ptr -= ptr_buffer->buffer_size;
	else if (*data_ptr < ptr_buffer->buffer_data) // wrap around
		*data_ptr += ptr_buffer->buffer_size;
	char* data = *data_ptr;
	*data_ptr += 1;
	return data;
}

BOOLEAN log_buffer_push_bytes(CHAR* data, SIZE_T size, CHAR** write_ptr, LOG_BUFFER* ptr_buffer)
{
	for (ULONG i = 0; i < size; i++)
		*log_buffer_byte_at(write_ptr, ptr_buffer) = data[i];
	return TRUE;
}

BOOLEAN log_buffer_get_bytes(CHAR* data, SIZE_T size, CHAR** read_ptr, LOG_BUFFER* ptr_buffer)
{
	for (ULONG i = 0; i < size; i++)
		data[i] = *log_buffer_byte_at(read_ptr, ptr_buffer);
	return TRUE;
}

LOG_BUFFER_SIZE_T log_buffer_get_size(CHAR** read_ptr, LOG_BUFFER* ptr_buffer)
{
	LOG_BUFFER_SIZE_T size = 0;
	log_buffer_get_bytes((char*)&size, sizeof(LOG_BUFFER_SIZE_T), read_ptr, ptr_buffer);
	return size;
}

LOG_BUFFER_SEQ_T log_buffer_get_seq_num(CHAR** read_ptr, LOG_BUFFER* ptr_buffer)
{
	LOG_BUFFER_SEQ_T seq_number = 0;
	log_buffer_get_bytes((char*)&seq_number, sizeof(LOG_BUFFER_SEQ_T), read_ptr, ptr_buffer);
	return seq_number;
}

CHAR* log_buffer_get_next(LOG_BUFFER_SEQ_T seq_number, LOG_BUFFER* ptr_buffer)
{
	// traverse the list backwards to find the next entry
	for (SIZE_T size_left = ptr_buffer->buffer_used; size_left > 0;)
	{
		CHAR* end_ptr = ptr_buffer->buffer_start_ptr + size_left - sizeof(LOG_BUFFER_SIZE_T);
		LOG_BUFFER_SIZE_T size = log_buffer_get_size(&end_ptr, ptr_buffer);
		SIZE_T total_size = size + sizeof(LOG_BUFFER_SIZE_T) * 2 + sizeof(LOG_BUFFER_SEQ_T);

		CHAR* read_ptr = end_ptr - total_size;

		CHAR* seq_ptr = read_ptr + sizeof(LOG_BUFFER_SIZE_T);
		LOG_BUFFER_SEQ_T cur_number = log_buffer_get_seq_num(&seq_ptr, ptr_buffer);

		if (cur_number == seq_number && size_left == ptr_buffer->buffer_used)
			return NULL; // the last entry in the list is the last one we already got, return NULL

		if (cur_number == seq_number + 1)
			return read_ptr; // this entry is the one after the last one we already got, return it

		size_left -= total_size;
	}

	if (ptr_buffer->buffer_used != 0)
		return ptr_buffer->buffer_start_ptr; // we havn't found the next entry and we have entries, so return the first entry
	return NULL; // the buffer is apparently empty, return NULL
}

/* Test Code
int main(int argc, char *argv[])
{
	LOG_BUFFER* my_buffer;
	my_buffer = log_buffer_init(60);

	for (__int64 i = 0; i < 5; i++)
	{
		char x = (i % 2) ? 0xbb : 0xaa;
		char test[4] = { x,x,x,x };

		CHAR* write_ptr = log_buffer_push_entry(4, my_buffer);
		printf("%s %d\n", QByteArray(my_buffer->buffer_data, my_buffer->buffer_size).toHex().data(), my_buffer->buffer_used);
		log_buffer_push_bytes(test, 4, &write_ptr, my_buffer);

		printf("%s %d\n\n", QByteArray(my_buffer->buffer_data, my_buffer->buffer_size).toHex().data(), my_buffer->buffer_used);
		//int pos = my_buffer.buffer_start_ptr - my_buffer.buffer_data;
		//printf("%s-%d\n", QByteArray(pos*2, ' ').data(), pos);

	}

	LOG_BUFFER_SEQ_T seq_number = 0;

	for (int i = 0; i < 50; i++)
	{
		CHAR* read_ptr = log_buffer_get_next(seq_number, my_buffer);
		if (!read_ptr)
			break;
		SIZE_T size = log_buffer_get_size(&read_ptr, my_buffer);
		LOG_BUFFER_SEQ_T cur_number = log_buffer_get_seq_num(&read_ptr, my_buffer);

		if (cur_number != seq_number + 1) {
			printf("overflow\n");
		}

		printf("%d\n", cur_number);

		seq_number = cur_number;
	}


	log_buffer_Free(my_buffer);

	return 0;
}
*/