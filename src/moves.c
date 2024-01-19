/*************

sean mcmillen
ronald yorgason

CS410 Combinatorial Games -- Spring 2002

amazons player. 

amazons -h for options

Project URL:     http://www.yorgalily.org/amazons/

***************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <assert.h>
#include <ctype.h>
#include <sys/poll.h>

#include "amazons.h"
#include "unit-test.h"


#define HEVAL     heval  // for testing differant hevals
#define MINDEPTH  1
#define VERBOSE   0
#define TIMED     1      // 1 if TIMEOUT is in effect, 0 otherwise

// macros

//#define MIN(a,b)    (a < b ? a : b)
//#define MAX(a,b)    (a > b ? a : b)
#define MMSET(p,q)    ((p) == 1 ? SHRT_MIN+q : SHRT_MAX-q)



/* local prototypes */
Move mtdf(State *s, int guess, int tdepth);


// some globals 
int onmove;
int nodes;                // statitics gatherer
int rawnodes;             // statitics gatherer
int hevalz;               // statitics gatherer
int hits;                 // statitics gatherer

int     ok;		   // global, if 0 then timeout condition 
time_t  start;             // global, start of search, used for timeout check
void    (*call_out)(void); // Called every nth loop during the search.

struct options options;
Game_states  states;


// command line flags -- set defaults here
#if 0
int white_player = FALSE;
int black_player = FALSE;
int print_statistics = FALSE;
int TIMEOUT = 5;
int MAXDEPTH = 20;
int MAXWIDTH = 5000;
#endif


struct {
   int  dx;
   int  dy;
} directions[] = {
   { 0,  1},	/* N */
   { 1,  1},	/* NE */
   { 1,  0},	/* E */
   { 1, -1},	/* SE */
   { 0, -1},	/* S */
   {-1, -1},	/* SW */
   {-1,  0},	/* W */
   {-1,  1},	/* NW */
};


/* ================================================================ */
/*                        Evaluation function                       */


/* FIXME: Move to eval.c */


static int countobst(State *s);


int obst_heval(State *s)
{
   int moves;

   int t1;
   int t2;
   int turn;

   if (onmove > 10)
      return heval(s);

   turn = s->turn;

   s->turn = WHITE;

   t1 =  countobst(s);

   s->turn = BLACK;

   t2 =  countobst(s);

   s->turn = turn;

   moves = t1 - t2;
   return moves;
}


static int
countobst(State *s)
{
   int i,j;
   int m=0, p=0;
   int xo=0, yo=0;
   int axo=0, ayo=0;
   int dir, adir;

   uchar *queens_x;
   uchar *queens_y;
   ull *board;

   int queen = 0;


   if(s->turn == WHITE)
     {
      queens_x = s->queens_x[WHITE];
      queens_y = s->queens_y[WHITE];
      board = s->white_bd;
     }
   else
     {
      queens_x = s->queens_x[BLACK];
      queens_y = s->queens_y[BLACK];
      board = s->black_bd;
     }

   // Loop through all queens
   for (queen = 0; queen < QUEENS; queen++)
     {
      for (dir = 0; dir < 8; dir++)
       	{
	 xo = directions[dir].dx;
	 yo = directions[dir].dy;

	 for (i=1; i < 10; i++) 
	   {
	    // out of bounds
	    if (queens_x[queen] + xo*i > 9 ||
		queens_y[queen] + yo*i > 9 ||
		queens_x[queen] + xo*i < 0 ||
		queens_y[queen] + yo*i < 0)
	      {
	       // count obst
	       for (adir = 0; adir < 8; adir++)
		 {
		  axo = directions[adir].dx;
		  ayo = directions[adir].dy;

		  if (dir == adir) continue;

		  for (j=1; j < 2; j++) 
		    {
		     if (queens_x[queen] + axo*j > 9 ||
			 queens_y[queen] + ayo*j > 9 ||
			 queens_x[queen] + axo*j < 0 ||
			 queens_y[queen] + ayo*j < 0)
		       {
		       	break;
		       }

		     if (test(s, queens_x[queen]+axo*j, 
				 queens_y[queen]+ayo*j))
		       {
		       	break;
		       }
		     m+=1;
		    }
		 }
	       break;
	      }

	    // hit a wall
	    if (test(s, queens_x[queen]+xo*i, queens_y[queen]+yo*i))
	      {
	       for (adir = 0; adir < 8; adir++)
		 {
		  axo = directions[adir].dx;
		  ayo = directions[adir].dy;

		  if (dir == adir) continue;

		  for (j=1; j < 2; j++) 
		    {
		     if (queens_x[queen] + axo*j > 9 ||
			     queens_y[queen] + ayo*j > 9 ||
			     queens_x[queen] + axo*j < 0 ||
			     queens_y[queen] + ayo*j < 0)
		       {
		       	break;
		       }

		     if (test(s, queens_x[queen]+axo*j, 
				 queens_y[queen]+ayo*j))
		       {
		       	break;
		       }
		     m+=1;
		    }
		 }
	       break;
	      }

	    m+=2;
	    /*
	       bitmap_flip(board, queens_x[queen], queens_y[queen]);
	       queens_x[queen] += i*xo;
	       queens_y[queen] += i*yo;
	       bitmap_flip(board, queens_x[queen], queens_y[queen]);

	    // look for arrow moves.
	    // order: N NE E SE S SW W NW 
	    bitmap_flip(board, queens_x[queen], queens_y[queen]);
	    queens_x[queen] -= i*xo;
	    queens_y[queen] -= i*yo;
	    bitmap_flip(board, queens_x[queen], queens_y[queen]);
	     */
	   }
       	}
     }
   return m;
}

