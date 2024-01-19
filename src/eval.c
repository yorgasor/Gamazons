#include <stdio.h>
#include <stdlib.h>
#include "amazons.h"
#include "unit-test.h"


/* local protos */
static void tt_copy(State *s, state_t *t, short alpha, short beta);


/* Globals */
int black_misc = 0;		//miscellaneous heuristic values
int white_misc = 0;


/* Heuristic Weights */
#define QUEEN_LOSS 	15	//point deduction when losing a queen
#define QUEEN_DANGER	10	//point deduction when queen in danger
#define QUEEN_CLUSTER	 5	//point deduction when a queen isn't in
				//  a quadrant
#define OWN_VALUE	 1.5 	//Value of owning a square



/*==============================================================================
 * get_forward_diag
 *
 * This gets a stream of bits from the given bit board in a "forwards" diagonal 
 * starting at the position diag, and returns the stream.
 */
int get_forward_diag(ull board_l, ull board_u, int diag)
{
   int len = GET_FDIAG_LEN(diag);
   int i;
   ushort res = 0;
   ull *board;
   int pos=diag;
   ushort mask = 0x1;

   for (i=0; i<len; i++)
     {
      if (pos > 49)
	 board = &board_u;
      else
	 board = &board_l;

      res |= (*board >> (pos % 50)) & mask;
      pos +=10;
      mask <<= 1;
     }

   return res;
}


/*==============================================================================
 * get_back_diag
 *
 * This gets a stream of bits from the given bit board in a "backwards" diagonal 
 * starting at the position diag, and returns the stream.
 */
int get_back_diag(ull board_l, ull board_u, int diag)
{
   int len = GET_BDIAG_LEN(diag);
   int i;
   ushort res = 0;
   int pos=diag;
   ushort mask = 0x1;

   for (i=0; i<=len; i++)
     {
      //This needs some funky footwork to get bits in the early 50's
      if (pos + i > 49)
	{
	 if (pos > 49)
	   {
	   res |= (board_u >> (pos % 50)) & mask;
	   }
	 else
	   {
	    res |= (board_u << (50 - pos)) & mask;
	   }
	}
      else
         res |= (board_l >> pos) & mask;

      pos +=8;
      mask <<= 1;
     }

   return res;
}

/*==============================================================================
 * put_forward_diag
 *
 * This takes a stream of bits and stores them on the given bit board in a 
 * "forward" diagonal starting at the position diag.
 */
void put_forward_diag(ull *board_l, ull *board_u, ushort stream, int diag)
{
   int len = GET_FDIAG_LEN(diag);
   int i;
   ull *board;
   int pos=diag;
   ushort mask = 0x1;

   for (i=0; i<len; i++)
     {
      if (pos > 49)
	 board = board_u;
      else
	 board = board_l;

      *board |= ((ull) (stream & mask ) << (pos % 50));
      pos +=10;
      mask <<= 1;
     }
}


/*==============================================================================
 * put_back_diag
 *
 * This takes a stream of bits and stores them on the given bit board in a 
 * "backwards" diagonal starting at the position diag.
 */
void put_back_diag(ull *board_l, ull *board_u, ushort stream, int diag)
{
   int len = GET_BDIAG_LEN(diag);
   int i;
   int pos=diag;
   ushort mask = 0x1;

   for (i=0; i<len; i++)
     {
      //This needs some funky footwork to get bits in the early 50's
      if (pos + i > 49)
	{
	 if (pos > 49)
	    *board_u |= ((ull) (stream & mask ) << (pos % 50));
	 else
	    *board_u |= ((ull) (stream & mask ) >> (50 - pos));
	}
      else
         *board_l |= ((ull) (stream & mask ) << pos);

      pos +=8;
      mask <<= 1;
     }
}


/*==============================================================================
 * heval
 *
 * This calculates the heuristic evaluation for a given state.  It currently 
 * bases it's evaluation on the number of squares the queens can move to and the
 * number of squares each side "owns".
 */
