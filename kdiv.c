/*
 * File       : kdiv.c                                                          
 * Description: Generator and calculator for division by integer constant 
 *              routines. Routines "magic" and "magicu" have been copied
 *              from Herny S. Warren's "Hacker's Delight". 
 * Author     : Nikolaos Kavvadias <nikolaos.kavvadias@gmail.com>                
 * Copyright  : (C) Nikolaos Kavvadias 2011, 2012, 2013, 2014                 
 * Website    : http://www.nkavvadias.com                            
 *                                                                          
 * This file is part of kdiv, and is distributed under the terms of the  
 * Modified BSD License.
 *
 * A copy of the Modified BSD License is included with this distrubution 
 * in the files COPYING.BSD.
 * kdiv is free software: you can redistribute it and/or modify it under the
 * terms of the Modified BSD License. 
 * kdiv is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the Modified BSD License for more details.
 * 
 * You should have received a copy of the Modified BSD License along with 
 * kdiv. If not, see <http://www.gnu.org/licenses/>. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* Absolute value of an integer. */
#define ABS(x)            ((x) >  0 ? (x) : (-x))

// ------------------------------ cut ----------------------------------
struct mu {unsigned int M;     // Magic number,
          int a;               // "add" indicator,
          int s;};             // and shift amount.

struct ms {int M;          // Magic number
          int s;};         // and shift amount.         
// ---------------------------- end cut --------------------------------

int divisor_val=1, width_val=32, lo_val=0, hi_val=65535;
int enable_debug=0, enable_errors=0;
int enable_signed=0, enable_unsigned=1;
int enable_nac=1, enable_ansic=0;


/* print_spaces:
 * Print a configurable number of space characters to an output file (specified 
 * by the given filename; the file is assumed already opened).
 */
void print_spaces(FILE *f, int nspaces)
{
  int i;  
  for (i = 0; i < nspaces; i++)
  {
    fprintf(f, " ");
  }
}

/* pfprintf: 
 * fprintf prefixed by a number of space characters. 
 */
void pfprintf(FILE *f, int nspaces, char *fmt, ...)
{
  va_list args;
  print_spaces(f, nspaces);
  va_start(args, fmt);
  vfprintf(f, fmt, args);
  va_end(args);
}

/* ispowof2:
 * Function to identify if the given unsigned integer is a power-of-2.
 */
int ispowof2(unsigned int v)
{
  int f;
  f = v && !(v & (v-1));
  return (f);
}

/* log2ceil:
 * Function to calculate the ceiling of the binary logarithm of a given positive 
 * integer n.
 */
int log2ceil(unsigned int inpval)
{
  unsigned int max = 1; // exp=0 => max=2^0=1
  unsigned int logval = 0;

  if (inpval < 0)
  {
    fprintf(stderr, "Error: Result of log2 computation is NAN.\n");
    exit(1);
  }
  else if (inpval == 0)
  {
    fprintf(stderr, "Error: Result of log2 computation is MINUS_INFINITY.\n");
    exit(1);
  }
  // inpval is positive
  else
  {
    // log computation loop
    while (max < inpval)
    {
      // increment exponent
      logval = logval + 1;
      //  max = 2^logval
      max = max * 2;
    }
  }

  // exponent that gives (2^logval) >= inpval
  return (logval);
}

/* ipowul:
 * Calculate integer power supporting results up to 64-bits.
 */
unsigned long long int ipowul(int base, int exponent)
{
  unsigned long long int temp;
  int i;
  
  temp = 1;
  
  for (i = 0; i < exponent; i++)
  {
    temp *= (unsigned int)base;     
  }

  return (temp);
}

/* magicu:
 * Calculates the multiplicative inverse of an integer divisor for unsigned 
 * division.
 */
struct mu magicu(unsigned d) {
                           // Must have 1 <= d <= 2**32-1.
   int p;
   unsigned nc, delta, q1, r1, q2, r2;
   struct mu magu;

