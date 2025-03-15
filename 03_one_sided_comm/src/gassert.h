#ifndef GASSERT_H_
#define GASSERT_H_

#define ASSERT(f...)                            \
do                                              \
{                                               \
  if (!(f))                                     \
  {                                             \
    gaspi_printf ("Assertion failed: '%s' [%s:%i]\n", #f, __FILE__, __LINE__); \
                                                \
    exit (EXIT_FAILURE);                        \
  }                                             \
} while (0);

#endif