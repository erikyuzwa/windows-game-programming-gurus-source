README.TXT for SOURCE files

All of the source code for this book has been written and tested under
Microsoft VC++ 5.0, 6.0 along with DirectX 8.0+. Some basic things to
remember when compiling are to set the search paths to the DirectX SDK
files AND additionally, include the DirectX .LIB files in your project
itself along with the .CPP files.

I. INSTALLING THE SOURCE

To install the source, simply drag the entire directory into the 
desired place on your hard drive and make a copy.

II. BORLAND/WATCOM Users

Although, I didn't test the code on these compilers, there is nothing
VC++ specific about the code. In fact, any Win32 compliant compiler must 
work. However, there are little things that you must take into consideration:

1. If you get linker errors when compiling then it's probably a simple syntax 
   error. For example, many functions in VC++ have a leading underscore, but 
   the BORLAND versions do not -- hence, remove the underscore on the function
   that is causing the problem. In addition, look at your compiliers help for 
   the function itself (try wildcard spellings) and make sure that you have 
   included the proper header file. For example, malloc.h has most of the
   memory management functions for VC++, but memory.h is needed additionally for
   BORLAND in some cases. 

2. DirectX for other compilers may not link correctly since the OBJECT formats may
   be incompatible with the .LIB files, hence, most compilers come with convertors to
   convert MS VC++ .LIB files, OR the DirectX SDK itself has additional object versions
   of the .LIB files usually located in a directory named after the compiler. Hence, for
   BORLAND for example, there is a directory called BORLAND that has the .LIB import 
   files that you must use rather than the standard ones in the DirectX LIB directory.

3. Finally, go to the website of the compiler manufacturer and read the Q&A about using
   and compiling DirectX programs for your particular compiler.

