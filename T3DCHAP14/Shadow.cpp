// SHADOW.CPP
// Compile as a CONSOLE application or straight DOS16/32 application
// make sure to enable the ANSI.SYS color driver by adding the line:
//  DEVICE=C:\WINDOWS\COMMAND\ANSI.SYS
// to your config.sys file

// I N C L U D E S ///////////////////////////////////////////////////////////

#include <io.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <fcntl.h>
#include <memory.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

// D E F I N E S /////////////////////////////////////////////////////////////

// size of universe in blocks

#define NUM_ROWS               32
#define NUM_COLUMNS            32

// id's for objects and geometry in universe

#define EMPTY_ID               ' '
#define WALL_ID                '*'

#define LAMP_ID                'l'
#define SANDWICH_ID            's'
#define KEYS_ID                'k'

// general directions

#define EAST                   0
#define WEST                   1
#define NORTH                  2
#define SOUTH                  3

// language details

#define MAX_TOKENS             64
#define NUM_TOKENS             28

#define FIRST_WORD             0
#define SECOND_WORD            1
#define THIRD_WORD             2
#define FOURTH_WORD            3
#define FIFTH_WORD             4

#define OBJECT_START           50

#define OBJECT_LAMP            50
#define OBJECT_SANDWICH        51
#define OBJECT_KEYS            52

#define OBJECT_END             52

// directions (adjectives)

#define DIR_1_START            100

#define DIR_1_EAST             100
#define DIR_1_WEST             101
#define DIR_1_NORTH            102
#define DIR_1_SOUTH            103

#define DIR_1_END              103



#define DIR_2_START            150

#define DIR_2_FORWARD          150
#define DIR_2_BACKWARD         151
#define DIR_2_RIGHT            152
#define DIR_2_LEFT             153

#define DIR_2_END              153


// define actions (verbs)

#define ACTION_START           200

#define ACTION_MOVE            200
#define ACTION_TURN            201
#define ACTION_SMELL           202
#define ACTION_LOOK            203
#define ACTION_LISTEN          204
#define ACTION_PUT             205
#define ACTION_GET             206
#define ACTION_EAT             207
#define ACTION_INVENTORY       208
#define ACTION_WHERE           209
#define ACTION_EXIT            210

#define ACTION_END             210

// articles

#define ART_START              300

#define ART_THE                300

#define ART_END                300

// prepositions

#define PREP_START             325

#define PREP_IN                325
#define PREP_ON                326
#define PREP_TO                327
#define PREP_DOWN              328

#define PREP_END               328

// the phrases that add meaning to us, but not to the computer

#define PHRASE_TO              0
#define PHRASE_THE             1
#define PHRASE_TO_THE          2
#define PHRASE_DOWN            3
#define PHRASE_DOWN_THE        4

// S T R U C T U R E S ///////////////////////////////////////////////////////

// this is the structure for a single token

typedef struct token_typ
        {

        char symbol[16];   // the string that represents the token
        int value;         // the integer value of the token

        } token, *token_ptr;

// this is the structure used to hold a single string that is used to
// describe something in the game like a smell, sight, sound...

typedef struct info_string_typ
        {
        char type;        // the type of info string i.e. what does it describe
        char string[100]; // the actual description string

        } info_string, *info_string_ptr;

// this structure holds everything pertaining to the player

typedef struct player_typ
        {
        char name[16];   // name of player
        int x,y;         // postion of player
        int direction;   // direction of player, east,west north,south

        char inventory[8]; // objects player is holding (like pockets)
        int num_objects;   // number of objects player is holding

        } player, *player_ptr;

// this structure holds an object

typedef struct object_typ
        {
        char thing;     // the actual object
        int x,y;        // position of object in universe

        } object, *object_ptr;

// P R O T O T Y P E S //////////////////////////////////////////////////////

void Introduction();

int Vision_System(int depth,
                  int direction,
                  object *stuff,
                  int *num_objects);

int Check_For_Phrase(int phrase,int index);

void Print_Info_Strings(info_string strings[],char where);

char *Get_Line(char *buffer);

int Get_Token(char *input,char *output,int *current_pos);

int Extract_Tokens(char *string);

void Verb_Parser(void);

int Verb_MOVE(void);

int Verb_TURN(void);

int Verb_SMELL(void);

int Verb_LOOK(void);

int Verb_LISTEN(void);

int Verb_PUT(void);

int Verb_GET(void);

int Verb_EAT(void);

int Verb_INVENTORY(void);

int Verb_EXIT(void);

int Verb_WHERE(void);

// G L O B A L S //////////////////////////////////////////////////////////////

// this is the entire "language" of the language.

token language[MAX_TOKENS] = {

 {"LAMP",      OBJECT_LAMP     },
 {"SANDWICH",  OBJECT_SANDWICH },
 {"KEYS",      OBJECT_KEYS     },
 {"EAST",      DIR_1_EAST      },
 {"WEST",      DIR_1_WEST      },
 {"NORTH",     DIR_1_NORTH     },
 {"SOUTH",     DIR_1_SOUTH     },
 {"FORWARD",   DIR_2_FORWARD   },
 {"BACKWARD",  DIR_2_BACKWARD  },
 {"RIGHT",     DIR_2_RIGHT     },
 {"LEFT",      DIR_2_LEFT      },
 {"MOVE",      ACTION_MOVE     },
 {"TURN",      ACTION_TURN     },
 {"SMELL",     ACTION_SMELL    },
 {"LOOK",      ACTION_LOOK     },
 {"LISTEN",    ACTION_LISTEN   },
 {"PUT",       ACTION_PUT      },
 {"GET",       ACTION_GET      },
 {"EAT",       ACTION_EAT      },
 {"INVENTORY", ACTION_INVENTORY},
 {"WHERE",     ACTION_WHERE    },
 {"EXIT",      ACTION_EXIT     },
 {"THE",       ART_THE         },
 {"IN",        PREP_IN         },
 {"ON",        PREP_ON         },
 {"TO",        PREP_TO         },
 {"DOWN",      PREP_DOWN       },

};

// now for the definition of the universe and the objects within it

// this array holds the geometry of the universe

// l - living room
// b - bedroom
// k - kitchen
// w - washroom
// h - hall way
// r - restroom
// e - entry way
// o - office

//           ^
//         NORTH
//
// < WEST           EAST >
//
//         SOUTH
//           v