int heval(State *s)
{
   int p1=0, p2=0;
   float own1L0 = 0, own2L0= 0;
   float own1L1 = 0, own2L1= 0;
   ull board_u, board_l;
   ull web_board_u=0, web_board_l=0;
   ull white_web1_u=0, white_web1_l=0, black_web1_u=0, black_web1_l=0;
   ull res1_l=0, res1_u=0, res2_l=0, res2_u=0;
   ull dirs_l=0, dirs_u=0;

   	//Generates quads that look like:
        // 1 1 1 1 1 0 0 0 0 0
        // 1 1 1 1 1 0 0 0 0 0
        // 1 1 1 1 1 0 0 0 0 0
        // 1 1 1 1 1 0 0 0 0 0
        // 1 1 1 1 1 0 0 0 0 0
        ull low_quad = 0x07c1f07c1f, high_quad = 0xf83e0f83e0;

   int i;
   int black_q_pos[4];
   int white_q_pos[4];

   ++heval_calls;
   white_misc = black_misc = 0;

   //establish queen positions
   for (i=0; i<4; i++)
     {
      black_q_pos[i] = XY_TO_POS(s->queens_x[BLACK][i], s->queens_y[BLACK][i]);
      white_q_pos[i] = XY_TO_POS(s->queens_x[WHITE][i], s->queens_y[WHITE][i]);
     }
   //printf("heval:\n");
   //state_print(*s);

   board_u = s->white_bd[1] | s->black_bd[1] | s->blocks_bd[1];
   board_l = s->white_bd[0] | s->black_bd[0] | s->blocks_bd[0];

   //pvec(board_u);
   //pvec(board_l);
   //pbvec(board_l, board_u);

   //calculate moves
   /*
   for (i=0; i<4; i++)
     {
      p1 += calc_moves(board_l, board_u, white_q_pos[i]);
      p2 += calc_moves(board_l, board_u, black_q_pos[i]);
     }
     */

   //make webs for queens & count # of moves
   for (i=0; i<4; i++)
     {
      if (!(p1 += gen_web_board_count(&white_web1_l, &white_web1_u, board_l, board_u, white_q_pos[i])))
	 white_misc -= QUEEN_LOSS;  //white queen is trapped.  
     
      if (!(p2 += gen_web_board_count(&black_web1_l, &black_web1_u, board_l, board_u, black_q_pos[i])))
	 black_misc -= QUEEN_LOSS;  //black queen is trapped.
     }

   /* look for queens in trouble */
   for (i=0; i<4; i++)
     {
      //check white queens
      gen_dirs_board(&dirs_l, &dirs_u, white_q_pos[i]);
      res1_l = white_web1_l & dirs_l;
      res1_u = white_web1_u & dirs_u;

      if (count_bits(res1_l, res1_u) < 3)
	{
	 res2_l = res1_l & black_web1_l;
	 res2_u = res1_u & black_web1_u;
	 if (!((res1_l ^ res2_l) || (res1_u ^ res2_u)))
	    white_misc -= QUEEN_DANGER;
	}

      //check black queens
      gen_dirs_board(&dirs_l, &dirs_u, black_q_pos[i]);
      res1_l = black_web1_l & dirs_l;
      res1_u = black_web1_u & dirs_u;
      if (count_bits(res1_l, res1_u) < 3)
	{
	 res2_l = res1_l & white_web1_l;
	 res2_u = res1_u & white_web1_u;
	 if (!((res1_l ^ res2_l) || (res1_u ^ res2_u)))
	    black_misc -= QUEEN_DANGER;
	}
     }

   /* Encourage queens to spread out a bit */
   for (i=0; i<2; i++)
     {
      if (!(s->white_bd[i] & low_quad))
	 white_misc -= QUEEN_CLUSTER;
      else if (!(s->black_bd[i] & low_quad))
	 white_misc += QUEEN_CLUSTER;  //give bonus points for owning a quadrant
      if (!(s->white_bd[i] & high_quad))
	 white_misc -= QUEEN_CLUSTER;
      else if (!(s->black_bd[i] & high_quad))
	 white_misc += QUEEN_CLUSTER;  //give bonus points for owning a quadrant
      if (!(s->black_bd[i] & low_quad))
	 black_misc -= QUEEN_CLUSTER;
      else if (!(s->white_bd[i] & low_quad))
	 black_misc += QUEEN_CLUSTER;
      if (!(s->black_bd[i] & high_quad))
	 black_misc -= QUEEN_CLUSTER;
      else if (!(s->white_bd[i] & high_quad))
	 black_misc += QUEEN_CLUSTER;
     }

   /*
    *calculate square ownership
    */

#ifdef DEBUG_HEVAL
   printf("white moves = %d, black moves = %d\n", p1, p2);
#endif

#ifdef DEBUG_WEB
      printf("White's web board:\n");
      pbvec(white_web1_l, white_web1_u);

      printf("Blacks's web board:\n");
      pbvec(black_web1_l, black_web1_u);
#endif
   //make webs for each empty square
   for (i=0; i<100; i++)
     {
      if (i > 49)
	{
	 if (board_u & ((ull)0x1 << (i % 50)))
	    continue;
	}
      else
	{
	 if (board_l & ((ull)0x1 << (i % 50)))
	    continue;
	}

      web_board_l = web_board_u = 0;
      gen_web_board(&web_board_l, &web_board_u, board_l, board_u, i);

#ifdef DEBUG_WEB
      printf("web board for square %d:\n", i);
      pbvec(web_board_l, web_board_u);
#endif

      if (s->turn == WHITE) //player 1 moves next
	{
	 if ((web_board_l & s->white_bd[0]) || (web_board_u & s->white_bd[1]))
	   {
	    //printf("Square %d is owned by white - level 0\n", i);
            ++own1L0;
	    if ((web_board_l & s->black_bd[0]) || (web_board_u & s->black_bd[1]))
	       own1L0 -= .5;
	   }
	 else if ((web_board_l & s->black_bd[0]) || (web_board_u & s->black_bd[1]))
	   {
	    //printf("Square %d is owned by black - level 0\n", i);
	    ++own2L0;
	   }
	 else if ((web_board_l & white_web1_l) || (web_board_u & white_web1_u))
	   {
	    //printf("Square %d is owned by white - level 1\n", i);
	    ++own1L1;
	    if ((web_board_l & black_web1_l) || (web_board_u & black_web1_u))
	       own1L1 -= .5;
	   }
	 else if ((web_board_l & black_web1_l) || (web_board_u & black_web1_u))
	   {
	    //printf("Square %d is owned by black - level 1\n", i);
	    ++own2L1;
	   }
	 else
	   {
	    //printf("Square %d is not owned by anyone\n", i);
	   }
	}
     else
	{
	 if ((web_board_l & s->black_bd[0]) || (web_board_u & s->black_bd[1]))
	   {
	    //printf("Square %d is owned by black - level 0\n", i);
	    ++own2L0;
	    if ((web_board_l & s->white_bd[0]) || (web_board_u & s->white_bd[1]))
	       own2L0 -= .5;
	   }
	 else if ((web_board_l & s->white_bd[0]) || (web_board_u & s->white_bd[1]))
	   {
            ++own1L0;
	    //printf("Square %d is owned by white - level 0\n", i);
	   }
	 else if ((web_board_l & black_web1_l) || (web_board_u & black_web1_u))
	   {
	    //printf("Square %d is owned by black - level 1\n", i);
	    ++own2L1;
	    if ((web_board_l & white_web1_l) || (web_board_u & white_web1_u))
	       own2L1 -= .5;
	   }
	 else if ((web_board_l & white_web1_l) || (web_board_u & white_web1_u))
	   {
	    //printf("Square %d is owned by white - level 1\n", i);
	    ++own1L1;
	   }
	 else
	   {
	    //printf("Square %d is not owned by anyone\n", i);
	   }
	}
     }


#ifdef DEBUG_HEVAL
   printf("Direct white squares = %d, direct black squares = %d\n", own1L0, own2L0);
   printf("Indirect white squares = %d, indirect black squares = %d\n", own1L1, own2L1);
   printf("white squares = %d, black squares = %d\n", own1, own2);
#endif
//	printf("own1 = %d p1 = %d own2 = %d p2 = %d\n", own1,p1,own2,p2);
   return (((own1L0 + own1L1) * OWN_VALUE + p1 + white_misc) - (p2 + (own2L0 + own2L1) * OWN_VALUE + black_misc));

}

