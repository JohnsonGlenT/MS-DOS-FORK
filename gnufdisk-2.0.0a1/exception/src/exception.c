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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <signal.h>

#define __USE_GNU
#include <pthread.h>

#include <gnufdisk-common.h>
#include <gnufdisk-exception.h>

enum
{
  EXCEPTION_NONE = 0x00,
  EXCEPTION_TRY = 0x45,
  EXCEPTION_THROW,
  EXCEPTION_CATCH
};

struct unwind_handler
{
  gnufdisk_exception_unwind_handler *handler;
  void *arg;
};

struct exception
{
  struct exception *prev;

  jmp_buf *jmp;
  gnufdisk_exception_handler *handler;
  void* handler_data;

  struct gnufdisk_exception_info* error_data;
  int state;

  struct unwind_handler **unwind_handlers;
  int nunwind_handlers;
};

struct context {
  pthread_t id;
  struct exception* current;
};

struct global_data {
  struct context** threads;
  int nthreads;
};

struct global_data gdata = {0};
pthread_mutex_t gmutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

static void fatal(const char* _file, const int _line, const char* _fmt, ...)
{
  va_list args;

  fprintf(stderr, "%s:%d: ", _file, _line);

  va_start(args, _fmt);
  vfprintf(stderr, _fmt, args);
  va_end(args);

  fprintf(stderr, " abort()\n");

  pthread_kill(pthread_self(), SIGABRT);
}

#define FATAL(_fmt...) fatal(__FILE__, __LINE__, _fmt)

static void* xmalloc(size_t _s)
{
  void* p;

  if((p = malloc(_s)) == NULL)
    FATAL("dynamic memory allocation failure");

  memset(p, 0, _s);

  return p;
}

static struct exception*
exception_new(jmp_buf* _jmp, gnufdisk_exception_handler* _h, void* _data)
{
  struct exception* e;

  e = xmalloc(sizeof(struct exception));

  e->jmp = _jmp;
  e->handler = _h;
  e->handler_data = _data;
  e->state = EXCEPTION_NONE;

  return e;
}

static void exception_delete(struct exception* _e)
{
  if(_e->unwind_handlers)
    {
      int iter;

      for(iter = 0; iter < _e->nunwind_handlers; iter++)
        {
          if(_e->unwind_handlers[iter] != NULL)
            free(_e->unwind_handlers[iter]);
        }

      free(_e->unwind_handlers);
    }

  free(_e);
}

static struct context* find_context(void)
{
  struct context* ret;
  pthread_t id;
  int iter;

  ret = NULL;
  id = pthread_self();

  pthread_mutex_lock(&gmutex); /* lock global data */

  for(iter = 0; iter < gdata.nthreads; iter++)
    if(gdata.threads[iter] != NULL && pthread_equal(gdata.threads[iter]->id, id))
      {
        ret = gdata.threads[iter];
        break;
      }
  
  pthread_mutex_unlock(&gmutex); /* lock global data */

  return ret;
}

static struct context* get_context(void)
{
  struct context* ret;
  pthread_t id;
  int iter;

  id = pthread_self();
  ret = NULL;

  pthread_mutex_lock(&gmutex); /* lock global data */

  ret = find_context();

  /* if ret is NULL we need a new fresh context */
  if(ret == NULL)
    {
      ret = xmalloc(sizeof(struct context));

      ret->id = id;

      for(iter = 0; iter < gdata.nthreads; iter++)
        if(gdata.threads[iter] == NULL)
          break;

      if(iter == gdata.nthreads)
        {
          struct context** threads;
          int nthreads;

          /* expand global vector */

          nthreads = gdata.nthreads + 4;
          threads = xmalloc(sizeof(struct context*) * nthreads);
          memset(threads, 0, sizeof(struct context*) * nthreads);

          if(gdata.threads)
            {
              memcpy(threads, gdata.threads, sizeof(struct context) * gdata.nthreads);
              free(gdata.threads);
            }

          gdata.threads = threads;
          gdata.nthreads = nthreads;
        }

      gdata.threads[iter] = ret;
    }

