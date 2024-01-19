#include <gnome.h>
#include "amazons.h"
#include "board.h"
#include "bstate.h"

static Board_State  bstate;
extern Game_states  states;
extern struct options options;

extern GtkWidget    *main_window;


/* ===========================================================================
 * bstate_set_just_finished
 *
 * tells the internal state machine what state was just finished.  It then 
 * figures out from that what the next state should be.  This is designed to 
 * eliminate the redundant code scattered all around that tries to figure
 * this out.
 */
int
bstate_set_just_finished(int finished)
{
   GtkWidget *auto_button;
   GtkWidget *force_button;
   GtkWidget *undo_button;
   GtkWidget *settings_menu;
   GtkWidget *stop_button;
   GtkWidget *delay_spinner;
   State     *s;

   /* Find some widgets on the screen. */
   auto_button   = (GtkWidget *) lookup_widget(main_window, "BT_AUTOFINISH");
   force_button  = (GtkWidget *) lookup_widget(main_window, "BT_FORCEMOVE");
   undo_button   = (GtkWidget *) lookup_widget(main_window, "BT_UNDO");
   settings_menu = (GtkWidget *) lookup_widget(main_window, "Settings");
   stop_button   = (GtkWidget *) lookup_widget(main_window, "BT_REPLAY_STOP");
   delay_spinner = (GtkWidget *) lookup_widget(main_window, "ReplayDelaySpinner"); 

   switch(finished)
     {
       case FIRE_ARROW:
	   if (bstate.turn == BLACK)
	     {
	      if (options.white_player == AI)
		{
		 bstate.what_next = WAIT_FOR_AI;
		 bstate.moving_ai = TRUE;

		 gtk_widget_set_sensitive (undo_button, FALSE);
		 gtk_widget_set_sensitive (auto_button, FALSE);
		 gtk_widget_set_sensitive (force_button, TRUE);
		}
	      else
		{
		 bstate.what_next = MOVE_WHITE_QUEEN;
		 bstate.moving_ai = FALSE;

		 gtk_widget_set_sensitive (undo_button, FALSE);
		 gtk_widget_set_sensitive (auto_button, TRUE);
		 gtk_widget_set_sensitive (force_button, FALSE);
		}
	      bstate.turn = WHITE;
	     }
	   else
	     {
	      if (options.black_player == AI)
		{
		 bstate.what_next = WAIT_FOR_AI;
		 bstate.moving_ai = TRUE;

		 gtk_widget_set_sensitive (undo_button, FALSE);
		 gtk_widget_set_sensitive (auto_button, FALSE);
		 gtk_widget_set_sensitive (force_button, TRUE);
		}
	      else
		{
		 bstate.what_next = MOVE_BLACK_QUEEN;
		 bstate.moving_ai = FALSE;

		 gtk_widget_set_sensitive (undo_button, FALSE);
		 gtk_widget_set_sensitive (auto_button, TRUE);
		 gtk_widget_set_sensitive (force_button, FALSE);
		}
	      bstate.turn = BLACK;
	     }
	   gtk_widget_set_sensitive (settings_menu, TRUE);
	   break;
       case MOVE_BLACK_QUEEN:
       case MOVE_WHITE_QUEEN:
	   bstate.what_next = FIRE_ARROW;
	   gtk_widget_set_sensitive (undo_button, TRUE);
	   gtk_widget_set_sensitive (auto_button, FALSE);
	   gtk_widget_set_sensitive (settings_menu, FALSE);
	   break;
       case WAIT_FOR_AI:
	   if (bstate.turn == BLACK)
	     {
	      if (options.white_player == AI)
		{
		 bstate.what_next = WAIT_FOR_AI;
		 bstate.moving_ai = TRUE;

		 gtk_widget_set_sensitive (undo_button, FALSE);
		 gtk_widget_set_sensitive (auto_button, FALSE);
		 gtk_widget_set_sensitive (force_button, TRUE);
		}
	      else
		{
		 bstate.what_next = MOVE_WHITE_QUEEN;
		 bstate.moving_ai = FALSE;

		 gtk_widget_set_sensitive (undo_button, FALSE);
		 gtk_widget_set_sensitive (auto_button, TRUE);
		 gtk_widget_set_sensitive (force_button, FALSE);
		}
	      bstate.turn = WHITE;
	     }
	   else
	     {
	      if (options.black_player == AI)
		{
		 bstate.what_next = WAIT_FOR_AI;
		 bstate.moving_ai = TRUE;

		 gtk_widget_set_sensitive (undo_button, FALSE);
		 gtk_widget_set_sensitive (auto_button, FALSE);
		 gtk_widget_set_sensitive (force_button, TRUE);
		}
	      else
		{
		 bstate.what_next = MOVE_BLACK_QUEEN;
		 bstate.moving_ai = FALSE;

		 gtk_widget_set_sensitive (undo_button, FALSE);
		 gtk_widget_set_sensitive (auto_button, TRUE);
		 gtk_widget_set_sensitive (force_button, FALSE);
		}
	      bstate.turn = BLACK;
	     }
	   break;
       case UNDO:
	   if (bstate.turn == BLACK)
	     {
	      bstate.what_next = MOVE_BLACK_QUEEN;
	     }
	   else
	     {
	      bstate.what_next = MOVE_WHITE_QUEEN;
	     }
	   gtk_widget_set_sensitive (settings_menu, TRUE);
	   gtk_widget_set_sensitive (undo_button, FALSE);
	   gtk_widget_set_sensitive (auto_button, TRUE);
	   break;
       case AUTO_FINISH:
	   bstate.what_next = WAIT_FOR_AI;
	   gtk_widget_set_sensitive (settings_menu, FALSE);
	   gtk_widget_set_sensitive (undo_button, FALSE);
	   gtk_widget_set_sensitive (auto_button, FALSE);
	   break;
       case FORCE_MOVE:
	   gtk_widget_set_sensitive (force_button, FALSE);
	   break;
       case START_GAME:
	   if (options.white_player == AI)
	     {
	      bstate.what_next = NEW_GAME;
	      gtk_widget_set_sensitive (undo_button, FALSE);
	      gtk_widget_set_sensitive (auto_button, FALSE);
	      gtk_widget_set_sensitive (force_button, FALSE);
	     }
	   else
	     {
	      bstate.what_next = MOVE_WHITE_QUEEN;
	      gtk_widget_set_sensitive (undo_button, FALSE);
	      gtk_widget_set_sensitive (auto_button, TRUE);
	      gtk_widget_set_sensitive (force_button, FALSE);
	     }
	   gtk_widget_set_sensitive (stop_button, FALSE);
	   gtk_widget_set_sensitive (delay_spinner, FALSE);
	   gtk_widget_set_sensitive (settings_menu, TRUE);
	   bstate.turn = WHITE;
	   bstate.from = INVALID_SQUARE_VALUE;
	   bstate.to = INVALID_SQUARE_VALUE;
	   bstate.last_arrow = INVALID_SQUARE_VALUE;
	   bstate.moving_ai = FALSE;
	   bstate.open_dialog = FALSE;
	   //bstate.new_game = FALSE;
	   bstate.quit_game = FALSE;
	   bstate.game_life++;
	   break;
       case NEW_GAME:
	   /*
	   if (bstate.moving_ai) //in the middle of an AI move
	      bstate.new_game = TRUE;
	   else
	      bstate.new_game = FALSE;
	      */

	   if (options.white_player == AI)
	     {
	      bstate.what_next = WAIT_FOR_AI;
	      bstate.moving_ai = TRUE;
	      gtk_widget_set_sensitive (undo_button, FALSE);
	      gtk_widget_set_sensitive (auto_button, FALSE);
	      gtk_widget_set_sensitive (force_button, TRUE);
	     }
	   else
	     {
	      bstate.what_next = MOVE_WHITE_QUEEN;
	      bstate.moving_ai = FALSE;
	      gtk_widget_set_sensitive (undo_button, FALSE);
	      gtk_widget_set_sensitive (auto_button, TRUE);
	      gtk_widget_set_sensitive (force_button, FALSE);
	     }
	   gtk_widget_set_sensitive (settings_menu, TRUE);
	   gtk_widget_set_sensitive (stop_button, FALSE);
	   gtk_widget_set_sensitive (delay_spinner, FALSE);
	   bstate.turn = WHITE;
	   bstate.from = INVALID_SQUARE_VALUE;
	   bstate.to = INVALID_SQUARE_VALUE;
	   bstate.last_arrow = INVALID_SQUARE_VALUE;
	   bstate.quit_game = FALSE;
	   bstate.replay_mode = FALSE;
	   bstate.game_life++;

	   break;

       case LOAD_GAME:
	   s = states.states[states.current_state];
	   bstate.turn = s->turn;
	   if (bstate.turn == WHITE)
	     {
	      if (options.white_player == AI)
		{
		 bstate.what_next = WAIT_FOR_AI;
		 bstate.moving_ai = TRUE;

		 gtk_widget_set_sensitive (undo_button, FALSE);
		 gtk_widget_set_sensitive (auto_button, FALSE);
		 gtk_widget_set_sensitive (force_button, TRUE);
		}
	      else
		{
		 bstate.what_next = MOVE_WHITE_QUEEN;
		 bstate.moving_ai = FALSE;

		 gtk_widget_set_sensitive (undo_button, FALSE);
		 gtk_widget_set_sensitive (auto_button, TRUE);
		 gtk_widget_set_sensitive (force_button, FALSE);
		}
	     }
	   else
	     {
	      if (options.black_player == AI)
		{
		 bstate.what_next = WAIT_FOR_AI;
		 bstate.moving_ai = TRUE;

		 gtk_widget_set_sensitive (undo_button, FALSE);
		 gtk_widget_set_sensitive (auto_button, FALSE);
		 gtk_widget_set_sensitive (force_button, TRUE);
		}
	      else
		{
		 bstate.what_next = MOVE_BLACK_QUEEN;
		 bstate.moving_ai = FALSE;

		 gtk_widget_set_sensitive (undo_button, FALSE);
		 gtk_widget_set_sensitive (auto_button, TRUE);
		 gtk_widget_set_sensitive (force_button, FALSE);
		}
	     }
	   gtk_widget_set_sensitive (settings_menu, TRUE);
	   gtk_widget_set_sensitive (stop_button, FALSE);
	   gtk_widget_set_sensitive (delay_spinner, FALSE);
	   bstate.replay_mode = FALSE;
	   bstate.game_life++;
	   game_over(); //check if the game is already over
	   break;
       case START_REPLAY:
	   gtk_widget_set_sensitive (stop_button, TRUE);
	   gtk_widget_set_sensitive (delay_spinner, TRUE);
	   gtk_widget_set_sensitive (force_button, FALSE);
	   gtk_widget_set_sensitive (auto_button, FALSE);
	   gtk_widget_set_sensitive (settings_menu, FALSE);
	   bstate.what_next = STOP_REPLAY;
	   bstate.replay_mode = TRUE;
	   bstate.game_life++;

	   break;
       case STOP_REPLAY:
	   gtk_widget_set_sensitive (stop_button, FALSE);
	   gtk_widget_set_sensitive (delay_spinner, FALSE);
	   bstate.replay_mode = FALSE;

	   s = states.states[states.current_state];
	   bstate.turn = s->turn;
	   if (bstate.turn == WHITE)
	     {
	      if (options.white_player == AI)
		{
		 bstate.what_next = WAIT_FOR_AI;
		 bstate.moving_ai = TRUE;

		 gtk_widget_set_sensitive (undo_button, FALSE);
		 gtk_widget_set_sensitive (auto_button, FALSE);
		 gtk_widget_set_sensitive (force_button, TRUE);
		}
	      else
		{
		 bstate.what_next = MOVE_WHITE_QUEEN;
		 bstate.moving_ai = FALSE;

		 gtk_widget_set_sensitive (undo_button, FALSE);
		 gtk_widget_set_sensitive (auto_button, TRUE);
		 gtk_widget_set_sensitive (force_button, FALSE);
		}
	     }
	   else
	     {
	      if (options.black_player == AI)
		{
		 bstate.what_next = WAIT_FOR_AI;
		 bstate.moving_ai = TRUE;

		 gtk_widget_set_sensitive (undo_button, FALSE);
		 gtk_widget_set_sensitive (auto_button, FALSE);
		 gtk_widget_set_sensitive (force_button, TRUE);
		}
	      else
		{
		 bstate.what_next = MOVE_BLACK_QUEEN;
		 bstate.moving_ai = FALSE;

		 gtk_widget_set_sensitive (undo_button, FALSE);
		 gtk_widget_set_sensitive (auto_button, TRUE);
		 gtk_widget_set_sensitive (force_button, FALSE);
		}
	     }
	   gtk_widget_set_sensitive (settings_menu, TRUE);
	   game_over(); //check if the game is already over

	   break;
       case GAME_OVER:
	   gtk_widget_set_sensitive (undo_button, FALSE);
	   gtk_widget_set_sensitive (auto_button, FALSE);
	   gtk_widget_set_sensitive (force_button, FALSE);
	   gtk_widget_set_sensitive (settings_menu, TRUE);
	   bstate.what_next = NEW_GAME;
	   bstate.game_life++;
	   break;

       case QUIT_GAME:
	   bstate.quit_game = TRUE;
	   bstate.game_life++;
	   break;
       default:
	   bstate.what_next = CONFUSED;
     }
   update_status_bar();
#ifdef DEBUG
   print_bstate();
#endif
   return bstate.what_next;

}