// heuristic function
int sean_heval(State *s)
{
   int moves;

   int t1;
   int t2;
   int turn;

   turn = s->turn;

   s->turn = WHITE;

   t1 =  state_gen_moves(s, NULL);

   s->turn = BLACK;

   t2 =  state_gen_moves(s, NULL);

   s->turn = turn;

   moves = t1 - t2;
   return moves;
}


/* ================================================================ */
/*                         Handling a State                         */


/* set up the state with the starting position.
 */
int
state_init(State *s)
{
   int i;

   s->blocks_bd[0] = s->blocks_bd[1] = 0;
   s->white_bd[0] = s->white_bd[1] = 0;
   s->black_bd[0] = s->black_bd[1] = 0;

   s->turn = WHITE;

   for (i=0; i < 4; i++)
     {
      s->queens_x[WHITE][i] = 0;
      s->queens_y[WHITE][i] = 0;
      s->queens_x[BLACK][i] = 0;
      s->queens_y[BLACK][i] = 0;
     }

   // some lines to trip things up

   //    bitmap_flip(s->blocks_bd, 0,2);
   //    bitmap_flip(s->blocks_bd, 1,2);
   //    bitmap_flip(s->blocks_bd, 2,2);
   //    bitmap_flip(s->blocks_bd, 3,2);
   //    bitmap_flip(s->blocks_bd, 4,2);
   //    bitmap_flip(s->blocks_bd, 5,2);
   //    bitmap_flip(s->blocks_bd, 6,2);
   //    bitmap_flip(s->blocks_bd, 7,2);
   //    bitmap_flip(s->blocks_bd, 8,3);
   //    bitmap_flip(s->blocks_bd, 9,2);

   // place queen on the board
   bitmap_flip(s->white_bd, 6, 0);
   s->queens_x[WHITE][0] = 6;
   s->queens_y[WHITE][0] = 0;

   bitmap_flip(s->white_bd, 3, 0);
   s->queens_x[WHITE][1] = 3;
   s->queens_y[WHITE][1] = 0;

   bitmap_flip(s->white_bd, 0, 3);
   s->queens_x[WHITE][2] = 0;
   s->queens_y[WHITE][2] = 3;

   bitmap_flip(s->white_bd,9,3);
   s->queens_x[WHITE][3] = 9;
   s->queens_y[WHITE][3] = 3;

   bitmap_flip(s->black_bd, 3, 9);
   s->queens_x[BLACK][0] = 3;
   s->queens_y[BLACK][0] = 9;

   bitmap_flip(s->black_bd, 6, 9);
   s->queens_x[BLACK][1] = 6;
   s->queens_y[BLACK][1] = 9;

   bitmap_flip(s->black_bd, 0, 6);
   s->queens_x[BLACK][2] = 0;
   s->queens_y[BLACK][2] = 6;

   bitmap_flip(s->black_bd,9,6);
   s->queens_x[BLACK][3] = 9;
   s->queens_y[BLACK][3] = 6;


   return 0;
}


/* test init function, used to set up potentially interesting 
 * board states. Change call in main of init to x_init to put into
 * action.
 */
