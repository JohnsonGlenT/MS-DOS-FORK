/* GNU Fidsk a program to manage partitions.
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
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <gnufdisk-common.h>
#include <gnufdisk-debug.h>
#include <gnufdisk-exception.h>
#include <gnufdisk-userinterface.h>

#if HAVE_CONFIG_H
# include "config.h"
#endif

#define MAIN_DEBUG 0

static void print_help(void)
{
  fprintf(stderr, 
          "USAGE:\n"
          "  gnufdisk IMPLEMENTATION ARGUMENT...\n"
          "\n"
          "Report bugs to %s\n"
          "\n",
          PACKAGE_BUGREPORT );
}

int main(int _argc, char** _argv)
{
  int ret;

  GNUFDISK_TRY(NULL, NULL)
    {
      struct gnufdisk_userinterface* ui;
      struct gnufdisk_string* implementation;
   
      if((ui = gnufdisk_userinterface_new()) == NULL)
       GNUFDISK_THROW(0, NULL, errno, NULL, "cannot create userinterface");
        
      if(_argc < 2)
        {
          print_help();
          GNUFDISK_THROW(0, NULL, ECANCELED, NULL, "invalid command line");
        }

      implementation = gnufdisk_string_new(_argv[1]);
  
      ret = gnufdisk_userinterface_run(ui, implementation, _argc - 2, &_argv[2]); 

      gnufdisk_string_delete(implementation);
      gnufdisk_userinterface_delete(ui);
    }
  GNUFDISK_CATCH_DEFAULT
    {
      ret = -1;
    }
  GNUFDISK_EXCEPTION_END;

  return ret != 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
