/*!
*********************************************************************
**
** \file       testApp.h
**
** \author	   mark russell (russelm2)
** \date       30/11/2015
**
** \brief	   		
**
** \code
** \endcode
**
 **********************************************************************/
#ifndef __APP_H__
#define __APP_H__

#define DEBUG												/*! debug print version. */
//#define WINV

#ifdef WINV
  #include <stdio.h>
  #include <string.h>
  #include "tmdshell.h" 								// uses neils emulator
#endif

#ifndef WINV
  #include "test_engine.h"                  /*! include TE_COMMAND  */ 
#endif

#ifndef	DEBUG
	//#define	dprintf(...)
#else
	//#define 
#endif


#define MAX_TERMINAL_WIDTH			78	// maximum chars per terminal line, fixed -could inprove with vary width
#define BOARDHEIGHT 50
#define BOARDWIDTH  80 

#define WIDTH       80
#define HEIGHT      50

#define TOTALMISSILECOUNT 4
#define TOTALENEMY        2

#define ENEMYMINION       1
#define ENEMYMFAST        2
#define ENEMYMSLOW        3
#define ENEMYMBOSS        4

#define CRAFT_ONE					0

#define MISSILE_TYPE_ONE	0


#define TOPBOUNDARY       10
#define BOTTOMBOUNDARY    (BOARDHEIGHT - 10)
#define RIGHTBOUNDARY     BOARDWIDTH
#define LEFTBOUNDARY      0

/*!
	\brief required by tmdshell.c, to set up the stb emulator.
*/
#ifdef WINV
  APPSTARTUPINFO appstartupinfo =
  {
    WIDTH,										//Client Width Suggested Size
    HEIGHT,										//Client Height Suggestrd Size
    60,										//Preferred sync rate
    ASI_MUSTMATCHMINSIZE	//see defines section for defs (ASI_)
  };
#endif

/*!
******************************************************************************
**
** \fn int32 init_game(void)
**
** \brief adds the test to the test engine cmd table. run when Nor driver is initialised ie in BLI_NOR_INIT()
** \return True if tests registerd successfully, else false.
**
*****************************************************************************/
int32 init_game(void);


int32 appmain(int argc, char **argv);

#endif /* __APP_H__ */
