#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdio.h>

#include <map>
#include <vector>
#include <set>
#include <algorithm>

////////////////////////////////////////////////////////////////////////////////

#ifndef NDEBUG
  //#define assert(_E_) (void)(_assert_((_E_), #_E_, __FILE__, __LINE__) ? 0 : (*((char *)0)) = 0)
  #define assert(_E_) (void)(_assert_((_E_), #_E_, __FILE__, __LINE__))
#else
  #define assert(_E_)
#endif

#define halt (void)(_assert_(0, "Halt reached", __FILE__, __LINE__))

bool _assert_(int exp, const char *exp_text, const char *file, int line);

////////////////////////////////////////////////////////////////////////////////

void mantissa_and_dec_exp(double value, long long &mantissa, int &dec_exp); //## IS THIS THE RIGHT PLACE FOR THIS FUNCTION?