   magu.a = 0;             // Initialize "add" indicator.
   nc = -1 - (-d)%d;       // Unsigned arithmetic here.
   p = 31;                 // Init. p.
   q1 = 0x80000000/nc;     // Init. q1 = 2**p/nc.
   r1 = 0x80000000 - q1*nc;// Init. r1 = rem(2**p, nc).
   q2 = 0x7FFFFFFF/d;      // Init. q2 = (2**p - 1)/d.
   r2 = 0x7FFFFFFF - q2*d; // Init. r2 = rem(2**p - 1, d).
   do {
      p = p + 1;
      if (r1 >= nc - r1) {
         q1 = 2*q1 + 1;            // Update q1.
         r1 = 2*r1 - nc;}          // Update r1.
      else {
         q1 = 2*q1;
         r1 = 2*r1;}
      if (r2 + 1 >= d - r2) {
         if (q2 >= 0x7FFFFFFF) magu.a = 1;
         q2 = 2*q2 + 1;            // Update q2.
         r2 = 2*r2 + 1 - d;}       // Update r2.
      else {
         if (q2 >= 0x80000000) magu.a = 1;
         q2 = 2*q2;
         r2 = 2*r2 + 1;}
      delta = d - 1 - r2;
   } while (p < 64 &&
           (q1 < delta || (q1 == delta && r1 == 0)));

   magu.M = q2 + 1;        // Magic number
   magu.s = p - 32;        // and shift amount to return
   return magu;            // (magu.a was set above).
}

/* magic:
 * Calculates the multiplicative inverse of an integer divisor for signed 
 * division.
 */
struct ms magic(int d) {   // Must have 2 <= d <= 2**31-1
                           // or   -2**31 <= d <= -2.
   int p;
   unsigned ad, anc, delta, q1, r1, q2, r2, t;
   const unsigned two31 = 0x80000000;     // 2**31.
   struct ms mag;

   ad = abs(d);
   t = two31 + ((unsigned)d >> 31);
   anc = t - 1 - t%ad;     // Absolute value of nc.
   p = 31;                 // Init. p.
   q1 = two31/anc;         // Init. q1 = 2**p/|nc|.
   r1 = two31 - q1*anc;    // Init. r1 = rem(2**p, |nc|).
   q2 = two31/ad;          // Init. q2 = 2**p/|d|.
   r2 = two31 - q2*ad;     // Init. r2 = rem(2**p, |d|).
   do {
      p = p + 1;
      q1 = 2*q1;           // Update q1 = 2**p/|nc|.
      r1 = 2*r1;           // Update r1 = rem(2**p, |nc|).
      if (r1 >= anc) {     // (Must be an unsigned
         q1 = q1 + 1;      // comparison here).
         r1 = r1 - anc;}
      q2 = 2*q2;           // Update q2 = 2**p/|d|.
      r2 = 2*r2;           // Update r2 = rem(2**p, |d|).
      if (r2 >= ad) {      // (Must be an unsigned
         q2 = q2 + 1;      // comparison here).
         r2 = r2 - ad;}
      delta = ad - r2;
   } while (q1 < delta || (q1 == delta && r1 == 0));

   mag.M = q2 + 1;
   if (d < 0) mag.M = -mag.M; // Magic number and
   mag.s = p - 32;            // shift amount to return.
   return mag;
}

/* 
   NOTES on unsigned division by constant.
1) Unsigned division by powers-of-2, with d = 2^k
  shr   q, n, k

2) Unsigned division with a = 0 (additive factor)
  li    M, dinv             // dinv = multiplicative inverse of d 
  mulhu q, M, n
  shri  q, q, s             // s is the shift factor
  
3) Unsigned division with a = 1
  li    M, dinv
  mulhu q, M, n
  add   q, q, n
  shrxi q, q, s             // an extended shr immediate using the carry and 
                            // q (concatenated); then performing logical shift
*/     

/* emit_kdivu_nac:
 * Emit the NAC (generic assembly language) implementation of unsigned division 
 * by constant.
 */
