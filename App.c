/*!
 *********************************************************************
 **
 ** \file       testApp.c
 **
 ** \author    mark russell (russelm2)
 ** \date       30/11/2015
 **
 ** \brief        
 **
 ** \code
 ** Copyright : PACE Microtechnology 2015 (c)
 **
 **             The copyright of this material is owned by Pace
 **             Microtechnology PLC ().  This material is
 **             regarded as a highly confidential trade secret of
 **             Pace.  It may not be reproduced, used, sold or in any
 **             other way exploited or transferred to any other
 **             third party without the prior consent of Pace.
 ** \endcode
 **
 **********************************************************************/
#ifndef __APP_C__
#define __APP_C__

/*
NOtes:
// either use void* or Missile * ...?
// what seasier  to read, neater, less cpu cycles bug fix? ... type, not void* 



goal one:
craft goes up/down/l/r
can use more than 1 type of craft
bonus: craft can fire

goal two:
same as one but for enemy

goal three
collision detection



Jobs...
move missiles
draw missile



don't print the whole line if there if nothing on it, think baud rate, 80x50 = 4000, at 115200 baud = 11kb ?

seperate the updatexxpos() into, remove, update position and redraw
then make up different things easier
or...

main
{
	introduce new pieces 
	move all pieces, do collisions
	clear board
	redraw the board
}

structs
{	uint32 num_of_pieces
	uint32 xpos leg_1
	uint32 ypos leg_1
	uint32 xpos leg_2
	uint32 ypos leg_2
	...
	//otherstuff
}



draw_pieces_on_board(void *craft, void *missi, void *Enemies, void * scenery, void *explosions)
{
	//draw_user( (Spacecraft)craft )
	draw_user(Spacecraft *craft)
	{
		//what type of craft?
			switch(craft.type)
				case CRAFT_ONE
					gameBoard[craft->x_pos][craft->y_pos] = '^';
					break;
				default
					gotoxy(0,0);
 		 			printf("Error with craft type");
					break;
	}

	//draw missiles(Missile *missile)
	{
		switch(missile.type)
			case MISSILE_TYPE_ONE
				gameBoard[craft->x_pos][craft->y_pos] = '.';
				break;
			default
				gotoxy(1,0);
		 		printf("Error with missile type");
				break;
	}

	//draw_enemies()

	//drawe_explosions()

	//draw_scenery()
}

*/

/*
putch(13) = gotoxy(0, y)
putch(8)  = gotoxy(x-1, y)
*/

#include "App.h"

//Typedef
/////////
typedef struct
{
	uint32 num_of_pieces;
  int32 x_pos;						// to have more than one here need to use pointers and malloc spacefor x/y
  int32 y_pos;
  bool   inuse;
	int32  type;
}Missile;

// either use void* or Missile * ...?

//typedef struct
//{
	//uint32 num_of_pieces;
  //int32 *coords;						// malloc (2* size of(int) * num_of_pieces )
  //bool   inuse;
	//int32  type;
//}Missile;



typedef struct
{
	uint32 num_of_pieces;
  int32 x_pos;
  int32 y_pos;
	int32 type;
}Spacecraft;


typedef struct
{
	uint32 	num_of_pieces;
  int32  x_pos;
  int32  y_pos;
  int32  type; // minion fighter, fast fighter, strong fighter, boss
  uint32  hits; //each type will only have ao many hits, 0 hits == dead
}Enemy;

//GLOBALS/
//////////
//static uint32 enemies_on_screen = 0;
static char   gameBoard[MAX_TERMINAL_WIDTH][BOARDHEIGHT];  // store game pieces in this array
static uint32 lastLinePosition[BOARDHEIGHT] = {0};         // is there anything on this line to print, if not draw function just new lines instead of printing ' '
                                            // what is the value of the right most game piece, after that just newline.

/* for input into the test engine */
#ifndef WINV
  TE_COMMAND game_funcs[] = 
  {
    {"game",
      "appmain",
      "init_game 1941",
      0,
      0,
      &appmain,
      TE_AUTO
    }
  };
#endif


//Prototypes
////////////
//static int32 updateCraftPos(Spacecraft *craft, Missile *missi);
//static int32 updateMissilePos(Missile *missil);
static int32 maybe_introduce_new_enemy(Enemy *enemiest, uint32 *enemyCnt); //write this
//static int32 updateEnemyPos(Enemy *enemies, uint32 *enemyCount);
static int32 hit_or_miss_enemy(Missile *missile, Enemy *enemy, Spacecraft *usercraft );



int32 detect_collisions(void);	// write this

static int32 drawExplosion(Missile *missile); // write this 
static int32 drawBigExplosion(void); //write this

static int32 last_line_position(uint32 * LstLinePos);
static int32 drawBoard(void *board, uint32 *lineLength);