/*==============================================================================
 * calc_moves
 *
 * This receives a complete bit board with all occupied square bits set.  From 
 * the given position, it calculates all possible moves in every direction.
 *
 * Returns the number of moves possible.
 */
int calc_moves(ull board_l, ull board_u, int pos)
{
   ushort stream;
   int row, col, fdiag, bdiag;
   int diag;


   //count row moves
   if (pos < 50)
      stream = GET_ROW(board_l, GET_COL_POS(pos));
   else
      stream = GET_ROW(board_u, GET_COL_POS(pos));
   row = calc_stream_moves(stream, GET_ROW_POS(pos), 10);
   //printf("%d row moves\n", row);


   //count col moves
   stream = GET_COL(board_l, board_u, GET_ROW_POS(pos));
   col = calc_stream_moves(stream, GET_COL_POS(pos), 10);
   //printf("%d col moves\n", col);

   //count forw diag moves
   diag = GET_FDIAG(pos);
   stream = get_forward_diag(board_l, board_u, diag);
   fdiag = calc_stream_moves(stream, GET_FDIAG_POS(pos), GET_FDIAG_LEN(diag));
   //printf("%d fdiag moves\n", fdiag);

   //count back diag moves
   diag = GET_BDIAG(pos);
   stream = get_back_diag(board_l, board_u, diag);
   bdiag = calc_stream_moves(stream, GET_BDIAG_POS(pos), GET_BDIAG_LEN(diag));
   //printf("%d bdiag moves\n", bdiag);

   //printf("%d total moves\n", row+col+fdiag+bdiag);
   return (row+col+fdiag+bdiag);


}

