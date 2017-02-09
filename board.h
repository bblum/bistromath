/****************************************************************************
 * board.h - rotated bitboard library; move generation and board state
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
#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include "util/linkedlist_u64.h"
#include "movelist.h"

/**
 * Piece/color constants. Used as indices to get the position bitboard of a
 * given piece of a given color, among other things.
 */
#define WHITE	0
#define BLACK	1

/* The color not thise one. Equivalent to "x == WHITE ? BLACK : WHITE" */
#define OTHERCOLOR(x) (1^(x))
/* This player's home-rank (0 for white, 7 for black) */
#define HOMEROW(x) ((x == WHITE) ? 0 : 7)

/* Piece types */
#define PAWN	0x0
#define KNIGHT	0x1
#define BISHOP	0x2
#define ROOK	0x3
#define QUEEN	0x4
#define KING	0x5
#define PIECE_SLIDES(p) (((p) > KNIGHT) && ((p) < KING))
typedef uint8_t piece_t;

extern char *piecename[2][6];

/**
 * Square constants. 6 lowest bits: |0|0|r|o|w|c|o|l|
 * These can serve as indexes to get bits out of a bitboard. Also used to get
 * masks for sliding pieces by row/etc
 */
#define A1 0x00
#define B1 0x01
#define C1 0x02
#define D1 0x03
#define E1 0x04
#define F1 0x05
#define G1 0x06
#define H1 0x07
#define A2 0x08
#define B2 0x09
#define C2 0x0a
#define D2 0x0b
#define E2 0x0c
#define F2 0x0d
#define G2 0x0e
#define H2 0x0f
#define A3 0x10
#define B3 0x11
#define C3 0x12
#define D3 0x13
#define E3 0x14
#define F3 0x15
#define G3 0x16
#define H3 0x17
#define A4 0x18
#define B4 0x19
#define C4 0x1a
#define D4 0x1b
#define E4 0x1c
#define F4 0x1d
#define G4 0x1e
#define H4 0x1f
#define A5 0x20
#define B5 0x21
#define C5 0x22
#define D5 0x23
#define E5 0x24
#define F5 0x25
#define G5 0x26
#define H5 0x27
#define A6 0x28
#define B6 0x29
#define C6 0x2a
#define D6 0x2b
#define E6 0x2c
#define F6 0x2d
#define G6 0x2e
#define H6 0x2f
#define A7 0x30
#define B7 0x31
#define C7 0x32
#define D7 0x33
#define E7 0x34
#define F7 0x35
#define G7 0x36
#define H7 0x37
#define A8 0x38
#define B8 0x39
#define C8 0x3a
#define D8 0x3b
#define E8 0x3c
#define F8 0x3d
#define G8 0x3e
#define H8 0x3f
typedef uint8_t square_t;

/* Converts int to string. Better than using malloc for 3-length strings. */
extern char *squarename[64];

#define RANK_1 0
#define RANK_2 1
#define RANK_3 2
#define RANK_4 3
#define RANK_5 4
#define RANK_6 5
#define RANK_7 6
#define RANK_8 7
#define COL_A 0
#define COL_B 1
#define COL_C 2
#define COL_D 3
#define COL_E 4
#define COL_F 5
#define COL_G 6
#define COL_H 7

/* converting square <--> (column,row) */
#define COL(s)      ((s) & 0x7)          /* 0<=s<64 */
#define ROW(s)      ((s) >> 3)           /* 0<=s<64 */
#define SQUARE(c,r) ((square_t)((c) | ((r) << 3)))   /* 0<=c,r<8 */
/* what color is the square - 1 or 0, not necessarily BLACK or WHITE */
#define PARITY(s)   ((COL(s) + ROW(s)) & 0x1)

/* Used mostly as array indices, especially for castling */
#define QUEENSIDE 0
#define KINGSIDE  1
/* Given the side of the board, which column will the king end up on */
#define CASTLE_DEST_COL(x) (((x) == QUEENSIDE) ? COL_C : COL_G)

/* Constants for board_mated return values */
#define BOARD_CHECKMATED 1
#define BOARD_STALEMATED 2