static int32 move_craft(Spacecraft *craft, Missile *missi);
static int32 move_pieces(void *craft, void *missi, void *Enemies, void * scenery, void *explosions);
static int32 move_left(void *thing);
static int32 move_right(void *thing);

// after pices have been moved put back on board then.. last line can count
static int32 place_pieces(void *craft, void *missi, void *Enemies, void * scenery, void *explosions);

//drae_Craft_type_00();
static int32 draw_user(void *craft); // void* or craft* here? if void star have to know which type eg craft 1/2 (found in type)

static int32 draw_minion(Enemy *e);
int32 move_minion(Enemy *e);

#ifndef WINV
	uint32 read_key(void);
#endif



int32 appmain(int argc, char **argv) //int32 xxx ( uint32 argc, char **argv )
{
  uint32 i;

  Spacecraft spacecraft;
  spacecraft.num_of_pieces = 1;
  spacecraft.x_pos = 30;
  spacecraft.y_pos = 30;
	spacecraft.type = CRAFT_ONE;

  Missile missile[4];
  memset(missile,0, sizeof(missile));
	missile[0].num_of_pieces = 1;
	// *coords = malloc (2* size of(int) * num_of_pieces )
	missile[1].num_of_pieces = 1;
	missile[2].num_of_pieces = 1;
	missile[3].num_of_pieces = 1;

  Enemy EnemyFighters[TOTALENEMY] = {0};
	EnemyFighters[0].num_of_pieces = 1;
	EnemyFighters[1].num_of_pieces = 1;

	void* scraft;			// array
	void* msiles;		// array
	void* scenery = NULL;			// array
	void* explosions = NULL;		// array

	scraft = (void*)&spacecraft;
	msiles = (void*)&missile[0];


#ifdef WINV
	//set terminal window options.
	setbgcolour(DK_BLUE);
	setfgcolour(LT_YELLOW);
#endif

  clrscr();

  memset(lastLinePosition, 0, sizeof(lastLinePosition));
	memset(gameBoard, 0x20, sizeof(gameBoard) );

	while (1)
	{
		//maybe_introduce_new_enemy(&EnemyFighters[0], &enemies_on_screen);

// must use void* so diff types of craft, missile_array and enemies_array
		move_pieces(scraft, msiles, &EnemyFighters[0], scenery, explosions);

		// put pieces on board
		place_pieces(scraft, msiles, &EnemyFighters[0], scenery, explosions);

    last_line_position(&lastLinePosition[0]);
    
    drawBoard(&gameBoard, &lastLinePosition[0]);
    

#ifdef DEBUG
    //gotoxy(0,0);
		printf("craft.xpos=%d, ypos=%d\n",spacecraft.x_pos, spacecraft.y_pos);
		for( i=0; i<(TOTALMISSILECOUNT-1) ;i++ )
		{
		  printf("missile[%d].xpos=%d ypos=%d, inuse=%d\n",i, missile[i].x_pos, missile[i].y_pos, missile[i].inuse);
		}
#endif

		flush_keybuffer();
    sleep(20);

    //getch();

    //clear image ready for new screen
    clrscr();

	}

	return (0);
}





/*
**
**
**/
static int32 maybe_introduce_new_enemy(Enemy *enemiest, uint32 *enemyCnt)
{

  enemiest[0].x_pos = 40;
  enemiest[0].y_pos = 5;
  enemiest[0].type = ENEMYMINION;
  
  return 0;
}


/*
  search through the array finding the last game piece oneach line
  start at the right hand side and go left until a game piece is found
  - could be improved to just alter neccessary lines and not all
*/
static int32 last_line_position(uint32 *LstLinePos)
{
  uint32 y;
  uint32 x;
  
  // clear all previous values - could be improved to just alter neccessary 
  memset(&LstLinePos[0], 0, sizeof(lastLinePosition) );

  //search through the array finding the last game piece oneach line
  for(y=0; y<BOARDHEIGHT; y++)  //goes to 49
  {    
    //start at the right hand side and go left until a game piece is found
    for( x=(MAX_TERMINAL_WIDTH-1); ((x>=0) && (x<MAX_TERMINAL_WIDTH)); x--) // make sure if x goes to 0 it does not continue to loop round
    {
      if(gameBoard[x][y] != 0x20 )  //' ')
      {
        LstLinePos[y] = x;
        break;
      }
      // else
      // {
        // LstLinePos[y] = 0;
      // }
    }
  }

  return 0;
}