/*==============================================================================
 * calc_stream_moves
 *
 * Calculates the number of empty bits in either direction from the given position
 * in the given stream.
 *
 * Returns the number of moves in that stream.
 */
int calc_stream_moves(ushort stream, ushort pos, ushort len)
{
   int i;
   int moves = 0;
   int before=0, after=0;

   //psvec(stream, len);
   //error check
   if (pos > len)
     {
      printf("duh, the position %d can't be greater than the length %d\n", pos, len);
      return 0;
     }

//   printf("stream len = %d\n", len);
   for (i=0; i<pos; i++)
     {
      if (stream & 0x1)
	 before = 0;// (pos - (i+1));
      else
	 ++before;
      stream >>= 1;
     }

   ++i;
   stream >>= 1;

   for ( ; i<len; i++)
     {
      if (stream & 0x1)
	 break;
      else
	 ++after;
      stream >>= 1;
     }
   moves = before + after;
   //printf("Pos = %d, before = %d, after = %d\n", pos, before, after);

   return moves;
}


/*==============================================================================
 * gen_web_stream
 *
 * This generates a stream of 1 bits (the 'web') in both directions, starting 
 * from the given position pos, and stops when it hits a set bit in the given 
 * stream or comes to the end of the stream.
 *
 * Returns the generated web.
 */
ushort gen_web_stream(ushort stream, int pos, int len)
{
   ushort web = 0;
   int i;

   web |= 0x1 << pos;

   for (i=pos-1; i>=0; i--)
     {
      if (stream & (0x1 << i))
	 break;
      else
	 web |= 0x1 << i;
     }

   for (i=pos+1; i<len; i++)
     {
      if (stream & (0x1 << i))
	 break;
      else
	 web |= 0x1 << i;
     }

   return web;
}

/*==============================================================================
 * gen_web_stream_plus
 *
 * This generates a stream of 1 bits (the 'web') in both directions, starting 
 * from the given position pos, and stops when it hits a set bit in the given 
 * stream or comes to the end of the stream.  This differs from gen_web_stream
 * in that it includes a set bit on the square that it ran into.  This enables 
 * me to & the generated web stream with a queen board and see if they coincide.
 *
 * Returns the generated web.
 */
ushort gen_web_stream_plus(ushort stream, int pos, int len)
{
   ushort web = 0;
   int i;

   web |= 0x1 << pos;

   for (i=pos-1; i>=0; i--)
     {
      if (stream & (0x1 << i))
	{
	 web |= 0x1 << i;
	 break;
	}
      else
	 web |= 0x1 << i;
     }

   for (i=pos+1; i<len; i++)
     {
      if (stream & (0x1 << i))
	{
	 web |= 0x1 << i;
	 break;
	}
      else
	 web |= 0x1 << i;
     }

   return web;
}

/*==============================================================================
 * gen_web_board
 *
 * This takes the bit boards, board_l & board_u that represent all the occupied
 * squares.  Then, from the given position pos, generates a stream of 1's in 
 * every direction until they hit an occupied square.  The result is stored
 * in the given web_u & web_l
 */