void emit_kdivu_nac(FILE *f, unsigned int M, int a, int s, unsigned int d, unsigned int W)
{ 
  pfprintf(f, 0, "procedure kdiv_u%d_p_%d (in u%d n, out u%d y)\n", 
    W, d, W, W);
  pfprintf(f, 0, "{\n");   
  pfprintf(f, 2, "localvar u%d q, M;\n", W);   
  pfprintf(f, 2, "localvar u%d t0, t1;\n", 2*W);   
  if (a == 1)
  {
    pfprintf(f, 2, "localvar u%d n0;\n", 2*W);   
  }
  pfprintf(f, 0, "S_1:\n");
  
  if (ispowof2(d) == 1)
  {
    // shr   q, n, k
    pfprintf(f, 2, "q <= shr n, %d;\n", log2ceil(d));
  }
  else if (a == 0)
  {
    pfprintf(f, 2, "M <= ldc %u;\n", M);
    // mulhu q, M, n
    pfprintf(f, 2, "t0 <= mul M, n;\n");
    pfprintf(f, 2, "t1 <= shr t0, %d;\n", W);
    pfprintf(f, 2, "q <= trunc t1;\n");
    // shri  q, q, s
    pfprintf(f, 2, "q <= shr q, %d;\n", s);
  }
  else if (a == 1)
  {
    pfprintf(f, 2, "M <= ldc %u;\n", M);
    // mulhu q, M, n
    pfprintf(f, 2, "t0 <= mul M, n;\n");
    pfprintf(f, 2, "t1 <= shr t0, %d;\n", W);
    pfprintf(f, 2, "q <= trunc t1;\n");
    // add   q, q, n
    // t = q + n;
    // q = t & 0xFFFFFFFF;
    pfprintf(f, 2, "t0 <= zxt q;\n");
    pfprintf(f, 2, "n0 <= zxt n;\n");
    pfprintf(f, 2, "t0 <= add t0, n0;\n");
    pfprintf(f, 2, "q <= trunc t0;\n");    
//    pfprintf(f, 2, "q <= and t, %u;\n", 0xFFFFFFFF);
    // shrxi q, q, s        // an extended shr immediate using the carry and 
                            // q (concatenated); then performing logical shift  
    pfprintf(f, 2, "q <= shr q, %d;\n", s);
  }
  else
  {
    fprintf(stderr, "Error: Unsupported constant division.\n");
    exit(1);
  }  
  pfprintf(f, 2, "y <= mov q;\n");
  pfprintf(f, 0, "}\n"); 
}

/* emit_kdivu_ansic:
 * Emit the ANSI C implementation of unsigned division by constant.
 */                       
void emit_kdivu_ansic(FILE *f, unsigned int M, int a, int s, unsigned int d, unsigned int W)
{
  pfprintf(f, 0, "unsigned int kdiv_u%d_p_%d (unsigned int n)\n", W, d);
  pfprintf(f, 0, "{\n");   
  pfprintf(f, 2, "unsigned int q, M=%u;\n", M);   
  pfprintf(f, 2, "unsigned long long int t;\n");   
  
  if (ispowof2(d) == 1)
  {
    pfprintf(f, 2, "q = n >> %d;\n", log2ceil(d));
  }
  else if (a == 0)
  {
    // mulhu q, M, n
    pfprintf(f, 2, "t = (unsigned long long int)M * (unsigned long long int)n;\n");
    pfprintf(f, 2, "q = t >> %d;\n", W);
    if (s > 0)
    {
      // shri  q, q, s
      pfprintf(f, 2, "q = q >> %d;\n", s);
    }
  }
  else if (a == 1)
  {
    // mulhu q, M, n
    pfprintf(f, 2, "t = (unsigned long long int)M * (unsigned long long int)n;\n");
    pfprintf(f, 2, "q = t >> %d;\n", W);    
    // add   q, q, n
    pfprintf(f, 2, "t = q + n;\n");
    pfprintf(f, 2, "q = t & %d;\n", ipowul(2, W)-1);
    // shrxi q, q, s        // an extended shr immediate using the carry and 
                            // q (concatenated); then performing logical shift  
    if (s > 0)
    {
      pfprintf(f, 2, "q = t >> %d;\n", s);
    }
  }
  else
  {
    fprintf(stderr, "Error: Unsupported constant division.\n");
    exit(1);
  }  
  pfprintf(f, 2, "return (q);\n");
  pfprintf(f, 0, "}\n");
}

/* calculate_kdivu:
 * Perform an unsigned division by constant according to "Hacker's Delight" 
 * routines.
 */