/*
  will onlt print if something on the line to print, else newlines except last 
*/
static int32 drawBoard(void *board, uint32 *lineLength)
{
  uint32 x;
  uint32 y;

  for(y=0; y<BOARDHEIGHT; y++)
  {
    // for each line print the chars until lastLinePosition
    for(x=0; x<=lineLength[y]; x++)
    {
      printf("%c",gameBoard[x][y] );
      //printf("%x",gameBoard[x][y] );
    }

    //debug
    //printf("(%d=%d)",y, lastLinePosition[y] );

    #ifdef WINV
    //goto next line, but not on last line else weirdness
    if (y<BOARDHEIGHT)
    {
      printf("\n");
    }
    #else //if real terminal always need a new line!!
    printf("\n");
    #endif

  }

  return 0;
}



/* hit or miss enemy with ship or missile*/
static int32 hit_or_miss_enemy(Missile *missile, Enemy *enemy, Spacecraft *usercraft )
{
  uint32 i,j;

  //loop through misiles in use
  for(i=0; i<=(TOTALMISSILECOUNT-1) ; i++)
  {
    // loop though the enemy fighters to see if a match for this missile
    for(j=0; j<(TOTALENEMY-1); j++)
    {
      //if one ccord match then go check the other
      if( missile[i].x_pos == enemy[j].x_pos)
      {
        if( missile[i].y_pos == enemy[j].y_pos)
        {
         //hit!
          
        }
      }
      else
      {
        //not match for this missile and fighter
      }
    }
  }


  // if missile == an emeny
  // inuse = false-
  // gameBoard[missile[i].x_pos][missile[i].y_pos] = 'X';
      

  return 0;
}

static int32 drawExplosion(Missile *m)
{
  //gotoxy(missile[num].x_pos, missile[num].y_pos);
	gameBoard[m->x_pos][m->y_pos] = 0x58;
  
  return 0;
}


static int32 draw_minion(Enemy *e)
{
	return 0;
}

static int32 move_left(void *thing)
{
	uint32 num_of_pieces_thing_has;
	uint32 i;
	int32 j;
	uint32 *ptr;

	ptr = thing;

	num_of_pieces_thing_has = *(uint32*)(ptr);	// first var in struct

	//move to start of xpos in sructure
	ptr += 1;

	for ( i=0; i<num_of_pieces_thing_has; i++ )
	{
		*(ptr) -= 1;	// shift left..casting?
		ptr += 2;	// move to next xpos by jumping over x/ypos
		//error checking for edge here?? 
		if ( *(ptr-2) < 0) 
		{
			//undo move
			for(j=i; j > 0; j--)
			{ 
				ptr -= 2;	// move to next xpos by jumping over x/ypos
				*ptr += 1;	// shift left
			}
			//return 1;
		}
		
		//did not hit the edge
	}
	
	//success shifted left
	return 0;
}


static int32 move_right(void *thing)
{
	uint32 num_of_pieces_thing_has;
	uint32 i;
	int32 j;
	uint32 *ptr;

	ptr = thing;

	num_of_pieces_thing_has = *ptr;	// first var in struct

	//move to start of xpos in sructure
	ptr += 1;

	for ( i=0; i<num_of_pieces_thing_has; i++ )
	{
		*(ptr) += 1;	// shift right
		ptr += 2;	// move to next xpos by jumping over x/ypos
		//error checking for edge here?? 
		if ( *(ptr-2) > (BOARDWIDTH-1) ) 
		{
			//undo move
			for(j=i; j > 0; j--)
			{ 
				ptr -= 2;	// move to next xpos by jumping over x/ypos
				*ptr -= 1;	// shift left
			}
			//return 1;
		}
		
		//did not hit the edge
	}
	
	//success shifted left
	return 0;
}

static int32 move_up(void *thing)
{
	uint32 num_of_pieces_thing_has;
	uint32 i;
	int32 j;
	uint32 *ptr;

	ptr = thing;

	num_of_pieces_thing_has = *ptr;	// first var in struct

	//move to start of Y_pos
	ptr += 2;

	for ( i=0; i<num_of_pieces_thing_has; i++ )
	{
		*(ptr) -= 1;	// shift up screen
		ptr += 2;	// move to next xpos by jumping over x/ypos
		//error checking for edge here?? 
		if ( *(ptr-2) > (BOARDHEIGHT-10) ) 
		{
			//undo move
			for(j=i; j > 0; j--)
			{ 
				ptr -= 2;	// move to next xpos by jumping over x/ypos
				*ptr += 1;	// shift down screen
			}
			//return 1;
		}
		
		//did not hit the edge
	}
	
	//success shifted left
	return 0;
}




/*

*/
static int32 move_down(void *thing)
{
	uint32 num_of_pieces_thing_has;
	uint32 i;
	int32 j;
	uint32 *ptr;

	ptr = thing;

	num_of_pieces_thing_has = *ptr;	// first var in struct

	//move to start of Y_pos
	ptr += 2;

	for ( i=0; i<num_of_pieces_thing_has; i++ )
	{
		*(ptr) += 1;	// shift down screen
		ptr += 2;	// move to next xpos by jumping over x/ypos
		//error checking for edge here?? 
		if ( *(ptr-2) > (BOARDHEIGHT-10) ) 
		{
			//undo move
			for(j=i; j > 0; j--)
			{ 
				ptr -= 2;	// move to next xpos by jumping over x/ypos
				*ptr -= 1;	// shift up screen
			}
			//return 1;
		}
		
		//did not hit the edge
	}
	
	//success shifted left
	return 0;
}