int
x_init(State *s)
{
   int i;

   s->blocks_bd[0] = s->blocks_bd[1] = 0;
   s->white_bd[0]  = s->white_bd[1]  = 0;
   s->black_bd[0]  = s->black_bd[1]  = 0;

   s->turn = WHITE;

   for (i=0; i < 4; i++)
     {
      s->queens_x[WHITE][i] = 0;
      s->queens_y[WHITE][i] = 0;
      s->queens_x[BLACK][i] = 0;
      s->queens_y[BLACK][i] = 0;
     }

   // place queen on the board
   bitmap_flip(s->white_bd, 0, 0);
   s->queens_x[WHITE][0] = 0;
   s->queens_y[WHITE][0] = 0;

   bitmap_flip(s->white_bd, 1, 0);
   s->queens_x[WHITE][1] = 1;
   s->queens_y[WHITE][1] = 0;

   bitmap_flip(s->white_bd, 0, 1);
   s->queens_x[WHITE][2] = 0;
   s->queens_y[WHITE][2] = 1;

   bitmap_flip(s->white_bd, 6, 0);
   s->queens_x[WHITE][3] = 6;
   s->queens_y[WHITE][3] = 0;


   bitmap_flip(s->black_bd, 8, 0);
   s->queens_x[BLACK][0] = 8;
   s->queens_y[BLACK][0] = 0;

   bitmap_flip(s->black_bd, 2, 0);
   s->queens_x[BLACK][1] = 2;
   s->queens_y[BLACK][1] = 0;

   bitmap_flip(s->black_bd, 2, 1);
   s->queens_x[BLACK][2] = 2;
   s->queens_y[BLACK][2] = 1;

   bitmap_flip(s->black_bd, 2, 2);
   s->queens_x[BLACK][3] = 2;
   s->queens_y[BLACK][3] = 2;

   /*
      bitmap_flip(s->blocks_bd, 1,1);
      bitmap_flip(s->blocks_bd, 1,2);
      bitmap_flip(s->blocks_bd, 0,2);
    */

   bitmap_flip(s->blocks_bd, 3,0);
   bitmap_flip(s->blocks_bd, 3,1);
   bitmap_flip(s->blocks_bd, 3,2);
   bitmap_flip(s->blocks_bd, 3,3);
   bitmap_flip(s->blocks_bd, 0,3);
   bitmap_flip(s->blocks_bd, 1,3);
   bitmap_flip(s->blocks_bd, 2,3);

   bitmap_flip(s->blocks_bd, 4,3);
   bitmap_flip(s->blocks_bd, 4,4);
   bitmap_flip(s->blocks_bd, 5,4);
   bitmap_flip(s->blocks_bd, 6,4);
   bitmap_flip(s->blocks_bd, 7,4);
   bitmap_flip(s->blocks_bd, 8,4);
   bitmap_flip(s->blocks_bd, 9,4);

   /*
      bitmap_flip(s->blocks_bd, 4,2);
      bitmap_flip(s->blocks_bd, 5,2);
      bitmap_flip(s->blocks_bd, 6,2);
      bitmap_flip(s->blocks_bd, 7,2);
      bitmap_flip(s->blocks_bd, 8,2);
      bitmap_flip(s->blocks_bd, 9,2);
    */

   /*
      bitmap_flip(s->blocks_bd, 4,0);
      bitmap_flip(s->blocks_bd, 4,1);
      bitmap_flip(s->blocks_bd, 5,0);
      bitmap_flip(s->blocks_bd, 5,1);
    */


   /*
      bitmap_flip(s->blocks_bd, 5,0);
      bitmap_flip(s->blocks_bd, 5,1);
      bitmap_flip(s->blocks_bd, 6,1);
      bitmap_flip(s->blocks_bd, 7,0);
      bitmap_flip(s->blocks_bd, 7,1);
      bitmap_flip(s->blocks_bd, 8,1);
      bitmap_flip(s->blocks_bd, 9,1);
      bitmap_flip(s->blocks_bd, 9,0);
    */

   return 0;
}


/* Copy a State.
 *
 * FIXME: Couldn't this just be a simple memcpy()?
 */

void
state_copy(State *s_old, State *s_new)
{
   int i;

   s_new->white_bd[0] = s_old->white_bd[0];
   s_new->white_bd[1] = s_old->white_bd[1];

   s_new->blocks_bd[0] = s_old->blocks_bd[0];
   s_new->blocks_bd[1] = s_old->blocks_bd[1];

   s_new->black_bd[0] = s_old->black_bd[0];
   s_new->black_bd[1] = s_old->black_bd[1];

   for (i=0; i<4; i++)
     {
      s_new->queens_x[WHITE][i] = s_old->queens_x[WHITE][i]; 
      s_new->queens_x[BLACK][i] = s_old->queens_x[BLACK][i];

      s_new->queens_y[WHITE][i] = s_old->queens_y[WHITE][i]; 
      s_new->queens_y[BLACK][i] = s_old->queens_y[BLACK][i]; 
     }

   s_new->turn = s_old->turn;
   s_new->value = s_old->value;
   s_new->depth = s_old->depth;
   s_new->winner = s_old->winner;
}


/*
 * create_hash
 *
 * Creates a hash value for the current state.  On the GUI side it's used to 
 * see if the state has changed recently.
 */
int
state_create_hash(State *s)
{
   ull board_u;
   ull board_l;

   board_u = s->white_bd[1] | s->black_bd[1] | s->blocks_bd[1];
   board_l = s->white_bd[0] | s->black_bd[0] | s->blocks_bd[0];

   return (board_u ^ board_l) % TT;
}


/* Populates movelist[] with all possible moves if movelist is not NULL.
 *
 * Returns the number of moves.
 *
 * The value of the move is also set here, that means 
 * this is the function that calls heval (heuristic) 
 *  and tt_lookup (transposition table retrieve)
 * NOTE: The latter is no longer true since this code is commented out.
 */
