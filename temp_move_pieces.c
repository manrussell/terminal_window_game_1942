// must use void* so diff types of craft, missile_array and enemies_array
static int32 move_pieces(void *craft, void *missi, void *Enemies, void * scenery, void *explosions)
{
	move_craft((Spacecraft)craft, (Missile)missi);
	
	//move baddies();

	//move missile();

	//move scenery();

	//detect collisions();
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
			//move_right(craft);
    }
    else if(k ==0x77) //KEY_W
    {
			//move_up(craft);
    }
    else if(k ==0x73) //KEY_S
    {
			//move_down(craft);
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
  else
  {

  }

  //flush_keybuffer();
  k = k & 0;
  
  return 0;
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