/**
 * move_t will contain a src square (lowest 6 bits), a dest square (next 6
 * bits), and several flags - is check, isenpassant, iscapture (and what
 * piece type is captured), ispromotion (and what is promoted to), and
 * also which color made the move.
 * 000000  JJJ  III  HHH  G  F  E  D  C  BBBBBB AAAAAA
 *      26   23   20   17 16 15 14 13 12      6      0
 * A: Source square
 * B: Dest square
 * C: Color (which side is making this move)
 * D: Castle
 * E: En passant
 * F: Capture
 * G: Promotion
 * H: Captured piece type (color will be the opposite of C)
 * I: Promotion result piece type (color indicated by C)
 * J: Which piece type is moving (color indicated by C)
 * 0: Unused bits
 * Note: Bits 25 and up are used for the repetition count in the transposition
 * table; see transposition.{h,c}
 */
typedef uint32_t move_t;
#define MOV_INDEX_SRC     0
#define MOV_INDEX_DEST    6
#define MOV_INDEX_COLOR  12
#define MOV_INDEX_CASTLE 13
#define MOV_INDEX_EP     14
#define MOV_INDEX_CAPT   15
#define MOV_INDEX_PROM   16
#define MOV_INDEX_CAPTPC 17
#define MOV_INDEX_PROMPC 20
#define MOV_INDEX_PIECE  23
#define MOV_INDEX_UNUSED 26

#define MOV_SRC(m)    ((square_t)(((m) >> MOV_INDEX_SRC) & 0x3f))
#define MOV_DEST(m)   ((square_t)(((m) >> MOV_INDEX_DEST) & 0x3f))
#define MOV_COLOR(m)  (((m) >> MOV_INDEX_COLOR) & 0x1)
#define MOV_CASTLE(m) (((m) >> MOV_INDEX_CASTLE) & 0x1)
#define MOV_EP(m)     (((m) >> MOV_INDEX_EP) & 0x1)
#define MOV_CAPT(m)   (((m) >> MOV_INDEX_CAPT) & 0x1)
#define MOV_PROM(m)   (((m) >> MOV_INDEX_PROM) & 0x1)
#define MOV_CAPTPC(m) ((piece_t)(((m) >> MOV_INDEX_CAPTPC) & 0x7))
#define MOV_PROMPC(m) ((piece_t)(((m) >> MOV_INDEX_PROMPC) & 0x7))
#define MOV_PIECE(m)  ((piece_t)(((m) >> MOV_INDEX_PIECE) & 0x7))

/**
 * 64-bit representation of a board's state for one set of piece, also used
 * for representing attacks, regions of the board, ...
 */
typedef uint64_t bitboard_t;

/**
 * Zobrist hash key
 */
typedef uint64_t zobrist_t;

/* Upper bound on how many halfmoves a game should take. Memory is cheap. */
#define HISTORY_STACK_SIZE 2048

/**
 * Nonrecomputable state stored on the history stack when a move is made
 */
typedef struct {
	zobrist_t hash;
	bitboard_t attackedby[2];
	unsigned char castle[2][2];
	square_t ep;
	unsigned char halfmoves;
	unsigned char reps;
	move_t move;
} history_t;

