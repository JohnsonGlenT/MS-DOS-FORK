#include "common.h"

struct list {
  void* data;
  struct list* prev;
  struct list* next;
};

static void list_check(struct list* _l)
{
  if(gnufdisk_check_memory(_l, sizeof(struct list), 0) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "invalid struct list* %p", _l); 
}

static void delete_list(void* _p)
{
  GNUFDISK_LOG((LIST, "delete struct list* %p", _p));
  free(_p);
}

struct list* list_prev(struct list* _n)
{
  list_check(_n);
  return _n->prev;
}

struct list* list_next(struct list* _n)
{
  list_check(_n);
  return _n->next;
}

void* list_data(struct list* _n)
{
  list_check(_n);
  return _n->data;
}

struct list* list_first(struct list* _n)
{
  struct list* iter;

  for(iter = _n; list_check(iter), iter->prev != NULL; iter = iter->prev)
    ;

  return iter;
}

struct list* list_last(struct list* _n)
{
  struct list* iter;

  for(iter = _n; list_check(iter), iter->next != NULL; iter = iter->next)
    ;

  return iter;
}

struct list* list_append(struct list* _n, void* _data)
{
  struct list* node;

  if((node = malloc(sizeof(struct list))) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&delete_list, node);

  memset(node, 0, sizeof(struct list));

  node->data = _data;

  if(_n != NULL)
    {
      struct list* last;

      last = list_last(_n);
    
      last->next = node;
      node->prev = last; 
    }

  gnufdisk_exception_unregister_unwind_handler(&delete_list, node);

  return _n != NULL ? list_first(_n) : node;
}

struct list* list_append_ordered(struct list* _n, int (*_compare)(const void*, const void*), void* _data)
{
  struct list* node;

  GNUFDISK_LOG((LIST, "perform append on struct list* %p", _n));

  if((node = malloc(sizeof(struct list))) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&delete_list, node);

  GNUFDISK_LOG((LIST, "new struct list* %p", node));

  memset(node, 0, sizeof(struct list));

  node->data = _data;

  if(_n != NULL)
    {
      struct list* iter;

      for(iter = list_first(_n); gnufdisk_check_memory(iter, sizeof(struct list), 0) == 0; iter = list_next(iter))
	{
	  void* iter_data;

	  GNUFDISK_LOG((LIST, "step iterating list, node: %p", iter));

	  iter_data = list_data(iter);

	  if((*_compare)(iter_data, _data) > 0)
	    {
	      struct list* prev;

	      prev = iter->prev;

	      iter->prev = node;
	      node->next = iter;
	      node->prev = prev;

	      if(gnufdisk_check_memory(prev, sizeof(struct list), 0) == 0)
		prev->next = node;

	      GNUFDISK_LOG((LIST, "new node placed before %p", iter));

	      break;
	    }
	} 
   
      if(node->prev == NULL && node->next == NULL)
	{
	  struct list* last;

	  GNUFDISK_LOG((LIST, "append node to the end of list"));

	  last = list_last(_n);

	  last->next = node;
	  node->prev = last;
	}	
    }

  gnufdisk_exception_unregister_unwind_handler(&delete_list, node);

  return _n != NULL ? list_first(_n) : node;
}

struct list* list_insert(struct list* _n, void* _data)
{
  struct list* node;

  if((node = malloc(sizeof(struct list))) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&delete_list, node);

  memset(node, 0, sizeof(struct list));

  node->data = _data;

  if(gnufdisk_check_memory(_n, sizeof(struct list), 0) == 0)
    {
      _n->prev = node;
      node->next = _n;
      
      gnufdisk_exception_unregister_unwind_handler(&delete_list, node);
      return list_first(_n);
    }
  
  gnufdisk_exception_unregister_unwind_handler(&delete_list, node);

  return node; 
}

void* list_remove(struct list* _n)
{
  struct list* prev;
  struct list* next;
  void* data;

  list_check(_n);
  
  prev = _n->prev;
  next = _n->next;
  data = _n->data;

  if(gnufdisk_check_memory(_n, sizeof(struct list), 0) == 0)
    prev->next = next;

  if(gnufdisk_check_memory(_n, sizeof(struct list), 0) == 0)
    next->prev = prev;

  free(_n);

  return data;
}

void list_delete(struct list* _n, void (*_free)(void*))
{
  list_check(_n);

  _n = list_first(_n);

  while(_n != NULL)
    {
      struct list* next;

      next = list_next(_n);

      if(gnufdisk_check_memory(_free, 1, 1) == 0)
	(*_free)(_n->data);

      free(_n);

      _n = next;
    }
}