int
state_gen_moves(State *s, Move movelist[])
{
   int i,j;
   int num_moves=0;
   int xo=0, yo=0;
   int axo=0, ayo=0;
   int dir, adir;

   uchar *queens_x;
   uchar *queens_y;
   ull *board;

   int queen = 0;

   if (s->turn == WHITE)
     {
      queens_x = s->queens_x[WHITE];
      queens_y = s->queens_y[WHITE];
      board = s->white_bd;
     }
   else
     {
      queens_x = s->queens_x[BLACK];
      queens_y = s->queens_y[BLACK];
      board = s->black_bd;
     }

   // look for moves.
   // order: N NE E SE S SW W NW

   // this loop moves a queen, shoots an arrow, all the while 
   // checking to see if its run into things. 
   // populated movelist (passing in as reference) with the valid moves.
   for (queen = 0; queen < QUEENS; queen++)
     {
      for (dir = 0; dir < 8; dir++)
       	{
	 xo = directions[dir].dx;
	 yo = directions[dir].dy;

	 for (i=1; i < 10; i++) 
	   {
	    // out of bounds
	    if (queens_x[queen] + xo*i > 9 ||
		queens_y[queen] + yo*i > 9 ||
		queens_x[queen] + xo*i < 0 ||
		queens_y[queen] + yo*i < 0)
	      {
	       break;
	      }

	    // hit a wall
	    if (test(s, queens_x[queen]+xo*i, queens_y[queen]+yo*i))
	      break;

	    bitmap_flip(board, queens_x[queen], queens_y[queen]);
	    queens_x[queen] += i*xo;
	    queens_y[queen] += i*yo;
	    bitmap_flip(board, queens_x[queen], queens_y[queen]);

	    // look for arrow moves.
	    for (adir = 0; adir < 8; adir++)
	      {
	       axo = directions[adir].dx;
	       ayo = directions[adir].dy;

	       for (j=1; j < 10; j++) 
		 {
		  if (queens_x[queen] + axo*j > 9 ||
		      queens_y[queen] + ayo*j > 9 ||
		      queens_x[queen] + axo*j < 0 ||
		      queens_y[queen] + ayo*j < 0)
		    {
		     break;
		    }

		  if (test(s, queens_x[queen]+axo*j, 
			      queens_y[queen]+ayo*j))
		    {
		     break;
		    }

		  /*
		  // put arrow
		  bitmap_flip(s->blocks_bd, queens_x[queen]+axo*j, queens_y[queen]+ayo*j);
		  s->turn = OTHER_COLOR(s->turn);

		  //                        printf("evaluating board:");
		  //                        state_print(*s);
		  if ((stt = tt_lookup(s)))
		  {
		  //tt_yes++;
		  hits++;
		  movelist[m].val = stt->value;
		  movelist[m].depth = stt->depth;
		  }
		  else
		  {
		  //tt_no++;
		  movelist[m].val = HEVAL(s);
		  hevalz++;
		  s->value = movelist[m].val;
		  s->depth = 1;
		  tt_store(s, SHRT_MIN, SHRT_MAX);
		  }

		  // remove arrow
		  bitmap_flip(s->blocks_bd, queens_x[queen]+axo*j, queens_y[queen]+ayo*j);
		  s->turn = OTHER_COLOR(s->turn);

		   */
		  if (movelist) {
		    movelist[num_moves].queen=queen;
		    movelist[num_moves].tocol=queens_x[queen];
		    movelist[num_moves].torow=queens_y[queen];
		    movelist[num_moves].wallcol=queens_x[queen]+axo*j;
		    movelist[num_moves].wallrow=queens_y[queen]+ayo*j;

#if 0
		    pmove(movelist[num_moves]);
		    printf("\n");
#endif
		  }

		  num_moves++;
		 }
	      }
	    bitmap_flip(board, queens_x[queen], queens_y[queen]);
	    queens_x[queen] -= i*xo;
	    queens_y[queen] -= i*yo;
	    bitmap_flip(board, queens_x[queen], queens_y[queen]);
	   }
       	}
     }

   return num_moves;
}


/* Check if the move is a legal move for 'color'.
 *
 * Return 1 if it is legal, and 0 if it is illegal.
 */

