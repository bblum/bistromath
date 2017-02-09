/****************************************************************************
 * search.c - traverse the game tree looking for the best position
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
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "search.h"
#include "eval.h"
#include "quiescent.h"
#include "movelist.h"
#include "transposition.h"
#include "assert.h"

/* Constants for the search algorithm */
#define SEARCHER_INFINITY 32767
#define SEARCHER_MATE 16383

/* could be changed if you wanted to {dis,en}courage draws
 * TODO: make the evaluator also use this */
#define SEARCHER_DRAW_SCORE 0
#define VALUE_ISDRAW(v) ((v) == SEARCHER_DRAW_SCORE)

static move_t alphabeta(board_t *, int16_t, int16_t, uint8_t, uint8_t, move_t, uint8_t, uint8_t, unsigned char);

int transposition_hits, transposition_misses;
extern int regen_hits, regen_misses;

static int nodes;
static int16_t lastval;

/* used for printing shit - too lazy to headerize these guys */
#define TTYOUT_COLOR "\033[00;37m"
#define DEFAULT_COLOR "\033[00m"
#define BUF_SIZE
extern FILE *ttyout;
extern char outbuf[BUF_SIZE];
void output(char *);

/* Used for iterative deepening and replacement policy in the trans table */
static uint8_t cur_searching_depth;

/* used for iterative deepening */
volatile unsigned char timeup;

#define SEARCHER_MIN_DEPTH 4
#define SEARCHER_MAX_DEPTH 63

#define VALUE_ISMATE(v) (((v) >= SEARCHER_MATE - SEARCHER_MAX_DEPTH) || ((v) <= -(SEARCHER_MATE - SEARCHER_MAX_DEPTH)))

/* aspiration windows - first try a window of aspir_1 around prevalpha; if
 * that fails go to aspir_2; if that fails use the full -inf,+inf search */
#define SEARCHER_ASPIRATION_1  50
#define SEARCHER_ASPIRATION_2 200

/* killer moves
 *
 * Tried at each move after the hashed bestmove, before generatemoves(). A
 * killer move is basically any move by one player that produces a beta cutoff
 * when the pther player makes a move from X position; the hope here is that
 * the same refutation move will produce a cutoff in the next children of X.
 * This is likely to happen if the first player is threatened somehow, and the
 * killer will be the followthrough on that threat if player one doesn't
 * respond to it.
 *
 * note - you will want to clear all the killers in the following cases:
 * 	1) Beginning/end of getbestmove - clear all depths
 * 	2) After a node finishes its children, clear the nodes at that
 * 	   child's ply
 * If the killers are not adequately cleared, you may end up applying some
 * illegal moves at other nodes, resulting in inconsistent board state.
 * Another thing to watch out for is some killers can still be illegal - most
 * cases are eliminated by preventing captures and castles, the rest need
 * checking against board_attacksfrom() (or pushes, in the case of pawns).
 */
#define SEARCHER_USE_KILLERS
#ifdef SEARCHER_USE_KILLERS
#define SEARCHER_NUM_KILLERS 3
static move_t killers[SEARCHER_MAX_DEPTH][SEARCHER_NUM_KILLERS];
#endif

/* margins for futility pruning - how far away from the window do we need
 * to be in order to consider doing futility pruning. note that the value at
 * 0 will never be used */
#ifdef SEARCHER_FUTILITY_PRUNING
static int16_t futility_margin[3] = { 0xbeef, 250, 450 };
#endif

int lazy, nonlazy;

/* determining the type of a child from a parent (first index - parent's type)
 * given whether the first child (0 or 1, second index) has been searched */
#define PV  0
#define CUT 1
#define ALL 2
static unsigned char childnodetype[3][2] = {
	/* first child already searched:
	 * NO  YES */
	{ PV,  CUT }, /* parent was PV  */
	{ ALL, CUT }, /* parent was CUT */
	{ CUT, CUT }  /* parent was ALL */
};

/* null-move depth reduction - dependent on depth */
#define NULLMOVE_R(d) (((d) > 6) ? 3 : 2)

/* Late move reduction - we reduce after searching this many nodes given the
 * current node type (allows us to be more conservative at PV nodes) */
static uint8_t lmr_movecount[3] = { 16, 8, 8 };

void sigalrm_handler()
{
	snprintf(outbuf, BUF_SIZE-1, "SEARCHER: TIMEUP!");
	output(outbuf);
	timeup = 1;
}

