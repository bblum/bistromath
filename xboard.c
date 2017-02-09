/****************************************************************************
 * xboard.c - interface between engine.c and xboard's communication protocol
 * copyright (C) 2008 Ben Blum
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "engine.h"
#include "util/linkedlist_u32.h"

void get_cmd();
void exit_error(char *);
void output(char *);
/* command-handling routines */
void cmd_new();
void makemove();
int usermove(char *str);
int input_ismove(char *str);
void checkgameover();

#define BUF_SIZE 2048
char inbuf[BUF_SIZE];
char outbuf[BUF_SIZE];

FILE *ttyout;
#ifndef TTYOUT_COLOR
#define TTYOUT_COLOR "\033[00;37m"
#endif
#ifndef DEFAULT_COLOR
#define DEFAULT_COLOR "\033[00m"
#endif
#ifndef ASSERT_COLOR
#define ASSERT_COLOR "\033[01;31m"
#endif

#ifndef SOURCE_URL
#define SOURCE_URL "https://github.com/bblum/bistromath"
#endif

#define OWNER_NAME "bblum"

/* the name of our opponent; used so random users can't set debug mode unless they're playing */
char opponent[BUF_SIZE];
/* flag telling whether we're in debug mode or not*/
unsigned char debug;

engine_t *e = NULL;
/* Used for xboard's "force" mode, when examining or resuming adjourned */
unsigned char force_mode;

