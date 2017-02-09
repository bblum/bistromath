/****************************************************************************
 * board.c - rotated bitboard library; move generation and board state
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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "assert.h"
#include "board.h"
#include "attacks.h"
#include "bitscan.h"
#include "rand.h"

/* For FEN conversion, and move->string conversion */
char *piecename[2][6] = {
	{ "P", "N", "B", "R", "Q", "K" },
	{ "p", "n", "b", "r", "q", "k" }
};
/* Converts int to string. Better than using malloc for 3-length strings. */
char *squarename[64] = {
	"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
	"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
	"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
	"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
	"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
	"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
	"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
	"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};
#ifndef BB_SHIFTFLIP /* see board.h */
/* Get a bitboard with only one bit set, on a specified square */
bitboard_t bb_square[64] = {
	BB(0x0000000000000001), BB(0x0000000000000002), BB(0x0000000000000004), BB(0x0000000000000008),
	BB(0x0000000000000010), BB(0x0000000000000020), BB(0x0000000000000040), BB(0x0000000000000080),
	BB(0x0000000000000100), BB(0x0000000000000200), BB(0x0000000000000400), BB(0x0000000000000800),
	BB(0x0000000000001000), BB(0x0000000000002000), BB(0x0000000000004000), BB(0x0000000000008000),
	BB(0x0000000000010000), BB(0x0000000000020000), BB(0x0000000000040000), BB(0x0000000000080000),
	BB(0x0000000000100000), BB(0x0000000000200000), BB(0x0000000000400000), BB(0x0000000000800000),
	BB(0x0000000001000000), BB(0x0000000002000000), BB(0x0000000004000000), BB(0x0000000008000000),
	BB(0x0000000010000000), BB(0x0000000020000000), BB(0x0000000040000000), BB(0x0000000080000000),
	BB(0x0000000100000000), BB(0x0000000200000000), BB(0x0000000400000000), BB(0x0000000800000000),
	BB(0x0000001000000000), BB(0x0000002000000000), BB(0x0000004000000000), BB(0x0000008000000000),
	BB(0x0000010000000000), BB(0x0000020000000000), BB(0x0000040000000000), BB(0x0000080000000000),
	BB(0x0000100000000000), BB(0x0000200000000000), BB(0x0000400000000000), BB(0x0000800000000000),
	BB(0x0001000000000000), BB(0x0002000000000000), BB(0x0004000000000000), BB(0x0008000000000000),
	BB(0x0010000000000000), BB(0x0020000000000000), BB(0x0040000000000000), BB(0x0080000000000000),
	BB(0x0100000000000000), BB(0x0200000000000000), BB(0x0400000000000000), BB(0x0800000000000000),
	BB(0x1000000000000000), BB(0x2000000000000000), BB(0x4000000000000000), BB(0x8000000000000000),
};
/* The bitwise negation of SINGLESQUARE(x) */
bitboard_t bb_allexcept[64] = {
	BB(0xfffffffffffffffe), BB(0xfffffffffffffffd), BB(0xfffffffffffffffb), BB(0xfffffffffffffff7),
	BB(0xffffffffffffffef), BB(0xffffffffffffffdf), BB(0xffffffffffffffbf), BB(0xffffffffffffff7f),
	BB(0xfffffffffffffeff), BB(0xfffffffffffffdff), BB(0xfffffffffffffbff), BB(0xfffffffffffff7ff),
	BB(0xffffffffffffefff), BB(0xffffffffffffdfff), BB(0xffffffffffffbfff), BB(0xffffffffffff7fff),
	BB(0xfffffffffffeffff), BB(0xfffffffffffdffff), BB(0xfffffffffffbffff), BB(0xfffffffffff7ffff),
	BB(0xffffffffffefffff), BB(0xffffffffffdfffff), BB(0xffffffffffbfffff), BB(0xffffffffff7fffff),
	BB(0xfffffffffeffffff), BB(0xfffffffffdffffff), BB(0xfffffffffbffffff), BB(0xfffffffff7ffffff),
	BB(0xffffffffefffffff), BB(0xffffffffdfffffff), BB(0xffffffffbfffffff), BB(0xffffffff7fffffff),
	BB(0xfffffffeffffffff), BB(0xfffffffdffffffff), BB(0xfffffffbffffffff), BB(0xfffffff7ffffffff),
	BB(0xffffffefffffffff), BB(0xffffffdfffffffff), BB(0xffffffbfffffffff), BB(0xffffff7fffffffff),
	BB(0xfffffeffffffffff), BB(0xfffffdffffffffff), BB(0xfffffbffffffffff), BB(0xfffff7ffffffffff),
	BB(0xffffefffffffffff), BB(0xffffdfffffffffff), BB(0xffffbfffffffffff), BB(0xffff7fffffffffff),
	BB(0xfffeffffffffffff), BB(0xfffdffffffffffff), BB(0xfffbffffffffffff), BB(0xfff7ffffffffffff),
	BB(0xffefffffffffffff), BB(0xffdfffffffffffff), BB(0xffbfffffffffffff), BB(0xff7fffffffffffff),
	BB(0xfeffffffffffffff), BB(0xfdffffffffffffff), BB(0xfbffffffffffffff), BB(0xf7ffffffffffffff),
	BB(0xefffffffffffffff), BB(0xdfffffffffffffff), BB(0xbfffffffffffffff), BB(0x7fffffffffffffff),
};
#endif

/* Which squares must be unoccupied for COLOR (1st) to castle on SIDE (2nd) */
static bitboard_t castle_clearsquares[2][2] = {
	/*   queenside               kingside */
	{ BB(0x000000000000000e), BB(0x0000000000000060) }, /* white */
	{ BB(0x0e00000000000000), BB(0x6000000000000000) }  /* black */
};
/* Which squares must be unthreatened for COLOR to castle on SIDE */
static bitboard_t castle_safesquares[2][2] = {
	/*   queenside               kingside */
	{ BB(0x000000000000001c), BB(0x0000000000000070) }, /* white */
	{ BB(0x1c00000000000000), BB(0x7000000000000000) }  /* black */
};
/* A mask for the cols adjacent to a given col - for pawn captures, AND with a
 * row-mask for the rank you need the pawns to be on */
bitboard_t bb_adjacentcols[8] = {
	           BB_FILEB,
	BB_FILEA | BB_FILEC,
	BB_FILEB | BB_FILED,
	BB_FILEC | BB_FILEE,
	BB_FILED | BB_FILEF,
	BB_FILEE | BB_FILEG,
	BB_FILEF | BB_FILEH,
	BB_FILEG
};
/* used for determining when a pawn is passed */
bitboard_t bb_passedpawnmask[2][64] = {
	/* WHITE */
	{
		BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000),
		BB(0x0303030303030000), BB(0x0707070707070000), BB(0x0e0e0e0e0e0e0000), BB(0x1c1c1c1c1c1c0000), BB(0x3838383838380000), BB(0x7070707070700000), BB(0xe0e0e0e0e0e00000), BB(0xc0c0c0c0c0c00000),
		BB(0x0303030303000000), BB(0x0707070707000000), BB(0x0e0e0e0e0e000000), BB(0x1c1c1c1c1c000000), BB(0x3838383838000000), BB(0x7070707070000000), BB(0xe0e0e0e0e0000000), BB(0xc0c0c0c0c0000000),
		BB(0x0303030300000000), BB(0x0707070700000000), BB(0x0e0e0e0e00000000), BB(0x1c1c1c1c00000000), BB(0x3838383800000000), BB(0x7070707000000000), BB(0xe0e0e0e000000000), BB(0xc0c0c0c000000000),
		BB(0x0303030000000000), BB(0x0707070000000000), BB(0x0e0e0e0000000000), BB(0x1c1c1c0000000000), BB(0x3838380000000000), BB(0x7070700000000000), BB(0xe0e0e00000000000), BB(0xc0c0c00000000000),
		BB(0x0303000000000000), BB(0x0707000000000000), BB(0x0e0e000000000000), BB(0x1c1c000000000000), BB(0x3838000000000000), BB(0x7070000000000000), BB(0xe0e0000000000000), BB(0xc0c0000000000000),
		BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000),
		BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000)
	},
	/* BLACK */
	{
		BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000),
		BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000),
		BB(0x0000000000000303), BB(0x0000000000000707), BB(0x0000000000000e0e), BB(0x0000000000001c1c), BB(0x0000000000003838), BB(0x0000000000007070), BB(0x000000000000e0e0), BB(0x000000000000c0c0),
		BB(0x0000000000030303), BB(0x0000000000070707), BB(0x00000000000e0e0e), BB(0x00000000001c1c1c), BB(0x0000000000383838), BB(0x0000000000707070), BB(0x0000000000e0e0e0), BB(0x0000000000c0c0c0),
		BB(0x0000000003030303), BB(0x0000000007070707), BB(0x000000000e0e0e0e), BB(0x000000001c1c1c1c), BB(0x0000000038383838), BB(0x0000000070707070), BB(0x00000000e0e0e0e0), BB(0x00000000c0c0c0c0),
		BB(0x0000000303030303), BB(0x0000000707070707), BB(0x0000000e0e0e0e0e), BB(0x0000001c1c1c1c1c), BB(0x0000003838383838), BB(0x0000007070707070), BB(0x000000e0e0e0e0e0), BB(0x000000c0c0c0c0c0),
		BB(0x0000030303030303), BB(0x0000070707070707), BB(0x00000e0e0e0e0e0e), BB(0x00001c1c1c1c1c1c), BB(0x0000383838383838), BB(0x0000707070707070), BB(0x0000e0e0e0e0e0e0), BB(0x0000c0c0c0c0c0c0),
		BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000), BB(0x0000000000000000)
	}
};

