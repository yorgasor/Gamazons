#include <stdio.h>
#include "amazons.h"
#include "unit-test.h"


void test_fdiag(State *s)
{
   int i;
   ull board_l, board_u;
   int moves;
   int diag;

   board_u = s->white_bd[1] | s->black_bd[1] | s->blocks_bd[1];
   board_l = s->white_bd[0] | s->black_bd[0] | s->blocks_bd[0];

   for (i=0; i<100; i++)
     {
      diag = GET_FDIAG(i);
      psvec(get_forward_diag(board_l, board_u, diag), GET_FDIAG_LEN(diag));
      printf("Pos %d is on FDIAG %d and has stream position %d of len %d\n",
	      i, diag, GET_FDIAG_POS(i), GET_FDIAG_LEN(diag));
      moves = calc_stream_moves(
	      get_forward_diag(board_l, board_u, diag),
	      GET_FDIAG_POS(i),
	      GET_FDIAG_LEN(diag)
	      );
      printf("Pos %d has %d FDIAG moves\n", i, moves);
     }


}

void test_bdiag(State *s)
{
   int i;
   ull board_l, board_u;
   int moves;
   int diag;

   board_u = s->white_bd[1] | s->black_bd[1] | s->blocks_bd[1];
   board_l = s->white_bd[0] | s->black_bd[0] | s->blocks_bd[0];

   for (i=0; i<100; i++)
     {
      diag = GET_BDIAG(i);
      psvec(get_back_diag(board_l, board_u, diag), GET_BDIAG_LEN(diag));
      printf("Pos %d is on BDIAG %d and has stream position %d of len %d\n",
	      i, diag, GET_BDIAG_POS(i), GET_BDIAG_LEN(diag));
      moves = calc_stream_moves(
	      get_back_diag(board_l, board_u, diag),
	      GET_BDIAG_POS(i),
	      GET_BDIAG_LEN(diag)
	      );
      printf("Pos %d has %d BDIAG moves\n", i, moves);
     }


}

void test_gen_web_stream(State *s)
{
   int i;
   ull board_l, board_u;
   int diag;
   ushort web;
   ushort stream;

   board_u = s->white_bd[1] | s->black_bd[1] | s->blocks_bd[1];
   board_l = s->white_bd[0] | s->black_bd[0] | s->blocks_bd[0];

   for (i=0; i<100; i++)
     {
      diag = GET_FDIAG(i);
      stream = get_forward_diag(board_l, board_u, diag);
      printf("Pos %d is on FDIAG %d and has stream position %d of len %d\n",
	      i, diag, GET_FDIAG_POS(i), GET_FDIAG_LEN(diag));
      psvec(stream, GET_FDIAG_LEN(diag));
      web = gen_web_stream(stream, GET_FDIAG_POS(i), GET_FDIAG_LEN(diag));
      psvec(web, GET_FDIAG_LEN(diag));
     }
}

void test_put_row(State *s)
{
   int i;
   ull board_l, board_u;
   ull web_board_l, web_board_u;
   ushort web;
   ushort stream;

   board_u = s->white_bd[1] | s->black_bd[1] | s->blocks_bd[1];
   board_l = s->white_bd[0] | s->black_bd[0] | s->blocks_bd[0];

   for (i=0; i<100; i++)
     {
      web_board_l = web_board_u = 0;

      if (i<50)
	 stream = GET_ROW(board_l,i/10);
      else
	 stream = GET_ROW(board_u,i/10);

      printf("Pos %d is on ROW %d and has stream position %d\n",
	      i, i/10, GET_ROW_POS(i));
      psvec(stream, 10);
      web = gen_web_stream(stream, GET_ROW_POS(i), 10);
      psvec(web, 10);

      if (i<50)
	{
	 PUT_ROW(web_board_l, i/10, web);
	 pvec(web_board_l);
	}
      else
	{
	 PUT_ROW(web_board_u, i/10, web);
	 pvec(web_board_u);
	}
     }
}

void test_put_col(State *s)
{
   int i;
   ull board_l, board_u;
   ull web_board_l, web_board_u;
   ushort web;
   ushort stream;

   board_u = s->white_bd[1] | s->black_bd[1] | s->blocks_bd[1];
   board_l = s->white_bd[0] | s->black_bd[0] | s->blocks_bd[0];

   for (i=0; i<100; i++)
     {
      web_board_l = web_board_u = 0;

      stream = GET_COL(board_l, board_u, i%10);

      printf("Pos %d is on COL %d and has stream position %d\n",
	      i, i%10, GET_COL_POS(i));
      psvec(stream, 10);
      web = gen_web_stream(stream, GET_COL_POS(i), 10);
      psvec(web, 10);

      PUT_COL(web_board_l, web_board_u, i%10, web);
      pvec(web_board_l);
      pvec(web_board_u);
     }
}

void test_put_fdiag(State *s)
{
   int i;
   ull board_l, board_u;
   ull web_board_l, web_board_u;
   ushort web;
   ushort stream;
   int diag, len;

   board_u = s->white_bd[1] | s->black_bd[1] | s->blocks_bd[1];
   board_l = s->white_bd[0] | s->black_bd[0] | s->blocks_bd[0];

   for (i=0; i<100; i++)
     {
      web_board_l = web_board_u = 0;

      diag = GET_FDIAG(i);
      len = GET_FDIAG_LEN(diag);
      stream = get_forward_diag(board_l, board_u, diag);

      printf("Pos %d is on FDIAG %d and has stream position %d of len %d\n",
	      i, diag, GET_FDIAG_POS(i), len);
      psvec(stream, len);
      web = gen_web_stream(stream, GET_FDIAG_POS(i), len);
      psvec(web, len);

      put_forward_diag(&web_board_l, &web_board_u, web, diag);
      pvec(web_board_l);
      pvec(web_board_u);
     }
}

void test_put_bdiag(State *s)
{
   int i;
   ull board_l, board_u;
   ull web_board_l, web_board_u;
   ushort web;
   ushort stream;
   int diag, len;

   bitmap_flip(s->white_bd, 0,5);

   board_u = s->white_bd[1] | s->black_bd[1] | s->blocks_bd[1];
   board_l = s->white_bd[0] | s->black_bd[0] | s->blocks_bd[0];

   for (i=0; i<100; i++)
     {
      web_board_l = web_board_u = 0;

      diag = GET_BDIAG(i);
      len = GET_BDIAG_LEN(diag);
      stream = get_back_diag(board_l, board_u, diag);

      printf("Pos %d is on BDIAG %d and has stream position %d of len %d\n",
	      i, diag, GET_BDIAG_POS(i), len);
      psvec(stream, len);
      web = gen_web_stream(stream, GET_BDIAG_POS(i), len);
      psvec(web, len);

      put_back_diag(&web_board_l, &web_board_u, web, diag);
      pvec(web_board_l);
      pvec(web_board_u);
     }
}


int psvec(ushort v, int len)
{
    int i;

    printf("->");
    for (i=len-1; i >= 0; i--)
    {
        printf("%d", (int)((v >> i) & (ull)1));
    }
    printf("<-\n");

    return 0;
}

