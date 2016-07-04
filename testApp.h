<<<<<<< HEAD
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
#ifndef __TESTAPP_H__
#define __TESTAPP_H__
/*! Documentation
 *		Test application Documentation: Feature of this app and how to use it.
 *	
 * 	Features...
 *		Register tests.
 *		run tests.
 *		list available tests.
 *		test history.
 * 		Help for each test - to use help, type help then the name of the function you want help with 
 * 
 * 	Tests are made from COMMAND structs, these are preloaded to the box. 
 * 	Tests need to be registered by the user before use, 
 * 	To register a 'category', or driver, at the cli type 'register <NAME_OF_CATEGORY>', eg 'register nand'
 * 	To run a test, type the name of the test and arguments, 'nand_soak_test 1 2 3'. do not add extra spaces between arguments.
 * 	To see a list of registered tests, type 'print_list <TEST_CATEGORY>', eg 'print_list nand'.
 * 	For help on a test, type 'help <TEST_NAME>', eg 'help nand'.
 * 	Cursor up and down go through the history for user commands run so far.
 * 	Editing text you can use; Tab complete, insert mode using L/R cursor keys, backspace.
 * 	Commands cannot not go over one line.
 * 
 * 
 * 		Command struct rules:
 * 			test names cannot have spaces in them.
 * 			must use lower case char for test_engine functions
 * 
 * 			do not use double spaces anwhere
 * 			size of screen...
 *
 *	To do 
 *		only if no cursor right been used! check the split line command
 *	 
 *	
 *	Below are examples of the command structs
 *	
 *		typedef struct {
 *			char cmd_category[MAX_CATEGORY_NAME_LEN];	
 *			char cmd_name[MAX_COMMAND_NAME_LEN];			
 *			char cmd_help[MAX_HELP_TEXT_SIZE];				
 *			int32 min_param_cont;											
 *			int32 max_param_count;										
 *			CMDFUNC cmdfunc;													
 *		}
 *		COMMAND;
 *	
 *		static COMMAND dummy_test = {
 *			"Dummy Test Category",
 *			"test",
 *			"Help on test... Perform a dummy soak test of the nand device\n" "usage ... nandread [partition]",
 *			1,
 *			25,
 *			&dummytest,
 *		};
 *	
 *	or create an array of these
 *	
 *		static COMMAND nand_test_funcs[] = {
 *			{"Nand Test Category",
 *					"nand_soaktest",
 *					"Perform a soak test of the nand device\n"  "usage ... nandread [partition]",
 *					1,
 *					3,
 *				&do_nand_soaktest}
 *			,
 *			{"Nand Test Category",
 *					"nand_endurancetest",
 *					"Hammer one nand block until it fails\n",
 *					1,
 *					5,
 *				&do_nand_endurancetest}
 *		};
 *	
 *	Command struct rules:
 *		test names cannot have spaces in them.
 *	
 *	
 *		do not use double spaces anwhere
 *	
 *	to register a category of test type, 'register nand'
 *	
 *		then by running register_command(COMMAND *cmd, int32 count) where;
 *		 * \param [in] cmd  	: COMMAND struct to be copied into memory.
 *		 * \param [in] count 	: amount of commands to add into the table.
 *	
 *		   Example use; 
 *			 register_command(&dummy_test, 1);
 *			 register_command(&nand_test_funcs[0], 2);  for array of 2 commands structs.
 *	
 *	
 *	size of screen...
 *	win or linux -- split_commandline() is #if WINAPP within tdshell.c
 *	CPUID (Processor must be at least a Pentium 1)
 *	
 *		
 *		CLI...
 *		history of run commands (if you edit an old command it will be stored as a new command at the end of the list.)
 *		text edit functions...
 *    	tab complete. (half done)
 *			insert mode (half complete) insert mode will update the history so that it is overwritten.
 *		
 *    bugs;
 *    if buffer full. returns -put read line into a function.
 *    if 2 consecutive space's then args mess up, look into 'command_line_split()'
 *    delete key does not work.
 *    occasionally get funny characters added.
 *    cannot go over one line of text.
 *    terminal width restrict. 78 - find length of screen, miniio.c, void get_console_size(int32* xs, int32* ys).
 *		
 * 		Todo
 *			
 *
 *
 *
 *	
 *		// c (or key) = ascii code eg a= 0x61, scan or s = 41
 *		// when shift is pressed key and scan are the same.
 *		// scan codes: up/dwn/left/right, F-key's  key = 0 scan a number
 *		//key=scan code; b-space = 08, space =20, escape = 1b
 *	
 */

#define WINV														/*! windows version. */
//#define DEBUG														/*! debug print version. */

#ifndef WINV														/*! set top box version. */
	#include "plibc.h"
#endif

#ifdef	WINV														/*! windows version. */
	#include <stdio.h>
	#include <string.h>
	#include "tmdshell.h" 								// uses neils emulator
#endif

#ifndef	DEBUG
//#define	dprintf(...)
#endif

//#include "test_engine.h"


//#define MAX_TERMINAL_WIDTH			78	// maximum chars per terminal line, fixed -could inprove with vary width


/*!
	\brief required by tmdshell.c, to set up the stb emulator.
*/
#ifdef	WINV	
APPSTARTUPINFO appstartupinfo =
{
	80,										//Client Width Suggested Size
	50,										//Client Height Suggestrd Size
	60,										//Preferred sync rate
	ASI_MUSTMATCHMINSIZE	//see defines section for defs (ASI_)
};
#endif