char *universe_geometry[NUM_ROWS]={"********************************",
                                   "*lllllllll*bbbbbbbbbbbbbbbbbbbb*",
                                   "*llllllllll*bbbbbbbbbbbbbbbbbbb*",
                                   "*lllllllllll*bbbbbbbbbbbbbbbbbb*",
                                   "*llllllllllll*bbbbbbbbbbbbbbbbb*",
                                   "*llllllllllll*bbbbbbbbbbbbbbbbb*",
                                   "*llllllllllll*bbbbbbbbbbbbbbbbb*",
                                   "*llllllllllll*bbbbbbbbbbbbbbbbb*",
                                   "*llllllllllll*bbbbbbbbbbbbbbbbb*",
                                   "*llllllllllll*bbbbbbbbbbbbbbbbb*",
                                   "*llllllllllll*bbbbbbbbbbbbbbbbb*",
                                   "*llllllllllll*bbbb*rrr**********",
                                   "*lllllllllllhhhhhh*rrrrrrrrrrrr*",
                                   "*lllllllllllhhhhhh*rrrrrrrrrrrr*",
                                   "*lllllllllhhh******rrrrrrrrrrrr*",
                                   "*********hhhh*rrrrrrrrrrrrrrrrr*",
                                   "*kkkkkkk*hhhh*rrrrrrrrrrrrrrrrr*",
                                   "*kkkkkkk*hhhh*rrrrrrrrrrrrrrrrr*",
                                   "*kkkkkkk*hhhh*rrrrrrrrrrrrrrrrr*",
                                   "*kkkkkkkhhhhh*******************",
                                   "*kkkkkkkhhhhhhhhhhhwwwwwwwwwwww*",
                                   "*kkkkkkkhhhhhhhhhhhwwwwwwwwwwww*",
                                   "*kkkkkkk*hhhhhhhhhhwwwwwwwwwwww*",
                                   "*kkkkkkk*hhhh*ooooo*************",
                                   "*kkkkkkk*hhhh*ooooooooooooooooo*",
                                   "*kkkkkkk*hhhh*ooooooooooooooooo*",
                                   "*kkkkkk*hhhhh*ooooooooooooooooo*",
                                   "*******hhhhhh*ooooooooooooooooo*",
                                   "*eeeeeeeeeeee*ooooooooooooooooo*",
                                   "*eeeeeeeeeeee*ooooooooooooooooo*",
                                   "*eeeeeeeeeeee*ooooooooooooooooo*",
                                   "********************************",};

// this array holds the objects within the universe

// l - lamp
// s - sandwich
// k - keys

//           ^
//         NORTH
//
// < WEST           EAST >
//
//         SOUTH
//           v

char *universe_objects[NUM_ROWS]={"                                ",
                                  " l                            k ",
                                  "                                ",
                                  "                                ",
                                  "                                ",
                                  "                                ",
                                  "                                ",
                                  "                                ",
                                  "                                ",
                                  " l                              ",
                                  "                                ",
                                  "                                ",
                                  "                                ",
                                  "                                ",
                                  "                                ",
                                  "                                ",
                                  "                                ",
                                  "                                ",
                                  "                                ",
                                  "                                ",
                                  "                                ",
                                  "                                ",
                                  "                                ",
                                  "  s                             ",
                                  "                                ",
                                  "                                ",
                                  "                                ",
                                  "                                ",
                                  "          s                     ",
                                  "                             l  ",
                                  "                                ",
                                  "                                ",};

// these info strings hold the views in each room

info_string views[]={

{'l',"You see an expansive white room with a vaulted ceiling. The walls are adorned "},
{'l',"with medieval art.  To the North, is a plate glass window through which colored"},
{'l',"strands of light pierce. They reflect off the carpeted floor and create a     "},
{'l',"silhouette of the towering pines just outdside. In the Northeast corner of    "},
{'l',"the room, you see a fire place with a half burnt log in it. Finally, against  "},
{'l',"the West wall there's a white leather couch and in each corner of the room    "},
{'l',"are large green palms potted in hexogonal black pots.                         "},

{'b',"You see a black lacquer bedroom set surrounding a king size platform bed."},
{'b',"On the walls, you see pictures of mystical landscapes and underwater cities.   "},
{'b',"To the North there is a window thru which you see a group of large trees just  "},
{'b',"beyond a small pond. On the floor of the room you see black silk stockings     "},
{'b',"and lingerie thrown with abandon about the area.                               "},

{'k',"You are surrounded by stone washed granite counters. On the counters are the"},
{'k',"normal appliances found in a kitchen. To the West there is a large glass    "},
{'k',"door refrigerator with a varitable plethora of food. Against the South wall"},
{'k',"there is a small nook with a white refuge container. Above your head dangle"},
{'k',"down many cooking utensils and exotic pans suspended from a anodized        "},
{'k',"aluminum structure.                                                         "},

{'w',"There is a large vanity mirror and a black porcellin wash basin. To the East"},
{'w',"is a rack of black and white towels hung on brass rails. On the counter     "},
{'w',"surrounding the walls there are small spherically shaped soap balls in a    "},
{'w',"myriad of colors set in cyrstal dishes. The floor is made of black and white"},
{'w',"marble with a hint of grey running thru it.                                 "},

{'h',"You see an ordinary hallway with track lighting above head. "},

{'r',"You see a large wash area with two basins. To the West through a glass  "},
{'r',"enclosure you see the outline of a young woman apperantly bathing... To "},
{'r',"the East you see a second smaller room with many plants hanging from the"},
{'r',"celing along with the shadow of a dark,muscular man moving around. On   "},
{'r',"the floor below you are plush tapestries of Egyption origin.            "},

{'e',"To the West you see a small opening into what appears to be a kitchen.        "},
{'e',"The lights are low and you can't make out much more in this direction.        "},
{'e',"Under your feet is a large black rectangular cut of carpet. Lastly, leading to"},
{'e',"the East and North are hallways to the remainder of the house.                "},

{'o',"You are astounded by the amount of computer equipment in every corner of the "},
{'o',"room. To the South is a Silicon Graphics OYNX Super Computer, the screen of  "},
{'o',"which seems to displaying a mesmorizing array of mathematical equations. To  "},
{'o',"the North you are faced with literally hundreds of books on every topic from "},
{'o',"quantum mechanics to molecular bio-chemistry. Against the East wall there    "},
{'o',"is a full collection of electrical engineering equipment with a oscilloscope "},
{'o',"displaying a curious waveform apparently an input from a black box resting on"},
{'o',"the floor. As you look around the walls of the room, you see schematics of   "},
{'o',"digital hardware and paintings of great scientific pioneers such as Einstein,"},
{'o',"Newton and Maxwell. Strewn about the floor are small electronic              "},
{'o',"components and pieces of paper with scribbles upon them.                     "},
{'X',""},  // terminate

};