int main()
{
	int protover = -1;
	force_mode = 0;
	debug = 0;
	opponent[0] = '\0';

	/* initial setup */
	do
	{
		ttyout = fopen("/dev/tty", "w");
		
		setbuf(stdout, NULL);
		
		/* input: "xboard" */
		get_cmd();
		if (strcmp(inbuf, "xboard"))
		{
			exit_error("Must run in xboard mode.");
		}
		
		/* input: "protover N" */
		get_cmd();
		if (1 != sscanf(inbuf, "protover %d", &protover))
		{
			exit_error("What happened to protover?");
		}
		else if (protover < 2)
		{
			exit_error("Protover must be >= 2");
		}
		
		/* output: "feature [...]" */
		fprintf(ttyout, "%sNow giving feature command...%s",
		        TTYOUT_COLOR, DEFAULT_COLOR);
		printf("feature sigint=0 myname=\"%s\" ping=1 done=1\n", ENGINE_NAME);
		fprintf(ttyout, "%sDone.%s\n", TTYOUT_COLOR, DEFAULT_COLOR);
	} while (0);

	/* main getcommand/parse/execute loop */
	while (1)
	{
		get_cmd();
		/* interpret the commands received */
		if (0 == strcmp(inbuf, "new"))
		{
			cmd_new();
			force_mode = 0;
			debug = 0;
		}
		/* force-mode */
		else if (0 == strcmp(inbuf, "force"))
		{
			force_mode = 1;
		}
		/* the ping command, which avoids race conditions */
		else if (0 == strncmp(inbuf, "ping", 4))
		{
			int n;
			sscanf(inbuf, "ping %d", &n);
			printf("pong %d\n", n);
		}
		/* setting the time controls */
		else if (0 == strncmp(inbuf, "level", 5))
		{
			/* mps is only for the sscanf argument, its value is
			 * ignored. time in mins, inc = increment in secs*/
			unsigned int mps = 0, time = 0, inc = 0;
			sscanf(inbuf, "level %u %u %u", &mps, &time, &inc);
			/* set the values in the engine */
			e->time_remaining = time * 60;
			e->time_increment = inc;
		}
		else if (0 == strncmp(inbuf, "time", 4))
		{
			unsigned int time;
			sscanf(inbuf, "time %u", &time);
			/* xboard gives us centiseconds */
			e->time_remaining = time / 100;
		}
		/* commands for making moves */
		else if (0 == strcmp(inbuf, "go"))
		{
			force_mode = 0;
			makemove();
			checkgameover();
		}
		else if (input_ismove(inbuf))
		{
			if (usermove(inbuf))
			{
				checkgameover();
				if (!force_mode)
				{
					makemove();
					checkgameover();
				}
			}
			else
			{
				char *fen;
				printf("Illegal move: %s\n", inbuf);
				fen = board_fen(e->board);
				snprintf(outbuf, BUF_SIZE-1, "Illegal move %s. I think the board is %s; white's pieces 0x%.16lx, black's 0x%.16lx",
				        inbuf, fen, e->board->piecesofcolor[WHITE], e->board->piecesofcolor[BLACK]);
				output(outbuf);
				free(fen);
			}
		}
		/* the name of our opponent */
		else if (0 == strncmp(inbuf, "name", 4))
		{
			sscanf(inbuf, "name %s", opponent);
		}
		/* ics user talking to us */
		else if (0 == strncmp(inbuf, "zippy", 5))
		{
			char username[BUF_SIZE];
			char usercmd[BUF_SIZE];
			sscanf(inbuf, "zippy %s %s", username, usercmd);
			if (0 == strcmp(usercmd, "source"))
			{
				printf("tellics t %s %s\n", username, SOURCE_URL);
			}
			else if (0 == strcmp(usercmd, "debug"))
			{
				/* verify that our opponent sent the command */
				if (0 == strcmp(username, opponent))
				{
					if (debug)
					{
						debug = 0;
						printf("tellics t %s Debug mode disabled.\n", username);
					}
					else
					{
						debug = 1;
						printf("tellics t %s Debug mode enabled. Use the debug command again to turn it off.\n", username);
					}
				}
				else
				{
					printf("tellics t %s Sorry, you can only use the debug command when playing against me.\n", username);
				}
			}
			else if (0 == strcmp(usercmd, "bug"))
			{
				printf("tellics t %s If you find a bug, message %s with a description of what happened. If you want to be really helpful, do as many as you can of the following: 1) Tell the position at which the bug happened (FEN works). 2) Tell what sort of bug appeared - did I make a terrible blunder? did I stop moving and let my time run out? 3) Save the game in your journal (and tell what slot it's in) so it can be examined even after it purges out of the history. 4) Turn debug mode on (with the debug command) and try to find the bug again - perhaps the debug output can be helpful.\n", username, OWNER_NAME);
			}
			else if (0 == strcmp(usercmd, "help"))
			{
				printf("tellics t %s Commands I understand:\n", username);
				printf("tellics t %s      source - get a URL for my source code\n", username);
				printf("tellics t %s      debug  - turn debug mode on/off\n", username);
				printf("tellics t %s      bug    - what to do if you find a bug\n", username);
				printf("tellics t %s      help   - display this help\n", username);
			}
			else
			{
				printf("tellics t %s I don't understand the command \"%s\". Try \"help\".\n", username, usercmd);
			}
		}
		else if (0 == strcmp(inbuf, "quit"))
		{
			break;
		}
	}
	
	engine_destroy(e);
	fclose(ttyout);
	return 0;
}


void get_cmd()
{
	if (fgets(inbuf, BUF_SIZE-1, stdin)) {
		if (inbuf[strlen(inbuf)-1] == '\n')
		{
			inbuf[strlen(inbuf)-1] = '\0';
		}
		snprintf(outbuf, BUF_SIZE-1, "STDIN: %s", inbuf);
		output(outbuf);
	} else {
		inbuf[0] = 0;
		snprintf(outbuf, BUF_SIZE-1, "STDIN: EOF - use 'quit' to exit!");
		output(outbuf);
	}
}

/**
 * Fuck, something bad happened. Bail out.
 */
void exit_error(char *msg)
{
	snprintf(outbuf, BUF_SIZE-1, "ERROR: %s", msg);
	output(outbuf);
	exit(1);
}

/**
 * Print something directly to the console, avoiding xboard's pipe
 * or tell it to ICS if debug mode is on
 */
