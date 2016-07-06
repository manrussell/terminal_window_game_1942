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
#ifndef __APP_C__
#define __APP_C__

/*
NOtes:
goal one:
craft goes up/down/l/r
can use more than 1 type of craft
bonus: craft can fire

goal two:
same as one but for enemy

goal three
collision detection

Jobs...

don't print the whole line if there if nothing on it, think baud rate, 80x50 = 4000, at 115200 baud = 11kb ?

*/
#include "move_pieces.h"
3include "draw_pieces.h"

#include "App.h"

//Typedef
/////////


//GLOBALS/
//////////

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

#ifndef WINV
	uint32 read_key(void);
#endif


/*
**
**
**/
int32 appmain(int argc, char **argv)
{
  uint32 i;

	setup_game_pieces_for_start();


#ifdef WINV
	//set terminal window options.
	setbgcolour(DK_BLUE);
	setfgcolour(LT_YELLOW);
#endif

  clrscr(); //terminal window

	clear_lst_line_n_gameBoard();	// game engine

	while (1)
	{
		// must use void* so diff types of craft, missile_array and enemies_array
		move_pieces();

		// put pieces on board
		place_pieces();

    last_line_position(&lastLinePosition[0]);
    
    drawBoard(&gameBoard, &lastLinePosition[0]);
    

#ifdef DEBUG
    //gotoxy(0,0);
		//printf("craft.xpos=%d, ypos=%d\n",spacecraft.x_pos, spacecraft.y_pos);
		for( i=0; i<(TOTALMISSILECOUNT-1) ;i++ )
		{
		  //printf("missile[%d].xpos=%d ypos=%d, inuse=%d\n",i, missile[i].x_pos, missile[i].y_pos, missile[i].inuse);
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

// must use void* so diff types of craft, missile_array and enemies_array

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
