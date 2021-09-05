// DEMO11_1.CPP - a linked list demo

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

typedef struct NODE_TYP
   {
   int id;         // id number of this object
   int age;        // age of person
   char name[32];  // name of person
   NODE_TYP *next; // this is the link to the next node
                   // more fields go here
   } NODE, *NODE_PTR;


// PROTOTYPES ////////////////////////////////////////////////////////////

void Traverse_List(NODE_PTR head);
NODE_PTR Insert_Node(int id, int age, char *name);
int Delete_Node(int id);

// GLOBALS //////////////////////////////////////////////////////////////

NODE_PTR head = NULL, // head pointer to list
         tail = NULL; // tail pointer to list

// FUNCTIONS ////////////////////////////////////////////////////////////

void Traverse_List(NODE_PTR head)
{
// this function traverses the linked list and prints out 
// each node

// test if head is null
if (head==NULL)
   {
   printf("\nLinked List is empty!\n");
   return;
   } // end if

// traverse while nodes 
while (head!=NULL)
      {
      // visit the node, print it out, or whatever...
      printf("\nNode Data: id=%d", head->id);
      printf("\nage=%d",head->age);
      printf("\nname=%s\n",head->name);
 
      // advance to next node (simple!)
      head = head->next;
      } // end while

printf("\n");

} // end Traverse_List

////////////////////////////////////////////////////////////////////////

NODE_PTR Insert_Node(int id, int age, char *name) 
{
// this function inserts a node at the end of the list
NODE_PTR new_node = NULL;

// step 1: create the new node
new_node = (NODE_PTR)malloc(sizeof(NODE)); // in C++ use new operator

// fill in fields
new_node->id  = id;
new_node->age = age;
strcpy(new_node->name,name); // memory must be copied!
new_node->next = NULL; // good practice

// step 2: what is the current state of the linked list?

if (head==NULL) // case 1
   {
   // empty list, simplest case
   head = tail = new_node;
 
   // return new node
   return(new_node);
   } // end if
else
if ((head != NULL) && (head==tail)) // case 2
   {
   // there is exactly one element, just a little 
   // finesse...
   head->next = new_node;
   tail = new_node;

   // return new node
   return(new_node);
   } // end if
else // case 3
   { 
   // there are 2 or more elements in list
   // simply move to end of the list and add
   // the new node
   tail->next = new_node;
   tail = new_node;

   // return the new node
   return(new_node);
   } // end else

} // end Insert_Node


////////////////////////////////////////////////////////////////////////

int Delete_Node(int id) // node to delete
{
// this function deletes a node from 
// the linked list given its id
NODE_PTR curr_ptr = head, // used to search the list
         prev_ptr = head; // previous record

// test if there is a linked list to delete from
if (!head) 
    return(-1); 

// traverse the list and find node to delete
while(curr_ptr && curr_ptr->id != id)
     {
     // save this position
     prev_ptr = curr_ptr;
     curr_ptr = curr_ptr->next;
     } // end while

// at this point we have found the node
// or the end of the list
if (curr_ptr == NULL)
    return(-1); // couldn't find record

// record was found, so delete it, but be careful, 
// need to test cases
// case 1: one element 
if (head==tail) 
   {
   // delete node
   free(head);
   
   // fix up pointers
   head=tail=NULL;

   // return id of deleted node   
   return(id);
   } // end if
else // case 2: front of list
if (curr_ptr == head)
   {
   // move head to next node
   head=head->next;

   // delete the node
   free(curr_ptr);

   // return id of deleted node
   return(id);

   } // end if
else // case 3: end of list
if (curr_ptr == tail)
   {
   // fix up previous pointer to point to null
   prev_ptr->next = NULL;
  
   // delete the last node
   free(curr_ptr);

   // point tail to previous node
   tail = prev_ptr; 

   // return id of deleted node
   return(id);

   } // end if
else  // case 4: node is in middle of list
   {     
   // connect the previous node to the next node
   prev_ptr->next = curr_ptr->next;

   // now delete the current node
   free(curr_ptr);

   // return id of deleted node
   return(id);

   } // end else

} // end Delete_Node


// MAIN /////////////////////////////////////////////////////////////////

void main(void)
{

int done     = 0, // exit flag
    node_num = 0, // node number, used for id
    sel      = 0; // used for input

// main event loop
while(!done)
     {
     // display menu
     printf("\nLinked List Manipulation Menu\n");
     printf("\n1 - Display linked list.");
     printf("\n2 - Insert new node.");
     printf("\n3 - Delete node.");
     printf("\n4 - Exit Program.");
     printf("\n\nSelect one please?");

     // get selection
     scanf("%d",&sel);

     // what to do
     switch(sel)
           {
           case 1:
                 {
                 // print out the list
                 Traverse_List(head);
                 }  break;
 
           case 2:
                 {
                 // locals to do the insertion
                 char name[32];
                 int age;
 
                 // get the info
                 printf("\nNew node entry form:\n");
                 printf("\nName?");
                 scanf("%s",name);
                 printf("\nAge?");
                 scanf("%d",&age);

                 // insert the node
                 Insert_Node(node_num++, age, name); 

                 } break;
 
           case 3:
                 {
                 int id; // id to delete
                 printf("\nEnter id number of node to delete?");
                 scanf("%d",&id);

                 // delete the node
                 Delete_Node(id);
 
                 } break;
 

           case 4:
                 {
                 done = 1;
                 } break;
 
           default: break;

           } // end switch

     } // end while

} // end main
