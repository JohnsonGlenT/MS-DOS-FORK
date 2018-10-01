#include "common.h"

static void reorder(unsigned char* _p, int _size)
{
  int iter1;
  int iter2;

  GNUFDISK_LOG((ENDIAN, "reorder buffer %p, size: %d", _p, _size));

  for(iter1 = 0, iter2 = _size -1; iter2 >= 0; iter1++, iter2--)
    {
      unsigned char tmp;

      tmp = _p[iter1];
      _p[iter1] = _p[iter2];
      _p[iter2] = tmp;
    }
}

UINT16 swap16(UINT16 _val)
{
  GNUFDISK_LOG((ENDIAN, "swap uint16_t " PRIu16, _val));
  
  reorder((unsigned char*) &_val, 2);
  return _val;
} 

UINT32 swap32(UINT32 _val)
{
  GNUFDISK_LOG((ENDIAN, "swap uint32_t " PRIu32, _val));

  reorder((unsigned char*) &_val, 4);
  return _val;
}

UINT64 swap64(UINT64 _val)
{
  GNUFDISK_LOG((ENDIAN, "swap uint64_t " PRIu64, _val));

  reorder((unsigned char*) &_val, 8);
  return _val;
}