int
state_is_legal_move(State *state, int color, Move *move)
{
  int  dist_move;
  int  dist_arrow;
  int  delta;
  int  sq;
  int  fromcol, fromrow;
  int  tocol, torow;
  int  arrcol, arrrow;
  ull  obstacles[2];		/* Everything on the board. */
  int  other;

  other = OTHER_COLOR(color);

  fromcol = state->queens_x[color][move->queen];
  fromrow = state->queens_y[color][move->queen];
  tocol   = move->tocol;
  torow   = move->torow;
  arrcol  = move->wallcol;
  arrrow  = move->wallrow;

  bitmap_copy(state->blocks_bd, obstacles);
  bitmap_or(obstacles, state->white_bd);
  bitmap_or(obstacles, state->black_bd);
#if 0
  printf("col = %d, row = %d\n", fromcol, fromrow);
  bitmap_print(stdout, obstacles);
  fputc('\n', stdout);
#endif
  bitmap_reset(obstacles, fromcol, fromrow);
#if 0
  bitmap_print(stdout, obstacles);
  fputc('\n', stdout);
#endif

  /* Must be on the board. */
  if (torow < 0 || torow > 9
      || tocol < 0 || tocol > 9
      || arrrow < 0 || arrrow > 9
      || arrcol < 0 || arrcol > 9)
    return 0;

  /* Must not be the same point(s). */
  if ((fromrow == torow && fromcol == tocol)
      || (torow == arrrow && tocol == arrcol))
    return 0;

  /* 'to' and 'arrow' must be EMPTY (unless arrow == from). */
  if (bitmap_isset(obstacles, tocol, torow))
    return 0;
  if (bitmap_isset(obstacles, arrcol, arrrow)
      && fromcol != arrcol && fromrow != arrrow)
    return 0;

  /* Must be along straight lines. */
  if ((fromrow != torow		               /* Horizontal */
       && fromcol != tocol	               /* Vertical */
       && tocol - fromcol != torow - fromrow   /* Diagonal 1 */
       && tocol - fromcol != fromrow - torow)) /* Diagonal 2 */
    return 0;
  if ((torow != arrrow		             /* Horizontal */
       && tocol != arrcol	             /* Vertical */
       && arrcol - tocol != arrrow - torow   /* Diagonal 1 */
       && arrcol - tocol != torow - arrrow)) /* Diagonal 2 */
    return 0;

  /* Check for free lines from 'from' to 'to' and from 'to' to 'arrow' */
  {
    int  col;
    int  row;
    int  dcol;
    int  drow;
    int  dist;
    
    /* 1. Check along the line from 'from' to 'to'. */
    dist = fromcol - tocol;
    if (dist == 0)
      dist = fromrow - torow;
    if (dist == 0)
      dist = fromrow - tocol;
    if (dist < 0)
      dist = -dist;

    dcol = (tocol - fromcol) / dist;
    drow = (torow - fromrow) / dist;

    col = fromcol;
    row = fromrow;
    do {
	if (bitmap_isset(obstacles, col, row))
	return 0;

      col += dcol;
      row += drow;
    } while (col != tocol && row != torow);

    /* 2. Check along the line from 'to' to 'arrow'. */
    dist = tocol - arrcol;
    if (dist == 0)
      dist = torow - arrrow;
    if (dist == 0)
      dist = torow - arrcol;
    if (dist < 0)
      dist = -dist;

    dcol = (arrcol - tocol) / dist;
    drow = (arrrow - torow) / dist;

    col = tocol;
    row = torow;
    do {
      if (bitmap_isset(obstacles, col, row))
	return 0;

      col += dcol;
      row += drow;
    } while (col != arrcol && row != arrrow);
  }

  /* We couldn't find anything wrong with the move.  It has to be correct. */
  return 1;
}


/* Print out a nice representation of the playing board.
 */

void
state_print(State *s)
{
   int board[12][12];
   int i,j;
   int row, col;

   // make the whole board blanks
   for (i=0; i < 12; i++)
      for (j=0; j < 12; j++)
	 board[i][j] = '.';

   // place player1 queens
   for (i=0; i < QUEENS; i++)
     {
      col = s->queens_x[WHITE][i];
      row = s->queens_y[WHITE][i];
#if 0
      if (options.white_player == HUMAN)
	board[col][row] = i+48;
      else
#endif
	board[col][row] = 'W';
     }

   // place player2 queens
   for (i=0; i < QUEENS; i++)
     {
      col = s->queens_x[BLACK][i];
      row = s->queens_y[BLACK][i];
#if 0
      if (options.black_player == HUMAN)
	board[col][row] = i+48;
      else
#endif
	board[col][row] = 'B';
     }

   // put walls on the board
   //pvec(s->blocks_bd[1]);
   for (i=0; i < 50; i++)
     {
      if ((s->blocks_bd[1] >> i) & 1)
       	{
	 board[i%10][i/10+5] = 'x';
       	}
     }
   //pvec(s->blocks_bd[0]);

   for (i=0; i < 50; i++)
     {
      if ((s->blocks_bd[0] >> i) & 1)
       	{
	 board[i%10][i/10] = 'x';
       	}
     }
   printf("\n");

   // print out the board
   printf("    A B C D E F G H I J\n");
   for (i=9; i >= 0; i--)
     {
      printf("%2d ", i + 1);
      for (j=0; j < 10; j++)
	 printf(" %c", board[j][i]);
      printf(" %2d", i + 1);
      printf("\n");
     }

   printf("    A B C D E F G H I J\n\n");
}


/* ================================================================ */
/*                             Searching                            */


// for qsort
static int
mincompare(Move *m1, Move *m2) // USED BY MAXIMIZER
{
   if (m1->val > m2->val)
      return -1;
   else if (m1->val < m2->val)
      return 1;
   return 0;
}

// for qsort
static int
maxcompare(Move *m1, Move *m2)
{
   if (m1->val < m2->val)
      return -1;
   else if (m1->val > m2->val)
      return 1;
   return 0;
}


/* top level iterative search function. Called from the main loop,
 * pretty self explanatory
 */