void gen_web_board(ull *web_l, ull *web_u, ull board_l, ull board_u, int pos)
{
   ushort row, col, fdiag, bdiag;
   ushort web_row, web_col, web_fdiag, web_bdiag;
   int diag;

   //row web
   if (pos > 49)
      row = GET_ROW(board_u, GET_COL_POS(pos));
   else
      row = GET_ROW(board_l, GET_COL_POS(pos));

   web_row = gen_web_stream_plus(row, GET_ROW_POS(pos), 10);
   //printf("Row for pos %d:", pos);

#ifdef DEBUG_WEB   
   printf("Generating web for row "); psvec(row, 10);
   printf("Web generated for row at pos %d:", pos);
   psvec(web_row, 10);
#endif

   if (pos > 49)
      PUT_ROW(*web_u, pos/10, web_row);
   else
      PUT_ROW(*web_l, pos/10, web_row);
#ifdef DEBUG_WEB   
   printf("web board after row inserted:\n");
   pbvec(*web_l, *web_u);
#endif

   //col web
   col = GET_COL(board_l, board_u, pos%10);
   web_col = gen_web_stream_plus(col, GET_COL_POS(pos), 10);

#ifdef DEBUG_WEB   
   printf("Generating web for col "); psvec(col, 10);
   printf("Web generated for col at pos %d:", pos);
   psvec(web_col, 10);
#endif

   PUT_COL(*web_l, *web_u, pos%10, web_col);
#ifdef DEBUG_WEB   
   printf("web board after col inserted:\n");
   pbvec(*web_l, *web_u);
#endif

   //fdiag web
   diag = GET_FDIAG(pos);
   fdiag = get_forward_diag(board_l, board_u, diag);
   web_fdiag = gen_web_stream_plus(fdiag, GET_FDIAG_POS(pos), GET_FDIAG_LEN(diag));

#ifdef DEBUG_WEB   
   printf("Generating web for fdiag "); psvec(fdiag, GET_FDIAG_LEN(diag));
   printf("Web generated for fdiag at pos %d:", pos);
   psvec(web_fdiag, 10);
#endif

   put_forward_diag(web_l, web_u, web_fdiag, diag);
#ifdef DEBUG_WEB   
   printf("web board after fdiag inserted:\n");
   pbvec(*web_l, *web_u);
#endif

   //bdiag web
   diag = GET_BDIAG(pos);
   bdiag = get_back_diag(board_l, board_u, diag);
   web_bdiag = gen_web_stream_plus(bdiag, GET_BDIAG_POS(pos), GET_BDIAG_LEN(diag));

#ifdef DEBUG_WEB   
   printf("Generating web for bdiag "); psvec(bdiag, GET_BDIAG_LEN(diag));
   printf("Web generated for bdiag at pos %d:", pos);
   psvec(web_bdiag, 10);
#endif

   put_back_diag(web_l, web_u, web_bdiag, diag);
#ifdef DEBUG_WEB   
   printf("web board after bdiag inserted:\n");
   pbvec(*web_l, *web_u);
#endif
}


/*==============================================================================
 * gen_web_board_count
 *
 * This takes the bit boards, board_l & board_u that represent all the occupied
 * squares.  Then, from the given position pos, generates a stream of 1's in 
 * every direction until they hit an occupied square.  The result is stored
 * in the given web_u & web_l
 */