/* Rotation translation from normal-oriented square indices to square indices
 * suitable for setting bits on board_t->occupied{90,45,315}. Note that we
 * don't use these for move generation at all, just for setting/clearing bits
 * on the occupied boards. */
#define ROT90SQUAREINDEX(s) rot90squareindex[s] /*(SQUARE(ROW(s),COL(s))) - slower*/
static int rot90squareindex[64] = {
	0,  8, 16, 24, 32, 40, 48, 56,
	1,  9, 17, 25, 33, 41, 49, 57,
	2, 10, 18, 26, 34, 42, 50, 58,
	3, 11, 19, 27, 35, 43, 51, 59,
	4, 12, 20, 28, 36, 44, 52, 60,
	5, 13, 21, 29, 37, 45, 53, 61,
	6, 14, 22, 30, 38, 46, 54, 62,
	7, 15, 23, 31, 39, 47, 55, 63
};
#define ROT45SQUAREINDEX(s) rot45squareindex[s]
static int rot45squareindex[64] = {
	 0,  2,  5,  9, 14, 20, 27, 35,
	 1,  4,  8, 13, 19, 26, 34, 42,
	 3,  7, 12, 18, 25, 33, 41, 48,
	 6, 11, 17, 24, 32, 40, 47, 53,
	10, 16, 23, 31, 39, 46, 52, 57,
	15, 22, 30, 38, 45, 51, 56, 60,
	21, 29, 37, 44, 50, 55, 59, 62,
	28, 36, 43, 49, 54, 58, 61, 63
};
#define ROT315SQUAREINDEX(s) rot315squareindex[s]
static int rot315squareindex[64] = {
	28, 21, 15, 10,  6,  3,  1,  0,
	36, 29, 22, 16, 11,  7,  4,  2,
	43, 37, 30, 23, 17, 12,  8,  5,
	49, 44, 38, 31, 24, 18, 13,  9,
	54, 50, 45, 39, 32, 25, 19, 14,
	58, 55, 51, 46, 40, 33, 26, 20,
	61, 59, 56, 52, 47, 41, 34, 27,
	63, 62, 60, 57, 53, 48, 42, 35,
};

/* for material count */
extern int16_t eval_piecevalue[6];



#define ZOBRIST_DEFAULT_HASH 0
/* Table of random numbers used to generate the zobrist hash. The indices are
 * first the color (WHITE/BLACK), then the piece type (PAWN-KING), then the
 * square the piece is on (A1-H8) */
zobrist_t zobrist_piece[2][6][64];
/* Whose move is it (WHITE/BLACK)? The value present if white has the move. */
zobrist_t zobrist_tomove;
/* Which square (A1-H8) is the enpassant square? If enpassant is not possible,
 * use the key at index 0. */
zobrist_t zobrist_ep[64];
/* Can (WHITE/BLACK, first) castle on the (KINGSIDE/QUEENSIDE, second)? If so,
 * the value here is xored into the hash. */
zobrist_t zobrist_castle[2][2];
/* Have the arrays been initialized? If you reinitialize them in the middle of
 * play, the keys will get fucked up. */
char zobrist_initialized = 0;

/**
 * Initialize the zobrist tables.
 */
void init_zobrist()
{
	int i, j, k;
	
	/* just in case... */
	if (zobrist_initialized)
	{
		return;
	}
	
	/* Init the piece position array */
	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < 6; j++)
		{
			for (k = 0; k < 64; k++)
			{
				zobrist_piece[i][j][k] = (zobrist_t)rand64();
			}
		}
	}
	
	/* Init the whose-move-is-it value */
	zobrist_tomove = (zobrist_t)rand64();
	
	/* Init the enpassant square array */
	for (i = 0; i < 64; i++)
	{
		zobrist_ep[i] = (zobrist_t)rand64();
	}
	
	/* Init the castling rights array */
	zobrist_castle[0][0] = (zobrist_t)rand64();
	zobrist_castle[0][1] = (zobrist_t)rand64();
	zobrist_castle[1][0] = (zobrist_t)rand64();
	zobrist_castle[1][1] = (zobrist_t)rand64();
	
	zobrist_initialized = 1;
	return;
}

/**
 * (Re)generate the zobrist hash for a board, store it in board->hash
 * This shouldn't be used every time you change the zobrist, only when
 * initializing the board.
 */
void zobrist_gen(board_t *board)
{
	unsigned char color;
	piece_t piece;
	square_t square;

	int i, j;
	bitboard_t pos;
	zobrist_t hash = ZOBRIST_DEFAULT_HASH;
	
	if (board == NULL)
	{
		return;
	}
	if (!zobrist_initialized)
	{
		init_zobrist();
	}
	
	/* XOR in the zobrist for each piece on the board */
	for (color = 0; color < 2; color++)
	{
		for (piece = 0; piece < 6; piece++)
		{
			pos = board->pos[color][piece];
			while (pos)
			{
				/* Find the first bit */
				square = BITSCAN(pos);
				/* throw in the hash */
				hash ^= zobrist_piece[color][piece][square];
				/* and clear the bit */
				pos &= BB_ALLEXCEPT(square);
			}
		}
	}
	/* Have the hash reflect special conditions on the board */
	/* En passant */
	hash ^= zobrist_ep[board->ep];
	/* Castling rights */
	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < 2; j++)
		{
			if (board->castle[i][j])
			{
				hash ^= zobrist_castle[i][j];
			}
		}
	}
	/* Who has the move */
	if (board->tomove == WHITE)
	{
		hash ^= zobrist_tomove;
	}

	board->hash = hash;
	return;
}

/****************************************************************************
 * BITBOARD FUNCTIONS
 ****************************************************************************/
static void board_addmoves_pawn(board_t *, square_t, unsigned char, movelist_t *);
static void board_addmoves_ep(board_t *, unsigned char, movelist_t *);
static void board_addmoves_king(board_t *, square_t, unsigned char, movelist_t *);
static void board_addmoves(board_t *, square_t, piece_t, unsigned char, movelist_t *);

static void board_addcaptures_pawn(board_t *, square_t, unsigned char, movelist_t *);
static void board_addcaptures_king(board_t *, square_t, unsigned char, movelist_t *);
static void board_addcaptures(board_t *, square_t, piece_t, unsigned char, movelist_t *);
static void board_togglepiece(board_t *, square_t, unsigned char, piece_t);
static void board_regeneratethreatened(board_t *);

/**
 * Generate a fresh board with the default initial starting position.
 */
board_t *board_init()
{
	board_t *board = malloc(sizeof(board_t));
	
	board->pos[WHITE][PAWN]   = BB_RANK2;
	board->pos[WHITE][KNIGHT] = BB_SQUARE(B1) | BB_SQUARE(G1);
	board->pos[WHITE][BISHOP] = BB_SQUARE(C1) | BB_SQUARE(F1);
	board->pos[WHITE][ROOK]   = BB_SQUARE(A1) | BB_SQUARE(H1);
	board->pos[WHITE][QUEEN]  = BB_SQUARE(D1);
	board->pos[WHITE][KING]   = BB_SQUARE(E1);
	board->pos[BLACK][PAWN]   = BB_RANK7;
	board->pos[BLACK][KNIGHT] = BB_SQUARE(B8) | BB_SQUARE(G8);
	board->pos[BLACK][BISHOP] = BB_SQUARE(C8) | BB_SQUARE(F8);
	board->pos[BLACK][ROOK]   = BB_SQUARE(A8) | BB_SQUARE(H8);
	board->pos[BLACK][QUEEN]  = BB_SQUARE(D8);
	board->pos[BLACK][KING]   = BB_SQUARE(E8);

	board->piecesofcolor[WHITE] = BB_RANK1 | BB_RANK2;
	board->piecesofcolor[BLACK] = BB_RANK8 | BB_RANK7;
	
	board->attackedby[WHITE] = BB_RANK3 | BB_RANK2 | (BB_RANK1 ^ BB_SQUARE(A1) ^ BB_SQUARE(H1));
	board->attackedby[BLACK] = BB_RANK6 | BB_RANK7 | (BB_RANK8 ^ BB_SQUARE(A8) ^ BB_SQUARE(H8));
	
	board->occupied    = BB_RANK1 | BB_RANK2 | BB_RANK7 | BB_RANK8;
	board->occupied90  = BB_FILEA | BB_FILEB | BB_FILEG | BB_FILEH;
	/* These ugly numbers, in contrast to the beautifully simple code
	 * above, are in fact correct. Check with ./bbtools/rotvisualizer */
	board->occupied45  = BB(0xecc61c3c3c386337);
	board->occupied315 = BB(0xfb31861c38618cdf);
	
	board->ep = 0;
	board->castle[WHITE][QUEENSIDE] = 1;
	board->castle[WHITE][KINGSIDE] = 1;
	board->castle[BLACK][QUEENSIDE] = 1;
	board->castle[BLACK][KINGSIDE] = 1;
	board->hascastled[WHITE] = 0;
	board->hascastled[BLACK] = 0;
	board->tomove = WHITE;
	board->halfmoves = 0;
	board->moves = 0;
	board->reps = 0;

	board->material[WHITE] = (8 * eval_piecevalue[PAWN]) +
	                         (2 * eval_piecevalue[KNIGHT]) +
	                         (2 * eval_piecevalue[BISHOP]) +
				 (2 * eval_piecevalue[ROOK]) +
				 (eval_piecevalue[QUEEN]) +
				 (eval_piecevalue[KING]);
	board->material[BLACK] = board->material[WHITE];
	
	zobrist_gen(board);
	
	return board;
}

/**
 * Free all memory associated with a board_t.
 */
void board_destroy(board_t *board)
{
	if (board)
	{
		free(board);
	}
}

/**
 * Generates a FEN string for the given position. Free it yourself when you're
 * done with it. This function makes no effort to be fast; you shouldn't be
 * using it during the search process anyway.
 */
