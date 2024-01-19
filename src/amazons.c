#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "amazons.h"


extern struct options  options;
extern Game_states     states;


extern int     onmove;
extern int     ok;
extern time_t  start;            // global, start of search, used for timeout


/* ================================================================ */
/*                    Input functions for moves                     */


/*
 * Receive input from the user on stdin and remove whitespace.
 * All input is made lower case prior to returning. 
 *
 * If EOF was entered, the program stops short.
 */

static char *
get_input(char *prompt, char *buffer, int buflen)
{
  /* Prompt the user so that he knows the program is alive. */
  fputs(prompt, stdout);
  fflush(stdout);

  /* Get input from the user and quit if we got EOF. */
  (void) fgets(buffer, buflen, stdin);
  if (feof(stdin)) {
    printf("Giving up, eh?\n");
    exit(0);
  }

  return strtok(buffer, " \t\n");
}


/*
 * getmove
 *
 * Gets the move coordinates from stdin.  Does lots of nice error
 * checking on the input, and makes sure the move was legal.  If no
 * moves are available, the human must have lost, so the game ends.
 */

Move
getmove(State *state, int player)
{
   char   buffer[100];
   char  *bufp;
   char   tok;
   int    done = FALSE;
   int    move_count;
   Move   move;
   char   move_str[10];
   int    i;

   move_count = state_gen_moves(state, NULL);
   if (move_count == 0)
     {
      printf("Ha ha ha, you lose SUCKER!\n");
      state->winner = OTHER_COLOR(state->turn);
      exit(0);
     }

   do 
     {
      i = 0;

      printf("player %d's move.\n", player);
      fflush(stdout);

      bufp = get_input("Move> ", buffer, 100);
#if 0
      ok = 1;
      isearch(state, THINK);
#endif

      if (bufp == NULL || string_to_move(state, bufp, &move) == 0) {
	printf("Error in move!\n");
	fflush(stdout);
	continue;
      }

      //make sure it's a valid move
      if (!state_is_legal_move(state, state->turn, &move)) {
	printf("%s is not a legal move\n", bufp);
	fflush(stdout);
	continue;
      }

      //User input verified
      done = TRUE;
     }
   while (done == FALSE);

   return move;
}


/* ================================================================ */
/* Main loop */


/* the main play loop. Gets a move, does a move, repeats.
 * global ok is set to 1 for use with timeouts 
 */

void
play_interactive_mode()
{
   State  *state;
   Move    temp;

   state = states.states[0];
   for (;;) {
      state_copy(state, states.states[++(states.current_state)]);
      state = states.states[states.current_state];
      if (states.current_state > states.max_state)
	 states.max_state = states.current_state;

      if (options.white_player == HUMAN)
       	{
	 temp = getmove(state, 1);
	 makemove(state, temp);
	 onmove++;
	 state_print(state);
       	}
      else
       	{
	 ok = 1;
	 start = time(NULL);
	 temp = isearch(state, NOTHINK, NULL);
	 printf("WHITE selected: ");
	 pmove(state, WHITE, temp);
	 makemove(state, temp);
	 onmove++;
	 state_print(state);
       	}

      state_copy(state, states.states[++(states.current_state)]);
      state = states.states[states.current_state];
      if (states.current_state > states.max_state)
	 states.max_state = states.current_state;

      if (options.black_player == HUMAN)
       	{
	 temp = getmove(state, 2);
	 makemove(state, temp);
	 onmove++;
	 state_print(state);
       	}
      else
       	{
	 ok = 1;
	 start = time(NULL);
	 temp = isearch(state, NOTHINK, NULL);
	 printf("BLACK selected: ");
	 pmove(state, BLACK, temp);
	 makemove(state,temp);
	 onmove++;
	 state_print(state);
       	}
     }
}


/* ================================================================ */
/*                           Main program                           */


#define MODE_INTERACTIVE  0
#define MODE_GTP          1

int  play_mode;


/* print_usage 
 *
 * Displays a list of all command line options
 */
void print_usage()
{
   printf("Amazons Usage:\n");
   printf("amazons [OPTIONS]\n");
   printf("     -G:     Play in GTP mode\n");
   printf("     -d #:   Sets the maximum search depth\n");
   printf("     -h:     Prints this help menu\n");
   printf("     -p #:   User plays against the computer (1 white, 2 black)\n");
   printf("     -s:     Prints out interesting statistics\n");
   printf("     -t #:   Sets the time limit between moves\n");
   printf("\n\n");
}


/* parse args
 *
 * Handles the arguments passed into the program.  
 *
 * Note: this expects all arguments to be listed separately, 
 * each with it's own '-'
 */
void 
parse_args(int argc, char *argv[])
{
   int i = 1;

   //Set defaults
   options.engine.maxdepth=20;
   options.engine.maxwidth=5000;
   options.engine.timeout=2;
   options.white_player=HUMAN;
   options.black_player=AI;
   options.print_statistics=FALSE;

   play_mode = MODE_INTERACTIVE;
   while (i < argc)
     {
      switch(argv[i][1])
       	{
	   case 'G':
	      play_mode = MODE_GTP;
	      i++;
	      break;
	  case 'd':
	      options.engine.maxdepth = atoi(argv[++i]);
	      i++;
	      break;
	  case 'h':
	      print_usage();
	      exit(0);
	  case 'p':
	      if (atoi(argv[++i]) == 1)
		 options.white_player = HUMAN;
	      else
		 options.black_player = HUMAN;
	      i++;
	      break;
	  case 's':
	      options.print_statistics = TRUE;
	      i++;
	      break;
	  case 't':
	      options.engine.timeout = atoi(argv[++i]);
	      i++;
	      break;
	  default:
	      printf("Unknown argument  %s\n", argv[i]);
	      print_usage();
	      exit(1);
       	}
     }
}


int
main(int argc, char *argv[])
{
   State  *state;
   Move    temp;
   int     i;

   // Initialize game states
   for (i=0; i<100; i++)
      states.states[i] = (State *) malloc(sizeof(State));
   states.current_state = 0;
   states.max_state = 0;
   state = states.states[0];

   parse_args(argc, argv);
   state_init(state);

   if (play_mode == MODE_INTERACTIVE) {
       printf("size of move = %d\n", sizeof(Move));
       printf("size of state = %d\n", sizeof(State));

       state_print(state);
   }

   //test_fdiag(state);
   //test_bdiag(state);
   //test_gen_web_stream(state);
   //test_put_col(state);
   //test_put_row(state);
   //test_put_fdiag(state);
   //test_put_bdiag(state);

#ifdef DEBUG_HEVAL
   bitmap_flip(state->white_bd, 9, 3);
   bitmap_flip(state->white_bd, 8, 2);
   state->white_q_x[3] = 8;
   state->white_q_y[3] = 2;
   bitmap_flip(state->blocks_bd, 8, 3);
   state->turn = BLACK_PLAYER;

   state_print(state);
   printf("board value is %d\n", HEVAL(state));

   pbvec(state->white_bd[0], state->white_bd[1]);

#else

   switch (play_mode) {
   case MODE_INTERACTIVE:
       play_interactive_mode();
       break;
   case MODE_GTP:
#if 0
       play_gtp_mode(stdin, NULL);
#endif
       break;
   default:
       exit(1);
   }

#endif

   return 0;

}
