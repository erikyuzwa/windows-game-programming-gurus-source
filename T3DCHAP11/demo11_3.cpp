// DEMO11_3.CPP - a binary search tree demo

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

typedef struct TNODE_TYP
   {
   int id;         // id number of this object
   int age;        // age of person
   char name[32];  // name of person
   TNODE_TYP *right; // this is the link to the right node
   TNODE_TYP *left;  // this is the link to the left node               

   // more fields go here
   } TNODE, *TNODE_PTR;


// PROTOTYPES ////////////////////////////////////////////////////////////

void BST_Inorder_Search(TNODE_PTR root);
void BST_Preorder_Search(TNODE_PTR root);
void BST_Postorder_Search(TNODE_PTR root);
TNODE_PTR BST_Insert_Node(TNODE_PTR root, int id, int age, char *name); 

// GLOBALS //////////////////////////////////////////////////////////////

TNODE_PTR root = NULL; // root of tree

// FUNCTIONS ////////////////////////////////////////////////////////////

TNODE_PTR BST_Insert_Node(TNODE_PTR root, int id, int age, char *name)
{
// test for empty tree
if (root==NULL)
   {
   // insert node at root
   root         = new(TNODE);
   root->id     = id;
   root->age    = age;
   strcpy(root->name,name);

   // set links to null
   root->right  = NULL;
   root->left   = NULL;

   printf("\nCreating tree");

   } // end if

// else there is a node here, lets go left or right
else
if (age >= root->age)
   {
   printf("\nTraversing right...");
   // insert on right branch

   // test if branch leads to another sub-tree or is terminal
   // if leads to another subtree then try to insert there, else
   // create a node and link
   if (root->right)
      BST_Insert_Node(root->right, id, age, name);
   else
      {
      // insert node on right link
      TNODE_PTR node   = new(TNODE);
      node->id     = id;
      node->age    = age;
      strcpy(node->name,name);

      // set links to null
      node->left   = NULL;
      node->right  = NULL;

      // now set right link of current "root" to this new node
      root->right = node;

      printf("\nInserting right.");

      } // end else

   } // end if
else // age < root->age
   {
   printf("\nTraversing left...");
   // must insert on left branch

   // test if branch leads to another sub-tree or is terminal
   // if leads to another subtree then try to insert there, else
   // create a node and link
   if (root->left)
      BST_Insert_Node(root->left, id, age, name);
   else
      {
      // insert node on left link
      TNODE_PTR node   = new(TNODE);
      node->id     = id;
      node->age    = age;
      strcpy(node->name,name);

      // set links to null
      node->left   = NULL;
      node->right  = NULL;

      // now set right link of current "root" to this new node
      root->left = node;

      printf("\nInserting left.");
      } // end else

} // end else

// return the root 
return(root);

} // end BST_Insert_Node


//////////////////////////////////////////////////////////////////////////

void BST_Inorder_Search(TNODE_PTR root)
{
// this searches a BST using the inorder search

// test for NULL
if (!root)
   return;

// traverse left tree
BST_Inorder_Search(root->left);

// visit the node
printf("\nname: %s, age: %d", root->name, root->age);

// traverse the right tree
BST_Inorder_Search(root->right);

} // end BST_Inorder_Search

///////////////////////////////////////////////////////////////////

void BST_Preorder_Search(TNODE_PTR root)
{
// this searches a BST using the preorder search

// test for NULL
if (!root)
   return;

// visit the node
printf("\nname: %s, age: %d", root->name, root->age);

// traverse left tree
BST_Inorder_Search(root->left);

// traverse the right tree
BST_Inorder_Search(root->right);

} // end BST_Preorder_Search

////////////////////////////////////////////////////////////////

void BST_Postorder_Search(TNODE_PTR root)
{
// this searches a BST using the postorder search

// test for NULL
if (!root)
   return;

// traverse left tree
BST_Inorder_Search(root->left);

// traverse the right tree
BST_Inorder_Search(root->right);

// visit the node
printf("\nname: %s, age: %d", root->name, root->age);

} // end BST_Postorder_Search

// MAIN /////////////////////////////////////////////////////////////////

void main(void)
{

int done     = 0, // exit flag
    node_num = 0, // used as node id
    sel      = 0; // used for input

// main event loop
while(!done)
     {
     // display menu
     printf("\n\nBinary Search Tree Demo\n");
     printf("\n1 - Traverse Tree Pre-order.");
     printf("\n2 - Traverse Tree In-order.");
     printf("\n3 - Traverse Tree Post-order.");
     
     printf("\n4 - Insert new node into tree.");
     printf("\n5 - Exit Program.");
     printf("\n\nSelect one please?");

     // get selection
     scanf("%d",&sel);

     // what to do
     switch(sel)
           {
           case 1:
                 {
                 // traverse preorder
                 if (root)
                    BST_Preorder_Search(root);
                 else
                     printf("\nTree empty!\n");
                 }  break;
 
           case 2:
                 {
                 // traverse inorder
                 if (root)
                    BST_Inorder_Search(root);
                 else
                     printf("\nTree empty!\n");
                 }  break;


           case 3:
                 {
                 // traverse posteorder
                 if (root)
                    BST_Postorder_Search(root);
                 else
                     printf("\nTree empty!\n");
                 }  break;


           case 4:
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
                 root = BST_Insert_Node(root, node_num++, age, name); 

                 } break;

           case 5:
                 {
                 done = 1;
                 } break;
 
           default: break;

           } // end switch

     } // end while

} // end main
