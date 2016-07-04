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
#ifndef 
#define

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



////

int32 setup_game_pieces_for_start(void);
int32 move_craft(Spacecraft *craft, Missile *missi);
int32 move_pieces(void);
int32 move_up(void *thing);
int32 move_down(void *thing);
int32 move_left(void *thing);
int32 move_right(void *thing);

//static int32 updateEnemyPos(Enemy *enemies, uint32 *enemyCount);
//int32 detect_collisions(void);	// write this

#endif //
