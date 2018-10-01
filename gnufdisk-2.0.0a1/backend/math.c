#include "common.h"

DIV_T math_div(gnufdisk_integer _a, gnufdisk_integer _b)
{
  if(_b == 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "avoid division by zero %lld and %lld", _a, _b);

  return DIV(_a, _b);
}

gnufdisk_integer math_round_up(gnufdisk_integer _val, gnufdisk_integer _grain)
{
  DIV_T res;
  gnufdisk_integer ret;

  res = math_div(_val, _grain);

  ret = res.quot * _grain;

  if(res.rem)
    ret += _grain;

  return ret;
}

gnufdisk_integer math_round_down(gnufdisk_integer _val, gnufdisk_integer _grain)
{
  DIV_T res;
  gnufdisk_integer ret;

  res = math_div(_val, _grain);

  ret = res.quot * _grain;

  if(ret < _grain)
    ret = _grain;

  return ret;
}

gnufdisk_integer math_round(gnufdisk_integer _val, gnufdisk_integer _grain)
{
  gnufdisk_integer down;
  gnufdisk_integer up;
  gnufdisk_integer ret;

  GNUFDISK_LOG((MATH, "round %" PRId64", grain: %"PRId64, _val, _grain));

  down = math_round_down(_val, _grain);
  up = math_round_up(_val, _grain);
  

  GNUFDISK_LOG((MATH, "round down: %"PRId64, down));
  GNUFDISK_LOG((MATH, "round up: %"PRId64, up));

  if(_val == down || _val - down < up - _val)
    ret = down;
  else
    ret = up;

  GNUFDISK_LOG((MATH, "done round, result: %"PRId64, ret));
  return ret;
}

