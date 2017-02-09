/****************************************************************************
 * eval.c - evaluation function to tell how good a position is
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
#include "eval.h"
#include "bitscan.h"
#include "popcnt.h"
#include "pawnstructure.h"
#include "attacks.h"

/* endgame starts when both sides have <= LIM_ENDGAME */
#define EVAL_LIM_ENDGAME  1600
/* use this limit when no queens on the board */
#define EVAL_LIM_ENDGAME2 2000

#define EVAL_CASTLE_BONUS 20

int16_t eval_piecevalue[6] = { 100, 300, 300, 500, 900, 0 };
int16_t eval_piecevalue_endgame[6] = { 125, 300, 300, 550, 1200, 0 };

/* Players get small point bonuses if their pieces are on good squares. If
 * white has a rook on d1, squarevalue[WHITE][ROOK][D1] will be added to
 * white's score. */
/* Differences between src/dest squares are considered when ordering moves, so
 * it is important that for any legal (src,dest) pair, abs(sqval(dest) -
 * sqval(src)) <= SOME_CONSTANT - that constant is 16 for now. (only in the
 * regular tables, not the endgame tables) */
int16_t eval_squarevalue[2][6][64] = {
	/********************************************************************
	 * WHITE
	 ********************************************************************/
	{
	/* PAWN */
	{
		 0,  0,  0,  0,  0,  0,  0,  0,
		 0,  0,  0, -4, -4,  4,  0,  0,
		 6,  2,  3,  4,  4,  3,  2,  8,
		 3,  4, 12, 12, 12,  8,  4,  3,
		 5,  8, 16, 20, 20, 16,  8,  5,
		20, 24, 24, 32, 32, 24, 24, 20,
		36, 36, 40, 40, 40, 40, 36, 36,
		 0,  0,  0,  0,  0,  0,  0,  0
	},
	/* KNIGHT */
	{
		-10, -6, -6, -6, -6, -6, -6,-10,
		 -6,  0,  0,  3,  3,  0,  0, -6,
		 -6,  0,  8,  4,  4, 10,  0, -6,
		 -6,  0,  8, 10, 10,  8,  0, -6,
		 -4,  0,  8, 10, 10,  8,  0, -4,
		 -4,  5, 12, 12, 12, 12,  5, -4,
		 -4,  0,  5,  3,  3,  5,  0, -4,
		-10, -4, -4, -4, -4, -4, -4,-10
	},
	/* BISHOP */
	{
		-6, -5, -5, -5, -5, -5, -5, -6,
		-5, 10,  5,  8,  8,  5, 10, -5,
		-5,  5,  3,  5,  5,  3,  5, -5,
		-5,  3, 10,  3,  3, 10,  3, -5,
		-5,  5, 10,  3,  3, 10,  5, -5,
		-5,  3,  8,  8,  8,  8,  3, -5,
		-5,  5,  5,  8,  8,  5,  5, -5,
		-6, -5, -5, -5, -5, -5, -5, -6
	},
	/* ROOK */
	{
		0,  3,  3,  3,  3,  3,  3,  0,
		0,  1,  2,  3,  3,  2,  1,  0,
		0,  1,  2,  3,  3,  2,  1,  0,
		0,  1,  2,  3,  3,  2,  1,  0,
		0,  1,  2,  3,  3,  2,  1,  0,
		0,  1,  2,  2,  2,  2,  1,  0,
		3,  5,  8,  8,  8,  8,  5,  3,
		0,  0,  0,  0,  0,  0,  0,  0
	},
	/* QUEEN */
	{
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  5,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0
	},
	/* KING */
	{
		  5,  5,  8,  0,  0, -5, 10, 10,
		  0,  0,  0,  0,  0,  0,  5,  5,
		  0,  0,  0, -5, -5,  0,  0,  0,
		  0,  0, -5,-10,-10, -5,  0,  0,
		  0, -5,-10,-10,-10,-10, -5,  0,
		 -5,-10,-10,-15,-15,-10,-10, -5,
		-20,-20,-20,-20,-20,-20,-20,-20,
		-20,-20,-20,-20,-20,-20,-20,-20
	}
	},
	/********************************************************************
	 * BLACK
	 ********************************************************************/
	{
	/* PAWN */
	{
		 0,  0,  0,  0,  0,  0,  0,  0,
		36, 36, 40, 40, 40, 40, 36, 36,
		20, 24, 24, 32, 32, 24, 24, 20,
		 5,  8, 16, 20, 20, 16,  8,  5,
		 3,  4, 12, 12, 12,  8,  4,  3,
		 6,  2,  3,  4,  4,  3,  2,  8,
		 0,  0,  0, -4, -4,  4,  0,  0,
		 0,  0,  0,  0,  0,  0,  0,  0
	},
	/* KNIGHT */
	{
		-10, -4, -4, -4, -4, -4, -4,-10,
		 -4,  0,  5,  3,  3,  5,  0, -4,
		 -4,  5, 12, 12, 12, 12,  5, -4,
		 -4,  0,  8, 10, 10,  8,  0, -4,
		 -6,  0,  8, 10, 10,  8,  0, -6,
		 -6,  0,  8,  4,  4, 10,  0, -6,
		 -6,  0,  0,  3,  3,  0,  0, -6,
		-10, -6, -6, -6, -6, -6, -6,-10
	},
	/* BISHOP */
	{
		-6, -5, -5, -5, -5, -5, -5, -6,
		-5,  5,  5,  8,  8,  5,  5, -5,
		-5,  5,  8,  8,  8,  8,  5, -5,
		-5,  3, 10,  5,  5, 10,  3, -5,
		-5,  5, 10,  5,  5, 10,  5, -5,
		-5,  3,  3,  5,  5,  3,  3, -5,
		-5, 10,  5,  8,  8,  5, 10, -5,
		-6, -5, -5, -5, -5, -5, -5, -6
	},
	/* ROOK */
	{
		0,  0,  0,  0,  0,  0,  0,  0,
		3,  5,  8,  8,  8,  8,  5,  3,
		0,  1,  2,  2,  2,  2,  1,  0,
		0,  1,  2,  3,  3,  2,  1,  0,
		0,  1,  2,  3,  3,  2,  1,  0,
		0,  1,  2,  3,  3,  2,  1,  0,
		0,  1,  2,  3,  3,  2,  1,  0,
		0,  3,  3,  3,  3,  3,  3,  0
	},
	/* QUEEN */
	{
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  5,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0
	},
	/* KING */
	{
		-20,-20,-20,-20,-20,-20,-20,-20,
		-20,-20,-20,-20,-20,-20,-20,-20,
		 -5,-10,-10,-15,-15,-10,-10, -5,
		  0, -5,-10,-10,-10,-10, -5,  0,
		  0,  0, -5,-10,-10, -5,  0,  0,
		  0,  0,  0, -5, -5,  0,  0,  0,
		  0,  0,  0,  0,  0,  0,  5,  5,
		  5,  5,  8,  0,  0, -5, 10, 10
	}
	}
};
int16_t eval_squarevalue_endgame[2][6][64] = {
	/********************************************************************
	 * WHITE
	 ********************************************************************/
	{
	/* PAWN */
	{
		  0,  0,  0,  0,  0,  0,  0,  0,
		-10,-10,-10,-10,-10,-10,-10,-10,
		  0,  0,  0,  0,  0,  0,  0,  0,
		 10, 10, 10, 10, 10, 10, 10, 10,
		 20, 20, 20, 20, 20, 20, 20, 20,
		 40, 40, 40, 40, 40, 40, 40, 40,
		 80, 80, 80, 80, 80, 80, 80, 80,
		  0,  0,  0,  0,  0,  0,  0,  0
	},
	/* KNIGHT */
	{
		-10, -5, -5, -5, -5, -5, -5,-10,
		 -8,  0,  0,  3,  3,  0,  0, -8,
		 -8,  0, 10,  8,  8, 10,  0, -8,
		 -8,  0,  8, 10, 10,  8,  0, -8,
		 -8,  0,  8, 10, 10,  8,  0, -8,
		 -8,  0, 12, 12, 12, 12,  0, -8,
		 -8,  0,  9,  3,  3,  9,  0, -8,
		-10, -5, -5, -5, -5, -5, -5,-10
	},
	/* BISHOP */
	{
		-8, -5, -5, -5, -5, -5, -5, -8,
		-5,  3,  5,  5,  5,  5,  3, -5,
		-5,  5,  5,  8,  8,  5,  5, -5,
		-5,  5, 10, 10, 10, 10,  5, -5,
		-5,  5, 10, 10, 10, 10,  5, -5,
		-5,  3,  8,  8,  8,  8,  3, -5,
		-5,  3,  5,  8,  8,  5,  3, -5,
		-8, -5, -5, -5, -5, -5, -5, -8
	},
	/* ROOK */
	{
		0,  3,  3,  5,  5,  3,  3,  0,
		0,  1,  2,  3,  3,  2,  1,  0,
		0,  1,  2,  3,  3,  2,  1,  0,
		0,  1,  2,  3,  3,  2,  1,  0,
		0,  1,  2,  3,  3,  2,  1,  0,
		0,  1,  2,  2,  2,  2,  1,  0,
		1,  3,  5,  5,  5,  5,  3,  1,
		0,  0,  0,  0,  0,  0,  0,  0
	},
	/* QUEEN */
	{
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0
	},
	/* KING */
	{
		-25,-15,-10,-10,-10,-10,-15,-25,
		-15, -5,  0,  0,  0,  0, -5,-15,
		-10,  0,  5, 10, 10,  5,  0,-10,
		-10,  0, 10, 15, 15, 10,  0,-10,
		 -5,  5, 15, 20, 20, 15,  5, -5,
		  0, 10, 20, 20, 20, 15, 10,  0,
		-15,  0,  5,  5,  5,  5,  0,-15,
		-25,-15,-10,-10,-10,-10,-15,-25
	}
	},
	/********************************************************************
	 * BLACK
	 ********************************************************************/
	{
	/* PAWN */
	{
		  0,  0,  0,  0,  0,  0,  0,  0,
		 80, 80, 80, 80, 80, 80, 80, 80,
		 40, 40, 40, 40, 40, 40, 40, 40,
		 20, 20, 20, 20, 20, 20, 20, 20,
		 10, 10, 10, 10, 10, 10, 10, 10,
		  0,  0,  0,  0,  0,  0,  0,  0,
		-10,-10,-10,-10,-10,-10,-10,-10,
		  0,  0,  0,  0,  0,  0,  0,  0
	},
	/* KNIGHT */
	{
		-10, -5, -5, -5, -5, -5, -5,-10,
		 -8,  0,  9,  3,  3,  9,  0, -8,
		 -8,  0, 12, 12, 12, 12,  0, -8,
		 -8,  0,  8, 10, 10,  8,  0, -8,
		 -8,  0,  8, 10, 10,  8,  0, -8,
		 -8,  0, 10,  8,  8, 10,  0, -8,
		 -8,  0,  0,  3,  3,  0,  0, -8,
		-10, -5, -5, -5, -5, -5, -5,-10
	},
	/* BISHOP */
	{
		-8, -5, -5, -5, -5, -5, -5, -8,
		-5,  3,  5,  8,  8,  5,  3, -5,
		-5,  3,  8,  8,  8,  8,  3, -5,
		-5,  5, 10, 10, 10, 10,  5, -5,
		-5,  5, 10, 10, 10, 10,  5, -5,
		-5,  5,  5,  8,  8,  5,  5, -5,
		-5,  3,  5,  5,  5,  5,  3, -5,
		-8, -5, -5, -5, -5, -5, -5, -8
	},
	/* ROOK */
	{
		0,  0,  0,  0,  0,  0,  0,  0,
		1,  3,  5,  5,  5,  5,  3,  1,
		0,  1,  2,  2,  2,  2,  1,  0,
		0,  1,  2,  3,  3,  2,  1,  0,
		0,  1,  2,  3,  3,  2,  1,  0,
		0,  1,  2,  3,  3,  2,  1,  0,
		0,  1,  2,  3,  3,  2,  1,  0,
		0,  3,  3,  5,  5,  3,  3,  0
	},
	/* QUEEN */
	{
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0
	},
	/* KING */
	{
		-25,-15,-10,-10,-10,-10,-15,-25
		-15,  0,  5,  5,  5,  5,  0,-15,
		  0, 10, 20, 20, 20, 15, 10,  0,
		 -5,  5, 15, 20, 20, 15,  5, -5,
		-10,  0, 10, 15, 15, 10,  0,-10,
		-10,  0,  5, 10, 10,  5,  0,-10,
		-15, -5,  0,  0,  0,  0, -5,-15,
		-25,-15,-10,-10,-10,-10,-15,-25,
	}
	}
};