/****************************************************************************
 * Representation of an entire board state.
 * The pos[][] array represents standard normal-oriented setups, accessed by
 * piece and color. Normally oriented bitboards look like this:
 *
 * (64-bit format by square)
 * H8 G8 F8 E8 D8 C8 B8 A8 H7 G7 ... A2 H1 G1 F1 E1 D1 C1 B1 A1
 * 63 62 61 60 59 58 57 56 55 54 ...  8  7  6  5  4  3  2  1  0
 *
 * (8x8 format by index (from the least significant bit))
 *    -------------------------
 * 8 | 56 57 58 59 60 61 62 63 |
 * 7 |               ... 54 55 |
 *   |           ...           |
 * 3 | 16 17 ...               |
 * 2 |  8  9 10 11 12 13 14 15 |
 * 1 |  0  1  2  3  4  5  6  7 |
 *    -------------------------
 *      a  b  c  d  e  f  g  h
 *
 * The n-th index row (0 for 1, 7 for 8) can be extracted by
 * "(bitboard >> (8*n)) & 0xFF". The rotated boards are slightly different.
 * Rot90, used for rook attacks, looks like this:
 *
 * H8 H7 H6 H5 H4 H3 H2 H1 G8 G7 ... B1 A8 A7 A6 A5 A4 A3 A2 A1
 * 63 62 61 60 59 58 57 56 55 54 ...  8  7  6  5  4  3  2  1  0
 *
 *    -------------------------
 * h | 56 57 58 59 60 61 62 63 |
 * g |               ... 54 55 |
 *   |           ...           |
 * c | 16 17 ...               |
 * b |  8  9 10 11 12 13 14 15 |
 * a |  0  1  2  3  4  5  6  7 |
 *    -------------------------
 *      1  2  3  4  5  6  7  8
 *
 * So the n-th index file (0 for a, 7 for h) can be obtained easily. Note that
 * "rot90" is a lie, it's actually flipped across the a1-h8 diagonal. For
 * bishops, we rotate so squares on a single diagonal are adjacent.
 *                rot45:                                rot315:
 *                 ,--,                                8 ,--,
 *               ,' 63 ',                            7 ,' 63 ',
 *             ,' 61  62 ',                        6 ,' 61  62 ',
 *           ,' 58  59  60 ',                    5 ,' 58  59  60 ',
 *         ,' 54  55  56  57 ',                4 ,' 54  55  56  57 ',
 *       ,' 49  50  51  52  53 ',            3 ,' 49  50  51  52  53 ',
 *     ,' 43  44  45  46  47  48 ',        2 ,' 43  44  45  46  47  48 ',
 *   ,' 36  37  38  39  40  41  42 ',    1 ,' 36  37  38  39  40  41  42 ',
 *  ( 28  29  30  31  32  33  34  35 )    ( 28  29  30  31  32  33  34  35 )
 * 8 ', 21  22  23  24  25  26  27 ,' h  a ', 21  22  23  24  25  26  27 ,'
 *   7 ', 15  16  17  18  19  20 ,' g      b ', 15  16  17  18  19  20 ,'
 *     6 ', 10  11  12  13  14 ,' f          c ', 10  11  12  13  14 ,'
 *       5 ', 06  07  08  09 ,' e              d ', 06  07  08  09 ,'
 *         4 ', 03  04  05 ,' d                  e ', 03  04  05 ,'
 *           3 ', 01  02 ,' c                      f ', 01  02 ,'
 *             2 ', 00 ,' b                          g ', 00 ,'
 *               1 '--' a                              h '--'
 * Diagonals are "numbered" 0 to 14 (15 in total); in each diagram the diag
 * on the bottom will be #0, the major diagonal will be #7, and the top #14.
 * See precomputed.h for values and macros relating to this implementation.
 ****************************************************************************/
typedef struct {
	/* These arrays contain basic information about where the pieces are
	 * on the board. Used generally for masking with attack bitboards. */
	bitboard_t pos[2][6];        /* Positions of each PIECE of COLOR */
	bitboard_t piecesofcolor[2]; /* All squares occupied by COLOR */
	bitboard_t attackedby[2];    /* All squares attacked by COLOR */
	/* Maintain the occupancy status of every square on the board in four
	 * rotated boards, for easy generation of sliding attacks. */
	bitboard_t occupied;
	bitboard_t occupied90;
	bitboard_t occupied45;
	bitboard_t occupied315;
	/* The zobrist hash key for the current position */
	zobrist_t hash;
	/* Stores nonrecomputable state for undo. Index into with ->moves. */
	history_t history[HISTORY_STACK_SIZE];
	/* Various flags relating to the current position. Note for the ep
	 * flag that the capture square will always be on the 3rd (2-index)
	 * row, or the 6th (5-index) row. A value of zero means no enpassant
	 * is possible in this position. */
	square_t ep;                 /* The square on which an ep capture
	                              * could occur */
	unsigned char castle[2][2];  /* Can {WHITE,BLACK} castle {KINGSIDE,
	                              * QUEENSIDE} */
	unsigned char hascastled[2]; /* has {WHITE,BLACK} castled yet */
	unsigned char tomove;        /* Who has the move (WHITE, BLACK) */
	unsigned char halfmoves;     /* The halfmove clock. A draw occurs when
	                              * this hits 100. */
	unsigned int moves;          /* The game move clock. This also counts
	                              * halfmoves, but is never reset. */
	unsigned char reps;          /* how many times has this position
	                              * appeared before? if 2, draw */
	int16_t material[2];         /* keeps track of material; see eval.c */
} board_t;

