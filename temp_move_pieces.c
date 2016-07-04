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
#ifndef __MOVE_PIECES_C__
#define __MOVE_PIECES_C__


#include "move_pieces.h"

// GLOBALS

  Spacecraft spacecraft;
  Missile missile[4];
  Enemy EnemyFighters[TOTALENEMY] = {0};
	//scenery


int32 setup_game_pieces_for_start(void)
{

  spacecraft.num_of_pieces = 1;
  spacecraft.x_pos = 30;
  spacecraft.y_pos = 30;
	spacecraft.type = CRAFT_ONE;


  memset(missile,0, sizeof(missile));
	missile[0].num_of_pieces = 1;
	// *coords = malloc (2* size of(int) * num_of_pieces )
	missile[1].num_of_pieces = 1;
	missile[2].num_of_pieces = 1;
	missile[3].num_of_pieces = 1;

	//scenery


	return 0;
}


// must use void* so diff types of craft, missile_array and enemies_array
static int32 move_pieces(void)
{
	move_craft((Spacecraft)craft, (Missile)missi);
	
	//move baddies();

	//move missile();

	//move scenery();

	//detect collisions();
}

/*
Works for eery types as long as structure is adheared to
move_left(&blah[0])
move_left(&blah[1])
*/
static int32 move_left(void *thing)
{
	uint32 num_of_pieces_thing_has;
	int32 i;

	num_of_pieces_thing_has = (uint32)*(thing);	// first var in struct

	//move to start of xpos in sructure
	thing += 4;

	for ( i=0; i<num_of_pieces_thing_has; i++ )
	{
		(*thing) -= 1;	// shift left..casting?
		thing += 8;	// move to next xpos by jumping over x/ypos
		//error checking for edge here?? 
		if (*(thing-8) == hit_edge) 
		{
			//undo what weve done
			for( ; num_of_pieces_thing_has > 0; --num_of_pieces_thing_has)
			{ 
				thing -= 8;	// move to next xpos by jumping over x/ypos
				*thing += 1;	// shift left
			}
			return hit_edge;
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

int32 move_up(void *thing)
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
int32 move_down(void *thing)
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





#endif //__MOVE_PIECES_C__
