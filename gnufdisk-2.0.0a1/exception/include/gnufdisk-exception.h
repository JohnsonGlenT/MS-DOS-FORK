
/* GNU Fidsk (gnufdisk-exception), a library to manage exceptions.
 *
 * Copyright (C) 2011 Free Software Foundation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA. */

#ifndef GNUFDISK_EXCEPTION_H_INCLUDED
#define GNUFDISK_EXCEPTION_H_INCLUDED

#include <setjmp.h>

extern void* memset( );

enum gnufdisk_exception_flags {
	GNUFDISK_EXCEPTION_LOCKABLE = 0x01,
	GNUFDISK_EXCEPTION_MANAGEABLE = 0x02,
        GNUFDISK_EXCEPTION_ALL = 0x03
};

struct gnufdisk_exception_info
{
  char *message;
  char *file;
  int line;
  int error;
};

typedef int gnufdisk_exception_handler (void* _handler_data, 
																				struct gnufdisk_exception_info* _edata, 
																				void* _exception_data);

typedef void gnufdisk_exception_unwind_handler(void*);
int gnufdisk_exception_register_unwind_handler(gnufdisk_exception_unwind_handler* _h, void* _a);
int gnufdisk_exception_unregister_unwind_handler(gnufdisk_exception_unwind_handler* _h, void* _a);

extern void gnufdisk_exception_try(jmp_buf* _jmp, gnufdisk_exception_handler* _handler, void* _arg);
extern void gnufdisk_exception_catch(struct gnufdisk_exception_info* _info);
extern void gnufdisk_exception_end(void);
extern void gnufdisk_exception_throw(const char* _file, const int _line, int _flags, 
																		 jmp_buf* _retry, int _error,  void* _data, const char* _fmt, ...);

#define GNUFDISK_TRY(_handler, _data)                                          \
	do {                                                                         \
		jmp_buf gnufdisk_exception_variable__jmp;                                  \
		int gnufdisk_exception_variable__jmp_state;                                \
		struct gnufdisk_exception_info exception_info;                             \
			                                                                         \
		memset(&exception_info, 0, sizeof(exception_info));                        \
		                                                                           \
		gnufdisk_exception_try(&gnufdisk_exception_variable__jmp, _handler, _data);\
                                                                               \
		if((gnufdisk_exception_variable__jmp_state                                 \
				= setjmp(gnufdisk_exception_variable__jmp)) == 0)
	
#define GNUFDISK_CATCH(_val)                                                   \
		else if(gnufdisk_exception_variable__jmp_state == _val                     \
						&& (gnufdisk_exception_catch(&exception_info), 1))

#define GNUFDISK_CATCH_DEFAULT                                                 \
		else if((gnufdisk_exception_catch(&exception_info), 1))

#define GNUFDISK_EXCEPTION_END                                                 \
		gnufdisk_exception_end();                                                  \
	} while(0)

#define GNUFDISK_THROW(_flags, _retry, _error, _data, _fmt...)                 \
	gnufdisk_exception_throw(__FILE__, __LINE__, _flags,                         \
													 _retry, _error, _data, _fmt)
#define GNUFDISK_RETRY jmp_buf

#define GNUFDISK_RETRY_SET(_arg) setjmp(_arg)

#endif /* GNUFDISK_EXCEPTION_H_INCLUDED */