/*==============================================================================
 * bstate_store_what_next
 *
 * This initializes the internal board state machine.  Use of this is discouraged
 * and should instead use bstate_set_just_finished.
void bstate_store_what_next(int next)
{
   switch(next)
     {
       case FIRE_ARROW:
	   bstate.what_next = FIRE_ARROW;
	   break;
       case MOVE_BLACK_QUEEN:
	   if (options.black_player == AI)
	      bstate.what_next = WAIT_FOR_AI;
	   else
	      bstate.what_next = MOVE_BLACK_QUEEN;
	   bstate.turn = BLACK;
	   break;
       case MOVE_WHITE_QUEEN:
	   if (options.white_player == AI)
	      bstate.what_next = WAIT_FOR_AI;
	   else
	      bstate.what_next = MOVE_WHITE_QUEEN;
	   bstate.turn = WHITE;
	   break;
       case WAIT_FOR_AI:
	   bstate.what_next = WAIT_FOR_AI;
	   break;
       default:
	   bstate.what_next = CONFUSED;
     }
   update_status_bar();
}
 */

/*==============================================================================
 * bstate_get_what_next
 *
 * This is the method for code throughout the program to find out what input
 * is expected next.
 */
int bstate_get_what_next()
{
   return bstate.what_next;
}

/*==============================================================================
 * bstate_set_move_from
 *
 * This lets the internal board state know that a queen has just been moved from
 * a given square.
 */