static int16_t eval_endgame(board_t *);

/* Penalty for trapping the d- e- pawns on 2nd rank, or c-pawn with the knight
 * if there's a pawn on d4 and no pawn on e4 */
#define EVAL_BLOCKED_PAWN -25
/* used for pawn-block checking N on c3 with pawns c2 d4 !e4 */
#define BB_C2D4   (BB_SQUARE(C2) | BB_SQUARE(D4))
#define BB_C2D4E4 (BB_C2D4 | BB_SQUARE(E4))
#define BB_C7D5   (BB_SQUARE(C7) | BB_SQUARE(D5))
#define BB_C7D5E5 (BB_C7D5 | BB_SQUARE(E5))
/* reward the bishop pair and penalize the knight pair */
#define EVAL_BISHOP_PAIR 20
#define EVAL_KNIGHT_PAIR -20
/* Rooks like to see towards the other end of the board */
#define EVAL_ROOK_OPENFILE 10
#define EVAL_ROOK_OPENFILE_MULTIPLIER 3
/* bonus for having multiple rooks on the 7th - take the number of heavies
 * on the 7th, leftshift this constant by that number */
#define EVAL_ROOK_RANK7_MULTIPLIER 12
/* bonus for having a minor piece on an "outpost" - a hole in the opp's pawn
 * structure protected by one of own pawns. */