Move
isearch(State *s, int think, void (*func)(void))
{
   int depth;
   Move temp1, temp2;
   Move even, odd;
   int temp2_valid = FALSE;

   even.val = 0;
   odd.val = 0;

   call_out = func;

   for (depth = MINDEPTH; depth <= options.engine.maxdepth; depth++)
     {
      rawnodes = 0;
      nodes = 0;
      hits = 0;
      hevalz =0;

      temp1 = search(s, 0, SHRT_MIN, SHRT_MAX, depth, think);

      /*

      // for mtdf NOT WORKING CURRENTLY
      if (i & 1)
      {
      temp1 = mtdf(s, odd.val, i);
      even = temp1;
      }
      else
      {
      temp1 = mtdf(s, even.val, i);
      odd = temp1;
      }
       */

#if 0
      printf("Searched to depth %d, %d cache hits of %d lookups. %d nodes searched.\n", depth, hits,rawnodes, nodes);
      printf("hevals = %d\n", hevalz);
#endif
      if (ok)          // using this global ok is pretty sloppy but works.
      	{
	 temp2 = temp1;
	 temp2_valid = TRUE;
       	}
      else
       	{
	 if (think)
	   {
#ifdef DEBUG
	    printf("searched %d nodes on your time\n", nodes);
#endif
	   }
#ifdef DEBUG
	 printf("time's up at depth %d\n", depth);
#endif
	 break;
       	}

     }
   if (temp2_valid)
      return temp2;
   else
      return temp1;
}


// NOT WORKING
Move mtdf(State *s, int guess, int tdepth)
{
   Move temp;
   //int f;
   int lowerbound = SHRT_MIN;
   int upperbound = SHRT_MAX;
   int beta;

   temp.val = guess;

   do
     {
      if (temp.val == lowerbound)
       	{
	 beta = temp.val + 1;
       	}
      else
       	{
	 beta = temp.val;
       	}

      //temp = search(s, 0, beta - 1, beta, tdepth, think);

      if (temp.val < beta)
       	{
	 upperbound = temp.val;
       	}
      else
       	{
	 lowerbound = temp.val;
       	}
     }
   while (!(lowerbound >= upperbound));

   return temp;
}


/* minimax search.
 *
 * tdepth is "target depth", that is the current iterative target level
 */

Move
search(State *s, int depth, int alpha, int beta, int tdepth, int think)
{
   state_t *stt;
   Move movelist[3000];
   int i;
   int index=0;
   Move stemp;
   Move temp;
   int movecount;

   int done;

   nodes++;

   if ((time_t)time(NULL) - start > options.engine.timeout && TIMED && !think)
     {
      ok = 0;
      return movelist[0];
     }

   movecount = state_gen_moves(s, movelist);

   if (movecount == 0) // no more moves
     {
      if (depth == 0)
       	{
#ifdef DEBUG
	 printf("player %d wins!\n", OTHER_COLOR(s->turn));
#endif
	 s->winner = OTHER_COLOR(s->turn);
	 if (options.print_statistics)
	    print_stats();
#ifndef GAMAZONS
	 exit(1);
#endif
       	}

      // if no more moves, then set the value here to 
      // a very high value plus/minus the depth
      // this ensure that the loser will select the longest path
      // to loss, and the winner will select the shortest path to
      // a win.
      movelist[0].val = MMSET(s->turn, depth);
     }

   // If this is depth 0 on the first iteration, then randomize the
   // order of the moves so that different games are played each time.
   // This only ensures that moves of the same valuation is ordered
   // randomly.  If there is one single best move, this will always be
   // selected anyhow.
   if (depth == 0 && tdepth == MINDEPTH) 
     {
       Move temp;
       int  j;

       for (i=0; i < movecount - 1; i++)
	 {
	   j = i + rand() % (movecount - i);
	   temp = movelist[i];
	   movelist[i] = movelist[j];
	   movelist[j] = temp;
	 }
     }

   // for node reordering, make sure everything has a value.
   for (i=0; i <movecount; i++)
     {

       if (call_out)
	  call_out();

      temp = savemove(s, movelist[i]);
      makemove(s, movelist[i]);
      if ((stt = tt_lookup(s)))
       	{
	 if (alpha >= stt->alpha && beta <= stt->beta) // correct
	   {
	    hits++;
	    movelist[i].val = stt->value;
	    movelist[i].depth = stt->depth;
	   }
	 else
	   {
	    movelist[i].val = HEVAL(s);
	    hevalz++;
	    s->value = movelist[i].val;
	    s->depth = 1;
	    tt_update(s, SHRT_MIN, SHRT_MAX);
	   }
       	}
      else
       	{
	 movelist[i].val = HEVAL(s);
	 hevalz++;
	 s->value = movelist[i].val;
	 s->depth = 1;
	 tt_store(s, SHRT_MIN, SHRT_MAX);
       	}
      undomove(s, temp);
     }

   rawnodes += movecount;

   // node reordering here.

   if (s->turn == WHITE)
      qsort(movelist, movecount, sizeof(Move), (void*)mincompare);
   else
      qsort(movelist, movecount, sizeof(Move), (void*)maxcompare);

   // this is the end of the line, return best move.
   if (depth+1 >= tdepth)
      return movelist[0];

   done = 0;
   for (i=0; i < movecount && !done && i < options.engine.maxdepth; i++)
     {
      // already searched farther than we will in the TT, just return it.
      if (movelist[i].depth > tdepth - depth)
	 return movelist[i];

      temp = savemove(s, movelist[i]);
      makemove(s, movelist[i]);

      stemp = search(s, depth+1, alpha, beta, tdepth, think);

      if (ok)
       	{
	 // this should perhaps check if tt_lookup still works 
	 // not life changing
	 if (tdepth-depth > movelist[i].depth)
	   {
	    s->value = stemp.val;
	    s->depth = tdepth-depth;
	    tt_update(s, alpha, beta);
	   }

	 movelist[i].val = stemp.val;
	 undomove(s,temp); // undo move.
      	}
      else // times up, return back up the tree as fast as possible.
      	{
	 undomove(s,temp); // undo move.
       	 return movelist[i];
       	}

      nodes++;

      // alpha-beta stuff:

      if (s->turn == WHITE) 
	{
	 if (movelist[i].val > alpha)
	   {
	    alpha = movelist[i].val;
	    index = i;
	   }
       	}
      else
       	{
	 if (movelist[i].val < beta)
	   {
	    beta = movelist[i].val;
	    index = i;
	   }
       	}

      if (alpha >= beta)
       	{
	 done = 1;
       	}
     }

   return movelist[index];
}