/**
 * Stores the nodecount in the given int pointer if it's nonnull and the alpha
 * value in the second pointer if that's nonnull.
 */
move_t getbestmove(board_t *board, unsigned int time, int *nodecnt, int16_t *alphaval)
{
	move_t result, prevresult;
	int16_t prevalpha;
	int16_t window_low, window_high;
	char *movestr;

	/********************************************************************
	 * Setup
	 ********************************************************************/
	nodes = 0;
	timeup = 0;
	
	transposition_hits = 0; transposition_misses = 0;
	regen_hits = 0; regen_misses = 0;

	lazy = 0; nonlazy = 0;
	
	#ifdef SEARCHER_USE_KILLERS
	/* clear all killers */
	memset(killers, 0, sizeof(killers));
	#endif
	/********************************************************************
	 * Searching
	 ********************************************************************/
	/* preliminary search */
	cur_searching_depth = SEARCHER_MIN_DEPTH;
	result = alphabeta(board, -SEARCHER_INFINITY, SEARCHER_INFINITY,
	                          cur_searching_depth, 0, 0, 0, 0, PV);
	prevresult = result;
	prevalpha = lastval;
	
	/* start the iterative timer */
	signal(SIGALRM, sigalrm_handler);
	snprintf(outbuf, BUF_SIZE-1, "SEARCHER: Will search for %u seconds",
	         time);
	output(outbuf);
	alarm(time);
	
	while (!timeup && cur_searching_depth < SEARCHER_MAX_DEPTH)
	{
		/* These guys store the result of the previous depth in case
 		 * our next search times up */
		prevresult = result;
		prevalpha = lastval;

		// Don't waste time if we have a mate
		/*if (lastval >= SEARCHER_MATE)
		{
			snprintf(outbuf, BUF_SIZE-1, "SEARCHER: Mate coming up!");
			output(outbuf);
			break;
		}*/
		
		//TODO: Check if we've used up enough time to not even bother with the next search

		/* advance to the next depth */
		movestr = move_tostring(result);
		snprintf(outbuf, BUF_SIZE-1, "SEARCHER: Depth %d gives %s for %d. \t(nodes %d)\t",
		         cur_searching_depth, movestr, lastval, nodes);
		if (VALUE_ISMATE(lastval))
		{
			char str[32] = "\0";
			snprintf(str, 31, "%s mates in %d",
			         ((lastval > 0) ?
			         	((board->tomove == WHITE) ? "White" : "Black")
			         :
			         	((board->tomove == WHITE) ? "Black" : "White")),
			         ((lastval > 0) ?
			         	((SEARCHER_MATE - lastval + 1) / 2)
			         :
			         	((SEARCHER_MATE + lastval + 1) / 2)));
			strcat(outbuf, str);
		}
		output(outbuf);
		free(movestr);
		cur_searching_depth++;
		nodes = 0;
		
		window_low  = prevalpha - SEARCHER_ASPIRATION_1;
		window_high = prevalpha + SEARCHER_ASPIRATION_1;
		result = alphabeta(board, window_low, window_high,
		                   cur_searching_depth, 0, 0, 0, 0, PV);
		if (timeup) break;
		/* test if aspir_1 window failed */
		if (lastval <= window_low || lastval >= window_high)
		{
			snprintf(outbuf, BUF_SIZE-1,
			         "SEARCHER:      [%d] Window (%d,%d) failed. Trying (%d,%d)...",
			         cur_searching_depth, window_low, window_high,
			         prevalpha - SEARCHER_ASPIRATION_2,
			         prevalpha + SEARCHER_ASPIRATION_2);
			output(outbuf);
			/* try wider window */
			window_low  = prevalpha - SEARCHER_ASPIRATION_2;
			window_high = prevalpha + SEARCHER_ASPIRATION_2;
			/* re-search */
			result = alphabeta(board, window_low, window_high,
			                   cur_searching_depth, 0, 0, 0, 0, PV);
		}
		if (timeup) break;
		/* test if aspir_2 window failed */
		if (lastval <= window_low || lastval >= window_high)
		{
			snprintf(outbuf, BUF_SIZE-1,
			         "SEARCHER:      [%d] Window (%d,%d) failed. Trying (%d,%d)...",
			         cur_searching_depth, window_low, window_high,
			         -SEARCHER_INFINITY, SEARCHER_INFINITY);
			output(outbuf);
			/* use full window */
			window_low  = -SEARCHER_INFINITY;
			window_high =  SEARCHER_INFINITY;
			/* re-search */
			result = alphabeta(board, window_low, window_high,
			                   cur_searching_depth, 0, 0, 0, 0, PV);
		}
	}
	
	/********************************************************************
	 * Teardown
	 ********************************************************************/
	movestr = move_tostring(prevresult);
	snprintf(outbuf, BUF_SIZE-1, "SEARCHER: Move %s gives us score %d",
	         movestr, prevalpha);
	output(outbuf);
	snprintf(outbuf, BUF_SIZE-1, "SEARCHER: Nodes %d on final depth, hit/miss: trans %d/%d, regen %d/%d",
	         nodes, transposition_hits, transposition_misses, regen_hits, regen_misses);
	output(outbuf);
	snprintf(outbuf, BUF_SIZE-1, "SEARCHER: Lazy evaluations %d, full evaluations %d",
	         lazy, nonlazy);
	output(outbuf);
	free(movestr);
	#ifdef SEARCHER_USE_KILLERS
	/* clear killers, just in case */
	memset(killers, 0, sizeof(killers));
	#endif
	alarm(0);
	signal(SIGALRM, SIG_IGN);
	if (nodecnt != NULL)
	{
		*nodecnt = nodes;
	}
	if (alphaval != NULL)
	{
		*alphaval = prevalpha;
	}
	return prevresult;
}