void bstate_set_move_from(Square from)
{
   bstate.from = from;
}

/*==============================================================================
 * bstate_set_move_from
 *
 * This lets the internal board state know that a queen has just been moved to
 * a given square.
 */
void bstate_set_move_to(Square to)
{
   bstate.to = to;
}

/*==============================================================================
 * bstate_get_move_from
 *
 * This lets the internal board state know that a queen has just been moved from
 * a given square.
 */
Square bstate_get_move_from()
{
   return bstate.from;
}

/*==============================================================================
 * bstate_get_move_from
 *
 * This lets the internal board state know that a queen has just been moved to
 * a given square.
 */
Square bstate_get_move_to()
{
   return bstate.to;
}

int bstate_get_moving_ai()
{
   return bstate.moving_ai;
}
/*
void bstate_set_moving_ai(int moving)
{
   bstate.moving_ai = moving;
}
*/
/*
int bstate_get_new_game()
{
   return bstate.new_game;
}
void bstate_set_new_game(int new_game)
{
    bstate.new_game = new_game;
}
*/

void bstate_set_moving_piece(int moving)
{
   bstate.moving_piece = moving;
}
int bstate_get_moving_piece()
{
   return bstate.moving_piece;
}

int bstate_get_turn()
{
   return bstate.turn;
}