/* ================================================================ */


// print out a longlong in binary
int pvec(ull v)
{
   int i;

   printf("->");
   for (i=49; i >= 0; i--)
     {
      printf("%d", (int)((v >> i) & (ull)1));
      if (i%10 == 0)
	 printf(" ");
     }
   printf("<-\n");

   return 0;
}


/* Convert a string to a square.
 *
 * Return the number of characters in the square (2 or 3).  If the
 * square was not ok, return 0.
 */

int
string_to_square(char *str, int *col, int *row)
{
  char  letters[] = "abcdefghij";
  char  *strend;

  if (strchr(letters, tolower(*str)))
    *col = tolower(*str) - 'a';
  else
    return 0;

  *row = strtol(str + 1, &strend, 10) - 1;
  if (strend == str || *row < 0 || *row > 9)
    return 0;

  return strend - str;
}


int
string_to_move(State *state, char *str, Move *move)
{
  char *strsave;
  int  len;
  int  fromcol;
  int  fromrow;
  int  col;
  int  row;
  int  i;

  strsave = str;

  while (*str == ' ')
    str++;

  len = string_to_square(str, &fromcol, &fromrow);
  if (len == 0)
    return 0;
  str += len;

  /* Check that there is a queen at this square. */
  for (i = 0; i < QUEENS; i++) {
    if (state->queens_x[WHITE][i] == fromcol
	&& state->queens_y[WHITE][i] == fromrow)
      break;
    if (state->queens_x[BLACK][i] == fromcol
	&& state->queens_y[BLACK][i] == fromrow)
      break;
  }
  if (i == QUEENS)
    return 0;
  move->queen = (uchar) i;

  if (*str == '-')
    str++;

  len = string_to_square(str, &col, &row);
  if (len == 0)
    return 0;
  str += len;

  move->tocol = (uchar) col;
  move->torow = (uchar) row;

  if (*str == '(' || *str == '-' || *str == ',')
    str++;

  len = string_to_square(str, &col, &row);
  if (len == 0)
    return 0;

  move->wallcol = (uchar) col;
  move->wallrow = (uchar) row;

  if (*str == ')')
    str++;

  return str - strsave;
}


// takes a move that is to be made and returns a move that will
// undo it.
Move savemove(State *s, Move m)
{
   Move temp;

   temp.wallcol = m.wallcol;
   temp.wallrow = m.wallrow;
   temp.queen = m.queen;

   if (s->turn == WHITE)
     {
      temp.tocol = s->queens_x[WHITE][m.queen];
      temp.torow = s->queens_y[WHITE][m.queen];
     }
   else
     {
      temp.tocol = s->queens_x[BLACK][m.queen];
      temp.torow = s->queens_y[BLACK][m.queen];
     }
   return temp;
}

int undomove(State *s, Move m)
{
   s->turn = OTHER_COLOR(s->turn);

   if (s->turn == WHITE)
     {
      bitmap_flip(s->white_bd, s->queens_x[WHITE][m.queen], s->queens_y[WHITE][m.queen]);
      s->queens_x[WHITE][m.queen] = m.tocol;
      s->queens_y[WHITE][m.queen] = m.torow;
      bitmap_flip(s->white_bd, s->queens_x[WHITE][m.queen], s->queens_y[WHITE][m.queen]);
     }
   else
     {
      bitmap_flip(s->black_bd, s->queens_x[BLACK][m.queen], s->queens_y[BLACK][m.queen]);
      s->queens_x[BLACK][m.queen] = m.tocol;
      s->queens_y[BLACK][m.queen] = m.torow;
      bitmap_flip(s->black_bd, s->queens_x[BLACK][m.queen], s->queens_y[BLACK][m.queen]);
     }

   bitmap_flip(s->blocks_bd, m.wallcol, m.wallrow);
   return 1;
}