#define EVAL_OUTPOST_BONUS 32

/**************
 * King safety
 **************/
/* Points for having N pawns on rank 2/3 in front of a castled king*/
static int16_t pawncover_rank2[4] = { -80, -40, 0,   5 };
static int16_t pawncover_rank3[4] = {   0,   5, 20, 40 };
/* penalty for having no pawns on king's file */
#define EVAL_KINGFILEOPEN  -35
/* penalty for having no pawns on file adjacent to king's file */
#define EVAL_ADJACENTFILEOPEN -15

/* tropism bonus from one square to another (one square has a piece of given
 * type on it, the other has the enemy king) */
#define EVAL_TROPISM_MAX 192
/* this lookup array gives shiftamounts for distance so we don't have to idiv
 *                              0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 */
static int tropism_rice[15] = { 0, 0, 0, 1, 1, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5};
/* this lookup array gives shiftamounts for scaling by piece value
 *                                   P  N  B  R  Q  K */
static int tropism_piecescale[6] = { 3, 2, 2, 1, 0, 0 };

static int16_t tropism(square_t a, square_t b, piece_t piece)
{
	int16_t rowdist, coldist;
	/* absolute values of the difference between the two squares */
	rowdist = ROW(a) - ROW(b);
	if (rowdist < 0) { rowdist = -rowdist; }
	coldist = COL(a) - COL(b);
	if (coldist < 0) { coldist = -coldist; }
	/* score inversely proportional to distance, also scaled by how
	 * heavy the piece is - queen right up against the opp king will
	 * get like 100 points */
	return (EVAL_TROPISM_MAX >> tropism_rice[rowdist + coldist]) >>
	       tropism_piecescale[piece];
}