int bstate_get_open_dialog()
{
   return bstate.open_dialog;
}

void bstate_set_open_dialog(int dialog)
{
   bstate.open_dialog = dialog;
}

void bstate_set_dont_slide(int slide)
{
   bstate.dont_slide = slide;
}

int bstate_get_dont_slide()
{
   return bstate.dont_slide;
}

int bstate_get_game_life()
{
   return bstate.game_life;
}

int bstate_get_replay_mode()
{
   return bstate.replay_mode;
}

void bstate_set_replay_mode(int mode)
{
   bstate.replay_mode = mode;
}

/*==============================================================================
 * bstate_update_player_settings
 *
 * After the player settings window closes, the game state must be updated to 
 * reflect those changes.  However, if we're waiting for a NEW_GAME to come, we
 * don't do anything.  Unfortunately, this means that if, when the game starts
 * and an AI is expected to move, and the user changes this player to HUMAN, the
 * user can't just go ahead and start moving that piece.  He must still select a
 * new game from the menu.  However, this does prevent weird things from happening
 * if, after a game is finished and we're again waiting for NEW_GAME and the losing
 * player is changed.
 */
void bstate_update_player_settings()
{
   if (bstate.what_next == NEW_GAME)
      return;
   
   if (bstate.turn == WHITE && options.white_player == HUMAN)
      bstate.what_next = MOVE_WHITE_QUEEN;
   if (bstate.turn == BLACK && options.black_player == HUMAN)
      bstate.what_next = MOVE_BLACK_QUEEN;

   if (bstate.turn == WHITE && options.white_player == AI)
      bstate.what_next = WAIT_FOR_AI;
   if (bstate.turn == BLACK && options.black_player == AI)
      bstate.what_next = WAIT_FOR_AI;
   update_status_bar();

}

int bstate_get_quit_game()
{
   return bstate.quit_game;
}


void print_bstate()
{
   printf("turn = %d\n", bstate.turn);
   printf("what_next = %d\n", bstate.what_next);
   printf("moving_ai = %d\n", bstate.moving_ai);
}