// takes a move and does it. 
int makemove(State *s, Move m)
{
  int  color;

  color = s->turn;
   if (s->turn == WHITE)
     {
      //printf("registering move w/ engine for white\n");
      bitmap_flip(s->white_bd, s->queens_x[WHITE][m.queen], s->queens_y[WHITE][m.queen]);
      s->queens_x[WHITE][m.queen] = m.tocol;
      s->queens_y[WHITE][m.queen] = m.torow;
      bitmap_flip(s->white_bd, s->queens_x[WHITE][m.queen], s->queens_y[WHITE][m.queen]);
     }
   else
     {
      //printf("registering move w/ engine for black\n");
      bitmap_flip(s->black_bd, s->queens_x[BLACK][m.queen], s->queens_y[BLACK][m.queen]);
      s->queens_x[BLACK][m.queen] = m.tocol;
      s->queens_y[BLACK][m.queen] = m.torow;
      bitmap_flip(s->black_bd, s->queens_x[BLACK][m.queen], s->queens_y[BLACK][m.queen]);
     }

   bitmap_flip(s->blocks_bd, m.wallcol, m.wallrow);
   s->turn = OTHER_COLOR(s->turn);
   return 1;
}


// print out a move struct
void pmove(State *state, int color, Move m)
{
   printf("%d) %c%d-%c%d-%c%d, val = %d\n", onmove, 
	  state->queens_x[color][m.queen] + 'a', 
	  state->queens_y[color][m.queen] + 1,
	  m.tocol+'a', m.torow + 1, m.wallcol+'a', m.wallrow + 1,
	  m.val);
}


void print_stats()
{

   printf("TT Stats:\n");
   printf("   # nodes over written: %d\n", tt_overwrite);
   printf("   # of nodes stored:    %d\n", tt_stores);
   printf("   # of updates made:    %d\n", tt_updates);
   printf("   # of lookups made:    %d\n", tt_lookups);
   printf("   # of lookups found:   %d\n", tt_lookup_finds);
   printf("\n");
   printf("Heuristic Stats:\n");
   printf("   # of heuristic calls  %d\n", heval_calls);
}


/* ================================================================ */
/*                          Bitboard handling                         */


/* Return 1 if there is anything - an arrow or a queen - at col, row.
 *
 * FIXME: Change this name to something better.
 */

int
test(State *s, uchar col, uchar row)
{
   //printf("sigh %d %d\n", col, row);
   if (row < 5)
     {
      if ( (s->blocks_bd[0] | s->white_bd[0] | s->black_bd[0] ) & 
	      (ull)1 << ((row*10) + col))
	 return 1;

      return 0;
     }
   else // row > 5
     {
      if ( (s->blocks_bd[1] | s->white_bd[1] | s->black_bd[1]) & 
	      (ull)1 << ((((row-5)*10) + col)))
       	{
	 return 1;
       	}
      return 0;
     }
}


/* Copy one the first bitmap into the second.
 */

void
bitmap_copy(ull bd_old[2], ull bd_new[2])
{
    bd_new[0] = bd_old[0];
    bd_new[1] = bd_old[1];
}


/* Clear the bitboard, i.e. set everything to 0.
 */

void
bitmap_clear(ull bd[2])
{
    bd[0] = 0;
    bd[1] = 0;
}


void
bitmap_print(FILE *outfile, ull bd[2])
{
    int  col;
    int  row;

    for (row = 9; row >= 0; row--) {
	for (col = 0; col < 10; col++)
	    fputc(bitmap_isset(bd, col, row) ? 'x' : '-', outfile);
	fputc('\n', outfile);
    }
}


/* Set the bit corresponding to col,row.
 */

void
bitmap_set(ull bd[2], uchar col, uchar row)
{
  if (row < 5)
      bd[0] |= ((ull)1 << ((row*10) + col));
  else // row >= 5
      bd[1] |= ((ull)1 << (((row - 5)*10) + col));
}


/* Reset the bit corresponding to col,row.
 */

void
bitmap_reset(ull bd[2], uchar col, uchar row)
{
  if (row < 5)
      bd[0] &= ~((ull)1 << ((row*10) + col));
  else // row >= 5
      bd[1] &= ~((ull)1 << (((row - 5)*10) + col));
}


/* Return 1 if the bit corresponding to col,row is set.
 */

int
bitmap_isset(ull bd[2], uchar col, uchar row)
{
  if (row < 5)
    return (bd[0] & ((ull)1 << ((row*10) + col))) != 0;
  else // row >= 5
    return (bd[1] & ((ull)1 << (((row - 5)*10) + col))) != 0;
}


/* Flip the bit corresponding to col,row.
 */

void
bitmap_flip(ull bd[2], uchar col, uchar row)
{
   if (row < 5)
      bd[0] ^= (ull)1 << ((row*10) + col);
   else // row > 5
      bd[1] ^= (ull)1 << ((((row-5)*10) + col));
}


/* Or one the second into the first.
 */

void
bitmap_or(ull bd_1[2], ull bd_2[2])
{
    bd_1[0] |= bd_2[0];
    bd_1[1] |= bd_2[1];
}