static int32 move_craft(Spacecraft *craft, Missile *missi)
{
  uint32 k;
  uint32 i;
	
  //user controls for space craft
  if (keypressed())
  {
    k = read_key();	             /* lower 8 bits are for ascii char, upper relate to shift state. */
    k = 255 & k;

    // print craft or, if keys pressed update craft new position, 

    if(k == 0x20 ) //KEY_SPACE missile fired!!
    {
			//loop through all missiles see if any free to fire
			for(i=0; i<TOTALMISSILECOUNT; i++)
			{
				//if free missile assign missile break out of loop
				if( missi[i].inuse == false )
				{
					missi[i].inuse = true;
					missi[i].x_pos = craft->x_pos;   //fire from craft
					missi[i].y_pos = craft->y_pos-1; //on the nose of the craft
					break;
				}
				//else do nothing
			}
    }
    else if(k==0x61) //KEY_A move left
    {
			move_left(craft);	  
    }
    else if(k ==0x64) //KEY_D
    {
			move_right(craft);
    }
    else if(k ==0x77) //KEY_W
    {
			move_up(craft);
    }
    else if(k ==0x73) //KEY_S
    {
			move_down(craft);
    }
    else if(k ==0x4B) //KEY_K
    {
      //quit game
      return -1;
    }
    else
    { // key not recognised

    }
  }
  //no key pressed

  //flush_keybuffer();
  k = k & 0;
  
  return 0;
}

// must use void* so diff types of craft, missile_array and enemies_array
static int32 move_pieces(void *craft, void *missi, void *Enemies, void * scenery, void *explosions)
{
	move_craft(craft, missi);
	
	//move baddies();

	//move missile();

	//move scenery();

	//detect collisions();

	return 0;
}

static int32 draw_user(void *craft)
{
	int32 *craftTypePtr;
	int32 *craftPtr_numPieces;
	int32 *craftPosPtr;
	int32 jump_2_type;
	int32 i=0;

	craftPtr_numPieces = craft;
	craftTypePtr = craft;

	//find num of pieces and jump past
	jump_2_type = (*craftPtr_numPieces) *2  ; // X_pos and Y_pos so jump so, *2

	craftTypePtr = *(craftPtr_numPieces + jump_2_type );

	switch(*craftTypePtr)
	{
		case CRAFT_ONE:
			craftPosPtr = craftTypePtr++; //goto x_Pos

			for(i=0; i<(*craftPtr_numPieces) ;  i++)
			{
				gameBoard[craftPosPtr[i]][craftPosPtr[i+1]] = '^';
			}
			break;
		//case CRAFT_TWO:
			//craftPtr = (Spacecraft*) craft;
			//gameBoard[craft->x_pos][craft->y_pos] = '<^>';
			//break;
		default:
	 		printf("Error with craft type");
			break;
	}
}


/*
	clears the game board
	puts each piece on the board
*/
static int32 place_pieces(void *craft, void *missi, void *Enemies, void * scenery, void *explosions)
{
	// pointer of type
	Spacecraft *s = craft;
	int32 i;

	// empty board??
	memset(gameBoard, 0x20, sizeof(gameBoard) );

	draw_user(craft );
	
	//draw missiles(Missile *missile)
	//{
		//switch(missile.type)
			//case MISSILE_TYPE_ONE:
				//gameBoard[craft->x_pos][craft->y_pos] = '.';
				//break;
			//default:
				//gotoxy(1,0);
		 		//printf("Error with missile type");

	//}

	
	// do missiles

	// enemies

	//do explosions

	//do scenery

	return 0;
}



#ifndef WINV

/*!
******************************************************************************
**
** \fn int32 init_game(void)
**
** \brief adds the test to the test engine cmd table. run when Nor driver is initialised ie in BLI_NOR_INIT()
** \return True if tests registerd successfully, else false.
**
*****************************************************************************/
int32 init_game(void)
{
  int32 res;
  
  /*! put the test into the command table of the test engine. */
  res = TE_register_command(&game_funcs[0], sizeof(game_funcs) / sizeof(TE_COMMAND));  
  if(res != true)
  {
    printf("TE_register_command() game failed.");          
    return (res);
  } 
  
  return res;
}

/* No read key in plibc*/
uint32 read_key(void) /* lower 8 bits are for ascii char, upper relate to shift state. this will not work here as getch() returns a char */
	{
		return (uint32)(getch());
	}
  
#endif

#endif /* __APP_C__ */
