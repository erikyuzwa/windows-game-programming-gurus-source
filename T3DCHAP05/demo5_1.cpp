// DEMO5_1.CPP - A ultra minimal working COM example
// NOTE: not fully COM compliant

// INCLUDES //////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <malloc.h>
#include <iostream.h>
#include <objbase.h> // note: you must include this header it contains important constants
                     // you must use in COM programs

// GUIDS /////////////////////////////////////////////////////////////////////////////////////

// these were all generated with GUIDGEN.EXE

// {B9B8ACE1-CE14-11d0-AE58-444553540000}
const IID IID_IX = 
{ 0xb9b8ace1, 0xce14, 0x11d0, { 0xae, 0x58, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0 } };


// {B9B8ACE2-CE14-11d0-AE58-444553540000}
const IID IID_IY = 
{ 0xb9b8ace2, 0xce14, 0x11d0, { 0xae, 0x58, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0 } };

// {B9B8ACE3-CE14-11d0-AE58-444553540000}
const IID IID_IZ = 
{ 0xb9b8ace3, 0xce14, 0x11d0, { 0xae, 0x58, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0 } };


// INTERFACES ////////////////////////////////////////////////////////////////////////////////

// define the IX interface
interface IX: IUnknown
{

virtual void __stdcall fx(void)=0;

}; 

// define the IY interface
interface IY: IUnknown
{

virtual void __stdcall fy(void)=0;

}; 


// CLASSES AND COMPONENTS ///////////////////////////////////////////////////////////////////

// define the COM object
class CCOM_OBJECT :	public IX,
                    public IY
{
public:

	CCOM_OBJECT() : ref_count(0) {}
	~CCOM_OBJECT() {}

private:

virtual HRESULT __stdcall QueryInterface(const IID &iid, void **iface);
virtual ULONG __stdcall AddRef();
virtual ULONG __stdcall Release();

virtual	void __stdcall fx(void) {cout << "Function fx has been called." << endl; }
virtual void __stdcall fy(void) {cout << "Function fy has been called." << endl; }

int ref_count;

};

// CLASS METHODS ////////////////////////////////////////////////////////////////////////////

HRESULT __stdcall CCOM_OBJECT::QueryInterface(const IID &iid, void **iface)
{
// this function basically casts the this pointer or the Iunknown
// pointer into the interface requested, notice the comparison with
// the GUIDs generated and defined in the begining of the program

// requesting the IUnknown base interface
if (iid==IID_IUnknown)
	{
	cout << "Requesting IUnknown interface" << endl;
	*iface = (IX*)this;
	
	} // end if

// maybe IX?
if (iid==IID_IX)
	{
	cout << "Requesting IX interface" << endl;
	*iface = (IX*)this;

	} // end if
else  // maybe IY
if (iid==IID_IY)
	{
	cout << "Requesting IY interface" << endl;
	*iface = (IY*)this;

	} // end if
else
	{ // cant find it!
	cout << "Requesting unknown interaface!" << endl;
	*iface = NULL;
	return(E_NOINTERFACE);
	} // end else

// if everything went well cast pointer to IUnknown and call addref()
((IUnknown *)(*iface))->AddRef();

return(S_OK);

} // end QueryInterface

////////////////////////////////////////////////////////////////////////////////////////////////

ULONG __stdcall CCOM_OBJECT::AddRef()
{
// increments reference count
cout << "Adding a reference" << endl;
return(++ref_count);

} // end AddRef

///////////////////////////////////////////////////////////////////////////////////////////////

ULONG __stdcall CCOM_OBJECT::Release()
{
// decrements reference count
cout << "Deleting a reference" << endl;
if (--ref_count==0)
	{
	delete this;
	return(0);
	} // end if
else
	return(ref_count);

} // end Release

///////////////////////////////////////////////////////////////////////////////////////////////

IUnknown *CoCreateInstance(void)
{
// this is a very basic implementation of CoCreateInstance()
// it creates an instance of the COM object, in this case
// I decided to start with a pointer to IX -- IY would have
// done just as well

IUnknown *comm_obj = (IX *)new(CCOM_OBJECT);

cout << "Creating Comm object" << endl;

// update reference count
comm_obj->AddRef();

return(comm_obj);

} // end CoCreateInstance

///////////////////////////////////////////////////////////////////////////////////////////////

void main(void)
{

// create the main COM object
IUnknown *punknown = CoCreateInstance();

// create two NULL pointers the the IX and IY interfaces
IX *pix=NULL;
IY *piy=NULL;

// from the original COM object query for interface IX
punknown->QueryInterface(IID_IX, (void **)&pix);

// try some of the methods of IX
pix->fx();

// release the interface
pix->Release();


// now query for the IY interface
punknown->QueryInterface(IID_IY, (void **)&piy);

// try some of the methods
piy->fy();

// release the interface
piy->Release();

// release the COM object itself
punknown->Release();

} // end main




