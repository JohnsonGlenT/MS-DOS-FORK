/* GNU Fidsk (gnufdisk-common), a library of common functions.
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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include <gnufdisk-common.h>

static jmp_buf sigsegv_jump;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void sigsegv_handler(int signum)
{
  longjmp(sigsegv_jump, signum);
} 

int gnufdisk_check_memory(void* _p, size_t _len, int _readonly)
{
  int err;
  int ret;
  struct sigaction action;
  struct sigaction old_action;

  pthread_mutex_lock(&mutex);

  memset(&action, 0, sizeof(action));
  action.sa_handler = &sigsegv_handler;
  action.sa_flags = SA_NODEFER;
  
  if(sigaction(SIGSEGV, &action, &old_action) != 0)
    {
      ret = -1;
      err = errno;
      goto lb_out;
    }

  if(setjmp(sigsegv_jump) == 0)
    {
      unsigned char* start;
      unsigned char* end;
      unsigned char byte;

      start = (unsigned char*) _p;
      end = start + _len;

      byte = *start;
  
      if(!_readonly)
        *start = byte;

      byte = *end;

      if(!_readonly)
        *end = byte;

      ret = 0;
      err = 0;
    }
  else
    {
      err = EFAULT;
      ret = -1;
    }

  sigaction(SIGSEGV, &old_action, NULL);

lb_out:

  pthread_mutex_unlock(&mutex);
 
  errno = err;

  return ret;
}

int gnufdisk_vfprintf(FILE* _f, const char* _fmt, va_list _args)
{
  int err;
  int ret;
  struct sigaction action;
  struct sigaction old_action;

  pthread_mutex_lock(&mutex);

  memset(&action, 0, sizeof(action));
  action.sa_handler = &sigsegv_handler;
  action.sa_flags = SA_NODEFER;
  
  if(sigaction(SIGSEGV, &action, &old_action) != 0)
    {
      err = errno;
      ret = -1;
      goto lb_out;
    }

  if(setjmp(sigsegv_jump) == 0)
    {
      if((ret = vfprintf(_f, _fmt, _args)) < 0)
	{
	  ret = -1;
	  err = errno;
	}
      else
	err = 0;
    }
  else
    {
      ret = -1;
      err = EFAULT;
    }
  
  sigaction(SIGSEGV, &old_action, NULL);

lb_out:

  pthread_mutex_unlock(&mutex);

  errno = err;
  return ret;
}

int gnufdisk_vasprintf(char** _dest, const char* _fmt, va_list _args)
{
  int ret;
  int err;
  struct sigaction action;
  struct sigaction old_action;
  char* buf;

  pthread_mutex_lock(&mutex);

  memset(&action, 0, sizeof(action));
  action.sa_handler = &sigsegv_handler;
  action.sa_flags = SA_NODEFER;
  
  if(sigaction(SIGSEGV, &action, &old_action) != 0)
    {
      ret = -1;
      err = errno;
      goto lb_out;
    }
  
  buf = NULL;

  if(setjmp(sigsegv_jump) == 0)
    {
      if((ret = vasprintf(&buf, _fmt, _args)) < 0)
	{
	  ret = -1;
	  err = errno;
	}
      else
	{
	  *_dest = buf;
	  err = 0;
	}
    }
  else
    {
      err = EFAULT;
      ret = -1;

      if(buf)
	free(buf);
    }

  sigaction(SIGSEGV, &old_action, NULL);

lb_out:

  pthread_mutex_unlock(&mutex);

  errno = err;
  return ret;
}