/****************************************************************************
 * Some special macros and precomputed bitboards
 ****************************************************************************/
#define BB(b) ((bitboard_t)(b))
#define BB_FILEA BB(0x0101010101010101)
#define BB_FILEB BB(0x0202020202020202)
#define BB_FILEC BB(0x0404040404040404)
#define BB_FILED BB(0x0808080808080808)
#define BB_FILEE BB(0x1010101010101010)
#define BB_FILEF BB(0x2020202020202020)
#define BB_FILEG BB(0x4040404040404040)
#define BB_FILEH BB(0x8080808080808080)
#define BB_FILE(c) (BB_FILEA << (c))
#define BB_RANK1 BB(0x00000000000000ff)
#define BB_RANK2 BB(0x000000000000ff00)
#define BB_RANK3 BB(0x0000000000ff0000)
#define BB_RANK4 BB(0x00000000ff000000)
#define BB_RANK5 BB(0x000000ff00000000)
#define BB_RANK6 BB(0x0000ff0000000000)
#define BB_RANK7 BB(0x00ff000000000000)
#define BB_RANK8 BB(0xff00000000000000)
#define BB_RANK(r) (BB_RANK1 << (8 * (r)))
#define BB_HALF_RANKS(s) (((s) == WHITE) ? BB(0x00000000ffffffff) : BB(0xffffffff00000000))
#define BB_HALF_FILES(s) (((s) == QUEENSIDE) ? BB(0x0f0f0f0f0f0f0f0f) : BB(0xf0f0f0f0f0f0f0f0))


/* pawnstructure needs these */
extern bitboard_t bb_adjacentcols[8];
extern bitboard_t bb_passedpawnmask[2][64];

/* BB_SQUARE: Get a bitboard with only one bit set, on a specified square
 * Bb_ALLEXCEPT: The bitwise negation of BB_SQUARE(x) */
//#define BB_SHIFTFLIP /* prefer shift/flip to cached memory lookups */
#ifdef BB_SHIFTFLIP
	#define BB_SQUARE(x)    BB((BB(0x1))<<(x))
	#define BB_ALLEXCEPT(x) BB(~(BB_SQUARE(x)))
#else
	#define BB_SQUARE(x)    bb_square[x]
	#define BB_ALLEXCEPT(x) bb_allexcept[x]
	extern bitboard_t bb_square[64];
	extern bitboard_t bb_allexcept[64];
#endif

/* If you are {white,black}, you ep-capture with a pawn on rank {5,4} */
#define BB_EP_FROMRANK(c) (((c) == WHITE) ? BB_RANK5 : BB_RANK4)
/* If you are {white,black}, your pawn will be ep-captured on {3,6} */
#define BB_EP_TORANK(c) (((c) == WHITE) ? BB_RANK3 : BB_RANK6)

/****************************************************************************
 * Function declarations
 ****************************************************************************/
void init_zobrist();
void zobrist_gen(board_t *);

board_t *board_init();
void board_destroy(board_t *);
char *board_fen(board_t *);
piece_t board_pieceatsquare(board_t *, square_t, unsigned char *);
int board_incheck(board_t *);
int board_colorincheck(board_t *, unsigned char);
int board_mated(board_t *);
int board_pawnpassed(board_t *, square_t, unsigned char);
int board_squareisattacked(board_t *, square_t, unsigned char);
int board_squaresareattacked(board_t *, bitboard_t, unsigned char);
bitboard_t board_attacksfrom(board_t *, square_t, piece_t, unsigned char);
bitboard_t board_pawnpushesfrom(board_t *, square_t, unsigned char);
void board_generatemoves(board_t *, movelist_t *);
void board_generatecaptures(board_t *, movelist_t *);
void board_applymove(board_t *, move_t);
void board_undomove(board_t *, move_t);
bitboard_t board_pawnattacks(bitboard_t pawns, unsigned char);
int board_threefold_draw(board_t *);

move_t move_fromstring(char *);
move_t move_islegal(board_t *, char *);
char *move_tostring(move_t);

#endif