  pthread_mutex_unlock(&gmutex);

  return ret;
}

static void context_delete(void)
{
  pthread_t id;
  int iter;

  id = pthread_self();

  pthread_mutex_lock(&gmutex);

  for(iter = 0; iter < gdata.nthreads; iter++)
    if(gdata.threads[iter] != NULL && pthread_equal(gdata.threads[iter]->id, id))
      {
        free(gdata.threads[iter]);
        gdata.threads[iter] = NULL;
        break;
      }

  pthread_mutex_unlock(&gmutex);
}

static void call_unwind_handlers(struct exception* e)
{
  int iter;

  for(iter = e->nunwind_handlers - 1; iter >= 0; iter--)
    if(gnufdisk_check_memory(e->unwind_handlers[iter], sizeof(struct unwind_handler), 0) == 0
     && gnufdisk_check_memory(e->unwind_handlers[iter]->handler, 1, 1) == 0)
      {
        (*e->unwind_handlers[iter]->handler)(e->unwind_handlers[iter]->arg);
  
        free(e->unwind_handlers[iter]);
        e->unwind_handlers[iter] = NULL;
      }
}

void gnufdisk_exception_try(jmp_buf* _jmp, gnufdisk_exception_handler* _handler, void* _arg) 
{
  struct exception* e;
  struct context* c;

  if(_handler && gnufdisk_check_memory(_handler, 1, 1) != 0)
    {
      fprintf(stderr, "%s:%d warning: invalid gnufdisk_exception_handler* %p\n",
              __FILE__, __LINE__, _handler);
      _handler = NULL;
      _arg = NULL;
    }
  else if(_arg && gnufdisk_check_memory(_arg, 1, 1) != 0)
    {
      fprintf(stderr, "%s:%d warning: invalid handler agrument* %p\n",
              __FILE__, __LINE__, _arg);
      _handler = NULL;
      _arg = NULL;
    }

  c = get_context();
  e = exception_new(_jmp, _handler, _arg);
  e->prev = c->current;
  e->state = EXCEPTION_TRY;

  c->current = e;
}

void gnufdisk_exception_throw(const char* _file, 
                              const int _line, 
                              int _flags,
                              jmp_buf* _retry, 
                              int _error,  
                              void* _data, 
                              const char* _fmt, ...)
{
  struct context* c;
  struct exception* e;
  int prev_state;

  printf("%s:%s:%d: THROW FROM %s:%d, error: %d\n", __FILE__, __FUNCTION__, __LINE__, _file, _line, _error);

  c = get_context();
  e = c->current;

lb_continue:

  if(e == NULL)
    FATAL("There is no try context in this thread. Can not raise the exception signaled by: %s:%d", _file, _line);

  if(e->state == EXCEPTION_THROW)
    {
      c->current = e->prev;
      
      if(c->current)
        c->current->error_data = e->error_data;
      else if(e->error_data)
        {
          free(e->error_data->message);
          free(e->error_data->file);
          free(e->error_data);
        }

      call_unwind_handlers(e);
      exception_delete(e);
      e = c->current;

      goto lb_continue;
    }

  prev_state = e->state;
  e->state = EXCEPTION_THROW;

  /* allocate error data only if it is NULL */
  if(e->error_data == NULL)
    {
      struct gnufdisk_exception_info* d;
      va_list args;
      int err;

      d = xmalloc(sizeof(struct gnufdisk_exception_info));

      va_start(args, _fmt);
      err = gnufdisk_vasprintf(&d->message, _fmt, args);
      va_end(args);

      if(err == -1)
        FATAL("Can not format exception message: %s", strerror(errno));

      if((d->file = strdup(_file)) == NULL)
        FATAL("dynamic memory allocation failure");

      d->line = _line;
      d->error = _error;

      e->error_data = d;
    }

  if((_flags & GNUFDISK_EXCEPTION_MANAGEABLE) && e->handler != NULL)
    {
      if((*e->handler)(e->handler_data, e->error_data, _data) == 0)
        {
          if(_flags & GNUFDISK_EXCEPTION_LOCKABLE)
            {
              e->state = prev_state;
              
              if(e->error_data != NULL)
                {
                  free(e->error_data->message);
                  free(e->error_data->file);
                  free(e->error_data);
                  e->error_data = NULL;
                }
              
              longjmp(*_retry, 0xff);
            }
        }
    }

  call_unwind_handlers(e);

  longjmp(*e->jmp, e->error_data->error);
} 