int eval_isendgame(board_t *board)
{
	if (board->pos[WHITE][QUEEN] | board->pos[BLACK][QUEEN])
	{
		return (board->material[WHITE] <= EVAL_LIM_ENDGAME) ||
		       (board->material[BLACK] <= EVAL_LIM_ENDGAME);
	}
	return (board->material[WHITE] <= EVAL_LIM_ENDGAME2) ||
	       (board->material[BLACK] <= EVAL_LIM_ENDGAME2);
}

/**
 * Evaluate - return a score for the given position for who's to move
 */
int16_t eval(board_t *board)
{
	int piece, square;
	bitboard_t piecepos;
	/* keeps track of {WHITE,BLACK}s score */
	int16_t score_white, score_black;
	/* king safety scores - scaled by the material on the board to
	 * encourage trading when under attack */
	int16_t ksafety_white, ksafety_black;
	square_t kingsq_white, kingsq_black; /* where's each king */
	/* counting pieces/pawns for king safety */
	int num_pieces;
	/* holes in <color>'s pawnstructure */
	bitboard_t holes_white, holes_black;

	/* use the special endgame evaluator if in the endgame */
	if (eval_isendgame(board))
	{
		return eval_endgame(board);
	}

	/* begin evaluating */
	score_white = 0; score_black = 0;
	ksafety_white = 0; ksafety_black = 0;
	kingsq_white = BITSCAN(board->pos[WHITE][KING]);
	kingsq_black = BITSCAN(board->pos[BLACK][KING]);
	
	/********************************************************************
	 * Piece/square tables
	 ********************************************************************/
	/********
	 * White
	 ********/
	/* regular pieces - pawns taken care of in pawnstructure eval */
	for (piece = 1; piece < 5; piece++)
	{
		piecepos = board->pos[WHITE][piece];
		while (piecepos)
		{
			square = BITSCAN(piecepos);
			piecepos ^= BB_SQUARE(square);
			score_white += eval_squarevalue[WHITE][piece][square];
			/* tropism scores - how close is to our/opp king? */
			//ksafety_white += tropism(square, kingsq_white, piece);
			ksafety_black -= tropism(square, kingsq_black, piece);
			/* Rooks like open files */
			if (piece == ROOK)
			{
				/* no pawns on this file */
				if (!(board->pos[WHITE][PAWN] & BB_FILE(COL(square))))
				{
					score_white += EVAL_ROOK_OPENFILE;
				}
				/* how far can we see? */
				score_white += EVAL_ROOK_OPENFILE_MULTIPLIER * POPCOUNT(colattacks[(board->occupied90 >> (8 * COL(square))) & 0xFF][ROW(square)]);
			}
		}
	}
	/* king */
	score_white += eval_squarevalue[WHITE][KING][kingsq_white];
	
	/********
	 * Black
	 ********/
	for (piece = 1; piece < 5; piece++)
	{
		piecepos = board->pos[BLACK][piece];
		while (piecepos)
		{
			square = BITSCAN(piecepos);
			piecepos ^= BB_SQUARE(square);
			score_black += eval_squarevalue[BLACK][piece][square];
			/* tropism scores - how close is to our/opp king? */
			ksafety_white -= tropism(square, kingsq_white, piece);
			//ksafety_black += tropism(square, kingsq_black, piece);
			/* Rooks like open files */
			if (piece == ROOK)
			{
				/* no pawns on this file */
				if (!(board->pos[BLACK][PAWN] & BB_FILE(COL(square))))
				{
					score_black += EVAL_ROOK_OPENFILE;
				}
				/* how far can we see? */
				score_black += EVAL_ROOK_OPENFILE_MULTIPLIER * POPCOUNT(colattacks[(board->occupied90 >> (8 * COL(square))) & 0xFF][ROW(square)]);
			}
		}
	}
	/* king */
	score_black += eval_squarevalue[BLACK][KING][kingsq_white];
	
	/********************************************************************
	 * Misc. bonuses
	 ********************************************************************/
	/* bishop/knight pair bonus/penalties */
	if (POPCOUNT(board->pos[WHITE][BISHOP]) > 1) { score_white += EVAL_BISHOP_PAIR; }
	if (POPCOUNT(board->pos[WHITE][KNIGHT]) > 1) { score_white += EVAL_KNIGHT_PAIR; }
	if (POPCOUNT(board->pos[BLACK][BISHOP]) > 1) { score_black += EVAL_BISHOP_PAIR; }
	if (POPCOUNT(board->pos[BLACK][KNIGHT]) > 1) { score_black += EVAL_KNIGHT_PAIR; }
	/* pawnstructure bonus */
	score_white += eval_pawnstructure(board, WHITE, &holes_white);
	score_black += eval_pawnstructure(board, BLACK, &holes_black);
	/* analysis of pawnstructure holes - all holes that our pawns attack
	 * and have a minor piece on them get an "outpost" bonus - don't allow
	 * bonus for outposts on the A and H files */
	/* find white's outposts */
	holes_black &= board_pawnattacks(board->pos[WHITE][PAWN], WHITE) &
	               (board->pos[WHITE][KNIGHT] | board->pos[WHITE][BISHOP]) &
	               ~(BB_FILEA | BB_FILEH);
	score_white += EVAL_OUTPOST_BONUS * POPCOUNT(holes_black);
	/* find black's outposts */
	holes_white &= board_pawnattacks(board->pos[BLACK][PAWN], BLACK) &
	               (board->pos[BLACK][KNIGHT] | board->pos[BLACK][BISHOP]) &
	               ~(BB_FILEA | BB_FILEH);
	score_black += EVAL_OUTPOST_BONUS * POPCOUNT(holes_white);
	/* pawn block penalties */
	if (((board->piecesofcolor[WHITE] ^ board->pos[WHITE][PAWN]) & BB_SQUARE(D3)) &&
	    (board->pos[WHITE][PAWN] & BB_SQUARE(D2))) /* blocked on D2 */
	{
		score_white += EVAL_BLOCKED_PAWN;
	}
	if (((board->piecesofcolor[WHITE] ^ board->pos[WHITE][PAWN]) & BB_SQUARE(E3)) &&
	    (board->pos[WHITE][PAWN] & BB_SQUARE(E2))) /* blocked on E2 */
	{
		score_white += EVAL_BLOCKED_PAWN;
	}
	if ((board->pos[WHITE][KNIGHT] & BB_SQUARE(C3)) && /* knight on C3 */
	    !((board->pos[WHITE][PAWN] ^ BB_C2D4) & BB_C2D4E4)) /* "closed" opening */
	{
		score_white += EVAL_BLOCKED_PAWN;
	}
	/* black */
	if (((board->piecesofcolor[BLACK] ^ board->pos[BLACK][PAWN]) & BB_SQUARE(D6)) &&
	    (board->pos[BLACK][PAWN] & BB_SQUARE(D7))) /* blocked on D7 */
	{
		score_black += EVAL_BLOCKED_PAWN;
	}
	if (((board->piecesofcolor[BLACK] ^ board->pos[BLACK][PAWN]) & BB_SQUARE(E6)) &&
	    (board->pos[BLACK][PAWN] & BB_SQUARE(E7))) /* blocked on E7 */
	{
		score_black += EVAL_BLOCKED_PAWN;
	}
	if ((board->pos[BLACK][KNIGHT] & BB_SQUARE(C6)) && /* knight on C6 */
	    !((board->pos[BLACK][PAWN] ^ BB_C7D5) & BB_C7D5E5)) /* "closed" defense */
	{
		score_black += EVAL_BLOCKED_PAWN;
	}
	/* rook/queen on the 7th bonus */
	score_white += EVAL_ROOK_RANK7_MULTIPLIER <<
	               POPCOUNT((board->pos[WHITE][ROOK] | board->pos[WHITE][QUEEN]) &
	                        BB_RANK7);
	score_black += EVAL_ROOK_RANK7_MULTIPLIER <<
	               POPCOUNT((board->pos[BLACK][ROOK] | board->pos[BLACK][QUEEN]) &
	                        BB_RANK2);
	
	/********************************************************************
	 * King safety - white
	 ********************************************************************/
	/* pawn shield */
	if (board->hascastled[WHITE])
	{
		ksafety_white += EVAL_CASTLE_BONUS;
		/* pawn shield one row in front of the king */
		num_pieces = POPCOUNT(kingattacks[kingsq_white] &
		                    board->pos[WHITE][PAWN] & BB_RANK2);
		ksafety_white += pawncover_rank2[num_pieces];
		/* pawn shield two rows in front of the king */
		num_pieces = POPCOUNT(kingattacks[kingsq_white + 8] &
		                    board->pos[WHITE][PAWN] & BB_RANK3);
		ksafety_white += pawncover_rank3[num_pieces];
	}
	/* open files near the king */
	if (!(board->pos[WHITE][PAWN] & BB_FILE(COL(kingsq_white))))
	{
		ksafety_white += EVAL_KINGFILEOPEN;
	}
	switch(COL(kingsq_white)) /* this idea taken from gnuchess */
	{
		case COL_A:
		case COL_E:
		case COL_F:
		case COL_G:
			if (!(board->pos[WHITE][PAWN] & BB_FILE(COL(kingsq_white) + 1)))
			{
				ksafety_white += EVAL_ADJACENTFILEOPEN;
			}
			break;
		case COL_H:
		case COL_D:
		case COL_C:
		case COL_B:
			if (!(board->pos[WHITE][PAWN] & BB_FILE(COL(kingsq_white) - 1)))
			{
				ksafety_white += EVAL_ADJACENTFILEOPEN;
			}
			break;
		default:
			break;
	}
	score_white += ksafety_white * board->material[BLACK] / 3100;
	/********************************************************************
	 * King safety - black
	 ********************************************************************/
	/* pawn shield */
	if (board->hascastled[BLACK])
	{
		ksafety_black += EVAL_CASTLE_BONUS;
		/* pawn shield one row in front of the king */
		num_pieces = POPCOUNT(kingattacks[kingsq_black] &
		                    board->pos[BLACK][PAWN] & BB_RANK7);
		ksafety_black += pawncover_rank2[num_pieces];
		/* pawn shield two rows in front of the king */
		num_pieces = POPCOUNT(kingattacks[kingsq_black - 8] &
		                    board->pos[BLACK][PAWN] & BB_RANK6);
		ksafety_black += pawncover_rank3[num_pieces];
	}
	/* open files near the king */
	if (!(board->pos[BLACK][PAWN] & BB_FILE(COL(kingsq_black))))
	{
		ksafety_black += EVAL_KINGFILEOPEN;
	}
	switch(COL(kingsq_black)) /* this idea taken from gnuchess */
	{
		case COL_A:
		case COL_E:
		case COL_F:
		case COL_G:
			if (!(board->pos[BLACK][PAWN] & BB_FILE(COL(kingsq_black) + 1)))
			{
				ksafety_black += EVAL_ADJACENTFILEOPEN;
			}
			break;
		case COL_H:
		case COL_D:
		case COL_C:
		case COL_B:
			if (!(board->pos[BLACK][PAWN] & BB_FILE(COL(kingsq_black) - 1)))
			{
				ksafety_black += EVAL_ADJACENTFILEOPEN;
			}
			break;
		default:
			break;
	}
	score_black += ksafety_black * board->material[WHITE] / 3100;
	/********************************************************************
	 * return the values
	 ********************************************************************/
	if (board->tomove == WHITE)
	{
		return (score_white + board->material[WHITE]) -
		       (score_black + board->material[BLACK]);
	}
	else
	{
		return (score_black + board->material[BLACK]) -
		       (score_white + board->material[WHITE]);
	}
}