// these info strings hold the smells in each room

info_string smells[]={

{'l',"You smell the sweet odor of Jasmine with an undertone of potpourri. "},

{'b',"The sweet smell of perfume dances within your nostrils...Realities possibly. "},

{'k',"You take a deep breath and your senses are tantallized with the smell of"},
{'k',"tender breasts of chicken marinating in a garlic sauce. Also, there is "},
{'k',"a sweet berry smell emanating from the oven.                          "},

{'w',"You are almost overwhelmed by the smell of bathing fragrance as you"},
{'w',"inhale.                                                              "},

{'h',"You smell nothing to make note of. "},

{'r',"Your nose is filled with steam and the smell of baby oil... "},

{'e',"You smell pine possibly from the air coming thru a small orifice near"},
{'e',"the front door.                                                            "},

{'o',"You are greeted with the familiar odor of burning electronics. As you inhale"},
{'o',"a second time, you can almost taste the rustic smell of aging books.        "},
{'X',""}, // terminate

};

// these info strings hold the sounds in each room

info_string sounds[]={

{'l',"You hear the faint sounds of Enigma playing in the background along with"},
{'l',"the wind howling against the exterior of the room.                      "},

{'b',"You hear the wind rubbing against the window making a sound similar to"},
{'b',"sheets being pulled off a bed.                                        "},

{'k',"You hear expansion of the hot ovens along with the relaxing sounds produced"},
{'k',"by the cooling fans.                                                       "},

{'w',"You hear nothing but a slight echo in the heating ducts."},

{'h',"You hear the sounds of music, but you can't make out any of the words or"},
{'h',"melodies.                                                               "},

{'r',"You hear the sound of driping water and the whispers of a femal voice"},
{'r',"ever so faintly in the background.                                   "},

{'e',"You hear the noises of the outside world muffled by the closed door to the"},
{'e',"South.                                                                    "},

{'o',"You hear nothing but the perpetual hum of all the cooling fans within the"},
{'o',"electronic equipment.                                                    "},
{'X',""},

};

int sentence[8];               // this array holds the current sentence
int num_tokens;                // number of words in current sentecne

// this is the player

player you={"Andre'",10,29,NORTH,{' ',' ',' ',' ',' ',' ',' ',' '},0};

int global_exit=0;             // global exit flag

char global_input[128],        // input string
     global_output[128];       // output string

// F U N C T I O N S //////////////////////////////////////////////////////////

char *Get_Line(char *buffer)
{
// this function gets a single line of input and tolerates white space

int c,index=0;

// loop while user hasn't hit return

while((c=getch())!=13)
     {

// implement backspace

     if (c==8 && index>0)
        {

        buffer[--index] = ' ';
        printf("%c %c",8,8);

        } // end if backspace
     else
     if (c>=32 && c<=122)
        {
        buffer[index++] = c;
        printf("%c",c);

        } // end if in printable range

     } // end while

// terminate string

buffer[index] = 0;

// return pointer to buffer or NULL

if (strlen(buffer)==0)
   return(NULL);
else
return(buffer);

} // end Get_Line

////////////////////////////////////////////////////////////////////////////////

int Get_Token(char *input,char *output,int *current_pos)
{

int index,   // loop index and working index
    start,   // points to start of token
    end;     // points to end of token

// set current positions

index=start=end=*current_pos;

// eat white space

while(isspace(input[index]) || ispunct(input[index]))
     {

     index++;

     } // end while

// test if end of string found

if (input[index]==NULL)
   {
   // emit nothing

   strcpy(output,"");
   return(0);

   } // end if no more tokens

// at this point, we must have a token of some kind, so find the end of it

start = index; // mark front of it
end   = index;

// find end of Token

while(!isspace(input[end]) && !ispunct(input[end]) && input[end]!=NULL)
     {

     end++;

     } // end while

// build up output string

for (index=start; index<end; index++)
    {

    output[index-start] = toupper(input[index]);

    } // end copy string

// place terminator

output[index-start] = 0;

// update current string position

*current_pos  = end;

return(end);

} // end Get_Token

///////////////////////////////////////////////////////////////////////////////

int Extract_Tokens(char *string)
{
// this function breaks the input string down into tokens and fills up
// the global sentence array with the tokens so that it can be processed

int curr_pos=0,      // current position in string
    curr_token=0,    // current token number
    found,           // used to flag if the token is valid in language
    index;           // loop index

char output[16];

// reset number of tokens and clear the sentence out

num_tokens=0;

for (index=0; index<8; index++)
    sentence[index]=0;

// extract all the words in the sentence (tokens)

while(Get_Token(string,output,&curr_pos))
     {

     // test to see if this is a valid token

     for (index=0,found=0; index<NUM_TOKENS; index++)
         {

         // do we have a match?

         if (strcmp(output,language[index].symbol)==0)
            {
            // set found flag

            found=1;

            // enter token into sentence

            sentence[curr_token++] = language[index].value;
        //    printf("\nEntering %s, %d in sentence",
        //                               output,language[index].value);

            break;

            } // end if

         } // end for index

         // test if token was part of language (grammar)

         if (!found)
            {
            printf("\n%s, I don't know what \"%s\" means.",you.name
                                                          ,output);

            // failure

            return(0);

            } // end if not found

         // else

         num_tokens++;

     } // end while

return(1);

} // end Extract_Tokens

///////////////////////////////////////////////////////////////////////////////