void gnufdisk_exception_catch(struct gnufdisk_exception_info* _info)
{
  struct context* c;
  struct exception* e;

  c = get_context();
  e = c->current;

  if(e == NULL)
    FATAL("There are no try context on this thread");
  else if(e->state != EXCEPTION_THROW)
    FATAL("There is no exception to handle");

  e->state = EXCEPTION_CATCH;

  if(_info)
    memcpy(_info, e->error_data, sizeof(struct gnufdisk_exception_info));
}

void gnufdisk_exception_end(void)
{
  struct context* c;
  struct exception* e;

  c = get_context();
  e = c->current;

  if(e == NULL)
    FATAL("There are no try context on this thread.");
  
  if(e->state == EXCEPTION_THROW)
    {
      c->current = e->prev;

      if(c->current)
        c->current->error_data = e->error_data;
      else
        {
          free(e->error_data->message);
          free(e->error_data->file);
          free(e->error_data);
          free(e);
        }
          
      gnufdisk_exception_throw(0, 0, 0, NULL, 0, NULL, NULL);
    }

  c->current = e->prev;
  exception_delete(e);

  if(c->current == NULL)
    context_delete();
}

int gnufdisk_exception_register_unwind_handler(gnufdisk_exception_unwind_handler* _h, void* _a)
{
  struct context* c;
  struct exception* e;
  int iter;
  int err;
  
  if((err = gnufdisk_check_memory(_h, 1, 1)) != 0)
    {
      errno = err;
      return -1;
    }
  else if(_a != NULL && (err = gnufdisk_check_memory(_a, 1, 1)) != 0)
    {
      errno = err;
      return -1;
    }
  else if((c = find_context()) == NULL)
    {
      errno = ENXIO;
      return -1;
    }

  e = c->current;

  for(iter = 0; iter < e->nunwind_handlers; iter++)
    if(e->unwind_handlers[iter] == NULL)
      break;

  if(iter == e->nunwind_handlers)
    {
      struct unwind_handler** handlers;
      int nhandlers;

      nhandlers = e->nunwind_handlers + 8;
      handlers = xmalloc(sizeof(struct unwind_handler*) * nhandlers);

      if(e->unwind_handlers)
        {
          memcpy(handlers, e->unwind_handlers, sizeof(struct unwind_handler*) * e->nunwind_handlers);
          free(e->unwind_handlers);
        }

      e->unwind_handlers = handlers;
      e->nunwind_handlers = nhandlers;
    }

  e->unwind_handlers[iter] = xmalloc(sizeof(struct unwind_handler));
  e->unwind_handlers[iter]->handler = _h;
  e->unwind_handlers[iter]->arg = _a;

  return 0;
}

int gnufdisk_exception_unregister_unwind_handler(gnufdisk_exception_unwind_handler* _h, void* _a)
{
  struct context* c;
  struct exception* e;
  int iter;

  if((c = find_context()) == NULL)
    {
      errno = ENXIO;
      return -1;
    }

  e = c->current;

  for(iter = e->nunwind_handlers - 1; iter >= 0; iter --)
    if(e->unwind_handlers[iter] != NULL 
       && e->unwind_handlers[iter]->handler == _h 
       && e->unwind_handlers[iter]->arg == _a)
      {
        free(e->unwind_handlers[iter]);

        while(iter < e->nunwind_handlers - 1)
          {
            e->unwind_handlers[iter] = e->unwind_handlers[iter+1];
            iter++;
          }

        e->unwind_handlers[e->nunwind_handlers-1] = NULL;
        errno = 0;
        return 0;
      }
   
  errno = ENXIO;
  return -1;
}