void output(char *msg)
{
	if (debug)
	{
		printf("tellics k %s\n", msg);
		fflush(stdout);
	}
	else
	{
		fprintf(ttyout, "%s%s%s\n", TTYOUT_COLOR, msg,
		        DEFAULT_COLOR);
		fflush(ttyout);
	}
	return;
}

void printmoves()
{
	movelist_t list;
	
	/* move list */
	board_generatemoves(e->board, &list);
	snprintf(outbuf, BUF_SIZE-1, "ENGINE: Moves possible: ");
	while (!movelist_isempty(&list))
	{
		move_t move = movelist_remove_max(&list);
		char *str = move_tostring(move);
		strcat(outbuf, str);
		strcat(outbuf, " ");
		free(str);
	}
	output(outbuf);
	movelist_destroy(&list);
	
	/* capture list*/
	board_generatecaptures(e->board, &list);
	snprintf(outbuf, BUF_SIZE-1, "ENGINE: Captures possible: ");
	while (!movelist_isempty(&list))
	{
		move_t move = movelist_remove_max(&list);
		char *str = move_tostring(move);
		strcat(outbuf, str);
		strcat(outbuf, " ");
		free(str);
	}
	output(outbuf);
	movelist_destroy(&list);
	
	return;
}

/**
 * Interpret the "new" command, starting a new engine with a new board
 */
void cmd_new()
{
	engine_destroy(e);
	e = engine_init(getbestmove);
	return;
}

/**
 * Have the engine make a move
 */
void makemove()
{
	char *str;
	char *fen = board_fen(e->board);
	
	printmoves();
	
	/* move is applied here */
	str = engine_generatemove(e);
	snprintf(outbuf, BUF_SIZE-1, "ENGINE: Current board state is %s...",
	         fen);
	output(outbuf);
	snprintf(outbuf, BUF_SIZE-1, "ENGINE: My move is %s, returnval %d",
	         str, engine_applymove(e, str));
	output(outbuf);
	free(fen);
	
	printf("move %s\n", str);
	free(str);
	
	fen = board_fen(e->board);
	output(fen);
	free(fen);
	return;
}

/**
 * The user made a move. Returns 1 if the move was legal, and applied to the
 * board; 0 if illegal and not applied.
 */
int usermove(char *str)
{
	return engine_applymove(e, str);
}

/**
 * Check if the input represents a move the user is making. 1 if yes, 0 if no.
 */
int input_ismove(char *str)
{
	return !!(move_fromstring(str));
}

void checkgameover()
{
	char *result;
	int type;
	if ((type = engine_checkgameover(e, &result)) != 0)
	{
		snprintf(outbuf, BUF_SIZE-1,"ENGINE: Game over: %s type %d",
		         result, type);
		output(outbuf);
		if (type == 2)
		{
			printf("offer draw\n");
		}
		printf("%s\n", result);
		//engine_destroy(e);
		//e = NULL;
	}
	return;
}

#include <unistd.h>
#include <signal.h>

void assert_fail(const char *expr, const char *file, int line, const char *func)
{
	sigset_t fuck;

	printf("tellics say You found a bug at %s:%d in %s! "
	       "Failed assertion: \"%s\"\n"
	       "tellics say Please save this game into your journal or paste "
	       "the moves that led to this position to http://pastebin.com "
	       "and message %s to let them know what happened. Thanks!\n"
	       "tellics resign\ntellics message bistromath %s:%d:%s: \"%s\"\n"
	       "tellics quit\n", file, line, func, expr, OWNER_NAME, file, line,
	       func, expr);
	snprintf(outbuf, BUF_SIZE-1,
	         "%s%s:%d: %s: Assertion \"%s\" failed!", ASSERT_COLOR, file,
	         line, func, expr);
	output(outbuf);
	sigemptyset(&fuck);
	sigaddset(&fuck, SIGUSR1);
	while (1)
		sigsuspend(&fuck);
}