void Verb_Parser(void)
{
// this function breaks down the sentence and based on the verb calls the
// appropriate "method" or function to apply that verb
// note: syntactic analysis could be done here, but I decided to place it
// in the action verb functions, so that you can see the way the errors are
// detected for each verb (even though there is a a lot of redundancy)

// what is the verb?

switch(sentence[FIRST_WORD])
      {

      case ACTION_MOVE:
           {
           // call the appropriate function

           Verb_MOVE();

           } break;

      case ACTION_TURN:
           {
           // call the appropriate function

           Verb_TURN();

           } break;

      case ACTION_SMELL:
           {
           // call the appropriate function

           Verb_SMELL();

           } break;

      case ACTION_LOOK:
           {
           // call the appropriate function

           Verb_LOOK();

           } break;

      case ACTION_LISTEN:
           {
           // call the appropriate function

           Verb_LISTEN();

           } break;

      case ACTION_PUT:
           {
           // call the appropriate function

           Verb_PUT();

           } break;

      case ACTION_GET:
           {
           // call the appropriate function

           Verb_GET();

           } break;

      case ACTION_EAT:
           {
           // call the appropriate function

           Verb_EAT();

           } break;

      case ACTION_WHERE:
           {
           // call the appropriate function

           Verb_WHERE();

           } break;

      case ACTION_INVENTORY:
           {
           // call the appropriate function

           Verb_INVENTORY();

           } break;

      case ACTION_EXIT:
           {
           // call the appropriate function

           Verb_EXIT();

           } break;

      default:
             {
             printf("\n%s, you must start a sentence with an action verb!",
                    you.name);

             return;

             } break;

      } // end switch

} // end Verb_Parser

// THE ACTION VERBS ///////////////////////////////////////////////////////////

int Verb_MOVE(void)
{
// this function will figure out which way the player wants to move,
// then move the player and test for syntax errors

int token_index,   // current token being processed
    dx,dy;         // ised to hold translation factors

// these look up tables are used to compute the translation factors
// needed to move the player in the requested direction based on the
// current direction, the problem occurs since the directives are not
// absolute directions, they are relative to the direction the player
// is facing

static int forward_x[]={1,-1,0,0};
static int forward_y[]={0,0,-1,1};

static int backward_x[]={-1,1,0,0};
static int backward_y[]={0,0,1,-1};

static int left_x[]={0, 0,-1,1};
static int left_y[]={-1,1,0,0};

static int right_x[]={0,0,1,-1};
static int right_y[]={1,-1,0,0};

// test if player didn't say which way, if so just move forward
// this functionality was added after the fact so is a slight cludge
// it is accomplished by synthetically inserting the direction "forward" into
// the sentence

if (num_tokens==1)
   {
   // no direction given so assume forward

   sentence[SECOND_WORD] = DIR_2_FORWARD;

   num_tokens++;

   } // end if no direction

   // begin further processing to figure out direction

   // check if the next word is a direction and if so move in that
   // direction

   // first test if the words 'to' or 'to the' are inserted bewteen action
   // verb and noun (object).  In this case the phrase "move to the right"
   // sounds ok and should be passed, but "move to the forward" will also
   // be passed even though it is grammatically incorrent, but that's life

   token_index=1;

   if (Check_For_Phrase(PHRASE_TO_THE,token_index))
      {
      // consume preposition since it has to bearing on the final
      // meaning sentence

      // index token scan to directon

      token_index=3;

      } // end if prep and article
   else
   if (Check_For_Phrase(PHRASE_TO,token_index))
      {
      // consume preposition since it has to bearing on the final
      // meaning sentence

      // index token scan to directon

      token_index=2;

      } // end if prep

   // at this point the token_index is pointing to the direction

   if (sentence[token_index] >= DIR_2_START &&
       sentence[token_index] <= DIR_2_END)
      {
      // at this point we finally know what the user is asking for, so
      // let's do it

      // based on direction asked for do movement and collision detection
      // note: the use of look up tables to decrease the complexity of the
      // conditional logic

      dx=dy=0;

      switch(sentence[token_index])
            {

            case DIR_2_FORWARD:  // move player forward
                 {
                 // compute translation factors using look up tables

                 dx = forward_x[you.direction];
                 dy = forward_y[you.direction];

                 } break;

            case DIR_2_BACKWARD: // move player backward
                 {
                 // compute translation factors using look up tables

                 dx = backward_x[you.direction];
                 dy = backward_y[you.direction];

                 } break;

            case DIR_2_RIGHT:    // parry right
                 {
                 // compute translation factors using look up tables

                 dx = right_x[you.direction];
                 dy = right_y[you.direction];

                 } break;

            case DIR_2_LEFT:     // parry left
                 {
                 // compute translation factors using look up tables

                 dx = left_x[you.direction];
                 dy = left_y[you.direction];

                 } break;

            } // end switch direction of motion

      // based on the translation factors move the player

      you.x+=dx;
      you.y+=dy;

      // test for collision with a wall

      if (universe_geometry[you.y][you.x]==WALL_ID)
         {
         // let' user know he hit a wall
         printf("\nOuch! that hurt. Can't you see this wall %s?\n",you.name);

         // back player up

         you.x-=dx;
         you.y-=dy;

         } // end collision detection
      else
         {
         printf("\nYou take a few steps.\n");

         } // end else ok

      return(1);

      } // end if direction is valid
   else
      {
      printf("\n%\"%s\" is an invalid sentence.",global_input);
      printf("\nI don't understand the direction you wish me to move in?");
      return(0);

      } // end else invalid direction

} // end Verb_MOVE

////////////////////////////////////////////////////////////////////////////////

int Verb_TURN(void)
{
// this function will figure out which way the player wants to turn,
// then turn the player and test for syntax errors

int token_index;

// if the player look in general then give him the full view, otherwise
// look for walls and objects

token_index=0;

// first test for a direction

if (num_tokens==1)
   {
   // no direction, so tell user to give one next time

   printf("\n%s, I don't know which way to turn?\n",you.name);
   return(0);

   } // end if only turn
else
   {
   // check if the next word is a direction and if so turn in that
   // direction

   // first test if the words 'to' or 'to the' are inserted bewteen action
   // verb and noun (object)

   token_index=1;

   if (Check_For_Phrase(PHRASE_TO_THE,token_index))
      {
      // consume preposition since it has to bearing on the final
      // meaning sentence

      // index token scan to directon

      token_index=3;

      } // end if prep and article
   else
   if (Check_For_Phrase(PHRASE_TO,token_index))
      {
      // consume preposition since it has to bearing on the final
      // meaning sentence

      // index token scan to directon

      token_index=2;

      } // end if prep

   // at this point the token_index is pointing to the direction

   if (sentence[token_index] >= DIR_1_START &&
       sentence[token_index] <= DIR_1_END)
      {
      // at this point we finally know what the user is asking for, so
      // let's do it

      // update the players direction based on new direction

      you.direction = sentence[token_index]-DIR_1_START;

      printf("\nYou turn...");

      return(1);

      } // end if direction is valid
   else
      {
      printf("\n%\"%s\" is an invalid sentence.",global_input);
      printf("\nI don't understand the direction you wish me to turn to?");
      return(0);
      } // end else invalid direction

   } // end else

} // end Verb_TURN