#define FEN_MAX_LENGTH 85
char *board_fen(board_t *board)
{
	char *fen;
	int row, col;
	char emptysquares[] = "0"; /* counts consecutive empty squares */
	char halfmove[8]; /* Move clock string - "8 45" maybe */
	piece_t piece;
	unsigned char color;
	unsigned char empty_end_of_line;
	
	if (board == NULL)
	{
		return NULL;
	}
	
	fen = malloc(FEN_MAX_LENGTH + 1);
	fen[0] = 0;
	
	for (row = 7; row >= 0; row--)
	{
		emptysquares[0] = '0';
		empty_end_of_line = 0;
		
		for (col = 0; col < 8; col++)
		{
			color = 0; /* really init'ed next line; this makes gcc happy */
			piece = board_pieceatsquare(board, SQUARE(col,row), &color);
			if (piece == (piece_t)-1)
			{
				emptysquares[0]++;
				if (col == 7)
				{
					empty_end_of_line = 1;
				}
			}
			else
			{
				/* print the emptysquare count if it exists */
				if (emptysquares[0] != '0')
				{
					strcat(fen, emptysquares);
					emptysquares[0] = '0';
				}
				strcat(fen, piecename[color][piece]);
			}
		}
		/* we finished printing this row, unless it was empty */
		if (empty_end_of_line)
		{
			strcat(fen, emptysquares);
		}
		/* print the row separator */
		strcat(fen, ((row > 0) ? "/" : " "));
	}
	/* now all that's left is the flags */
	strcat(fen, ((board->tomove == WHITE) ? "w " : "b "));
	/* castle privileges - this is kinda messy */
	if (!(board->castle[WHITE][QUEENSIDE] || board->castle[WHITE][KINGSIDE] ||
	      board->castle[BLACK][QUEENSIDE] || board->castle[BLACK][KINGSIDE]))
	{
		strcat(fen, "-");
	}
	else
	{
		if (board->castle[WHITE][KINGSIDE])  strcat(fen, "K");
		if (board->castle[WHITE][QUEENSIDE]) strcat(fen, "Q");
		if (board->castle[BLACK][KINGSIDE])  strcat(fen, "k");
		if (board->castle[BLACK][QUEENSIDE]) strcat(fen, "q");
	}
	strcat(fen, " ");
	/* enpassant square */
	strcat(fen, ((board->ep != 0) ? squarename[board->ep] : "-"));
	strcat(fen, " ");
	/* move clocks - 0 for absolute move because fuck that shit */
	sprintf(halfmove, "%d %d", board->halfmoves, (board->moves + 1)/2);
	strcat(fen, halfmove);
	
	return fen;
}

/**
 * Gets the piece at the given square index. If the square is empty, -1.
 * You should already know that there's a piece there when you call this.
 * If the board is NULL or the square is invalid, -2.
 * If the third argument is non-null, stores the color of the piece in there
 * (unmodified if piece not present).
 */
piece_t board_pieceatsquare(board_t *board, square_t square, unsigned char *c)
{
	bitboard_t mask;
	piece_t piece;
	unsigned char color;
	
	if (board == NULL || square > 63)
	{
		return -2;
	}

	mask = BB_SQUARE(square);
	
	/* find what color the piece is */
	if (board->piecesofcolor[WHITE] & mask)
	{
		color = WHITE;
	}
	else if (board->piecesofcolor[BLACK] & mask)
	{
		color = BLACK;
	}
	else
	{
		return -1;
	}
	
	if (c != NULL)
	{
		*c = color;
	}
	
	/* find what piece type it is */
	piece = 0;
	while (!(board->pos[color][piece] & mask))
	{
		piece++;
	}
	return piece;
}

/**
 * Is the player whose move it is in check? (1 if so, 0 if not)
 */
int board_incheck(board_t *board)
{
	return board_colorincheck(board, board->tomove);
}

/**
 * Is the given player in check? (1 if so, 0 if not)
 */
int board_colorincheck(board_t *board, unsigned char color)
{
	return !!(board->pos[color][KING] & board->attackedby[OTHERCOLOR(color)]);
}

/**
 * Check if the player to move is checkmated or stalemated. Returns either
 * BOARD_CHECKMATED or BOARD_STALEMATED (from board.h), or 0 if neither.
 * This is a very expensive call. Only the engine, not the searcher, should
 * use this in-between moves to determine when the game ends.
 */
int board_mated(board_t *board)
{
	int result;
	movelist_t moves;
	move_t move;
	unsigned char color;
	
	color = board->tomove;
	result = board_colorincheck(board, color) ?
	               BOARD_CHECKMATED : BOARD_STALEMATED;
	board_generatemoves(board, &moves);


	while (!movelist_isempty(&moves))
	{
		move = movelist_remove_max(&moves);
		/* try the move, see if it puts us in check */
		board_applymove(board, move);
		if (!board_colorincheck(board, color))
		{
			/* found a legal move */
			board_undomove(board, move);
			result = 0;
			break;
		}
		board_undomove(board, move);
	}
	movelist_destroy(&moves);
	return result;
}

/**
 * Is the pawn of given color at given square passed?
 */
int board_pawnpassed(board_t *board, square_t square, unsigned char color)
{
	return !(bb_passedpawnmask[color][square] &
	         board->pos[OTHERCOLOR(color)][PAWN]);
}

/**
 * Is the given square attacked by any piece of the given color? 1 or 0
 * Presumably the square has the OTHERCOLOR(color)'s king on it
 * Note: This is "the" expensive call now that the board library doesn't use
 * attacked bitboards.
 */
int board_squareisattacked(board_t *board, square_t square, unsigned char color)
{
	return !!(BB_SQUARE(square) & board->attackedby[color]);
}

/**
 * Returns true if *any* of the given squares are attacked by color. Useful
 * for determining castling legality
 */
int board_squaresareattacked(board_t *board, bitboard_t squares, unsigned char color)
{
	return !!(squares & board->attackedby[color]);
}


/******************
 * Move generation
 ******************/

/**
 * Given the current board state, returns a bitmask representing all possible
 * attacks from a given square assuming a piece of the specified type/color at
 * that square. This is not the same as all the moves the piece at that square
 * can make - see the special cases: pawns (capture/ep) and kings (castling).
 * For pawns, forward pushes are NOT reflected in this mask, but diagonally
 * threatened squares ARE, EVEN IF there is a friendly piece there.
 *
 * Note: Considering how the attack bitboards work, realize that we want to
 * keep both attacks on other color's pieces and attacks on same color's
 * pieces in here. The board_addmoves functions take care of making sure we
 * don't capture our own pieces.
 *
 * For a cache-friendly optimization, consider taking the slidingpiece's
 * index OUT of the occupancy mask before looking up - i.e. for 11101011 and
 * the rook is the 3rd index from the right, use 11100011 as a lookup key
 * instead, so subsequent accesses with that mask (if you move the rook along
 * the column/row, etc) stay in cache.
 * UPDATE: It seems to not help. I took it out.
 */
bitboard_t board_attacksfrom(board_t *board, square_t square, piece_t piece, unsigned char color)
{
	/* Used for rook/bishop attack generation */
	int shiftamount;
	int diag;
	bitboard_t movemask;
	bitboard_t rowatk, colatk;
	bitboard_t diag45attacks, diag315attacks;
	
	/* NOTE: We mask with ~piecesofcolor in the calling function. */
	switch (piece)
	{
	case PAWN:
		return pawnattacks[color][square];
	case KNIGHT:
		return knightattacks[square];
	case BISHOP:
		/* The first table lookup */
		diag = rot45diagindex[square];
		movemask = rot45attacks[(board->occupied45 >> rot45index_shiftamountright[diag]) & 0xFF]
		                       [COL(square)];
		diag45attacks = (movemask << rotresult_shiftamountleft[diag])
		                >> rotresult_shiftamountright[diag];
		/* The second table lookup */
		diag = rot315diagindex[square];
		movemask = rot315attacks[((board->occupied315 << rot315index_shiftamountleft[diag]) >> rot315index_shiftamountright[diag]) & 0xFF]
					[COL(square)];
		diag315attacks = (movemask << rotresult_shiftamountleft[diag])
		                 >> rotresult_shiftamountright[diag];
		/* Put them together */
		return (diag45attacks | diag315attacks);
	case ROOK:
		/* The first table lookup */
		shiftamount = 8 * ROW(square);
		movemask = rowattacks[(board->occupied  >> shiftamount) & 0xFF][COL(square)];
		rowatk = movemask << shiftamount;
		/* The second table lookup */
		shiftamount = COL(square);
		movemask = colattacks[(board->occupied90 >> (shiftamount * 8)) & 0xFF][ROW(square)];
		colatk = movemask << shiftamount;
		/* Put them together */
		return (rowatk | colatk);
	case QUEEN:
		return board_attacksfrom(board, square, BISHOP, color) |
		       board_attacksfrom(board, square, ROOK, color);
	case KING:
		return kingattacks[square];
	}
	/* If we got to here then piece was invalid */
	panic("Invalid piece!");
	return BB(0x0);
}

/**
 * A bitboard of all legal pawn pushes from a square (in the direction
 * determined by color) considering the occupied state of the board
 */
bitboard_t board_pawnpushesfrom(board_t *board, square_t square, unsigned char color)
{
	bitboard_t moves, emptysquares;
	
	emptysquares = ~(board->occupied);
	moves = pawnmoves[color][square] & emptysquares; /* can't push if blocked */
	/* handle double push only if a single push was available */
	if (moves)
	{
		if (color == WHITE)
		{
			if (ROW(square) == RANK_2)
			{
				moves |= pawnmoves[color][square + 8] & emptysquares;
			}
		}
		else if (ROW(square) == RANK_7)
		{
			moves |= pawnmoves[color][square - 8] & emptysquares;
		}
	}
	return moves;
}

