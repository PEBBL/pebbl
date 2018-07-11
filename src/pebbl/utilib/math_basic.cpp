/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

#include <pebbl_config.h>
#include <stdlib.h>
#include <pebbl/utilib/math_basic.h>
#include <math.h>

namespace utilib {

std::string pscientific(const double t, int mantissa, int expwidth,
	std::ostream* os) {
  int my_exp=0;
  double my_t=t;
  bool neg, showpos, uc, negexp;
  std::string s;        // final number to print... as a string

  std::ios::fmtflags flags;

  if (os != NULL) {
    flags = os->flags();                // save current flags
    showpos = !!(flags & std::ios::showpos);
    uc = !!(flags & std::ios::uppercase );
    // mantissa = os->precision();      // if os passed, mantissa already set
    // width = os->width();             // nyi
  } else {                              // defaults if os not passed in
    showpos = false;
    uc = false;
  }

  // std::cout << mantissa << "," << expwidth << " pos/pnt/uc " << showpos << uc << std::endl;

  // work with positive numbers only
  if (my_t < 0.0) {
    neg = true;
    my_t = -my_t;
  } else
    neg = false;

  if (my_t != 0.0) {
    long double base10 = std::log10(static_cast<long double>(my_t));
    my_exp = static_cast<int>(base10);
    my_t = static_cast<double>(std::pow(static_cast<long double>(10), base10 - my_exp));
    // do nothing on 0.0 or error will occur
  }

// want to move decimal pt just past the first digit
// problem is the step function on the (int) log10(my_t)
  if (my_t > 0.0 && my_t < 1.0) {
    --my_exp;
    my_t *= 10.0;
  }
  while (my_t >= (10.0-std::pow(4.0,-1.0*(mantissa+1)))) {
	  my_t /= 10.0;
	  my_exp++;
  }

  if (my_exp < 0) {     // for ease of printing
    negexp = true;
    my_exp = -my_exp;
  } else
    negexp = false;

  std::string format;
  char* ss = new char [mantissa + expwidth + 10];

  // Ex: cout.setf(std::ios::showpos); cout << pscientific(.0123,4,3,&cout)
  format = neg?"-":(showpos?"+":"");		// +
  format += "%." + utilib::tostring(mantissa) + "f";	// +1.2300
  format += uc?'E':'e';				// +1.2300e
  format += negexp?'-':'+';			// +1.2300e-
  format += "%0" + utilib::tostring(expwidth) + 'd';	// +1.2300e-02
  // std::cout << format << endl;
#ifdef _MSC_VER
  sprintf_s(ss,mantissa+expwidth+10,format.c_str(),my_t,my_exp);
#else
  sprintf (ss, format.c_str(), my_t, my_exp);
#endif

  s = std::string(ss);
  delete ss;

//  if (os)			// restore flags
//    os->flags(flags);

  return s;
}



}   // namespace utilib




// JE this code used to be in utilib/math.c

#ifndef UTILIB_HAVE_LROUND
extern long int lround(double x);
#endif
void setup_bufexp(int tabsz, double xmin, double xmax);
double bufexp(double x);


#ifndef UTILIB_HAVE_LROUND
long int lround(double x)
{
double temp;

temp = floor(x);
if ((x - temp) < 0.5)
   return( (long int) temp );
else
   return( (long int) ceil(x) );
}
#endif

/* We hope this is a portable method for rounding a double to num_digits
   number of decimal digits after the decimal point. We may want to add
   some tolerances. */

double d_round(double to_round, unsigned int num_digits)
{
double intpart;
double shift_factor;
double val;

intpart = floor(to_round);
to_round -= intpart;
shift_factor = pow(10, num_digits);

val = to_round * shift_factor;
val += 0.5;
val = floor(val);
val /= shift_factor;
val += intpart;

return(val);
}




/*  Table for 'exp'
 *
 *  I don't know if this is a good way to do this.
 */

static int exp_tabsz=0;
static double exp_xmin=-999.0;
static double exp_xmax=-999.0;
static double exp_step=-999.0;
static double* exp_tab=0;

void setup_bufexp(int tabsz, double xmin, double xmax)
{
abort();
exp_tabsz=tabsz;
exp_xmin=xmin;
exp_xmax=xmax;
}

double bufexp(double x)
{
double ndx;
int lndx;

if (exp_tabsz == 0)
   setup_bufexp(5000,-70.0,70.0);

ndx = (x - exp_xmin)/exp_step;
if (ndx <= 0)
   return exp_tab[0];
if (ndx >= exp_tabsz-1)
   return exp_tab[exp_tabsz-1];
lndx = (int) floor(ndx);
return exp_tab[lndx] + (ndx - (double)lndx)*(exp_tab[lndx+1] - exp_tab[lndx]);
}


/// Calculates how many bits are needed to hold an unsigned value.
/// If the argument is 0, it returns 0.

unsigned int bitWidth(unsigned int x)
{
  unsigned int result;

  result = 0;

  while (x != 0)
    {
      result++;
      x >>= 1;
    }

  return result;
}


/* Computes the greatest common divisor of two integers using the */
/* classical Euclidean algorithm (remainder version).  Got this   */
/* particularly tight loop from Wikipedia!                        */

unsigned int gcd(unsigned int a, unsigned int b)
{
  while (b != 0)
    {
      register unsigned int temp = b;
      b = a % b;
      a = temp;
    }
  return a;
}