unsigned int calculate_kdivu(unsigned int M, int a, int s, unsigned int n, unsigned int d, unsigned int W)
{
  unsigned long long int t;
  unsigned int q;
  
  if (ispowof2(d) == 1)
  {
    // shr   q, n, k
    q = n >> log2ceil(d);  // NOTE: log2ceil() accepts int and not unsigned int.
  }
  else if (a == 0)
  {
    // mulhu q, M, n
    t = (unsigned long long int)M * (unsigned long long int)n;
    q = t >> W;
    // shri  q, q, s
    q = q >> s;
  }
  else if (a == 1)
  {
    // mulhu q, M, n
    t = (unsigned long long int)M * (unsigned long long int)n;
    q = t >> W;    
    // add   q, q, n
    t = q + n;
    q = t & (ipowul(2, W) - 1);
    // shrxi q, q, s        // an extended shr immediate using the carry and 
                            // q (concatenated); then performing logical shift  
    q = t >> s;    
  }
  else
  {
    fprintf(stderr, "Error: Unsupported constant division.\n");
    exit(1);
  }  
  return (q);
}

/* 
   NOTES on signed division by constant.
4) Signed division for non-powers-of-2
  li    M, dinv
  mulhs q, M, n
  add   q, q, n             // correction term only for d = 7
  shrsi q, q, s
  shri  t, n, W-1           // W is the word length
  add   q, q, t
  add   q, q, 1             // for negative divisors (d < 0)

5) Signed division by powers-of-2, with d = 2^k
  shrsi t, n, k-1
  shri  t, t, W-k
  add   t, n, t
  shrsi q, t, k
  neg   q, q                // for negative divisors (d < 0)
*/     

/* emit_kdivs_nac:
 * Emit the NAC (generic assembly language) implementation of signed division 
 * by constant.
 */                       
void emit_kdivs_nac(FILE *f, int M, int s, int d, unsigned int W)
{
  int k;
  
  pfprintf(f, 0, "procedure kdiv_s%d_", W);
  if (d < 0)
  {
    fprintf(f, "m_");
  }
  else
  {
    fprintf(f, "p_");
  }
  fprintf(f, "%d (in s%d n, out s%d y)\n", 
    ABS(d), W, W);
  pfprintf(f, 0, "{\n");   
  pfprintf(f, 2, "localvar s%d q, M, c;\n", W);   
  pfprintf(f, 2, "localvar s%d t, u, v;\n", 2*W);   
  pfprintf(f, 0, "S_1:\n");
  
  k = log2ceil(ABS(d));
  if (d == 1)
  {
    // mov q, n
    pfprintf(f, 2, "q <= mov n;\n");
  }
  else if (d == -1)
  {
    // neg q, n
    pfprintf(f, 2, "q <= neg n;\n");
  }
  else if (ispowof2(d) == 1)
  {
    // shrsi t, n, k-1
    pfprintf(f, 2, "t <= sxt n;\n");
    pfprintf(f, 2, "t <= shr t, %d;\n", k-1);   
    // shri  t, t, W-k
//    u = t >> (W-k);
    pfprintf(f, 2, "u <= shr t, %d;\n", W-k);
    // add   t, n, t
    pfprintf(f, 2, "v <= sxt n;\n");
    pfprintf(f, 2, "t <= add v, u;\n");
    // shrsi q, t, k
    pfprintf(f, 2, "t <= shr t, %d;\n", k);
    pfprintf(f, 2, "q <= trunc t;\n");
    // neg   q, q                // for negative divisors (d < 0)
    if (d < 0)
    {
      pfprintf(f, 2, "q <= neg q;\n");
    }
  }
  else
  {
    pfprintf(f, 2, "M <= ldc %d;\n", M);
    // mulhs q, M, n
    pfprintf(f, 2, "t <= mul M, n;\n");    
    pfprintf(f, 2, "u <= shr t, %d;\n", W);
    pfprintf(f, 2, "q <= trunc u;\n");    
    // add|sub  q, q, n             // correction term for certain divisors
    if ((d > 0) && (M < 0))
    {
      pfprintf(f, 2, "q <= add q, n;\n");
    }    
    else if ((d < 0) && (M > 0))
    {
      pfprintf(f, 2, "q <= sub q, n;\n");
    }
    // shrsi q, q, s
//      q = q >> s;
    if (s > 0)
    {
      pfprintf(f, 2, "q <= shr q, %d;\n", s);
    }
    // shri  t, n, W-1           // W is the word length
//    c = n >> (W-1);
    pfprintf(f, 2, "c <= shr n, %d;\n", W-1);
    // add   q, q, t
//    q = q + c;
    pfprintf(f, 2, "q <= add q, c;\n");
    // add   q, q, 1             // for negative divisors (d < 0) and (n != 0)
    if (d < 0)
    {
      pfprintf(f, 2, "c <= setne n, 0;\n");
      pfprintf(f, 2, "q <= add q, c;\n");
    }
  }
  pfprintf(f, 2, "y <= mov q;\n");
  pfprintf(f, 0, "}\n"); 
}