/**
 * Generates a linkedlist of "legal" moves for the color to play at the given
 * position. The moves are guaranteed legal, with all appropriate flags set,
 * suitable for being given to makemove(), EXCEPT it is possible that the move
 * might leave the player in check. We leave this task to the searcher - since
 * it is necessary to call makemove() to see if a move leaves the player in
 * check, and because the searcher calls makemove() anyway; we don't want to
 * waste time calling it twice.
 * This list needs to be movelist_destroy()ed. If there are no legal moves,
 * returns an empty list; if the board is NULL, returns a null pointer.
 */
void board_generatemoves(board_t *board, movelist_t *ml)
{
	bitboard_t position;
	piece_t piece;
	square_t square;
	unsigned char color;
	
	assert(board);
	
	color = board->tomove;
	movelist_init(ml);
	/* we only need to do this part once, not once per pawn */
	if (board->ep)
	{
		board_addmoves_ep(board, color, ml);
	}
	/* now do the rest of the moves */
	for (piece = 0; piece < 6; piece++)
	{
		position = board->pos[color][piece];
		while (position)
		{
			/* find a piece */
			square = BITSCAN(position);
			/* clear the bit */
			position ^= BB_SQUARE(square);
			/* special functions for special cases */
			switch (piece)
			{
			case PAWN:
				board_addmoves_pawn(board, square, color, ml);
				break;
			case KING:
				board_addmoves_king(board, square, color, ml);
				break;
			/* for knights, rooks, bishops, queens,
			 * we can use attacksfrom() directly */
			default:
				board_addmoves(board, square, piece, color, ml);
			}
		}
	}
	return;
}

/**
 * The board_addmoves* series of functions take pointers to two linkedlists,
 * and add each legal move for the specified piece of the specified color at
 * the specified square to the lists, sorted by capture/noncapture.
 */
static void board_addmoves_pawn(board_t *board, square_t square, unsigned char color,
                                movelist_t *ml)
{
	bitboard_t moves;
	square_t destsquare;
	piece_t captpiece;
	move_t move;
	/* note enpassant is handled separately. first we do captures: pawns
	 * can't move to a square they threaten unless it's a capture */
	moves = pawnattacks[color][square] & /* inlined attacksfrom */
	        board->piecesofcolor[OTHERCOLOR(color)];
	while (moves)
	{
		destsquare = BITSCAN(moves);
		moves ^= BB_SQUARE(destsquare);
		captpiece = board_pieceatsquare(board, destsquare, NULL);
		move = (square << MOV_INDEX_SRC) |
		       (destsquare << MOV_INDEX_DEST) |
		       (color << MOV_INDEX_COLOR) |
		       (0x1 << MOV_INDEX_CAPT) |
		       (captpiece << MOV_INDEX_CAPTPC) |
		       (PAWN << MOV_INDEX_PIECE);
		/* handle promotions */
		if (ROW(destsquare) == HOMEROW(OTHERCOLOR(color)))
		{
			move |= (0x1 << MOV_INDEX_PROM);
			movelist_add(ml, board->attackedby, (move | (BISHOP << MOV_INDEX_PROMPC)));
			movelist_add(ml, board->attackedby, (move | (ROOK << MOV_INDEX_PROMPC)));
			movelist_add(ml, board->attackedby, (move | (KNIGHT << MOV_INDEX_PROMPC)));
			movelist_add(ml, board->attackedby, (move | (QUEEN << MOV_INDEX_PROMPC)));
		}
		else /* normal capture */
		{
			movelist_add(ml, board->attackedby, move);
		}
	}
	/* now we handle pushes */
	moves = board_pawnpushesfrom(board, square, color);
	/* and now we have the proper movemask generated */
	while (moves)
	{
		destsquare = BITSCAN(moves);
		moves ^= BB_SQUARE(destsquare);
		move = (square << MOV_INDEX_SRC) |
		       (destsquare << MOV_INDEX_DEST) |
		       (color << MOV_INDEX_COLOR) |
		       (PAWN << MOV_INDEX_PIECE);
		/* handle promotions - add to the captures list because pawn
		 * promotions, like captures, change the board's material */
		if (ROW(destsquare) == HOMEROW(OTHERCOLOR(color)))
		{
			move |= (0x1 << MOV_INDEX_PROM);
			movelist_add(ml, board->attackedby, (move | (BISHOP << MOV_INDEX_PROMPC)));
			movelist_add(ml, board->attackedby, (move | (ROOK << MOV_INDEX_PROMPC)));
			movelist_add(ml, board->attackedby, (move | (KNIGHT << MOV_INDEX_PROMPC)));
			movelist_add(ml, board->attackedby, (move | (QUEEN << MOV_INDEX_PROMPC)));
		}
		else /* normal push */
		{
			movelist_add(ml, board->attackedby, move);
		}
	}
	/* and we're done, phew */
	return;
}
/* don't call this if board->ep = 0 */
static void board_addmoves_ep(board_t *board, unsigned char color, movelist_t *ml)
{
	bitboard_t pawns;
	square_t fromsquare; /* we know where the dest is; we need this now */
	move_t move;

	assert(board->ep);
	
	/* find all pawns eligible to make the capture */
	pawns = bb_adjacentcols[COL(board->ep)] & /* the col the pawn is on */
	        BB_EP_FROMRANK(color) & /* the rank a pawn needs to be on */
		board->pos[color][PAWN] /* where your pawns actually are */;
	while (pawns)
	{
		fromsquare = BITSCAN(pawns);
		pawns ^= BB_SQUARE(fromsquare);
		move = (fromsquare << MOV_INDEX_SRC) |
		       (board->ep << MOV_INDEX_DEST) |
		       (0x1 << MOV_INDEX_EP) |
		       (0x1 << MOV_INDEX_CAPT) |
		       (PAWN << MOV_INDEX_CAPTPC) |
		       (PAWN << MOV_INDEX_PIECE);
		movelist_add(ml, board->attackedby, move);
	}
	return;
}
/* wrapper for board_addmoves() with a special case handler for castling */
static void board_addmoves_king(board_t *board, square_t square, unsigned char color,
                                movelist_t *ml)
{
	move_t move;
	square_t destsquare;
	int side;
	/* used in the second half */
	bitboard_t moves, capts;
	piece_t captpiece;
	
	/* deal with castling - iterate side={0,1} */
	for (side = 0; side < 2; side++)
	{
		if (board->castle[color][side])
		{
			/* If the squares are occupied (1st two lines) or
			 * threatened (2nd two) then we cannot castle */
			if (!((castle_clearsquares[color][side] &
			       board->occupied) ||
			      board_squaresareattacked(board, castle_safesquares[color][side],
			                               OTHERCOLOR(color))))
			{
				/* find where the king will end up */
				destsquare = SQUARE(CASTLE_DEST_COL(side),
				                    HOMEROW(color));
				/* generate and add the move */
				move = (square << MOV_INDEX_SRC) |
				       (destsquare << MOV_INDEX_DEST) |
				       (color << MOV_INDEX_COLOR) |
				       (0x1 << MOV_INDEX_CASTLE) |
				       (KING << MOV_INDEX_PIECE);
				movelist_add(ml, board->attackedby, move);
			}
		}
	}
	/* now the special case is out of the way, the rest is simple. this
	 * could be a call to board_addmoves, but for move ordering purposes
	 * we do it separately. also, if we filter out suicide moves here,
	 * we avoid wasting time with applymove() in the searcher before
	 * seeing if the move is legal. */
	/* inlined attacksfrom */
	moves = kingattacks[square] & (~(board->piecesofcolor[color]));
	/* here's the special part - we can't move the king anywhere where it
	 * will be in check - note, this still produces an illegal move if the
	 * king walks straight backward from a sliding piece, but we're only
	 * producing pseudo-legal moves, so this is merely an optimization.
	 * the movelist relies on not producing an obvious king-hang move. */
	moves &= (~(board->attackedby[OTHERCOLOR(color)]));
	/* the rest is the same as board_addmoves */
	capts = moves & board->piecesofcolor[OTHERCOLOR(color)];
	moves ^= capts;
	/* that's a lie - the other special thing we do here is ordering so
	 * the king moves/capts are grouped with the pawn moves/capts */
	while (capts)
	{
		/* find and clear a bit */
		destsquare = BITSCAN(capts);
		capts ^= BB_SQUARE(destsquare);
		/* find what piece was captured */
		captpiece = board_pieceatsquare(board, destsquare, NULL);
		/* generate the move_t */
		move = (square << MOV_INDEX_SRC) |       /* from square */
		       (destsquare << MOV_INDEX_DEST) |  /* to square */
		       (color << MOV_INDEX_COLOR) |      /* color flag */
		       (0x1 << MOV_INDEX_CAPT) |         /* capture flag */
		       (captpiece << MOV_INDEX_CAPTPC) | /* captured piece */
		       (KING << MOV_INDEX_PIECE);        /* our piece */
		/* and add it to the list */
		movelist_add(ml, board->attackedby, move);
	}
	/* then process noncapture moves */
	while (moves)
	{
		/* find and clear a bit */
		destsquare = BITSCAN(moves);
		moves ^= BB_SQUARE(destsquare);
		/* generate the move_t */
		move = (square << MOV_INDEX_SRC) |      /* from square */
		       (destsquare << MOV_INDEX_DEST) | /* to square */
		       (color << MOV_INDEX_COLOR) |     /* color flag */
		       (KING << MOV_INDEX_PIECE);       /* our piece */
		/* and add it to the list */
		movelist_add(ml, board->attackedby, move);
	}

	return;
}
/* for non-special-case pieces: knight bishop rook queen. see above comment */
static void board_addmoves(board_t *board, square_t square, piece_t piece, unsigned char color,
                           movelist_t *ml)
{
	bitboard_t moves, capts;
	square_t destsquare;
	piece_t captpiece;
	move_t move;
	
	/* find all destination squares -- all attacked squares, but can't
	 * capture our own pieces */
	moves = board_attacksfrom(board, square, piece, color) & (~(board->piecesofcolor[color]));
	/* separate captures from noncaptures */
	capts = moves & board->piecesofcolor[OTHERCOLOR(color)];
	moves ^= capts;
	/* process captures first */
	while (capts)
	{
		/* find and clear a bit */
		destsquare = BITSCAN(capts);
		capts ^= BB_SQUARE(destsquare);
		/* find what piece was captured */
		captpiece = board_pieceatsquare(board, destsquare, NULL);
		/* generate the move_t */
		move = (square << MOV_INDEX_SRC) |       /* from square */
		       (destsquare << MOV_INDEX_DEST) |  /* to square */
		       (color << MOV_INDEX_COLOR) |      /* color flag */
		       (0x1 << MOV_INDEX_CAPT) |         /* capture flag */
		       (captpiece << MOV_INDEX_CAPTPC) | /* captured piece */
		       (piece << MOV_INDEX_PIECE);       /* our piece */
		/* and add it to the list */
		movelist_add(ml, board->attackedby, move);
	}
	/* then process noncapture moves */
	while (moves)
	{
		/* find and clear a bit */
		destsquare = BITSCAN(moves);
		moves ^= BB_SQUARE(destsquare);
		/* generate the move_t */
		move = (square << MOV_INDEX_SRC) |      /* from square */
		       (destsquare << MOV_INDEX_DEST) | /* to square */
		       (color << MOV_INDEX_COLOR) |     /* color flag */
		       (piece << MOV_INDEX_PIECE);      /* our piece */
		/* and add it to the list */
		movelist_add(ml, board->attackedby, move);
	}
	return;
}