////////////////////////////////////////////////////////////////////////////////

int Verb_SMELL(void)
{
// this function will just smell without paying attention to the rest
// of the sentence

if (num_tokens==1)
   {

   Print_Info_Strings(smells,universe_geometry[you.y][you.x]);
   return(1);
   } // end if smell
else
   {

   printf("\n%\"%s\" is an invalid sentence.",global_input);
   printf("\nI don't understand what you want me to smell?");
   return(0);
   } // end else invalid

} // end Verb_SMELL

////////////////////////////////////////////////////////////////////////////////

int Verb_LOOK(void)
{
// this function will look in the direction commanded to

int token_index,   // current word being processed
    direction,     // direction player wants to look
    num_items,     // number of objects seen during vision scan
    index;         // used as look index

object objects[8]; // holds the objects

// if the player look in general then give him the full view, otherwise
// look for walls and objects

token_index=0;

// first test for a direction

if (num_tokens==1)
   {
   // no direction, so give long version

   Print_Info_Strings(views,universe_geometry[you.y][you.x]);
   return(1);
   } // end if only look
else
   {
   // check if the next word is a direction and if so look in that
   // direction

   // first test if the words 'to' or 'to the' are inserted bewteen action
   // verb and noun (object)

   token_index=1;

   if (Check_For_Phrase(PHRASE_TO_THE,token_index))
      {
      // consume preposition since it has to bearing on the final
      // meaning sentence

      // index token scan to directon

      token_index=3;

      } // end if prep and article
   else
   if (Check_For_Phrase(PHRASE_TO,token_index))
      {
      // consume preposition since it has to bearing on the final
      // meaning sentence

      // index token scan to directon

      token_index=2;

      } // end if prep

   // at this point the token_index is pointing to the direction

   if (sentence[token_index] >= DIR_1_START &&
       sentence[token_index] <= DIR_1_END)
      {
      // at this point we finally know what the user is asking for, so
      // let's do it

      printf("\nYou see walls");

      // compute direction

      direction = sentence[token_index] - DIR_1_START;

      // test if there are any objects in sight

      if (Vision_System(24,direction,objects,&num_items))
         {

         // print out what was seen

         printf(" and,");

         for (index=0; index<num_items; index++)
             {

             // print out the correct description

             switch(objects[index].thing)
                   {

                   case LAMP_ID:
                        {
                        printf("\na torch lamp.");
                        } break;

                   case KEYS_ID:
                        {
                        printf("\na set of car keys.");
                        } break;

                   case SANDWICH_ID:
                        {
                        printf("\na turkey sandwich.");
                        } break;

                   default:break;

                   } // end switch

             } // end for index

         } // end if we saw something

      return(1);

      } // end if direction is valid
   else
      {
      printf("\n%\"%s\" is an invalid sentence.",global_input);
      printf("\nI don't understand the direction you wish me to look in?");
      return(0);
      } // end else invalid direction

   } // end else

} // end Verb_LOOK

////////////////////////////////////////////////////////////////////////////////

int Verb_LISTEN(void)
{
// this function will just listen without paying attention to the rest of
// the sentence


if (num_tokens==1)
   {

   Print_Info_Strings(sounds,universe_geometry[you.y][you.x]);
   return(1);
   } // end if sound
else
   {

   printf("\n%\"%s\" is an invalid sentence.",global_input);
   printf("\nI don't understand what you want me to listen to?");
   return(0);
   } // end else invalid

} // end Verb_LISTEN

////////////////////////////////////////////////////////////////////////////////

int Verb_PUT(void)
{
// this function will put down the object requested

// this look up table is used to convert object token numbers into object id's

static char object_to_id[]={'l','s','k'};

int token_index, // current toek nbeing precessed
          index; // loop index

char object;     // object we are currently looking at

token_index=0;

// first test for a object

if (num_tokens==1)
   {
   // no object, so tell user to give one next time

   printf("\n%s, I don't know what you want me to put down?\n",you.name);
   return(0);

   } // end if only put
else
   {
   // check if the next word is an object, if so put is down
   // direction

   // first test if the words 'down' or 'down the' are inserted bewteen action
   // verb and noun (object)

   token_index=1;

   if (Check_For_Phrase(PHRASE_DOWN_THE,token_index))
      {
      // consume preposition since it has to bearing on the final
      // meaning sentence

      // index token scan to object

      token_index=3;

      } // end if prep and article
   else
   if (Check_For_Phrase(PHRASE_DOWN,token_index))
      {
      // consume preposition since it has to bearing on the final
      // meaning sentence

      // index token scan to object

      token_index=2;

      } // end if prep

   // at this point the token_index is pointing to the object

   if (sentence[token_index] >= OBJECT_START &&
       sentence[token_index] <= OBJECT_END)
      {
      // at this point we finally know what the user is asking for, so
      // let's do it

      // check to see if object is in inventory..if so then put it down
      // in the current square

      // first convert object token to object id

      object = object_to_id[sentence[token_index]-OBJECT_START];

      // do scan in pockets

      for (index=0; index<you.num_objects; index++)
          {
          // test if this pocket has the object we are looking for

          if (you.inventory[index]==object)
             {
             // take object out of pocket

             you.inventory[index] = EMPTY_ID;

             // decrement number of objects

             you.num_objects--;

             // place the object back into object universe

             universe_objects[you.y][you.x] = object;

             // say something

             printf("\nPutting down the ");

             switch(object)
                   {
                   case LAMP_ID:
                        {printf("torch lamp.\n");}break;

                   case SANDWICH_ID:
                        {printf("sandwich.\n");}break;

                   case KEYS_ID:
                        {printf("keys.\n");}break;

                   default:break;

                   } // end switch

             // **************************************************************
             // if the player puts the keys in the kitchen where they should be
             // then he will win! Test that condition here

             if (object==KEYS_ID && universe_geometry[you.y][you.x]=='k')
                {

                printf("\nCongratulation %s! You have solved the game.\n",you.name);

                } // end if win

             // **************************************************************

             return(1);

             } // end if found object

          } // end for index

      // if we get this far then the player wasn't carrying the requested
      // object to be dropped

      printf("\n%s, you don't have that to drop!\n",you.name);

      return(1);

      } // end if object is valid
   else
      {
      printf("\n%\"%s\" is an invalid sentence.",global_input);
      printf("\nI don't understand the object you wish me to put down?");
      return(0);
      } // end else invalid object

   } // end else

} // end Verb_PUT