/* emit_kdivs_ansic:
 * Emit the ANSI C implementation of signed division by constant.
 */  
void emit_kdivs_ansic(FILE *f, int M, int s, int d, unsigned int W)
{
  int k;
  
  pfprintf(f, 0, "signed int kdiv_s%d_", W);
  if (d < 0)
  {
    fprintf(f, "m_");
  }
  else
  {
    fprintf(f, "p_");
  }
  fprintf(f, "%d (signed int n)\n", ABS(d));
  pfprintf(f, 0, "{\n");   
  pfprintf(f, 2, "signed int q, M=%d, c;\n", M);   
  pfprintf(f, 2, "signed long long int t, u, v;\n");   

  k = log2ceil(ABS(d));
  if (d == 1)
  {
    pfprintf(f, 2, "q = n;\n");
  }
  else if (d == -1)
  {
    pfprintf(f, 2, "q = -n;\n");
  }
  else if (ispowof2(d) == 1)
  {
    // shrsi t, n, k-1
    pfprintf(f, 2, "t = n >> %d;\n", k-1);
    // shri  t, t, W-k
    pfprintf(f, 2, "u = t >> %d;\n", W-k);
    // add   t, n, t
    pfprintf(f, 2, "t = n + u;\n");
    // shrsi q, t, k
    pfprintf(f, 2, "q = t >> %d;\n", k);
    // neg   q, q                // for negative divisors (d < 0)
    if (d < 0)
    {
      pfprintf(f, 2, "q = -q;\n");
    }
  }
  else
  {
    // mulhs q, M, n
    pfprintf(f, 2, "t = (signed long long int)M * (signed long long int)n;\n");
    pfprintf(f, 2, "q = t >> %d;\n", W);   
    // add|sub  q, q, n             // correction term for certain divisors
    if ((d > 0) && (M < 0))
    {
      pfprintf(f, 2, "q = q + n;\n");
    }    
    else if ((d < 0) && (M > 0))
    {
      pfprintf(f, 2, "q = q - n;\n");
    }
    if (s > 0)
    {
      // shrsi q, q, s
      pfprintf(f, 2, "q = q >> %d;\n", s);
    }
    // shri  t, n, W-1           // W is the word length
    pfprintf(f, 2, "c = n >> %d;\n", W-1);
    // add   q, q, t
    pfprintf(f, 2, "q = q + c;\n");
    // add   q, q, 1             // for negative divisors (d < 0) and (n != 0)
    if (d < 0)
    {
      pfprintf(f, 2, "c = (n != 0);\n");
      pfprintf(f, 2, "q = q + 1;\n");
    }
  }
  pfprintf(f, 2, "return (q);\n");
  pfprintf(f, 0, "}\n");
}

/* calculate_kdivs:
 * Perform a signed division by constant according to "Hacker's Delight" 
 * routines.
 */
int calculate_kdivs(int M, int s, int n, int d, unsigned int W)
{
  signed long long int t, u;
  int q;
  int k;
  unsigned int c;
  
  k = log2ceil(ABS(d));
  if (d == 1)
  {
    q = n;
  }
  else if (d == -1)
  {
    q = -n;
  }
  else if (ispowof2(d) == 1)
  {
    // shrsi t, n, k-1
    t = n >> (k-1);
    // shri  t, t, W-k
    u = t >> (W-k);
    // add   t, n, t
    t = n + u;
    // shrsi q, t, k
    q = t >> k;
    // neg   q, q                // for negative divisors (d < 0)
    if (d < 0)
    {
      q = -q;
    }
  }
  else
  {
    // To use the results of this program, the compiler should generate the 
    // li and mulhs instructions, generate the add if d > 0 and M < 0, or 
    // the sub if d < 0 and M > 0, and generate the shrsi if s > 0. Then, 
    // the shri and final add must be generated.  
    // mulhs q, M, n
    t = (signed long long int)M * (signed long long int)n;
    q = t >> W;   
    // add|sub  q, q, n             // correction term for certain divisors
    if ((d > 0) && (M < 0))
    {
      q = q + n;
    }    
    else if ((d < 0) && (M > 0))
    {
      q = q - n;
    }
    // shrsi q, q, s
    q = q >> s;
    // shri  t, n, W-1           // W is the word length
    c = n >> (W-1);
    // add   q, q, t
    q = q + c;
    // add   q, q, 1             // for negative divisors (d < 0) and (n != 0)
    if (d < 0)
    {
      c = (n != 0);
      q = q + c;
    }
  }
  return (q);
}