/**
 * The same as board_generatemoves, except we generate only captures. This is
 * optimized to not even think about noncapture moves to be especially fast
 * for quiescence searching.
 */
void board_generatecaptures(board_t *board, movelist_t *ml)
{
	bitboard_t position;
	piece_t piece;
	square_t square;
	unsigned char color;

	assert(board);
	
	color = board->tomove;
	movelist_init(ml);
	/* same call as in addmoves... ep is always a capture */
	if (board->ep)
	{
		//XXX: Take this out, as it won't help that much in quiescence? Depends on how fast it is
		board_addmoves_ep(board, color, ml);
	}
	for (piece = 0; piece < 6; piece++)
	{
		position = board->pos[color][piece];
		while (position)
		{
			/* find a piece */
			square = BITSCAN(position);
			/* clear the bit */
			position ^= BB_SQUARE(square);
			switch (piece)
			{
			case PAWN:
				board_addcaptures_pawn(board, square, color, ml);
				break;
			case KING:
				board_addcaptures_king(board, square, color, ml);
				break;
			default:
				board_addcaptures(board, square, piece, color, ml);
			}
		}
	}
	return;
}

static void board_addcaptures_pawn(board_t *board, square_t square, unsigned char color, movelist_t *ml)
{
	bitboard_t moves;
	square_t destsquare;
	piece_t captpiece;
	move_t move;
	/* captures */
	moves = pawnattacks[color][square] & /* inlined attacksfrom */
	        board->piecesofcolor[OTHERCOLOR(color)];
	while (moves)
	{
		destsquare = BITSCAN(moves);
		moves ^= BB_SQUARE(destsquare);
		captpiece = board_pieceatsquare(board, destsquare, NULL);
		move = (square << MOV_INDEX_SRC) |
		       (destsquare << MOV_INDEX_DEST) |
		       (color << MOV_INDEX_COLOR) |
		       (0x1 << MOV_INDEX_CAPT) |
		       (captpiece << MOV_INDEX_CAPTPC) |
		       (PAWN << MOV_INDEX_PIECE);
		/* handle promotions */
		if (ROW(destsquare) == HOMEROW(OTHERCOLOR(color)))
		{
			move |= (0x1 << MOV_INDEX_PROM);
			movelist_add(ml, board->attackedby, (move | (BISHOP << MOV_INDEX_PROMPC)));
			movelist_add(ml, board->attackedby, (move | (ROOK << MOV_INDEX_PROMPC)));
			movelist_add(ml, board->attackedby, (move | (KNIGHT << MOV_INDEX_PROMPC)));
			movelist_add(ml, board->attackedby, (move | (QUEEN << MOV_INDEX_PROMPC)));
		}
		else /* normal capture */
		{
			movelist_add(ml, board->attackedby, move);
		}
	}
	/* now we handle pushes only if it's a passed pawn */
	if (board_pawnpassed(board, square, color))
	{
		moves = board_pawnpushesfrom(board, square, color);
		/* and now we have the proper movemask generated */
		while (moves)
		{
			destsquare = BITSCAN(moves);
			moves ^= BB_SQUARE(destsquare);
			move = (square << MOV_INDEX_SRC) |
			       (destsquare << MOV_INDEX_DEST) |
			       (color << MOV_INDEX_COLOR) |
			       (PAWN << MOV_INDEX_PIECE);
			/* handle promotions */
			if (ROW(destsquare) == HOMEROW(OTHERCOLOR(color)))
			{
				move |= (0x1 << MOV_INDEX_PROM);
				movelist_add(ml, board->attackedby, (move | (BISHOP << MOV_INDEX_PROMPC)));
				movelist_add(ml, board->attackedby, (move | (ROOK << MOV_INDEX_PROMPC)));
				movelist_add(ml, board->attackedby, (move | (KNIGHT << MOV_INDEX_PROMPC)));
				movelist_add(ml, board->attackedby, (move | (QUEEN << MOV_INDEX_PROMPC)));
			}
			else /* normal push */
			{
				movelist_add(ml, board->attackedby, move);
			}
		}
	}
	return;
}

static void board_addcaptures_king(board_t *board, square_t square, unsigned char color, movelist_t *ml)
{
	move_t move;
	square_t destsquare;
	bitboard_t moves, capts;
	piece_t captpiece;
	
	/* No dealing with castling! :D this is the same as addcaptures() but
	 * again we do the special shits */
	/* inlined attacksfrom */
	moves = kingattacks[square] & (~(board->piecesofcolor[color]));
	moves &= (~(board->attackedby[OTHERCOLOR(color)]));
	capts = moves & board->piecesofcolor[OTHERCOLOR(color)];
	moves ^= capts;
	/* like in regular movegen, captures by king go at the beginning */
	while (capts)
	{
		/* find and clear a bit */
		destsquare = BITSCAN(capts);
		capts ^= BB_SQUARE(destsquare);
		/* find what piece was captured */
		captpiece = board_pieceatsquare(board, destsquare, NULL);
		/* generate the move_t */
		move = (square << MOV_INDEX_SRC) |       /* from square */
		       (destsquare << MOV_INDEX_DEST) |  /* to square */
		       (color << MOV_INDEX_COLOR) |      /* color flag */
		       (0x1 << MOV_INDEX_CAPT) |         /* capture flag */
		       (captpiece << MOV_INDEX_CAPTPC) | /* captured piece */
		       (KING << MOV_INDEX_PIECE);        /* our piece */
		/* and add it to the list */
		movelist_add(ml, board->attackedby, move);
	}
	return;
}

static void board_addcaptures(board_t *board, square_t square, piece_t piece, unsigned char color, movelist_t *ml)
{
	bitboard_t moves, capts;
	square_t destsquare;
	piece_t captpiece;
	move_t move;
	
	/* find all destination squares -- all attacked squares, but can't
	 * capture our own pieces */
	moves = board_attacksfrom(board, square, piece, color) &
	        (~(board->piecesofcolor[color]));
	/* separate captures from noncaptures */
	capts = moves & board->piecesofcolor[OTHERCOLOR(color)];
	moves ^= capts;
	/* process captures first */
	while (capts)
	{
		/* find and clear a bit */
		destsquare = BITSCAN(capts);
		capts ^= BB_SQUARE(destsquare);
		/* find what piece was captured */
		captpiece = board_pieceatsquare(board, destsquare, NULL);
		/* generate the move_t */
		move = (square << MOV_INDEX_SRC) |       /* from square */
		       (destsquare << MOV_INDEX_DEST) |  /* to square */
		       (color << MOV_INDEX_COLOR) |      /* color flag */
		       (0x1 << MOV_INDEX_CAPT) |         /* capture flag */
		       (captpiece << MOV_INDEX_CAPTPC) | /* captured piece */
		       (piece << MOV_INDEX_PIECE);       /* our piece */
		/* and add it to the list */
		movelist_add(ml, board->attackedby, move);
	}
	return;
}


/***********************
 * Changing board state
 ***********************/

/**
 * Given a move, which MUST be properly formatted AND legal (only use moves
 * returned by board_generatemoves() or move_islegal() here), applies the move
 * to change the board position. Updates the zobrist hash, undo stacks,
 * everything.
 */