#endif /* __TESTAPP_H__ */
=======
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
#ifndef __TESTAPP_H__
#define __TESTAPP_H__
/*! Documentation
 *		Test application Documentation: Feature of this app and how to use it.
 *	
 * 	Features...
 *		Register tests.
 *		run tests.
 *		list available tests.
 *		test history.
 * 		Help for each test - to use help, type help then the name of the function you want help with 
 * 
 * 	Tests are made from COMMAND structs, these are preloaded to the box. 
 * 	Tests need to be registered by the user before use, 
 * 	To register a 'category', or driver, at the cli type 'register <NAME_OF_CATEGORY>', eg 'register nand'
 * 	To run a test, type the name of the test and arguments, 'nand_soak_test 1 2 3'. do not add extra spaces between arguments.
 * 	To see a list of registered tests, type 'print_list <TEST_CATEGORY>', eg 'print_list nand'.
 * 	For help on a test, type 'help <TEST_NAME>', eg 'help nand'.
 * 	Cursor up and down go through the history for user commands run so far.
 * 	Editing text you can use; Tab complete, insert mode using L/R cursor keys, backspace.
 * 	Commands cannot not go over one line.
 * 
 * 
 * 		Command struct rules:
 * 			test names cannot have spaces in them.
 * 			must use lower case char for test_engine functions
 * 
 * 			do not use double spaces anwhere
 * 			size of screen...
 *
 *	To do 
 *		only if no cursor right been used! check the split line command
 *	 
 *	
 *	Below are examples of the command structs
 *	
 *		typedef struct {
 *			char cmd_category[MAX_CATEGORY_NAME_LEN];	
 *			char cmd_name[MAX_COMMAND_NAME_LEN];			
 *			char cmd_help[MAX_HELP_TEXT_SIZE];				
 *			int32 min_param_cont;											
 *			int32 max_param_count;										
 *			CMDFUNC cmdfunc;													
 *		}
 *		COMMAND;
 *	
 *		static COMMAND dummy_test = {
 *			"Dummy Test Category",
 *			"test",
 *			"Help on test... Perform a dummy soak test of the nand device\n" "usage ... nandread [partition]",
 *			1,
 *			25,
 *			&dummytest,
 *		};
 *	
 *	or create an array of these
 *	
 *		static COMMAND nand_test_funcs[] = {
 *			{"Nand Test Category",
 *					"nand_soaktest",
 *					"Perform a soak test of the nand device\n"  "usage ... nandread [partition]",
 *					1,
 *					3,
 *				&do_nand_soaktest}
 *			,
 *			{"Nand Test Category",
 *					"nand_endurancetest",
 *					"Hammer one nand block until it fails\n",
 *					1,
 *					5,
 *				&do_nand_endurancetest}
 *		};
 *	
 *	Command struct rules:
 *		test names cannot have spaces in them.
 *	
 *	
 *		do not use double spaces anwhere
 *	
 *	to register a category of test type, 'register nand'
 *	
 *		then by running register_command(COMMAND *cmd, int32 count) where;
 *		 * \param [in] cmd  	: COMMAND struct to be copied into memory.
 *		 * \param [in] count 	: amount of commands to add into the table.
 *	
 *		   Example use; 
 *			 register_command(&dummy_test, 1);
 *			 register_command(&nand_test_funcs[0], 2);  for array of 2 commands structs.
 *	
 *	
 *	size of screen...
 *	win or linux -- split_commandline() is #if WINAPP within tdshell.c
 *	CPUID (Processor must be at least a Pentium 1)
 *	
 *		
 *		CLI...
 *		history of run commands (if you edit an old command it will be stored as a new command at the end of the list.)
 *		text edit functions...
 *    	tab complete. (half done)
 *			insert mode (half complete) insert mode will update the history so that it is overwritten.
 *		
 *    bugs;
 *    if buffer full. returns -put read line into a function.
 *    if 2 consecutive space's then args mess up, look into 'command_line_split()'
 *    delete key does not work.
 *    occasionally get funny characters added.
 *    cannot go over one line of text.
 *    terminal width restrict. 78 - find length of screen, miniio.c, void get_console_size(int32* xs, int32* ys).
 *		
 * 		Todo
 *			
 *
 *
 *
 *	
 *		// c (or key) = ascii code eg a= 0x61, scan or s = 41
 *		// when shift is pressed key and scan are the same.
 *		// scan codes: up/dwn/left/right, F-key's  key = 0 scan a number
 *		//key=scan code; b-space = 08, space =20, escape = 1b
 *	
 */

#define WINV														/*! windows version. */
//#define DEBUG														/*! debug print version. */

#ifndef WINV														/*! set top box version. */
	#include "plibc.h"
#endif

#ifdef	WINV														/*! windows version. */
	#include <stdio.h>
	#include <string.h>
	#include "tmdshell.h" 								// uses neils emulator
#endif

#ifndef	DEBUG
//#define	dprintf(...)
#endif

//#include "test_engine.h"


//#define MAX_TERMINAL_WIDTH			78	// maximum chars per terminal line, fixed -could inprove with vary width


/*!
	\brief required by tmdshell.c, to set up the stb emulator.
*/
#ifdef	WINV	
APPSTARTUPINFO appstartupinfo =
{
	80,										//Client Width Suggested Size
	50,										//Client Height Suggestrd Size
	60,										//Preferred sync rate
	ASI_MUSTMATCHMINSIZE	//see defines section for defs (ASI_)
};
#endif

#endif /* __TESTAPP_H__ */
>>>>>>> 000639d155741e22ec15a38c535a1cce49fe0792