/* print_usage:
 * Print usage instructions for the "kdiv" program.
 */
static void print_usage()
{
  printf("\n");
  printf("* Usage:\n");
  printf("* kdiv [options]\n");
  printf("* \n");
  printf("* Options:\n");
  printf("* \n");
  printf("*   -h:\n");
  printf("*         Print this help.\n");
  printf("*   -d:\n");
  printf("*         Enable debug/diagnostic output.\n");
  printf("*   -errors:\n");
  printf("*         Report only inconsistencies to the expected division results. Debug\n");
  printf("*         output (-d) must be enabled.\n");
  printf("*   -div <num>:\n");
  printf("*         Set the value of the divisor (an integer except zero). Default: 1.\n");
  printf("*   -width <num>:\n");
  printf("*         Set the bitwidth of all operands: dividend, divisor and\n");
  printf("*         quotient. Default: 32.\n");
  printf("*   -lo <num>:\n");
  printf("*         Set the lower integer bound for divident testing. Debug output (-d)\n");
  printf("*         must be enabled. Default: 0.\n");
  printf("*   -hi <num>:\n");
  printf("*         Set the higher integer bound for divident testing. Debug output (-d)\n");
  printf("*         must be enabled. Default: 65535.\n");
  printf("*   -signed:\n");
  printf("*         Construct optimized routine for signed division.\n");
  printf("*   -unsigned:\n");
  printf("*         Construct optimized routine for unsigned division (default).\n");
  printf("*   -nac:\n");
  printf("*         Emit software routine in the NAC general assembly language (default).\n");
  printf("*   -ansic:\n");
  printf("*         Emit software routine in ANSI C (only for width=32).\n");
  printf("* \n");
  printf("* For further information, please refer to the website:\n");
  printf("* http://www.nkavvadias.com\n");
}

/* main:
 * Program entry.
 */