////////////////////////////////////////////////////////////////////////////////

int Verb_GET(void)
{
// this function will get the object in the current square

object objects[8];  // the objects within reaching distance of player

// this look up table is used to convert object token numbers into object id's

static char object_to_id[]={'l','s','k'};

int token_index=0,   // current token being processed
          index,     // loop index
          index_2,   // loop index
          num_items; // number of items found during vision scan

char object;       // item we are currently looking at

// first test for a object

if (num_tokens==1)
   {
   // no object, so ask what get

   printf("\n%s, what do you want me to pick up?\n",you.name);

   return(1);

   } // end if only get
else
   {
   // check if the next word is an object

   // first test if the word 'the' is inserted bewteen action
   // verb and noun (object)

   token_index=1;

   if (Check_For_Phrase(PHRASE_THE,token_index))
      {
      // consume article since it has to bearing on the final
      // meaning sentence

      // index token scan to directon

      token_index=2;

      } // end if article

   // at this point the token_index is pointing to the object to pick up

   if (sentence[token_index] >= OBJECT_START &&
       sentence[token_index] <= OBJECT_END)
      {
      // at this point we finally know what the user is asking for, so
      // let's do it

      // using vision system scan forward a depth of 2 units this will simulate
      // the player reaching for the object within a reasonable radius

      // convert object to id

      object = object = object_to_id[sentence[token_index]-OBJECT_START];

       // do forward radial scan

      if (Vision_System(3,you.direction,objects,&num_items))
         {

         // test if the object we desire to pick up is within the objects array

         for (index=0; index<num_items; index++)
             {
             // is this what player wants to pick up?

             if (object==objects[index].thing)
                {

                // remove object from universe

                universe_objects[objects[index].y][objects[index].x] = EMPTY_ID;

                // update players inventory (find an empty pocket and place
                // object in it)

                for (index_2=0; index_2<8; index_2++)
                    {
                    if (you.inventory[index_2]==EMPTY_ID)
                       {
                       // put it in pocket

                       you.inventory[index_2]=object;

                       you.num_objects++;

                       break;

                       } // end if an open spot was found

                    } // end for index_2

                // let user know it was picked up

                printf("\nGot it!\n");

                return(1);

                } // end if object found

             } // end for index

         } // end if any objects within reach
      else
         {

         printf("\nYou are too far from it!\n");
         return(0);

         } // end else can't reach

      } // end if object is valid
   else
      {
      printf("\nI don't understand what you want me to pick up?");
      return(0);

      } // end else invalid object

   } // end else

return(0);

} // end Verb_GET

////////////////////////////////////////////////////////////////////////////////

int Verb_EAT(void)
{
// the function will eat the object it is directed to

// this look up table is used to convert object token numbers into object id's

static char object_to_id[]={'l','s','k'};

int token_index=0, // current token being processed
          index;   // loop index

char object;       // object we are currently looking at

// first test for a direction

if (num_tokens==1)
   {
   // no object, so ask what to eat

   printf("\n%s, what do you want me to eat?\n",you.name);

   return(1);

   } // end if only eat
else
   {
   // check if the next word is an object

   // first test if the word 'the' is inserted bewteen action
   // verb and noun (object)

   token_index=1;

   if (Check_For_Phrase(PHRASE_THE,token_index))
      {
      // consume article since it has to bearing on the final
      // meaning sentence

      // index token scan to directon

      token_index=2;

      } // end if article

   // at this point the token_index is pointing to the object to eat

   if (sentence[token_index] >= OBJECT_START &&
       sentence[token_index] <= OBJECT_END)
      {
      // at this point we finally know what the user is asking for, so
      // let's do it

      // scan thru the inventory and test if the player is carrying the
      // object.  Actually, the only thing he can eat is the sandwich, but
      // we need a little comedy relief and this is a good place for it

      for (index=0; index<8; index++)
          {

          // test if this pocket has object we are interested in


          // first convert object token to object id

          object = object_to_id[sentence[token_index]-OBJECT_START];

          if (you.inventory[index]==object)
          {

          switch(you.inventory[index])
                {

                case LAMP_ID:
                     {
                     printf("\nI don't think I can fit this 6 foot long lamp in my mouth!\n");
                     return(1);
                     } break;

                case KEYS_ID:
                     {
                     printf("\nIf you say so...gulp\n");

                     // extract keys from pocket

                     you.inventory[index]= EMPTY_ID;

                     // decrement number of objects on player

                     you.num_objects--;

                     return(1);

                     } break;

                case SANDWICH_ID:
                     {
                     printf("\nThat was good, but it needed more mustard.\n");

                     // extract sanwich from pocket

                     you.inventory[index]= EMPTY_ID;

                     // decrement number of objects on player

                     you.num_objects--;

                     return(1);

                     } break;

                } // end switch pockets

          } // end if this is the object

          } // end for index

      // didn't find the object

      printf("\nI would really like to eat that, but you don't seem to have one.\n");

      return(1);

      } // end if object is valid
   else
      {
      printf("\nI don't understand what you want me to eat?");
      return(0);

      } // end else invalid object

   } // end else

} // end Verb_EAT

////////////////////////////////////////////////////////////////////////////////

