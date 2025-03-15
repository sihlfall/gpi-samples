#ifndef SUCCESS_OR_DIE_H_
#define SUCCESS_OR_DIE_H_

#include <GASPI.h>
#include <GASPI_Ext.h>
#include <stdlib.h>

#define SUCCESS_OR_DIE(f...)                    \
do                                              \
{                                               \
  const gaspi_return_t r = f;                   \
                                                \
  if (r != GASPI_SUCCESS)                       \
  {                                             \
    gaspi_printf ("Error: '%s' [%s:%i]: %i\n", #f, __FILE__, __LINE__, r);\
                                                \
    exit (EXIT_FAILURE);                        \
  }                                             \
} while (0);

#endif