/**
 * Alpha-beta search, with trans table, check extension, futility pruning
 * board      - the current node's position
 * alpha      - lower bound
 * beta       - upper bound
 * depth      - how deep we will search this node
 * ply        - how far this position is from the board's real position (used
 *              because depth can change based on extensions/reductions)
 * prevmove   - the move that was just made (used in futility pruning)
 * num_checks - how many checks have occurred so far in the search (used for
 *              check extension)
 * nodetype   - what type of node we're searching - PV, CUT, or ALL
 */
static move_t alphabeta(board_t *board, int16_t alpha, int16_t beta,
                        uint8_t depth, uint8_t ply, move_t prevmove,
                        uint8_t num_checks, uint8_t null_extended,
                        unsigned char nodetype)
{
	movelist_t moves;
	move_t curmove, returnmove;
	/* who has the move */
	int color;
	/* used for telling if the movelist we got had no non-suicide moves,
	 * also for telling what the child's node type will be */
	unsigned char children_searched;
	
	/* transposition table */
	trans_data_t trans_data;
	int trans_flag;
	/* for move ordering */
	move_t bestmove;
	
	int killer_index;
	
	nodes++;
	if (timeup)
	{
		return 0;
	}
	/********************************************************************
	 * threefold repetition/50 move - always check first
	 ********************************************************************/
	/* draw score if only even -one- prior repetition, not two. this is
	 * more efficient (taken from crafty). note that it obsoletes storing
	 * the repetition count in the trans table, but if it gets put back,
	 * transing the reps will still be essential. */
	if (board->halfmoves >= 100 || board_threefold_draw(board))
	{
		//fprintf(ttyout, "%sSEARCHER: Detected threefold repetition at depth %d%s\n",
		//        TTYOUT_COLOR, cur_searching_depth - depth, DEFAULT_COLOR);
		lastval = SEARCHER_DRAW_SCORE;
		return 0;
	}
	/********************************************************************
	 * check transposition table
	 ********************************************************************/
	trans_data = trans_get(board->hash);
	if (trans_data_valid(trans_data))
	{
		if (TRANS_SEARCHDEPTH(trans_data) >= depth &&
		    TRANS_REPS(trans_data) >= board->reps)
		{
			int16_t storedval = TRANS_VALUE(trans_data);
			
			/* direct hit! */
			if (TRANS_FLAG(trans_data) == TRANS_FLAG_EXACT)
			{
				transposition_hits++;
				lastval = storedval;
				return TRANS_MOVE(trans_data);
			}
			/* if we do this on the root node, we run the risk of
			 * getting a1a1 as the returnmove, so prevent it */
			else if (depth != cur_searching_depth)
			{
				/* we have a lower bound for the true value */
				if (TRANS_FLAG(trans_data) == TRANS_FLAG_BETA)
				{
					/* above our current window? */
					if (storedval >= beta)
					{
						transposition_hits++;
						lastval = storedval;
						return 0;
					}
					/* not above the window but inside? */
					if (storedval > alpha)
					{
						alpha = storedval;
					}
				}
				/* this means we'll have an upper bound */
				else if (TRANS_FLAG(trans_data) ==
					 TRANS_FLAG_ALPHA)
				{
					/* below current window entirely? */
					if (storedval <= alpha)
					{
						transposition_hits++;
						lastval = storedval;
						return 0;
					}
					/* inside window - cut the top off */
					if (storedval < beta)
					{
						beta = storedval;
					}
				}
			}
		}
		/* well, we can still use move-ordering information. note:
		 * we'll want to grab it regardless of the flag. if it's beta
		 * we'll get a move; if it's alpha we'll get 0; oh well */
		bestmove = TRANS_MOVE(trans_data);
	}
	else
	{
		bestmove = (move_t)0;
		transposition_misses++;
	}
	/********************************************************************
	 * terminal condition - search depth ran out
	 * futility pruning done here
	 ********************************************************************/
	if (depth == 0)
	{
		lastval = quiesce(board, alpha, beta);
		/* we time up in the middle of quiescence and get zero, and go
		 * to add it in the trans table, OH SHI-- */
		if (timeup)
		{
			return 0;
		}
		/* if we already have an entry, and this is an end condition,
		 * we don't want to replace our better entry */
		if (!trans_data_valid(trans_data))
		{
			trans_add(board->hash, (move_t)0, board->reps, lastval,
				  (board->moves - cur_searching_depth),
			          board->moves, 0, TRANS_FLAG_EXACT);
		}
		return 0;
	}
	#ifdef SEARCHER_FUTILITY_PRUNING
	/* futility pruning - prune only 1 and 2 plies from the horizon */
	else if (depth < 3)
	{
		/* if we're in check or possibly in a capture sequence, trying
		 * to prune will be too dangerous */
		if (!(board_incheck(board) || MOV_CAPT(prevmove)))
		{
			int16_t score = eval_lazy(board);
			/* check how far we are from the window */
			if ((score + futility_margin[depth] < alpha) ||
			    (score - futility_margin[depth] > beta))
			{
				lastval = quiesce(board, alpha, beta);
				return 0;
			}
		}
	}
	#endif
	/********************************************************************
	 * Null-move pruning
	 ********************************************************************/
	/* only do it if all of the following:
	 * 	we're a fail-high (CUT) node
	 * 	depth is not three or less
	 * 	we're not in the endgame
	 * 	a lazy eval fails high
	 *	we're not in check
	 */
	if ((nodetype != PV) && (depth > 3) && !eval_isendgame(board) &&
	    (eval_lazy(board) >= beta) && !board_incheck(board))
	{
		/* make null move */
		board_applymove(board, 0);
		alphabeta(board, -beta, -beta+1, depth - NULLMOVE_R(depth) - 1,
			  ply+1, 0, num_checks, null_extended, ALL);
		board_undomove(board, 0);
		
		/* failed high, we can prune */
		if (-lastval >= beta)
		{
			lastval = -lastval;
			return 0;
		}
		/* mate threat - if we do nothing here, we get mated. better
		 * search deeper. only extend this way once per line. */
		if (VALUE_ISMATE(lastval) && !null_extended)
		{
			depth++;
			null_extended = 1;
		}
	}
	/********************************************************************
	 * Main iteration over all the moves
	 ********************************************************************/
	/* set some preliminary values */
	color = board->tomove;
	children_searched = 0;
	returnmove = 0;
	trans_flag = TRANS_FLAG_ALPHA;
	/* and we're ready to go - first do hash move and killers */
	if (bestmove)
	{
		board_applymove(board, bestmove);
		/* if this puts us in check something's really wrong... */
		assert(!board_colorincheck(board, color));
		/* check extension - if this checks the opp king */
		if (board_incheck(board))
		{
			/* only extend search if >1 checks in tree */
			if (num_checks)
			{
				alphabeta(board, -beta, -alpha, depth, ply+1,
				          bestmove, num_checks+1, null_extended,
				          childnodetype[nodetype][!!children_searched]);
			}
			else
			{
				alphabeta(board, -beta, -alpha, depth-1, ply+1,
				          bestmove, num_checks+1, null_extended,
				          childnodetype[nodetype][!!children_searched]);
			}
		}
		else /* no extensions, regular search */
		{
			alphabeta(board, -beta, -alpha, depth-1, ply+1,
			          bestmove, num_checks, null_extended,
			          childnodetype[nodetype][!!children_searched]);
		}
		board_undomove(board, bestmove);
		children_searched++;

		if (-lastval > alpha)
		{
			alpha = -lastval;
			returnmove = bestmove;
			trans_flag = TRANS_FLAG_EXACT;
		}
		if (alpha >= beta)
		{
			trans_flag = TRANS_FLAG_BETA;
			goto alphabeta_after_iteration;
		}
	}
	#ifdef SEARCHER_USE_KILLERS
	/* no cutoff from the bestmove (or no bestmove); let's try killers */
	for (killer_index = 0; killer_index < SEARCHER_NUM_KILLERS; killer_index++)
	{
		move_t killer = killers[ply][killer_index];
		piece_t killerpiece;
		square_t killersrc, killerdest;
		/* check if a killer is available */
		if (killer == 0 || timeup)
		{
			break;
		}
		killerpiece = MOV_PIECE(killer);
		killersrc = MOV_SRC(killer);
		killerdest = MOV_DEST(killer);
		/* need to check if the killer is legal in this position; it's
		 * guaranteed to not be a capture or castle, so the following
		 * legality checks are made:
		 * 1) We have the given piece on that square
		 * 2) The destination square is not occupied (by either color)
		 * 3) If a sliding piece, there must be nothing in the way */
		if ((!(BB_SQUARE(killersrc) & board->pos[color][killerpiece])) ||
		    (BB_SQUARE(killerdest) & board->occupied) || /* #2 */
		    (PIECE_SLIDES(killerpiece) && /* #3 */
		     !(BB_SQUARE(killerdest) &
		       board_attacksfrom(board, killersrc, killerpiece, color))))
		{
			continue;
		}
		/* next we check if our piece can move to the destsquare */
		if (killerpiece == PAWN)
		{
			/* destination square not among the legal pushes? */
			if (!(BB_SQUARE(killerdest) &
			      board_pawnpushesfrom(board, killersrc, color)))
			{
				continue;
			}
		}
		else
		{
			/* destination square not among the legal moves? */
			if (!(BB_SQUARE(killerdest) &
			      board_attacksfrom(board, killersrc, killerpiece,
			                        color)))
			{
				continue;
			}
		}
		/* we have a pseudolegal killer move to try, good */
		board_applymove(board, killer);
		/* this might put us in check, unlike the bestmove */
		if (board_colorincheck(board, color))
		{
			board_undomove(board, killer);
			continue;
		}
		/* check extension - if this checks the opp king */
		if (board_incheck(board))
		{
			/* only extend search if >1 checks in tree */
			if (num_checks)
			{
				alphabeta(board, -beta, -alpha, depth, ply+1,
				          killer, num_checks+1, null_extended,
					  childnodetype[nodetype][!!children_searched]);
			}
			else
			{
				alphabeta(board, -beta, -alpha, depth-1, ply+1,
				          killer, num_checks+1, null_extended,
					  childnodetype[nodetype][!!children_searched]);
			}
		}
		else /* no extensions, regular search */
		{
			alphabeta(board, -beta, -alpha, depth-1, ply+1,
			          killer, num_checks, null_extended,
			          childnodetype[nodetype][!!children_searched]);
		}
		board_undomove(board, killer);
		children_searched++;
		
		if (-lastval > alpha)
		{
			alpha = -lastval;
			returnmove = killer;
			trans_flag = TRANS_FLAG_EXACT;
		}
		if (alpha >= beta)
		{
			trans_flag = TRANS_FLAG_BETA;
			goto alphabeta_after_iteration;
		}
	}
	#endif
	/* okay, no beta cutoff; let's continue... */
	board_generatemoves(board, &moves);
	while (!movelist_isempty(&moves))
	{
		if (timeup)
		{
			break;
		}
		curmove = movelist_remove_max(&moves);
		
		board_applymove(board, curmove);
		/* see if this move puts us in check */
		if (board_colorincheck(board, color))
		{
			board_undomove(board, curmove);
			continue;
		}
		/* check extension - if this checks the opp king */
		if (board_incheck(board))
		{
			/* only extend search if >1 checks in tree */
			if (num_checks)
			{
				alphabeta(board, -beta, -alpha, depth, ply+1,
				          curmove, num_checks+1, null_extended,
				          childnodetype[nodetype][!!children_searched]);
			}
			else
			{
				alphabeta(board, -beta, -alpha, depth-1, ply+1,
				          curmove, num_checks+1, null_extended,
				          childnodetype[nodetype][!!children_searched]);
			}
		}
		/* Late move reductions (LMR)
		 * The idea here is if a move occurs late in the movelist,
		 * it's probably not worth searching to a full depth (if it
		 * happens to return a value within the window, re-search with
		 * full depth). This works recursively, so strings of moves
		 * that are all Late get searched to (optimal case) 1/2 depth
		 * overall, while lines with just one Late move are searched
		 * with depth reduced by just one overall. What happens if we
		 * encounter a seemingly bad move that only pays off when
		 * searched to the full depth (i.e. sacrifice -> attack)?
		 * Typically, such lines will only have one or two Late moves
		 * in them, and we rely on the more extreme reductions on the
		 * completely nonsense lines (1/2-depth mentioned before) to
		 * possibly give us another ply so we find the sacrifice. */
		else if (children_searched > lmr_movecount[nodetype] && depth > 3 &&
		         !MOV_CAPT(curmove) && !MOV_CAPT(prevmove))
		{
			alphabeta(board, -beta, -alpha, depth-2, ply+1,
			          curmove, num_checks, null_extended,
			          childnodetype[nodetype][1]);
			/* oops, it fell within our window. full re-search */
			if (-lastval > alpha)
			{
				alphabeta(board, -beta, -alpha, depth-1, ply+1,
				          curmove, num_checks, null_extended,
				          childnodetype[nodetype][1]);
			}
		}
		else /* no extensions/reductions, regular search */
		{
			alphabeta(board, -beta, -alpha, depth-1, ply+1,
			          curmove, num_checks, null_extended,
			          childnodetype[nodetype][!!children_searched]);
		}
		board_undomove(board, curmove);
		children_searched++;
		
		if (-lastval > alpha)
		{
			alpha = -lastval;
			returnmove = curmove;
			/* we've gotten above the bottom of the window */
			trans_flag = TRANS_FLAG_EXACT;
		}
		if (beta <= alpha)
		{
			/* oops, above the top of the window */
			trans_flag = TRANS_FLAG_BETA;
			#ifdef SEARCHER_USE_KILLERS
			/* Not a killer if capture or castle */
			if (!(MOV_CAPT(curmove)) && !(MOV_CASTLE(curmove)))
			{
				/* We found a killer move! Add it in... note
				 * we will never here cause duplicate killers
				 * because we would have tested this killer
				 * earlier and skipped this */
				killer_index = 0;
				while ((killers[ply][killer_index] != 0) &&
				       (killer_index < SEARCHER_NUM_KILLERS-1))
				{
					/* move to first empty killer slot,
					 * or if full replace at the end */
					killer_index++;
				}
				killers[ply][killer_index] = curmove;
			}
			#endif
			break;
		}
	}
	movelist_destroy(&moves);
	/* now we are done iterating; cleanup and exit this node. note, this
	 * label is here so we can skip over iteration if the bestmove/killers
	 * produce a cutoff */
alphabeta_after_iteration:
	/* The transposition table will get really sad if we shit all over it
 	 * with bogus results from after we get cut off */
	if (timeup)
	{
		return 0;
	}
	#ifdef SEARCHER_USE_KILLERS
	/* okay we need to clear the killers for the children here */
	memset(killers[ply+1], 0, (SEARCHER_NUM_KILLERS * sizeof(move_t)));
	#endif
	/* check if there were no legal moves */
	if (children_searched)
	{
		/* this is the "typical case" function ending
		 * set the value so the calling function sees it properly */
		lastval = alpha;
		/* Don't add mates because they are depth-dependent. Don't add
		 * draws because they are state-dependent. */
		if (!VALUE_ISMATE(alpha) && !VALUE_ISDRAW(alpha))
		{
			/* update the trans table - here we always replace */
			trans_add(board->hash, returnmove, board->reps, alpha,
			          (board->moves + depth - cur_searching_depth),
			          board->moves, depth, trans_flag);
		}
	}
	else /* no children were searched - must be an end condition */
	{
		/* checkmate vs stalemate */
		if (board_incheck(board))
		{
			lastval = -1 * (SEARCHER_MATE - ply);
		}
		else
		{
			lastval = 0;
		}
		/* DON'T add to the transposition table here... this can screw
		 * up which mates are sooner and cause 3-rep draws - besides,
		 * if a mate is coming up and we've seen it there's no use
		 * trying to search deeper/faster */
		return 0;
	}
	/* and we're done */
	return returnmove;
}