void board_applymove(board_t *board, move_t move)
{
	square_t rooksrc, rookdest; /* how the rook moves in castling */
	square_t epcapture;         /* where a pawn will disappear from */
	int i;
	
	/* save history. */
	history_t *h = &board->history[board->moves];
	assert(board->moves < HISTORY_STACK_SIZE);
	h->hash                     = board->hash;
	h->attackedby[WHITE]        = board->attackedby[WHITE];
	h->attackedby[BLACK]        = board->attackedby[BLACK];
	h->castle[WHITE][QUEENSIDE] = board->castle[WHITE][QUEENSIDE];
	h->castle[WHITE][KINGSIDE]  = board->castle[WHITE][KINGSIDE];
	h->castle[BLACK][QUEENSIDE] = board->castle[BLACK][QUEENSIDE];
	h->castle[BLACK][KINGSIDE]  = board->castle[BLACK][KINGSIDE];
	h->ep                       = board->ep;
	h->halfmoves                = board->halfmoves;
	h->reps                     = board->reps;
	h->move                     = move;

	/* cleanly handle null-move */
	if (move == 0)
	{
		/* clear ep square */
		board->hash ^= zobrist_ep[board->ep];
		board->ep = 0;
		board->hash ^= zobrist_ep[board->ep];
		/* update the move clock */
		board->halfmoves++;
		board->moves++;
		/* null move is illegal, so it can't cause a threefold draw */
		board->reps = 0;
		/* switch who has the move */
		board->tomove = OTHERCOLOR(board->tomove);
		board->hash ^= zobrist_tomove;
		return;
	}
	
	assert(MOV_PIECE(move) >= 0 && MOV_PIECE(move) < 6);

	/* Take care of captures - we need to remove the captured piece. */
	if (MOV_CAPT(move))
	{
		/* special case - en passant - square behind the dest square */
		if (MOV_EP(move))
		{
			assert(ROW(MOV_SRC(move)) == 4 - board->tomove);
			/* find where the pawn disappears from */
			epcapture = (board->tomove == WHITE) ?
			            (MOV_DEST(move) - 8) :
				    (MOV_DEST(move) + 8);
			/* kill the pawn */
			board_togglepiece(board, epcapture,
			                  OTHERCOLOR(board->tomove), PAWN);
			/* change the material count */
			board->material[OTHERCOLOR(board->tomove)] -=
				eval_piecevalue[PAWN];
		}
		/* standard capture - square is the dest square */
		else
		{
			board_togglepiece(board, MOV_DEST(move),
			                  OTHERCOLOR(board->tomove),
			                  MOV_CAPTPC(move));
			/* If the captured piece is a rook on its home square,
			 * and the player was still able to castle that side,
			 * we need to clear the castle privileges there */
			if (MOV_CAPTPC(move) == ROOK &&
			    ROW(MOV_DEST(move)) == HOMEROW(OTHERCOLOR(board->tomove)) &&
			    (COL(MOV_DEST(move)) == COL_H ||
			     COL(MOV_DEST(move)) == COL_A))
			{
				/* (COLUMN == COL_H) will evaluate to 1 if on
				 * KINGSIDE (this depends on KINGSIDE == 1) */
				unsigned char side = (COL(MOV_DEST(move)) == COL_H);
				/* if castle rights is already gone there, we
				 * don't do anything. this is important when
				 * toggling the zobrist */
				if (board->castle[OTHERCOLOR(board->tomove)][side])
				{
					board->castle[OTHERCOLOR(board->tomove)][side] = 0;
					board->hash ^= zobrist_castle[OTHERCOLOR(board->tomove)]
					                             [side];
				}

			}
			/* change the material count */
			board->material[OTHERCOLOR(board->tomove)] -=
				eval_piecevalue[MOV_CAPTPC(move)];
		}
		board->halfmoves = 0;
	}
	/* here we detect irreversible moves for the halfmove clock */
	else
	{
		if (MOV_PIECE(move) == PAWN)
		{
			board->halfmoves = 0;
		}
		else
		{
			board->halfmoves++;
		}
	}
	/* the game clock as well */
	board->moves++;
	
	/* castling, need to move the rook. castle privileges handled below */
	if (MOV_CASTLE(move))
	{
		/* detect kingside vs queenside */
		if (COL(MOV_DEST(move)) == COL_G)
		{
			rooksrc  = SQUARE(COL_H,HOMEROW(board->tomove));
			rookdest = SQUARE(COL_F,HOMEROW(board->tomove));
		}
		else
		{
			rooksrc  = SQUARE(COL_A,HOMEROW(board->tomove));
			rookdest = SQUARE(COL_D,HOMEROW(board->tomove));
		}
		/* and move the rook */
		board_togglepiece(board, rooksrc, board->tomove, ROOK);
		board_togglepiece(board, rookdest, board->tomove, ROOK);
		/* set the flag */
		board->hascastled[board->tomove] = 1;
	}
	
	/* here we move the moving piece itself; in the former case it turns
	 * into a different type of piece */
	board_togglepiece(board, MOV_SRC(move), board->tomove,
	                  MOV_PIECE(move));
	if (MOV_PROM(move))
	{
		assert(ROW(MOV_DEST(move)) ==
		       HOMEROW(OTHERCOLOR(board->tomove)));
		board_togglepiece(board, MOV_DEST(move), board->tomove,
		                 MOV_PROMPC(move));
		/* and of course account for this in the material count */
		board->material[board->tomove] +=
			(eval_piecevalue[MOV_PROMPC(move)] -
			 eval_piecevalue[PAWN]);
	}
	else
	{
		board_togglepiece(board, MOV_DEST(move), board->tomove,
		                 MOV_PIECE(move));
	}

	/* detect an enpassantable move */
	if (MOV_PIECE(move) == PAWN)
	{
		/* if the difference between the square values is 16 not 8
		 * then it was a double push */
		if ((board->tomove == WHITE) &&
		    ((MOV_DEST(move) - MOV_SRC(move)) == 16))
		{
			/* clear the old ep status in the hash */
			board->hash ^= zobrist_ep[board->ep];
			/* set the new value */
			board->ep = MOV_SRC(move) + 8;
			/* throw in the new key */
			board->hash ^= zobrist_ep[board->ep];
		}
		else if ((board->tomove == BLACK) &&
		         ((MOV_SRC(move) - MOV_DEST(move)) == 16))
		{
			board->hash ^= zobrist_ep[board->ep];
			board->ep = MOV_DEST(move) + 8;
			board->hash ^= zobrist_ep[board->ep];
		}
		else
		{
			board->hash ^= zobrist_ep[board->ep];
			board->ep = 0;
			board->hash ^= zobrist_ep[board->ep];
		}
	}
	/* not a pawn move. ep square goes away. */
	else
	{
		board->hash ^= zobrist_ep[board->ep];
		board->ep = 0;
		board->hash ^= zobrist_ep[board->ep];
		/* handle castling rights */
		if (MOV_PIECE(move) == KING)
		{
			if (board->castle[board->tomove][KINGSIDE])
			{
				board->castle[board->tomove][KINGSIDE] = 0;
				board->hash ^= zobrist_castle[board->tomove][KINGSIDE];
			}
			if (board->castle[board->tomove][QUEENSIDE])
			{
				board->castle[board->tomove][QUEENSIDE] = 0;
				board->hash ^= zobrist_castle[board->tomove][QUEENSIDE];
			}
		}
		else if (MOV_PIECE(move) == ROOK)
		{
			if ((board->castle[board->tomove][KINGSIDE]) &&
			    (COL(MOV_SRC(move)) == COL_H))
			{
				board->castle[board->tomove][KINGSIDE] = 0;
				board->hash ^= zobrist_castle[board->tomove][KINGSIDE];
			}
			else if ((board->castle[board->tomove][QUEENSIDE]) &&
			         (COL(MOV_SRC(move)) == COL_A))
			{
				board->castle[board->tomove][QUEENSIDE] = 0;
				board->hash ^= zobrist_castle[board->tomove][QUEENSIDE];
			}
		}
	}

	/* switch who has the move */
	board->tomove = OTHERCOLOR(board->tomove);
	board->hash ^= zobrist_tomove;

	/* calculate repetition count */
	board->reps = 0;
	for (i = 1; i <= board->halfmoves; i++)
	{
		if (board->history[board->moves - i].hash == board->hash)
		{
			board->reps = board->history[board->moves - i].reps+1;
			break;
		}
	}
	
	/* and finally */
	board_regeneratethreatened(board);
	return;
}

/**
 * Go back one move. The cleaner implementation is to keep a stack of the
 * moves, but it's faster to have the searcher/legalitychecker just pass in
 * the move again rather than mucking about with data structures.
 * Note if you pass in a move different from the one that was most recently
 * made, the board state will shit and die horribly.
 */
