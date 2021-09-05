// DEMO11_2.CPP - an example of recursion via factorial

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

// DEFINES ///////////////////////////////////////////////////////////////

// TYPES //////////////////////////////////////////////////////////////////

// PROTOTYPES ////////////////////////////////////////////////////////////

int Factorial_Rec(int n);

// GLOBALS //////////////////////////////////////////////////////////////

// FUNCTIONS ////////////////////////////////////////////////////////////

int Factorial_Rec(int n)
{
// test for terminal cases
if (n==0 || n==1) 
   return(1);
else
   return(n*Factorial_Rec(n-1));

} // end Factorial_Rec

// MAIN /////////////////////////////////////////////////////////////////

void main(void)
{

// main event loop
while(1)
     {
     int n;
     printf("\nEnter number to compute factorial of?");
     scanf("%d",&n);

     printf("\n%d! = %d",n,Factorial_Rec(n));

     } // end while

} // end main
