/*!
 *********************************************************************
 **
 ** \file      App.c
 **
 ** \author    mark russell (russelm2)
 ** \date      30/11/2015
 **
 ** \brief        
 **
 ** \code
 ** Copyright : 
 ** \endcode
 **
 **********************************************************************/
#ifndef __DRAW_PIECES_C__
#define __DRAW_PIECES_C__

#include "draw_pieces.h"


static char   gameBoard[MAX_TERMINAL_WIDTH][BOARDHEIGHT];  // store game pieces in this array
static uint32 lastLinePosition[BOARDHEIGHT] = {0};         // is there anything on this line to print, if not draw function just new lines instead of printing ' '
 


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
static int32 place_pieces(void)
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

/*
**	\fn 
**	\brief 
**
**/
int32 drawBoard(void *board, uint32 *lineLength)
{
  uint32 x;
  uint32 y;

  for(y=0; y<BOARDHEIGHT; y++)
  {
    //if nothing on this line goto the next line
    if(lineLength[y] == 0)
    {
      //nothing on line

    }
    else
    {
      //if somthing on line print upto the last game piece
      for(x=0; x<=lineLength[y]; x++)
      {
        printf("%c",gameBoard[x][y] );
      }
    }

    //printf("x=%d, y=%d, lineLength[y] = %d \n", x,y, lineLength[y]);
  }
  
  //debug
  //printf("(%d=%d)",y, lastLinePosition[y] );

  //#ifdef WINV
  //goto next line, but not on last line else weirdness
  //  if (y<BOARDHEIGHT)
  //  {
  //    printf("\n");
  //  }
  //#else //if real terminal always need a new line!!
  //  printf("\n");
  //#endif

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

int32 clear_lst_line_n_gameBoard(void)
{
	memset(lastLinePosition, 0, sizeof(lastLinePosition));
	memset(gameBoard, 0x20, sizeof(gameBoard) );

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



#endif // __DRAW_PIECES_C__
