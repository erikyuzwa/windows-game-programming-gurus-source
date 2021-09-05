// DEMO11_4.CPP - a fixed point demo

// INCLUDES ///////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN  // make sure certain headers are included correctly

#include <windows.h>         // include the standard windows stuff
#include <windowsx.h>        // include the 32 bit stuff
#include <conio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <io.h>
#include <fcntl.h>

// DEFINES /////////////////////////////////////////////////////////////

// comment the one out you dont want to use
#define FIXPOINT16_16
//#define FIXPOINT24_8


#ifdef FIXPOINT16_16
// for 16.16 represenation
#define FP_SHIFT        16    // number of binary decimal digits
#define FP_SCALE        65536 // 2^(32-FP_SHIFT), used during conversion of floats
#endif

#ifdef FIXPOINT24_8
// if you want to change the representation to 24.8 then use these
#define FP_SHIFT        8   // number of binary decimal digits
#define FP_SCALE        256 // (32-2^FP_SHIFT), used during conversion of floats
#endif

// TYPES ///////////////////////////////////////////////////////

// define our new magical FIXPOINT point data type

typedef int FIXPOINT;

// MACROS ////////////////////////////////////////////////////////////

#define INT_TO_FIXP(n) (FIXPOINT((n << FP_SHIFT)))
#define FLOAT_TO_FIXP(n) (FIXPOINT((float)n * FP_SCALE))

// FUNCTIONS //////////////////////////////////////////////////////////

inline FIXPOINT Assign_Integer(int integer)
{
// this function assigns a integer to a fixed point type by shifting
// functional version of macro

return((FIXPOINT)integer << FP_SHIFT);

} // end Assign_Integer

///////////////////////////////////////////////////////////////////////////////

inline FIXPOINT Assign_Float(float number)
{
// this function assigns a floating point number to a fixed point type
// by multiplication since it makes no sense to shift a floating point data type
// functional version of macro

return((FIXPOINT)(number * FP_SCALE));

} // end Assign_Float

///////////////////////////////////////////////////////////////////////////////

inline FIXPOINT Mul_Fixed(FIXPOINT f1,FIXPOINT f2)
{
// this function mulitplies two fixed point numbers and returns the result
// notice how the final result is shifted back

return((f1*f2) >> FP_SHIFT);

} // end Mul_Fixed

///////////////////////////////////////////////////////////////////////////////

inline FIXPOINT Div_Fixed(FIXPOINT f1,FIXPOINT f2)
{
// this function divvides two fixed point numbers and returns the result
// notice how the divedend is pre-shifted before the division

return((f1<<FP_SHIFT)/f2);

} // end Div_Fixed

///////////////////////////////////////////////////////////////////////////////

inline FIXPOINT Add_Fixed(FIXPOINT f1,FIXPOINT f2)
{

// this function adds two fixed point numbers and returns the result
// notice how no shifting is necessary

return(f1+f2);

} // end Add_Fixed

///////////////////////////////////////////////////////////////////////////////

inline FIXPOINT Sub_Fixed(FIXPOINT f1,FIXPOINT f2)
{

// this function subtracts two fixed point numbers and returns the result
// notice how no shifting is necessary

return(f1-f2);

} // end Sub_Fixed

///////////////////////////////////////////////////////////////////////////////

inline void Print_Fixed(FIXPOINT f1)
{
// this function prints out a fixed point number, it does this by
// extracting the portion to the left of the imaginary decimal and
// extracting the portion to the right of the imaginary decimal point

#ifdef FIXPOINT16_16
printf("%d.%d",f1 >> FP_SHIFT, 100*(unsigned int)(f1 & 0x0000ffff)/FP_SCALE);
#endif

#ifdef FIXPOINT24_8
// use this line if you change the representation to 24.8
printf("%d.%d",f1 >> FP_SHIFT, 100*(unsigned int)(f1 & 0x000000ff)/FP_SCALE);
#endif

} // end Print_Fixed


//M A I N //////////////////////////////////////////////////////////////////////

void main(void)
{

FIXPOINT fp1,fp2,fp3; // defines some fixed point numbers

float fl1,fl2,fl3;    // used to mimic the fixed point math with real float
                   // so you can see the discrepancies


while(1)
{
printf("\nEnter first floating value fl1 (note used as denominator in division)?");
scanf("%f",&fl1);

printf("\nEnter second floating value fl2 (note used as numerator in division)?");
scanf("%f",&fl2);

// convert to fixed point
fp1 = FLOAT_TO_FIXP(fl1);
fp2 = FLOAT_TO_FIXP(fl2);

printf("\n\nFixed Point Computations:");
printf("\nfp1:=");
Print_Fixed(fp1);

printf("\nfp2:=");
Print_Fixed(fp2);

printf("\nfp1+fp2:=");
fp3 = Add_Fixed(fp1,fp2);
Print_Fixed(fp3);

printf("\nfp1-fp2:=");
fp3 = Sub_Fixed(fp1,fp2);
Print_Fixed(fp3);

printf("\nfp1*fp2:=");
fp3 = Mul_Fixed(fp1,fp2);
Print_Fixed(fp3);

printf("\nfp2/fp1:=");
fp3 = Div_Fixed(fp2,fp1);
Print_Fixed(fp3);

printf("\n\nFloating Point Computations with same numbers:");
printf("\nfl1+fl2:=%f", fl1+fl2);
printf("\nf1-f2:=%f"  , fl1-fl2);
printf("\nf1*f2:=%f"  , fl1*fl2);
printf("\nf2/f1:=%f"  , fl2/fl1);
} // end while


} // end main