int gen_web_board_count(ull *web_l, ull *web_u, ull board_l, ull board_u, int pos)
{
   ushort row, col, fdiag, bdiag;
   ushort web_row, web_col, web_fdiag, web_bdiag;
   int diag;
   int row_count, col_count, fdiag_count, bdiag_count;

#ifdef DEBUG_HEVAL
   printf("Counting moves for queen on position %d\n", pos);
#endif
   //row web
   if (pos > 49)
      row = GET_ROW(board_u, GET_COL_POS(pos));
   else
      row = GET_ROW(board_l, GET_COL_POS(pos));

   web_row = gen_web_stream(row, GET_ROW_POS(pos), 10);
   row_count = count_contig_bits(web_row, 10);
   //printf("Row for pos %d:", pos);

#ifdef DEBUG_HEVAL
   printf("Counting moves for queen on position %d\n", pos);
   printf("  Row moves: %d\n", row_count);
#endif

#ifdef DEBUG_WEB   
   printf("Generating web for row "); psvec(row, 10);
   printf("Web generated for row at pos %d:", pos);
   psvec(web_row, 10);
#endif

   if (pos > 49)
      PUT_ROW(*web_u, pos/10, web_row);
   else
      PUT_ROW(*web_l, pos/10, web_row);
#ifdef DEBUG_WEB   
   printf("web board after row inserted:\n");
   pbvec(*web_l, *web_u);
#endif

   //col web
   col = GET_COL(board_l, board_u, pos%10);
   web_col = gen_web_stream(col, GET_COL_POS(pos), 10);
   col_count = count_contig_bits(web_col, 10);

#ifdef DEBUG_HEVAL
   printf("  Col moves: %d\n", col_count);
#endif

#ifdef DEBUG_WEB   
   printf("Generating web for col "); psvec(col, 10);
   printf("Web generated for col at pos %d:", pos);
   psvec(web_col, 10);
#endif

   PUT_COL(*web_l, *web_u, pos%10, web_col);
#ifdef DEBUG_WEB   
   printf("web board after col inserted:\n");
   pbvec(*web_l, *web_u);
#endif

   //fdiag web
   diag = GET_FDIAG(pos);
   fdiag = get_forward_diag(board_l, board_u, diag);
   web_fdiag = gen_web_stream(fdiag, GET_FDIAG_POS(pos), GET_FDIAG_LEN(diag));
   fdiag_count = count_contig_bits(web_fdiag, GET_FDIAG_LEN(diag));

#ifdef DEBUG_HEVAL
   printf("  Fdiag moves: %d\n", fdiag_count);
#endif

#ifdef DEBUG_WEB   
   printf("Generating web for fdiag "); psvec(fdiag, GET_FDIAG_LEN(diag));
   printf("Web generated for fdiag at pos %d:", pos);
   psvec(web_fdiag, 10);
#endif

   put_forward_diag(web_l, web_u, web_fdiag, diag);
#ifdef DEBUG_WEB   
   printf("web board after fdiag inserted:\n");
   pbvec(*web_l, *web_u);
#endif

   //bdiag web
   diag = GET_BDIAG(pos);
   bdiag = get_back_diag(board_l, board_u, diag);
   web_bdiag = gen_web_stream(bdiag, GET_BDIAG_POS(pos), GET_BDIAG_LEN(diag));
   bdiag_count = count_contig_bits(web_bdiag, GET_BDIAG_LEN(diag));

#ifdef DEBUG_HEVAL
   printf("  Bdiag moves: %d\n", bdiag_count);
#endif
#ifdef DEBUG_WEB   
   printf("Generating web for bdiag "); psvec(bdiag, GET_BDIAG_LEN(diag));
   printf("Web generated for bdiag at pos %d:", pos);
   psvec(web_bdiag, 10);
#endif

   put_back_diag(web_l, web_u, web_bdiag, diag);
#ifdef DEBUG_WEB   
   printf("web board after bdiag inserted:\n");
   pbvec(*web_l, *web_u);
#endif

   return (row_count + col_count + fdiag_count + bdiag_count - 4);
}


/*==============================================================================
 * count_contig_bits
 *
 * This counts the number of contiguous bits in a stream.  It starts looking at
 * the 0th bit, and starts counting when it finds the first set bit, and stops
 * counting on the next unset bit.
 *
 * Returns the number of contiguous set bits.
 */
int count_contig_bits(ushort stream, int len)
{
   int i;
   int count = 0;

   for (i=0; i<len; i++)
     {
      if (stream & (0x1 << i))
	 ++count;
      else if (count)
	 return count;
     }

   return count;
}



/*==============================================================================
 * pbvec
 *
 * This prints out a bit board with the same ordering as state_print.  
 * 
 */
int pbvec(ull l, ull u)
{
    int i,j;

//   pvec(u);
    for (i=4; i>=0; i--)
      {
       for (j=0; j<10; j++)
         {
          printf("%d ", (int)((u >> (i*10 + j)) &0x1));
         }
       printf("\n");
      }


//    pvec(l);
    for (i=4; i>=0; i--)
      {
       for (j=0; j<10; j++)
         {
          printf("%d ", (int)((l >> (i*10 + j)) &0x1));
         }
       printf("\n");
      }

    return 0;
}

