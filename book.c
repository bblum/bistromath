/****************************************************************************
 * book.c - simple interface for retrieving moves from a plaintext book
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
 * ****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "book.h"
#include "rand.h"
#include "util/linkedlist_u32.h"

#define BOOK_FILENAME "book.txt"
#define BOOK_MOVE_LENGTH 4

FILE *book = NULL;
unsigned char book_present;

/* for error printing */
extern FILE *ttyout;
void output(char *);

static void book_open()
{
	book = fopen(BOOK_FILENAME, "r");
	if (!book)
	{
		output("BOOK: No opening book found!");
		book_present = 0;
		return;
	}
	book_present = 1;
	return;
}

static void book_close()
{
	if (book_present)
	{
		fclose(book);
		book_present = 0;
	}
	return;
}

/**
 * Get a move from the book. Line is a string like "e2e4 e7e5 g1f3 " - note
 * the space at the end! and the board is for passing to move_islegal.
 */
move_t book_move(char *line, board_t *board)
{
	char book_line[BOOK_LINE_MAX_LENGTH];
	char book_move_str[BOOK_MOVE_LENGTH+1];
	move_t book_move;
	unsigned long rand_index;

	char outbuf[BOOK_LINE_MAX_LENGTH];

	linkedlist_u32_t *movelist;

	/* always favor the sicilian if available - sharp tactical positions
	 * are best for bots, so try to avoid closed openings */
	if (0 == strcmp(line, ""))
	{
		return move_islegal(board, "e2e4");
	}
	else if (0 == strcmp(line, "e2e4 "))
	{
		return move_islegal(board, "c7c5");
	}
	
	book_open();

	if (!book_present)
	{
		output("BOOK: No book present...");
		return 0;
	}
	
	snprintf(outbuf, BOOK_LINE_MAX_LENGTH-1, "BOOK: Current line: \"%s\"", line);
	output(outbuf);
	
	movelist = ll_u32_init();

	/* read lines out of the book */
	while (fgets(book_line, BOOK_LINE_MAX_LENGTH-1, book))
	{
		/* see if our line matches the book line so far */
		if (0 == strncmp(line, book_line, strlen(line)))
		{
			/* copy the move string out of the book line */
			strncpy(book_move_str, &(book_line[strlen(line)]), BOOK_MOVE_LENGTH);
			book_move_str[BOOK_MOVE_LENGTH] = '\0';
			
			/* convert to move_t */
			if ((book_move = move_islegal(board, book_move_str)))
			{
				ll_u32_add(movelist, book_move);
			}
		}
	}
	book_close();
	/* check if no moves were found */
	if (!movelist->count)
	{
		ll_u32_destroy(movelist);
		return 0;
	}
	/* now we have a linkedlist of book moves - gen a random number, pull
	 * the move at that index out, and kill the rest of the list */
	rand_index = rand64() % movelist->count;
	book_move = ll_u32_get(movelist, rand_index);
	sprintf(outbuf, "BOOK: Chose move %lu out of %lu total\n", rand_index, movelist->count);
	output(outbuf);
	ll_u32_destroy(movelist);
	return book_move;
}
