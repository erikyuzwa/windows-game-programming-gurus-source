// DEMO12_9.CPP - Fuzzy Lingustic Variable manifold demo 
// note, I used a little C++ for fun, so you straight C guys pay
// attention

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
#include <iostream.h>

// DEFINES ///////////////////////////////////////////////////////////////


// TYPES //////////////////////////////////////////////////////////////////


// CLASSES ///////////////////////////////////////////////////////////////

// this holds a single fuzzy linguistic variable
class CFLV 
      {
      private:

      char name[80];         // name of FLV  
      float min_range;       // start value of triangle
      float max_range;       // end value of triangle

      public:
      
      // constructors
      CFLV(char *nameflv, float min, float max) { strcpy(name, nameflv); min_range = min; max_range = max; }
      CFLV()  {min_range = 0; max_range = 0; }

      // destructor
      ~CFLV() {}

      // method to set range
      void CSetRange(float min, float max) { min_range = min; max_range = max; }

      // data access member
      char *GetName() {return(name);}

      // method to compute degree of membershiop
      float DegreeOfMembership(float xi)
            {
            // this method computes the degree of membership for the give input
            // if any and returns the value
   
            float degree_of_membership = 0.0,
                  slope                = 0.0;

           // first test if the input is in range
           if (xi >= min_range && xi <= max_range)
              {
              // compute intersection with left edge or right
              // always assume height of triangle is 1.0
              float center_point = (max_range + min_range)/2;

             // compare xi to center
             if (xi <= center_point)
                {
                // compute intersection on left edge
                // dy/dx = 1.0/(center - left)
                slope = 1.0/(center_point - min_range);
                degree_of_membership = (xi - min_range) * slope;

                } // end if
             else
                {
                // compute intersection on right edge
                // dy/dx = 1.0/(center - right)
                slope = 1.0/(center_point - max_range);
                degree_of_membership = (xi - max_range) * slope;
                } // end else

             } // end if
          else // not in range
             degree_of_membership = 0.0;

            // return value
            return(degree_of_membership);

            } // end DegreeOfMembership
      }; // end class CFLV

// create a pointer type of class
typedef CFLV *CFLV_PTR;

// this holds the manifold
class CFuzzyManifold
      {
      private:
 
      float domain_min;    // the range of the domain
      float domain_max;

      int num_flvs;        // number of fuzzy linguistic variables

      CFLV_PTR flvs[16];   // pointer to array of flvs, hold up to 16

      public: 

      // constructors
      CFuzzyManifold() { domain_min = 0; domain_max = 0; num_flvs = 0; }
      
      CFuzzyManifold(float min, float max) { domain_min = min; domain_max = max; }

      // destructor
      ~CFuzzyManifold() { for (int index=0; index < num_flvs; free(flvs[index++])); }

      // data access members
      float GetMin() {return(domain_min);}
      float GetMax() {return(domain_max);}
      
      // used to set the domain
      void SetDomain(float min, float max) { domain_min = min; domain_max = max; }

      // this method is used to enter in an FLV
      void EnterFLV(char *name, float min, float max)
          {
          // create a new flv
          flvs[num_flvs++] = new CFLV(name, min, max);

          } // end EnterFLV

      // this tests the membership of the input against all flvs
      // and prints out the results
      void PrintMemberships(float xi)
           {
           cout << "\nMemberships are:";           

           float sum = 0; // used to normalize data
 
           // for each category, that is FLV in manifold test the degree of membership
           for (int index=0; index < num_flvs; index++)
               { 
               cout << "\nDegree of membership for '" <<  flvs[index]->GetName() << "' ";
               cout << "is " << flvs[index]->DegreeOfMembership(xi);
       
               // add dom to sum
               sum+=flvs[index]->DegreeOfMembership(xi);
               } // end for index  

           cout << "\nand the normalized memberships are:";
           for (index=0; index < num_flvs; index++)
               { 
               cout << "\nNormalized Degree of membership for '" <<  flvs[index]->GetName() << "' ";
               cout << "is " << flvs[index]->DegreeOfMembership(xi)/sum;
               } // end for index  

            cout << "\n";
           }  // end PrintMemberships
     
      }; // end class CFuzzyManifold

typedef CFuzzyManifold *CFuzzyManifold_PTR;

// PROTOTYPES ////////////////////////////////////////////////////////////



// GLOBALS //////////////////////////////////////////////////////////////



// FUNCTIONS ////////////////////////////////////////////////////////////



// MAIN /////////////////////////////////////////////////////////////////

void main()
{

float min,  // used to input ranges
      max,
      xi;   // used for inputs

char name[80];  // used for string names

int num_flvs = 0;  // track number of flvs

CFuzzyManifold manifold; // the manifold

// begin code

// show sequence/instructions
cout << "\nFUZZiCON - Fuzzy Manifold Construction Kit\n";
cout << "\nStep 1: Enter the domain range of input variable.";
cout << "\nStep 2: Enter in the number of FLVs.";
cout << "\nStep 3: Enter data for each of the FLVs.\n";

cout << "\nBegin Fuzzy Manifold Construction...\n";


// get domain range
cout << "\nStep 1: Domain Range input.";
cout << "\nEnter Domain Range Minimum of Fuzzy Input Variable?";
cin >> min;

cout << "Enter Domain Range Maximum of Fuzzy Input Variable?";
cin >> max;

// set the domain of the manifold
manifold.SetDomain(min,max);

// enter number of flvs
cout << "\nStep 2: Number of FLVs";
cout << "\nEnter the number of Fuzzy Linguistic Variables in your fuzzy manifold?";
cin >> num_flvs;

cout << "\nStep 3: FLV data entry loop";

// get each of the flvs
for (int index=0; index < num_flvs; index++)
    {
    cout << "\nEnter Name of Fuzzy Linguistic Variable #" << index+1 << "?";
    cin >> name;

    cout << "\nEnter Minimum of Fuzzy Linguistic Variable #" << index+1 << "?";
    cin >> min;

    cout << "Enter Maximum of Fuzzy Linguistic Variable #" << index+1 << "?";    
    cin >> max;

    // create the flv in the manifold
    manifold.EnterFLV(name, min, max);

    } // end for index

cout << "\nFuzzy Manifold is ready to take input values \nand compute the degree of membership(s)!\n";

// input membership loop
while(1)
     {
     cout << "\nEnter input value to test in range " << manifold.GetMin() << " to " << manifold.GetMax() << "?";
     cin >> xi;

     // test membership in each flv
     manifold.PrintMemberships(xi);

     } // end while

} // end main