/*==============================================================================
 * tt_lookup
 *
 * This looks a given state up in the transposition table (TT).  If it's found, it 
 * returns a pointer to the tt entry and lets the caller decide if it's usable.
 *
 * Otherwise, it returns NULL.
 *
 * Note: this also allocates the initial TT.  On first run, if the TT hasn't been
 * created yet, this will create it.  This means that it MUST be called before 
 * tt_store, otherwise very bad things will happen.
 *
 */
state_t * tt_lookup(State *s)
{
   ull board_u, board_l;
   int hash_index;  
   int found = 0;
   int max = 0;

   if (tt == NULL)
      tt = (state_t **) calloc(TT, sizeof(state_t *));

   
   board_u = s->white_bd[1] | s->black_bd[1] | s->blocks_bd[1];
   board_l = s->white_bd[0] | s->black_bd[0] | s->blocks_bd[0];

   hash_index = (board_u ^ board_l) % TT;

   while(tt[hash_index] && !found && (max < MAX_TT_SEARCH) && (hash_index < TT))
     {
      found = tt_compare(s,tt[hash_index]);
      ++hash_index;
      ++max;
     }

   ++tt_lookups;
   if (found)
     {
      ++tt_lookup_finds;
      return (tt[--hash_index]);
      /*
      s->value = tt[--hash_index]->value;
      s->depth = tt[hash_index]->depth;
      return TRUE;
      */
     }
   else
      return NULL;
}


/*==============================================================================
 * tt_compare
 *
 * This compares a state to a state_t.  Returns TRUE if they're equivalent, FALSE
 * if not.
 */
int tt_compare(State *s, state_t *t)
{
   if ((s->white_bd[0] == t->white_bd[0]) && (s->white_bd[1] == t->white_bd[1]) &&
       (s->black_bd[0] == t->black_bd[0]) && (s->black_bd[1] == t->black_bd[1]) &&
       (s->blocks_bd[0] == t->blocks_bd[0]) && (s->blocks_bd[1] == t->blocks_bd[1]) &&
       (s->turn == t->turn)) 
      return TRUE;
   else
      return FALSE;
}

/*==============================================================================
 * tt_store
 *
 * tt_store takes a state and stores the necessary components into the TT.  If
 * the calculated hash index is already occupied (collision), it will proceed on
 * to the next one, etc etc...  If it doesn't find an empty spot withint 
 * MAX_TT_SEARCH, it will go back to the initial hash index it calculated, free
 * whatever was there, and replace it with the new one.
 *
 */
void tt_store(State *s, short alpha, short beta)
{
   ull board_u, board_l;
   int hash_index;  
   int max = 0;

   board_u = s->white_bd[1] | s->black_bd[1] | s->blocks_bd[1];
   board_l = s->white_bd[0] | s->black_bd[0] | s->blocks_bd[0];

   hash_index = (board_u ^ board_l) % TT;

   ++tt_stores;

   //looks at a max of MAX_TT_SEARCH items before freeing the first one and storing there
   while(tt[hash_index] && (max < MAX_TT_SEARCH) && (hash_index < TT))
     {
      ++hash_index;
      ++max;
     }

   if (max == MAX_TT_SEARCH)
     {
      hash_index -= max;
      ++tt_overwrite;
     }

   if (tt[hash_index])
      free(tt[hash_index]);

   tt[hash_index] = (state_t *) malloc(sizeof(state_t));
   tt_copy(s, tt[hash_index], alpha, beta);
   
}

/*==============================================================================
 * tt_copy
 *
 * Copies a state struct into a state_t struct.
 */
static void tt_copy(State *s, state_t *t, short alpha, short beta)
{
  
   t->white_bd[0] = s->white_bd[0];
   t->white_bd[1] = s->white_bd[1]; 

   t->black_bd[0] = s->black_bd[0];
   t->black_bd[1] = s->black_bd[1];

   t->blocks_bd[0] = s->blocks_bd[0];
   t->blocks_bd[1] = s->blocks_bd[1];

   t->turn = s->turn; 
   t->value = s->value; 
   t->depth = s->depth;
   t->alpha = alpha;
   t->beta = beta;
}