void board_undomove(board_t *board, move_t move)
{
	square_t rooksrc, rookdest, epcapture;
	
	/* switch who has the move */
	board->tomove = OTHERCOLOR(board->tomove);
	board->hash ^= zobrist_tomove;

	/* cleanly handle null-move */
	if (move == 0)
	{
		board->moves--;
		board->halfmoves--;
		board->hash ^= zobrist_ep[board->ep];
		board->ep = board->history[board->moves].ep;
		board->hash ^= zobrist_ep[board->ep];
		board->reps = board->history[board->moves].reps;
		return;
	}
	
	/* The non-special things (see bottom of function for special things)
	 * are basically applymove in reverse. First move the piece back. */
	board_togglepiece(board, MOV_SRC(move), board->tomove, MOV_PIECE(move));
	if (MOV_PROM(move))
	{
		board_togglepiece(board, MOV_DEST(move), board->tomove,
		                  MOV_PROMPC(move));
		/* material count */
		board->material[board->tomove] -=
			(eval_piecevalue[MOV_PROMPC(move)] -
			 eval_piecevalue[PAWN]);
	}
	else
	{
		board_togglepiece(board, MOV_DEST(move), board->tomove,
		                  MOV_PIECE(move));
	}
	/* castling, need to move the rook back. */
	if (MOV_CASTLE(move))
	{
		/* detect kingside vs queenside */
		if (COL(MOV_DEST(move)) == COL_G)
		{
			/* note src is where it needs to end up because we're
			 * moving backwards not forwards */
			rooksrc  = SQUARE(COL_H,HOMEROW(board->tomove));
			rookdest = SQUARE(COL_F,HOMEROW(board->tomove));
		}
		else
		{
			rooksrc  = SQUARE(COL_A,HOMEROW(board->tomove));
			rookdest = SQUARE(COL_D,HOMEROW(board->tomove));
		}
		/* and move the rook */
		board_togglepiece(board, rooksrc, board->tomove, ROOK);
		board_togglepiece(board, rookdest, board->tomove, ROOK);
		/* flag */
		board->hascastled[board->tomove] = 0;
	}
	/* for captures, put the captured piece back */
	if (MOV_CAPT(move))
	{
		/* special case - en passant - square behind the dest square */
		if (MOV_EP(move))
		{
			epcapture = (board->tomove == WHITE) ?
			            (MOV_DEST(move) - 8) :
				    (MOV_DEST(move) + 8);
			board_togglepiece(board, epcapture,
			                  OTHERCOLOR(board->tomove), PAWN);
			board->material[OTHERCOLOR(board->tomove)] +=
				eval_piecevalue[PAWN];
		}
		/* standard capture - square is the dest square */
		else
		{
			board_togglepiece(board, MOV_DEST(move),
			                  OTHERCOLOR(board->tomove),
			                  MOV_CAPTPC(move));
			board->material[OTHERCOLOR(board->tomove)] +=
				eval_piecevalue[MOV_CAPTPC(move)];
		}
	}

	/* decrement the game clock */
	board->moves--;
	
	/* Get the previous special board state information from the stacks */
	board->attackedby[WHITE] = board->history[board->moves].attackedby[WHITE];
	board->attackedby[BLACK] = board->history[board->moves].attackedby[BLACK];
	
	/* clear the old castle keys */
	if (board->castle[WHITE][QUEENSIDE]) { board->hash ^= zobrist_castle[WHITE][QUEENSIDE]; }
	if (board->castle[WHITE][KINGSIDE])  { board->hash ^= zobrist_castle[WHITE][KINGSIDE]; }
	if (board->castle[BLACK][QUEENSIDE]) { board->hash ^= zobrist_castle[BLACK][QUEENSIDE]; }
	if (board->castle[BLACK][KINGSIDE])  { board->hash ^= zobrist_castle[BLACK][KINGSIDE]; }
	/* set the new castle rights */
	board->castle[WHITE][QUEENSIDE] = board->history[board->moves].castle[WHITE][QUEENSIDE];
	board->castle[WHITE][KINGSIDE]  = board->history[board->moves].castle[WHITE][KINGSIDE];
	board->castle[BLACK][QUEENSIDE] = board->history[board->moves].castle[BLACK][QUEENSIDE];
	board->castle[BLACK][KINGSIDE]  = board->history[board->moves].castle[BLACK][KINGSIDE];
	/* set the new castle keys */
	if (board->castle[WHITE][QUEENSIDE]) { board->hash ^= zobrist_castle[WHITE][QUEENSIDE]; }
	if (board->castle[WHITE][KINGSIDE])  { board->hash ^= zobrist_castle[WHITE][KINGSIDE]; }
	if (board->castle[BLACK][QUEENSIDE]) { board->hash ^= zobrist_castle[BLACK][QUEENSIDE]; }
	if (board->castle[BLACK][KINGSIDE])  { board->hash ^= zobrist_castle[BLACK][KINGSIDE]; }

	/* restore ep information */
	board->hash ^= zobrist_ep[board->ep];
	board->ep = board->history[board->moves].ep;
	board->hash ^= zobrist_ep[board->ep];

	/* set the halfmove clock */
	board->halfmoves = board->history[board->moves].halfmoves;

	/* and repetitions */
	board->reps = board->history[board->moves].reps;

	/* consistency check */
	assert(board->hash == board->history[board->moves].hash);
	assert(move == board->history[board->moves].move);
	return;
}

/**
 * The togglepiece function is a wrapper to simplify moving pieces around on
 * the board. They modify the bits in the piece-specific position masks, the
 * all-pieces-of-one-color mask, and in the four occupied boards, and adjust
 * the zobrist hash.
 */
static void board_togglepiece(board_t *board, square_t square,
                              unsigned char color, piece_t piece)
{
	/* no other piece ought be on this square */
	assert(!(board->occupied & BB_SQUARE(square) &
		 ~board->pos[color][piece]));
	/* piece-specific position mask */
	board->pos[color][piece] ^= BB_SQUARE(square);
	
	/* all the color's pieces */
	board->piecesofcolor[color] ^= BB_SQUARE(square);
	
	/* general occupied masks */
	board->occupied ^= BB_SQUARE(square);
	board->occupied90 ^= BB_SQUARE(ROT90SQUAREINDEX(square));
	board->occupied45 ^= BB_SQUARE(ROT45SQUAREINDEX(square));
	board->occupied315 ^= BB_SQUARE(ROT315SQUAREINDEX(square));
	
	/* adjust the zobrist */
	board->hash ^= zobrist_piece[color][piece][square];
}

/**
 * This hashtable stores threatened square masks so we don't have to always
 * regenerate them.
 */
typedef struct {
	bitboard_t key;
	bitboard_t white;
	bitboard_t black;
} regen_entry_t;

#ifndef REGEN_NUM_BUCKETS
#define REGEN_NUM_BUCKETS 4194304
#endif
static regen_entry_t regen_array[REGEN_NUM_BUCKETS];

static void regen_add(bitboard_t key, bitboard_t white, bitboard_t black)
{
	unsigned long bucket = key % REGEN_NUM_BUCKETS;
	regen_array[bucket].key = key;
	regen_array[bucket].white = white;
	regen_array[bucket].black = black;
}

static int regen_get(bitboard_t key, bitboard_t *white, bitboard_t *black)
{
	unsigned long bucket = key % REGEN_NUM_BUCKETS;
	if (regen_array[bucket].key == key)
	{
		*white = regen_array[bucket].white;
		*black = regen_array[bucket].black;
		return 1;
	}
	return 0;
}

int regen_hits = 0;
int regen_misses = 0;

/**
 * Regenerate the threatened squares masks for both sides. Should be used when
 * a move is made. Don't use in undomove, use the special stacks instead
 */
static void board_regeneratethreatened(board_t *board)
{
	bitboard_t mask;
	bitboard_t position;
	piece_t piece;
	square_t square;
	
	/* check hashtable */
	if (regen_get(board->hash,
		      &board->attackedby[WHITE], &board->attackedby[BLACK]))
	{
		regen_hits++;
		return;
	}

	/********
	 * White
	 ********/
	mask = BB(0x0);
	/* pawn threatened can be done for all pawns at once */
	mask |= board_pawnattacks(board->pos[WHITE][PAWN], WHITE);
	/* Knights and kings manually inlined */
	position = board->pos[WHITE][KNIGHT];
	while (position)
	{
		square = BITSCAN(position);
		position ^= BB_SQUARE(square);
		mask |= knightattacks[square];
	}
	position = board->pos[WHITE][KING];
	while(position)
	{
		square = BITSCAN(position);
		position ^= BB_SQUARE(square);
		mask |= kingattacks[square];
	}
	/* other piece threatened */
	for (piece = 2; piece < 5; piece++)
	{
		position = board->pos[WHITE][piece];
		/* keep going until we've found all the pieces */
		while (position)
		{
			/* Find one of the pieces, clear its bit */
			square = BITSCAN(position);
			position ^= BB_SQUARE(square);
			/* Put the attacks from this square in the mask */
			mask |= board_attacksfrom(board, square, piece, WHITE);
		}
	}
	board->attackedby[WHITE] = mask;
	/********
	 * Black
	 ********/
	mask = BB(0x0);
	/* pawn threatened can be done for all pawns at once */
	mask |= board_pawnattacks(board->pos[BLACK][PAWN], BLACK);
	/* Knights and kings manually inlined */
	position = board->pos[BLACK][KNIGHT];
	while (position)
	{
		square = BITSCAN(position);
		position ^= BB_SQUARE(square);
		mask |= knightattacks[square];
	}
	position = board->pos[BLACK][KING];
	while(position)
	{
		square = BITSCAN(position);
		position ^= BB_SQUARE(square);
		mask |= kingattacks[square];
	}
	/* other piece threatened */
	for (piece = 2; piece < 5; piece++)
	{
		position = board->pos[BLACK][piece];
		/* keep going until we've found all the pieces */
		while (position)
		{
			/* Find one of the pieces, clear its bit */
			square = BITSCAN(position);
			position ^= BB_SQUARE(square);
			/* Put the attacks from this square in the mask */
			mask |= board_attacksfrom(board, square, piece, BLACK);
		}
	}
	board->attackedby[BLACK] = mask;

	/* save to hashtable */
	regen_add(board->hash, board->attackedby[WHITE], board->attackedby[BLACK]);
	regen_misses++;
	
	return;
}

/**
 * Get a bitmap of all squares attacked by the pawns in the given position of
 * the given color. Used by regenthreatened, and also in pawnstructure eval
 */
bitboard_t board_pawnattacks(bitboard_t pawns, unsigned char color)
{
	/* a-pawns don't attack left; h-pawns don't attack right */
	if (color == WHITE)
	{
		return ((pawns & ~BB_FILEA) << 7) | ((pawns & ~BB_FILEH) << 9);
	}
	else
	{
		return ((pawns & ~BB_FILEA) >> 9) | ((pawns & ~BB_FILEH) >> 7);
	}
}

/**
 * Is the current position a threefold draw?
 */
int board_threefold_draw(board_t *board)
{
	return board->reps >= 2;
}

/****************************************************************************
 * END:   BITBOARD FUNCTIONS
 * BEGIN: MOVE FUNCTIONS
 ****************************************************************************/

static move_t move_checksuicide(board_t *, move_t);

/**
 * Parses computer friendly ("d7c8N" or "e1g1") notation to generate a simple
 * move_t template. Will return 0 on a malformatted move (note 0 is never a
 * valid move_t). This will only set the prom flag, not castle/ep/capt. As a
 * result, the resultant move is not suitable for throwing directly into
 * applyMove.
 */