int Verb_WHERE(void)
{
// the function tells the player where he is

// based on the tile the player is standing on in the geometry array
// indicate where he is

if (num_tokens==1)
    {
    // what room is player in

    switch(universe_geometry[you.y][you.x])
          {

          case 'l': //- living room
             {
             printf("\n\nYou are in the living room");
             } break;

          case 'b': //- bedroom
             {
             printf("\n\nYou are in the bedroom");
             } break;

          case 'k': //- kitchen
             {
             printf("\n\nYou are in the kitchen");
             } break;

          case 'w': //- washroom
             {
             printf("\n\nYou are in the washroom");
             } break;

          case 'h': //- hall way
             {
             printf("\n\nYou are in the hallway");
             } break;

          case 'r': //- restroom
             {
             printf("\n\nYou are in the master bathroom");
             } break;

          case 'e': //- entry way
             {
             printf("\n\nYou are in the entry way");
             } break;

          case 'o': //- office
             {
             printf("\n\nYou are in the computer office");
             } break;

          default:break;

          } // end switch


    // now state the direction

    switch(you.direction)
          {

          case EAST:
               {
               printf(" facing East.\n");
               } break;

          case WEST:
               {
               printf(" facing West.\n");
               } break;

          case NORTH:
               {
               printf(" facing North.\n");
               } break;

          case SOUTH:
               {
               printf(" facing South.\n");
               } break;

          default:break;

          } // end switch
    return(1);
    } // end if a valid sentence structure
else
   {

   printf("\n%\"%s\" is an invalid sentence.",global_input);
   printf("\n%s, I just don't get it?",you.name);
   return(0);
   } // end else invalid

} // end Verb_WHERE

////////////////////////////////////////////////////////////////////////////////

int Verb_INVENTORY(void)
{
// this function will print out the current inventory

int index; // loop index


// print out the inventory and then test if the player typed too many words
// if so complain a little

printf("\nYou have the following items within your possesion.\n");

// scan thru inventory array and print out the objects that the player is
// holding

for (index=0; index<8; index++)
    {

    // test if this "pocket" has an object in it

    switch(you.inventory[index])
          {

          case LAMP_ID:
               {
               printf("\nA torch lamp.");
               } break;

          case KEYS_ID:
               {
               printf("\nA set of car keys.");
               } break;

          case SANDWICH_ID:
               {
               printf("\nA turkey sandwich.");
               } break;

          default:break;

          } // end switch

    } // end for index

// test if player has nothing on him

if (you.num_objects==0)
   printf("\nYour pockets are empty %s.",you.name);

// test if there are too many words

if (num_tokens>1)
   printf("\n%s, all you hade to say was \"Inventory!\"",you.name);

return(1);

} // end Verb_INVENTORY

////////////////////////////////////////////////////////////////////////////////

int Verb_EXIT(void)
{
// this function sets the global exit and terminates the game

global_exit=1;

return(1);

} // end Verb_EXIT

////////////////////////////////////////////////////////////////////////////////

int Vision_System(int depth,         // depth of scan
                  int direction,     // direction of scan N,E,S,W
                  object *stuff,     // objects seen in scan
                  int *num_objects)  // number of objects seen in scan

{
// this function is rather complex. It is responsible for the vision of
// the player. It works by scanning a upside down pryamid of squares in
// front of the player.  The objects within this vision window will be
// returned in the objects array along withe the number of them.
// the vision window is built up by consequtively scanning rows of blocks
// in the geometry universe along with testing the objects universe for
// instances of objects. It is sort of like ray casting, but the distance
// is irrelevant and only four directions are used.  For example if the
// player was looking north and a depth of 3 was sent for the scan then the
// scan pattern would look like:
//
//       .....
//        ...
//         P
// where 'P' is the position of player and the '.' is a scanned block
// similary a scan of depth 5 to the east would like like
//      .
//      ..
//      ...
//      ....
//      ....P
//      ....
//      ...
//      ..
//      .
// note: the field of view (FOV) will always be 90 degrees
// anyway the function is basically used for the "LOOK" verb and the "GET"
// verb and I admit that the code is redundant for each case, but to merge
// it all would make it too hard to follow!

int x,y,           // used to hold current universe cell location
    index,         // loop index
    scan_level;    // current level or iteration of the scan

*num_objects=0;

// which direction is vision requested in?

switch(direction)
      {

      case NORTH:
           {
           // scan like this
           //  .....
           //   ...
           //    P

           for (y=you.y,scan_level=0; y>=(you.y-depth); y--,scan_level++)
               {
               for (x=you.x-scan_level; x<=you.x+scan_level; x++)
                   {
                   // x,y is test point, make sure it is within the universe
                   // boundaries and within the same room

                   if (x>=1 && x<NUM_COLUMNS-1 &&
                       y>=1 && x<NUM_ROWS-1 &&
                       universe_geometry[y][x]==universe_geometry[you.y][you.x])


                      {
                      // test to see if square has an object in it

                      if (universe_objects[y][x]!=' ')
                         {
                         // insert the object into object list

                         stuff[*num_objects].thing = universe_objects[y][x];
                         stuff[*num_objects].x     = x;
                         stuff[*num_objects].y     = y;

                         // increment the number of objects

                         (*num_objects)++;

                         } // end if an object was found

                      } // end if in boundaries

                   } // end for x

               } // end for y

           // return number of objects found

           return(*num_objects);

           } break;

      case SOUTH:
           {
           // scan like this
           //  P
           // ...
           //.....

           for (y=you.y,scan_level=0; y<=(you.y+depth); y++,scan_level++)
               {
               for (x=you.x-scan_level; x<=you.x+scan_level; x++)
                   {
                   // x,y is test point, make sure it is within the universe
                   // boundaries

                   if (x>=1 && x<NUM_COLUMNS-1 &&
                       y>=1 && x<NUM_ROWS-1 &&
                       universe_geometry[y][x]==universe_geometry[you.y][you.x])
                      {
                      // test to see if square has an object in it

                      if (universe_objects[y][x]!=' ')
                         {
                         // insert the object into object list

                         stuff[*num_objects].thing = universe_objects[y][x];
                         stuff[*num_objects].x     = x;
                         stuff[*num_objects].y     = y;

                         // increment the number of objects

                         (*num_objects)++;

                         } // end if an object was found

                      } // end if in boundaries

                   } // end for x

               } // end for y

           // return number of objects found

           return(*num_objects);

           } break;


      case EAST:
           {
           // scan like this
           //   .
           //  ..
           // P..
           //  ..
           //   .

           for (x=you.x,scan_level=0; x<=(you.x+depth); x++,scan_level++)
               {
               for (y=you.y-scan_level; y<=you.y+scan_level; y++)
                   {
                   // x,y is test point, make sure it is within the universe
                   // boundaries

                   if (x>=1 && x<NUM_COLUMNS-1 &&
                       y>=1 && x<NUM_ROWS-1 &&
                       universe_geometry[y][x]==universe_geometry[you.y][you.x])
                      {
                      // test to see if square has an object in it

                      if (universe_objects[y][x]!=' ')
                         {
                         // insert the object into object list

                         stuff[*num_objects].thing = universe_objects[y][x];
                         stuff[*num_objects].x     = x;
                         stuff[*num_objects].y     = y;

                         // increment the number of objects

                         (*num_objects)++;

                         } // end if an object was found

                      } // end if in boundaries

                   } // end for y

               } // end for x

           // return number of objects found

           return(*num_objects);

           } break;

      case WEST:
           {
           // scan like this
           // .
           // ..
           // ..P
           // ..
           // .
           //

           for (x=you.x,scan_level=0; x>=(you.x-depth); x--,scan_level++)
               {
               for (y=you.y-scan_level; y<=you.y+scan_level; y++)
                   {
                   // x,y is test point, make sure it is within the universe
                   // boundaries

                   if (x>=1 && x<NUM_COLUMNS-1 &&
                       y>=1 && x<NUM_ROWS-1 &&
                       universe_geometry[y][x]==universe_geometry[you.y][you.x])
                      {
                      // test to see if square has an object in it

                      if (universe_objects[y][x]!=' ')
                         {
                         // insert the object into object list

                         stuff[*num_objects].thing = universe_objects[y][x];
                         stuff[*num_objects].x     = x;
                         stuff[*num_objects].y     = y;

                         // increment the number of objects

                         (*num_objects)++;

                         } // end if an object was found

                      } // end if in boundaries

                   } // end for y

               } // end for x

           // return number of objects found

           return(*num_objects);

           } break;

      } // end switch direction

return(0);

} // end Vision_System