int main(int argc, char *argv[]) 
{
   struct mu magu;
   struct ms mags;
   int i, j;
   unsigned int uquotapprox=0, uquotexact=0;   
   int squotapprox=0, squotexact=0;
   FILE *fout;
   char *fout_name, suffix[4], ch='X';
   
  // Read input arguments
  for (i=1; i < argc; i++)
  {
    if (strcmp("-h", argv[i]) == 0)
    {
      print_usage();
      exit(1);
    }
    else if (strcmp("-d", argv[i]) == 0)
    {
      enable_debug = 1;
    }
    else if (strcmp("-errors", argv[i]) == 0)
    {
      enable_errors = 1;
    }
    else if (strcmp("-unsigned", argv[i]) == 0)
    {
      enable_unsigned = 1;
      enable_signed   = 0;
    }
    else if (strcmp("-signed", argv[i]) == 0)
    {
      enable_unsigned = 0;
      enable_signed   = 1;
    }
    else if (strcmp("-nac", argv[i]) == 0)
    {
      enable_nac   = 1;
      enable_ansic = 0;
    }
    else if (strcmp("-ansic", argv[i]) == 0)
    {
      enable_nac   = 0;
      enable_ansic = 1;
    }
    else if (strcmp("-div",argv[i]) == 0)
    {
      if ((i+1) < argc)
      {
        i++;
        divisor_val = atoi(argv[i]);
      }
    }    
    else if (strcmp("-width",argv[i]) == 0)
    {
      if ((i+1) < argc)
      {
        i++;
        width_val = atoi(argv[i]);
      }
    }    
    else if (strcmp("-lo",argv[i]) == 0)
    {
      if ((i+1) < argc)
      {
        i++;
        if (argv[i][0] == '-')
        {
          lo_val = -atoi(argv[i]+1);
        }
        else
        {
          lo_val = atoi(argv[i]);
        }
      }
    }    
    else if (strcmp("-hi",argv[i]) == 0)
    {
      if ((i+1) < argc)
      {
        i++;
        if (argv[i][0] == '-')
        {
          hi_val = -atoi(argv[i]+1);
        }
        else
        {
          hi_val = atoi(argv[i]);
        }
      }
    }    
    else
    {
      if (argv[i][0] != '-')
      {
        print_usage();
        exit(1);
      }
    }
  }
  
  if (divisor_val == 0)
  {
    fprintf(stderr, "Error: Requested division by zero.\n");
    exit(1);
  }
  if ((enable_unsigned == 1) && (divisor_val < 0))
  {
    fprintf(stderr, "Error: Divisor must be positive for unsigned division.\n");
    exit(1);
  }

  fout_name = malloc(25 * sizeof(char));
  if (enable_nac == 1)
  {
    strcpy(suffix, "nac");
  }
  else if (enable_ansic == 1)
  {
    strcpy(suffix, "c");
  }
  if (enable_unsigned == 1)
  {
    ch = 'u';
  }
  else if (enable_signed == 1)
  {
    ch = 's';
  }
  if (divisor_val > 0)
  {
    sprintf(fout_name, "kdiv_%c%d_p_%d.%s", ch, width_val, divisor_val, suffix);
  }
  else
  {
    sprintf(fout_name, "kdiv_%c%d_m_%d.%s", ch, width_val, ABS(divisor_val), suffix);
  }
  fout = fopen(fout_name, "w");
  
  if (enable_unsigned == 1)
  {
    magu = magicu(divisor_val);
    if (enable_nac == 1)
    {
      emit_kdivu_nac(fout, magu.M, magu.a, magu.s, divisor_val, width_val);
    }
    else if (enable_ansic == 1)
    {
      emit_kdivu_ansic(fout, magu.M, magu.a, magu.s, divisor_val, width_val);
    }
  }
  else if (enable_signed == 1)
  {
    mags = magic(divisor_val);
    if (enable_nac == 1)
    {
      emit_kdivs_nac(fout, mags.M, mags.s, divisor_val, width_val);
    }
    else if (enable_ansic == 1)
    {
      emit_kdivs_ansic(fout, mags.M, mags.s, divisor_val, width_val);
    }
  }  
  if (enable_debug == 1)
  {
    for (j = lo_val; j <= hi_val; j++)
    {
      if (enable_unsigned == 1)
      {
        uquotapprox = calculate_kdivu(magu.M, magu.a, magu.s, j, divisor_val, width_val);
        uquotexact  = j/divisor_val;
      }
      else if (enable_signed == 1)
      {
        squotapprox = calculate_kdivs(mags.M, mags.s, j, divisor_val, width_val);
        squotexact  = j/divisor_val;
      }
      if ((enable_unsigned == 1) && (uquotapprox != uquotexact))
      {
        if (enable_errors == 1)
        {
          printf("Result NOT exact: %d/%d = %d (%d)\n", 
            j, divisor_val, uquotapprox, uquotexact);
        }
      }
      else if ((enable_unsigned == 1) && (uquotapprox == uquotexact))
      {
        if (enable_errors == 0)
        {
          printf("%d/%d = %d (%d)\n", j, divisor_val, uquotapprox, uquotexact);
        }
      }
      else if ((enable_signed == 1) && (squotapprox != squotexact))
      {
        if (enable_errors == 1)
        {
          printf("Result NOT exact: %d/%d = %d (%d)\n", 
            j, divisor_val, squotapprox, squotexact);
        }
      }
      else if ((enable_signed == 1) && (squotapprox == squotexact))
      {
        if (enable_errors == 0)
        {
          printf("%d/%d = %d (%d)\n", j, divisor_val, squotapprox, squotexact);
        }
      }
    }
  }   
#ifdef EMIT_TABLES   
  for (i = 1; i < 32; i++)
  {
    magu = magicu(i);
    printf("%03d: M = %08x a = %d s = %d\n", i, magu.M, magu.a, magu.s);
  }
  for (i = 1; i < 32; i++)
  {
    mags = magic(i);
    printf("%03d: M = %08x s = %d\n", i, mags.M, mags.s);
  }
#endif   
  free(fout_name);
  fclose(fout);
  return 0;
}