/**
 * Special-case endgame evaluator
 */
static int16_t eval_endgame(board_t *board)
{
	int piece, square;
	bitboard_t piecepos;
	/* keeps track of {WHITE,BLACK}s score */
	int16_t score_white = 0, score_black = 0;
	int16_t difference = board->material[WHITE] -
	                 board->material[BLACK];
	
	/* draw evaluation - if no pawns are left, many possibilities
	 * note, our goal in this if block is to put the conditionals most
	 * likely to fail first, so we don't waste time saying "is the pawn's
	 * promotion square the opposite color of the bishop" when there's a
	 * queen on the board. */
	if (!(board->pos[WHITE][PAWN] | board->pos[BLACK][PAWN]))
	{
		/* absolute value */
		if (difference < 0) { difference = -difference; }
		
		/* a good heuristic to use is if the material difference is
		 * less than 400, the position is drawn */
		if (difference < 400)
		{
			return 0;
		}

		/* KNN vs K */
		else if (difference == 600)
		{
			/* check if white has the two knights */
			if ((board->material[BLACK] == 0) &&
			    (board->piecesofcolor[WHITE] ==
			     (board->pos[WHITE][KING] |
			      board->pos[WHITE][KNIGHT])))
			{
				return 0;
			}
			/* or black has the two knights */
			if ((board->material[WHITE] == 0) &&
			    (board->piecesofcolor[BLACK] ==
			     (board->pos[BLACK][KING] |
			      board->pos[BLACK][KNIGHT])))
			{
				return 0;
			}
			/* here it must be a different piece configuration */
		}
	}
	/* KB + wrong color rook pawn vs K */
	else if (difference == 400)
	{
		/* first make sure it's KB rookpawn v K */
		if ((board->material[BLACK] == 0) &&
		    (board->pos[WHITE][BISHOP] != BB(0x0)) &&
		    (board->pos[WHITE][PAWN] & (COL_A | COL_H)))
		{
			square_t pawnsquare = BITSCAN(board->pos[WHITE][PAWN]);
			square_t bishopsquare = BITSCAN(board->pos[WHITE][BISHOP]);
			square_t promotionsquare = SQUARE(COL(pawnsquare),RANK_8);
			/* check the square parities and also where's
			 * the enemy king */
			if ((PARITY(bishopsquare) != PARITY(promotionsquare)) &&
			    (kingattacks[BITSCAN(board->pos[BLACK][KING])] &
			     BB_SQUARE(promotionsquare)))
			{
				return 0;
			}
		}
		/* same thing for the other color */
		else if ((board->material[WHITE] == 0) &&
		         (board->pos[BLACK][BISHOP] != BB(0x0)) &&
		         (board->pos[BLACK][PAWN] & (COL_A | COL_H)))
		{
			square_t pawnsquare = BITSCAN(board->pos[BLACK][PAWN]);
			square_t bishopsquare = BITSCAN(board->pos[BLACK][BISHOP]);
			square_t promotionsquare = SQUARE(COL(pawnsquare),RANK_1);
			/* check the square parities and also where's
			 * the enemy king */
			if ((PARITY(bishopsquare) != PARITY(promotionsquare)) &&
			    (kingattacks[BITSCAN(board->pos[WHITE][KING])] &
			     BB_SQUARE(promotionsquare)))
			{
				return 0;
			}
		}
	}
	/* KP vs K - here we code just two cases, to facilitate the searcher;
	 * once we hit the back rank the searcher will see the draw itself.
	 * the first position is with the Ks in opposition with the pawn
	 * in between (either side to move, it's a draw); the second is
	 * with the pawn behind the winning king, in opposition (zugzwang) */
	else if (difference == 100)
	{
		/* white has the pawn */
		if (board->material[BLACK] == 0)
		{
			square_t whiteking = BITSCAN(board->pos[WHITE][KING]);
			square_t blackking = BITSCAN(board->pos[BLACK][KING]);
			square_t whitepawn = BITSCAN(board->pos[WHITE][PAWN]);
			unsigned char pawncol = COL(whitepawn);
			/* test for rook pawn draws */
			if (pawncol == COL_A || pawncol == COL_H)
			{
				/* if black king gets in front of the pawn,
				 * it's a draw no matter what */
				if ((COL(blackking) == pawncol) &&
				    (blackking > whitepawn))
				{
					return 0;
				}
				/* if the white king is in front of his pawn,
				 * but trapped by the black king, it'll end in
				 * stalemate */
				if (COL(whiteking) == pawncol)
				{
					if (pawncol == COL_A)
					{
						if (blackking - whiteking == 2)
						{
							return 0;
						}
					}
					else /* pawncol == COL_H */
					{
						if (whiteking - blackking == 2)
						{
							return 0;
						}
					}
				}
			}
			if ((blackking - whiteking == 16) &&
			    (ROW(blackking) != RANK_8))
			{
				if (whitepawn - whiteking == 8)
				{
					return 0;
				}
				else if ((whiteking - whitepawn == 8) &&
					 (board->tomove == WHITE))
				{
					return 0;
				}
			}	
		}
		/* black has the pawn */
		else if (board->material[WHITE] == 0)
		{
			square_t whiteking = BITSCAN(board->pos[WHITE][KING]);
			square_t blackking = BITSCAN(board->pos[BLACK][KING]);
			square_t blackpawn = BITSCAN(board->pos[BLACK][PAWN]);
			unsigned char pawncol = COL(blackpawn);
			/* test for rook pawn draws */
			if (pawncol == COL_A || pawncol == COL_H)
			{
				/* if black king gets in front of the pawn,
				 * it's a draw no matter what */
				if ((COL(whiteking) == pawncol) &&
				    (whiteking < blackpawn))
				{
					return 0;
				}
				/* if the white king is in front of his pawn,
				 * but trapped by the black king, it'll end in
				 * stalemate */
				if (COL(blackking) == pawncol)
				{
					if (pawncol == COL_A)
					{
						if (whiteking - blackking == 2)
						{
							return 0;
						}
					}
					else /* pawncol == COL_H */
					{
						if (blackking - whiteking== 2)
						{
							return 0;
						}
					}
				}
			}
			if ((blackking - whiteking == 16) &&
			    (ROW(whiteking) != RANK_1))
			{
				if (blackking - blackpawn == 8)
				{
					return 0;
				}
				else if ((blackpawn - blackking == 8) &&
					 (board->tomove == BLACK))
				{
					return 0;
				}
			}	
		}
	}
	
	/* calculate piece/square totals */
	for (piece = 0; piece < 6; piece++)
	{
		piecepos = board->pos[WHITE][piece];
		while (piecepos)
		{
			square = BITSCAN(piecepos);
			piecepos ^= BB_SQUARE(square);
			score_white += eval_piecevalue_endgame[piece] +
			               eval_squarevalue_endgame[WHITE][piece][square];
		}
	}
	for (piece = 0; piece < 6; piece++)
	{
		piecepos = board->pos[BLACK][piece];
		while (piecepos)
		{
			square = BITSCAN(piecepos);
			piecepos ^= BB_SQUARE(square);
			score_black += eval_piecevalue_endgame[piece] +
			               eval_squarevalue_endgame[BLACK][piece][square];
		}
	}
	
	/* return the values - we revalued the pieces to make pawns more
	 * important so board->material is inaccurate here */
	if (board->tomove == WHITE)
	{
		return score_white - score_black;
	}
	else
	{
		return score_black - score_white;
	}
}

/**
 * Lazy evaluator
 */
int16_t eval_lazy(board_t *board)
{
	return board->material[board->tomove] -
	       board->material[OTHERCOLOR(board->tomove)];
}