////////////////////////////////////////////////////////////////////////////////

int Check_For_Phrase(int phrase,int index)
{
// this function is used to test for small phrases that when extracted don't
// change the measning of a sentence for example:"look to the west" and
// "loo west" and "look to west" all mean the same thing.

// test which phrase is to be checked

switch(phrase)
      {
      case PHRASE_TO: // have we found the prep. "to"
           {

           if (sentence[index]==PREP_TO)
              return(1);

           } break;

      case PHRASE_THE: // have we found the article "the"
           {

           if (sentence[index]==ART_THE)
              return(1);

           } break;


     case PHRASE_DOWN: // have we found the prep/adj "down"
          {

          if (sentence[index]==PREP_DOWN)
             return(1);

          } break;

      case PHRASE_TO_THE: // have we found the prep. phrase "to the"
           {

           if (sentence[index]==PREP_TO)
              {
              if (sentence[index+1]==ART_THE)
                  return(1);
              else
                  return(0);
              } // end if got "to the"
           } break;

      case PHRASE_DOWN_THE: // have we found the prep. phrase "down the"
           {

           if (sentence[index]==PREP_DOWN)
              {
              if (sentence[index+1]==ART_THE)
                  return(1);
              else
                  return(0);
              } // end if got "down the"
           } break;


      default:break; // there is a serious problem!

      } // end switch

// we have failed

return(0);

} // end Check_For_Phrase

///////////////////////////////////////////////////////////////////////////////

void Print_Info_Strings(info_string strings[],char where)
{
// this function will print the info strings out of the sent array based
// on the the current location of the player i.e. bedroom, kitchen etc.

int index=0;

printf("\n");

// traverse list and print all string relating to this room

while(strings[index].type!='X')
     {

     // should this string be printed

     if (strings[index].type==where)
        {

        printf("\n%s",strings[index].string);

        } // end if this is a relevant string

     // next string

     index++;

     } // end while

printf("\n");

} // end Print_Info_Strings

///////////////////////////////////////////////////////////////////////////////

void Introduction(void)
{

int index;

for (index=0; index<50; index++,printf("\n"));

// make the screen blue with white characters
// only works with ansi driver

printf("%c%c37;44m",27,91);

printf("\n     SSSSSSSSSS  H      H  AAAAAAAA  DDDDD     OOOOOOOO  W       W");
printf("\n     S           H      H  A      A  D    D    O      O  W       W");
printf("\n     S           H      H  A      A  D     D   O      O  W       W");
printf("\n     S           H      H  A      A  D      D  O      O  W       W");
printf("\n     S           H      H  A      A  D      D  O      O  W       W");
printf("\n     SSSSSSSSSS  HHHHHHHH  AAAAAAAA  D      D  O      O  W       W");
printf("\n              S  H      H  A      A  D      D  O      O  W       W");
printf("\n              S  H      H  A      A  D      D  O      O  W   W   W");
printf("\n              S  H      H  A      A  D      D  O      O  W   W   W");
printf("\n              S  H      H  A      A  D     D   O      O  W   W   W");
printf("\n              S  H      H  A      A  D    D    O      O  W   W   W");
printf("\n     SSSSSSSSSS  H      H  A      A  DDDDD     OOOOOOOO  WWWWWWWWW");
printf("\n                                                                  ");
printf("\n     L         AAAAAAA  NNNNNNN  DDDDDD                           ");
printf("\n     L         A     A  N     N  D     D                          ");
printf("\n     L         A     A  N     N  D      D                         ");
printf("\n     L         A     A  N     N  D      D                         ");
printf("\n     L         AAAAAAA  N     N  D      D                         ");
printf("\n     L         A     A  N     N  D      D                         ");
printf("\n     L         A     A  N     N  D     D                          ");
printf("\n     L         A     A  N     N  D    D                           ");
printf("\n     LLLLLLL   A     A  N     N  DDDDD                            ");
printf("\n                                                                  ");
printf("\n     By Andre' LaMothe                                            ");


while(!kbhit());

for (index=0; index<50; index++,printf("\n"));

} // end Introduction

// M A I N /////////////////////////////////////////////////////////////////////

void main(void)
{

// call up intro

Introduction();

printf("\n\nWelcome to the world of  S H A D O W  L A N D...\n\n\n");

// obtain users name to make game more personal

printf("\nWhat is your first name?");
scanf("%s",you.name);

// main event loop,note: it is NOT real-time

while(!global_exit)
     {
     // put up an input notice to user

     printf("\n\nWhat do you want to do?");

     // get the line of text

     Get_Line(global_input);

     printf("\n");

     // break the text down into tokens and build up a sentence

     Extract_Tokens(global_input);

     // parse the verb and execute the command

     Verb_Parser();

     } // end main event loop

printf("\n\nExiting the universe of S H A D O W  L A N D...see you later %s.\n",you.name);

// restore screen color

printf("%c%c37;40m",27,91);

} // end main