/*==============================================================================
 * tt_update
 *
 * This is used to update the value and depth members of a TT entry that already
 * exists.  Sometimes, if the value stored in the TT isn't accurate to a very deep
 * level, it's desireable to go ahead and search deeper and update the entry with
 * a more accurate reading.  
 *
 * Of course, it's possible in the meantime for the original entry to have been 
 * overwritten, so if it isn't found, it recreates the entire entry.
 */
void tt_update(State *s, short alpha, short beta)
{
   ull board_u, board_l;
   int hash_index;  
   int found = 0;
   int max = 0;

   if (tt == NULL)
      tt = (state_t **) calloc(TT, sizeof(state_t *));

   board_u = s->white_bd[1] | s->black_bd[1] | s->blocks_bd[1];
   board_l = s->white_bd[0] | s->black_bd[0] | s->blocks_bd[0];

   hash_index = (board_u ^ board_l) % TT;

   while(tt[hash_index] && !found && (max < MAX_TT_SEARCH) && (hash_index < TT))
     {
      found = tt_compare(s,tt[hash_index]);
      ++hash_index;
      ++max;
     }

   if (found)
     {
      tt[--hash_index]->value = s->value;
      tt[hash_index]->depth = s->depth;
      ++tt_updates;
     }
   else //create a new one
     {
      hash_index-= MAX_TT_SEARCH;
      if (tt[hash_index])
	 free(tt[hash_index]);

      tt[hash_index] = (state_t *) malloc(sizeof(state_t));
      tt_copy(s, tt[hash_index], alpha, beta);

      ++tt_overwrite;
     }

}

/*==============================================================================
 * gen_dirs_board
 *
 * This function generates a bitmask board that completely surrounds a single 
 * position.
 *
 * The generated mask will look something like:
 *
 *   1 1 1
 *   1 0 1
 *   1 1 1
 * 
 * Where the 0 bit is the position passed in.  This gets tricky when the position 
 * is right next to a border and part of the mask will need to be amputated 
 * accordingly.
 */
void gen_dirs_board(ull *board_l, ull *board_u, int pos)
{
   int row = GET_COL_POS(pos);
   int row_adj = row % 5;
   int pos_adj = pos % 50;
   ull *board_ptr = NULL;

 /* Generate top row */
   if (row < 9) //position is not against top border, generate this row
     {
      if (pos > 39) //use upper board
	 board_ptr = board_u;
      else
	 board_ptr = board_l;
      //The 2nd half of this expressions is a row bitmask that takes care of
      //positions next to the left or right borders, ensuring that the bits
      //placed on the board stay in the row
      if (pos == 40) 
	 *board_ptr |= 0x3;
      else
	 *board_ptr |= (((ull)0x7 << ((pos_adj + 9) % 50)) & ((ull)0x3ff << (((row_adj + 1)%5) * 10)));
     }

 /* Generate middle row */
   if (pos < 50)
      board_ptr = board_l;  //otherwise board_ptr is still pointing to board_u
   else 
      board_ptr = board_u;  //otherwise board_ptr is still pointing to board_u

   if (pos_adj == 0) 
      *board_ptr |= (ull) (0x2); //in bottom left corner of board half, can't shift neg
   else
      *board_ptr |= (((ull) 0x5 << (pos_adj - 1)) & ((ull) 0x3ff << (row_adj * 10))); 

 /* Generate bottom row */   
   if (row > 0) //position is not against bottom border, generate this row
     {
      if (pos < 60)
	 board_ptr = board_l;  //otherwise board_ptr is still pointing to board_u
      else
	 board_ptr = board_u;  //otherwise board_ptr is still pointing to board_u
      if (pos_adj == 10)
	 *board_ptr |= (ull) 0x3; //in bottom left corner of board half, can't shift neg
      else
	 *board_ptr |= (((ull) 0x7 << ((pos - 11)%50)) & ((ull) 0x3ff << (((row - 1)%5) * 10)));
     }


}

int count_bits(ull board_l, ull board_u)
{
   int count = 0;
   int i;

   for (i=0; i< 64; i++)
     {
      if (board_l & 0x1)
	 ++count;
      board_l >>= 1;

      if (board_u & 0x1)
	 ++count;
      board_u >>= 1;
     }
   return count;
}
