BISTROMATH - A "simple" open-source chess engine by Ben Blum

Features:
- Rotated bitboard backend (board.c (documented with ASCII art in board.h))
- Evaluation function (eval.c) which considers:
	- Material and piece/square tables
	- Territory
	- Pawn structure (pawnstructure.c) - chains, backwards/isolated, passed
	- King safety - tropism, pawn shield, open files
	- Specialized endgame evaluator
- Alpha-beta search (search.c)
	- Transposition table (transposition.c)
	- Quiescence (quiescent.c)
	- Check extension
	- Killer moves
	- Iterative deepening
- xboard/ICS interface (xboard.c)
- Not yet: pondering, multithreading

bistromath plays on FICS, the Free Internet Chess Server (http://freechess.org)
under the same name. It's rated about 2300.

To run locally: ```make```, install xboard, ```xboard -fcp ./bistromath```.

Resources for chess engine design which inspired me:
- http://www2.imm.dtu.dk/pubdb/views/edoc_download.php/3267/ps/imm3267.ps
	- Academic paper with a focus on parallel search algorithms and bitboards
- http://chessprogramming.wikispaces.com/
	- Wiki with a nice introduction to most every known technique/algorithm/heuristic
- http://members.home.nl/matador/Inside%20Rebel.pdf
	- Technical details on the workings of an advanced engine
- http://www.tim-mann.org/xboard/engine-intf.html
	- xboard interface guide

I also stole the king safety evaluation heuristics from gnuchess, the opening
book from TSCP, and the move ordering strategy from Rebel.

Bistromathics itself is simply a revolutionary new way of understanding the
behaviour of numbers. Just as Albert Einstein's general relativity theory
observed that space was not an absolute but depended on the observer's movement
in time, and that time was not an absolute, but depended on the observer's
movement in space, so it is now realized that numbers are not absolute, but
depend on the observer's movement in restaurants.

[ Douglas Adams, "Life, the Universe, and Everything" ]

This document was last modified 10 Feb 2017.