move_t move_fromstring(char *str)
{
	size_t length = strlen(str);
	move_t move = 0;
	
	/* check for malformatted move string */
	if (length < 4 || length > 5 || /* too short/long */
	    !(str[0] >= 'a' && str[0] <= 'h') || /* bad src/dest squares */
	    !(str[1] >= '1' && str[1] <= '8') ||
	    !(str[2] >= 'a' && str[2] <= 'h') ||
	    !(str[3] >= '1' && str[3] <= '8') ||
	    (length == 5 && /* bad promotion piece */
	     !(str[4] == 'N' || str[4] == 'B' ||
	       str[4] == 'R' || str[4] == 'Q' ||
	       str[4] == 'n' || str[4] == 'b' ||
	       str[4] == 'r' || str[4] == 'q')))
	{
		return (move_t)0;
	}
	
	/* set the src and dest */
	move |= SQUARE((str[0] - 'a'), (str[1] - '1')) << MOV_INDEX_SRC;
	move |= SQUARE((str[2] - 'a'), (str[3] - '1')) << MOV_INDEX_DEST;
	
	/* promotion */
	if (length == 5)
	{
		move |= 1 << MOV_INDEX_PROM; /* set the flag */
		switch(str[4])
		{
		case 'N':
		case 'n':
			move |= KNIGHT << MOV_INDEX_PROMPC;
			break;
		case 'B':
		case 'b':
			move |= BISHOP << MOV_INDEX_PROMPC;
			break;
		case 'R':
		case 'r':
			move |= ROOK << MOV_INDEX_PROMPC;
			break;
		default:
			move |= QUEEN << MOV_INDEX_PROMPC;
		}
	}
	
	return move;
}

/**
 * A more advanced form of move_fromstring. Given also a board_t, will return
 * a move_t for the move represented by str if it is 1) properly formatted,
 * 2) valid, 3) legal. Otherwise returns 0. All appropriate flags will be set,
 * so this is suitable for applymove.
 * This code is not fast, as it will apply/undo the move from the board. Use
 * it for checking the opponent's move, not the engine's move.
 */
move_t move_islegal(board_t *board, char *str)
{
	move_t move;
	square_t src, dest;
	piece_t piece;
	bitboard_t moves, clearsquares;
	
	/* Parse the string */
	move = move_fromstring(str);
	src = MOV_SRC(move);
	dest = MOV_DEST(move);
	
	/* First check: sanity; this also handles if move_fromstring failed */
	if (src == dest)
	{
		fprintf(stderr, "MOVE_ISLEGAL: src == dest\n");
		return 0;
	}
	/* Does this piece belong to us */
	if (!(BB_SQUARE(src) & board->piecesofcolor[board->tomove]))
	{
		fprintf(stderr, "MOVE_ISLEGAL: you don't have a piece on that square\n");
		return 0;
	}
	
	/* Find our piece and where it can go */
	piece = board_pieceatsquare(board, src, NULL);
	moves = board_attacksfrom(board, src, piece, board->tomove) &
	        (~(board->piecesofcolor[board->tomove])); /* can't capture own piece */
	move |= piece << MOV_INDEX_PIECE; /* set the piece-type flag */
	
	/* of course, pawns and kings are special... */
	if (piece == PAWN) /* pushes and enpassants */
	{
		/* can't make an attacking move unless it's a capture */
		moves &= board->piecesofcolor[OTHERCOLOR(board->tomove)];
		/* Trying to make an illegal promotion, or not promoting on an
		 * advance to the promotion rank */
		if ((!!(MOV_PROM(move))) ^
		    (!!(ROW(dest) == HOMEROW(OTHERCOLOR(board->tomove)))))
		{
			fprintf(stderr, "MOVE_ISLEGAL: illegal promotion semantics for pawn\n");
			return 0;
		}
		/* move is a regular capture */
		if (BB_SQUARE(dest) & moves)
		{
			move |= 1 << MOV_INDEX_CAPT;
			move |= board_pieceatsquare(board, dest, NULL) << MOV_INDEX_CAPTPC;
			return move_checksuicide(board, move);
		}
		/* enpassant */
		if (board->ep && dest == board->ep)
		{
			move |= (0x1 << MOV_INDEX_EP) |
			        (0x1 << MOV_INDEX_CAPT) |
			        (PAWN << MOV_INDEX_CAPTPC);
			return move_checksuicide(board, move);
		}
		/* check for push */
		clearsquares = ~board->occupied;
		/* is pawn free to move one square ahead? we don't need to set
		 * any special additional flags here */
		if (pawnmoves[board->tomove][src] & clearsquares)
		{
			/* Are we trying to move 1 square ahead? Note: relies
			 * on the fact that pawnmoves[][] has one bit set. */
			if (BB_SQUARE(dest) == pawnmoves[board->tomove][src])
			{
				return move_checksuicide(board, move);
			}
			/* 2-square push from 2nd rank */
			if (((board->tomove == WHITE) &&
			     (pawnmoves[board->tomove][src+8] & clearsquares) &&
			     (BB_SQUARE(dest) == pawnmoves[board->tomove][src+8])) ||
			    ((board->tomove == BLACK) &&
			     (pawnmoves[board->tomove][src-8] & clearsquares) &&
			     (BB_SQUARE(dest) == pawnmoves[board->tomove][src-8])))
			{
				return move_checksuicide(board, move);
			}
		}
		fprintf(stderr, "MOVE_ISLEGAL: that doesn't seem to be a valid pawn move\n");
		return 0;
	}
	/* not a pawn, trying to promote a different piece */
	else if (MOV_PROM(move))
	{
		fprintf(stderr, "MOVE_ISLEGAL: can't promote a non-pawn piece\n");
		return 0;
	}
	/* note - for king moves, we don't need to apply the move to check if
	 * it puts the player in check; we use squareisattacked() directly */
	else if (piece == KING)
	{
		/* handle castling - queenside */
		if (board->castle[board->tomove][QUEENSIDE] &&
		    (src == SQUARE(COL_E,HOMEROW(board->tomove))) &&
		    (dest == SQUARE(COL_C,HOMEROW(board->tomove)))) /* "e1c1" or "e8c8" */
		{
			/* check if special squares are blocked or attacked */
			if ((castle_clearsquares[board->tomove][QUEENSIDE] & board->occupied) ||
			    board_squaresareattacked(board, castle_safesquares[board->tomove][QUEENSIDE], OTHERCOLOR(board->tomove)))
			{
				fprintf(stderr, "MOVE_ISLEGAL: you are unable to castle queenside\n");
				return 0;
			}
			/* here, the castle is legal */
			move |= 0x1 << MOV_INDEX_CASTLE;
			return move;
		}
		/* kingside */
		if (board->castle[board->tomove][KINGSIDE] &&
		    (src == SQUARE(COL_E,HOMEROW(board->tomove))) &&
		    (dest == SQUARE(COL_G,HOMEROW(board->tomove)))) /* "e1g1" or "e8g8" */
		{
			/* check if special squares are blocked or attacked */
			if ((castle_clearsquares[board->tomove][KINGSIDE] & board->occupied) ||
			    board_squaresareattacked(board, castle_safesquares[board->tomove][KINGSIDE], OTHERCOLOR(board->tomove)))
			{
				fprintf(stderr, "MOVE_ISLEGAL: you are unable to castle kingside\n");
				return 0;
			}
			/* here, the castle is legal */
			move |= 0x1 << MOV_INDEX_CASTLE;
			return move;
		}
		/* otherwise it must be a normal king move to be legal */
		/* we can make this move */
		if (BB_SQUARE(dest) & moves)
		{
			/* making a capture */
			if (BB_SQUARE(dest) & board->piecesofcolor[OTHERCOLOR(board->tomove)])
			{
				move |= 1 << MOV_INDEX_CAPT;
				move |= board_pieceatsquare(board, dest, NULL) << MOV_INDEX_CAPTPC;
			}
			if (board_squareisattacked(board, dest, OTHERCOLOR(board->tomove)))
			{
				fprintf(stderr, "MOVE_ISLEGAL: Can't move king into check");
				return 0;
			}
			return move;
		}
		fprintf(stderr, "MOVE_ISLEGAL: that is not a valid king move\n");
		return 0;
	}
	/* typical case piece - N B R Q */
	else
	{
		/* is this one of our legal moves? */
		if (BB_SQUARE(dest) & moves)
		{
			/* making a capture? */
			if (BB_SQUARE(dest) & board->piecesofcolor[OTHERCOLOR(board->tomove)])
			{
				move |= 1 << MOV_INDEX_CAPT;
				move |= board_pieceatsquare(board, dest, NULL) << MOV_INDEX_CAPTPC;
			}
			return move_checksuicide(board, move);
		}
		fprintf(stderr, "MOVE_ISLEGAL: that is not a valid regular-piece move\n");
		return 0;
	}
}

/**
 * Given a move on a given position (must be legal and suitably formatted for
 * applymove), check if making this move would leave the player in check.
 * Returns the move if it's good, 0 if not.
 */
static move_t move_checksuicide(board_t *board, move_t move)
{
	board_applymove(board, move);
	if (board_colorincheck(board, OTHERCOLOR(board->tomove)))
	{
		board_undomove(board, move);
		return 0;
	}
	board_undomove(board, move);
	return move;
}

/**
 * Returns the string representation of a move in computer-friendly format
 * (that is, "squarename[src]squarename[dest]promotionpiece"). Free the string
 * when you're done with it.
 */
char *move_tostring(move_t m)
{
	char *string;
	/* longest move in this format is a promotion, "a7a8Q" */
	string = malloc(6);
	string[0] = 0;
	strcat(string, squarename[MOV_SRC(m)]);
	strcat(string, squarename[MOV_DEST(m)]);
	if (MOV_PROM(m))
	{
		/* use WHITE as an index to get the uppercase */
		strcat(string, piecename[WHITE][MOV_PROMPC(m)]);
	}
	return string;
}
