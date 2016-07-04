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

#ifndef  //__DRAW_PIECES_H__
#define  //__DRAW_PIECES_H__


int32 clear_lst_line_n_gameBoard(void);

static int32 drawExplosion(Missile *missile); // write this 
static int32 drawBigExplosion(void); //write this

static int32 last_line_position(uint32 * LstLinePos);
int32 drawBoard(void *board, uint32 *lineLength);


static int32 draw_user(void *craft); // void* or craft* here? if void star have to know which type eg craft 1/2 (found in type)

static int32 draw_minion(Enemy *e);


// after pices have been moved put back on board then.. last line can count
int32 place_pieces(void);



#endif //__DRAW_PIECES_H__
