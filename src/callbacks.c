#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include <libgen.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "amazons.h"
#include "board.h"
#include "bstate.h"

extern Board *board;
extern struct options options;
extern Game_states    states;
extern Square legal_moves[100];
extern int ok;
extern GtkWidget *main_window;
extern state_hash;
int grabbed_queen;
GtkWidget *PlayerSettingsWindow;
extern int moving_ai;

GtkWidget *file_selector;
gchar *selected_filename;

/* Local Prototypes */
void load_new_theme (GtkFileSelection *selector, gpointer user_data);
void save_game_history(GtkFileSelection *selector, gpointer user_data);
void on_ThemeCancelButton_clicked(GtkButton *button, gpointer user_data);
void on_save_as1CancelButton_clicked(GtkButton *button, gpointer user_data);
void on_OpenCancelButton_clicked(GtkButton *button, gpointer user_data);
void load_saved_game(GtkFileSelection *selector, gpointer user_data);


/* ================================================================ */


/* Called when the "New game" meny item is chosen.
 */

void
on_new1_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   /*
   if (bstate_get_moving_piece())
     {
      gnome_warning_dialog("Unable to start a new game while a piece is being moved.  Please try again.");
      return;
     }
     */
   free_all_memory();
   init_engine();
   board_destroy();
   board_init_game(main_window);
   load_values_from_file();

   bstate_set_just_finished(NEW_GAME);
   while (bstate_get_what_next() == WAIT_FOR_AI)
      move_ai();  //if both players are AI, keeps on running
}




void
on_quit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   state_hash = 0;
   bstate_set_just_finished(QUIT_GAME);
   gtk_main_quit();
}



void
on_clear1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_preferences1_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   static GtkWidget *about;
   GdkPixbuf *pixbuf = NULL;

   const gchar *authors[] = {"Ron Yorgason", "Sean McMillen", NULL};
   gchar *documenters[] = {
      NULL
   };
   /* Translator credits */
   gchar *translator_credits = _("translator_credits");

   if (about != NULL) {
      gdk_window_raise (about->window);
      gdk_window_show (about->window);
      return;
   }

     {
      char filename[1024];

      strcpy(filename, PACKAGE_DATA_DIR "/pixmaps/gnome-gamazons.png");
      pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
     }

   about = gnome_about_new (_("Gamazons"), VERSION,
	   "(C) 2002 Ronald Yorgason and Sean McMillen",
	   _("Send comments and bug reports to: "
	       "yorgasor@cs.pdx.edu"),
	   (const char **)authors,
	   (const char **)documenters,
	   strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
	   pixbuf);
   g_signal_connect (G_OBJECT (about), "destroy", G_CALLBACK
	   (gtk_widget_destroyed), &about);
   gtk_widget_show (about);
}


void
on_network1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_player1_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   GtkWidget *time, *width, *depth;
   GtkWidget *white_ai, *white_h, *black_ai, *black_h;


   if (bstate_get_open_dialog())
      return;

   bstate_set_open_dialog(TRUE);
   PlayerSettingsWindow = create_PlayerSettings();

   /* grab all the widgets */
   time = lookup_widget(PlayerSettingsWindow, "TimeSpinner");
   width = lookup_widget(PlayerSettingsWindow, "WidthSpinner");
   depth = lookup_widget(PlayerSettingsWindow, "DepthSpinner");
   white_ai = lookup_widget(PlayerSettingsWindow, "WhiteAIRadio");
   white_h = lookup_widget(PlayerSettingsWindow, "WhiteHumanRadio");
   black_ai = lookup_widget(PlayerSettingsWindow, "BlackAIRadio");
   black_h = lookup_widget(PlayerSettingsWindow, "BlackHumanRadio");




   /* Load current values */
   load_values_from_file();
   gtk_spin_button_set_value( (GtkSpinButton *)time, options.engine.timeout);
   gtk_spin_button_set_value( (GtkSpinButton *)width, options.engine.maxwidth);
   gtk_spin_button_set_value( (GtkSpinButton *)depth, options.engine.maxdepth);

   if (options.white_player == HUMAN)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(white_h), TRUE);
   else
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(white_ai), TRUE);

   if (options.black_player == HUMAN)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(black_h), TRUE);
   else
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(black_ai), TRUE);


   gtk_widget_show(PlayerSettingsWindow);

}


void
on_how_to_play1_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    GError *error = NULL;
    if (gnome_help_display("gamazons",NULL,&error))
      {
#ifdef DEBUG
       printf("I should pop up the help display now\n");
#endif
      }
    else
      {
#ifdef DEBUG
       printf("I couldn't display the help screen, sorry.\n");
#endif
       printf("%s\n", error->message );
      }
}


void
on_help2_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
}


void
on_BT_UNDO_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
   int temp;
   GtkWidget *undo_button;
   int i;

   if (bstate_get_moving_piece())
      return;
   if (bstate_get_open_dialog())
      return;

   if (bstate_get_what_next() == FIRE_ARROW) //Undo during mid-move
     {
      int to_col, from_col, to_row, from_row;
      Square to, from;

      from = bstate_get_move_to();
      to = bstate_get_move_from();

      bstate_set_move_to(to);
      bstate_set_move_from(from);

      move_piece_to (to, board->selected_queen);

      //roll back the state of the board
      from_col = get_x_int_from_square(bstate_get_move_from());
      from_row = get_y_int_from_square(bstate_get_move_from());

      to_col = get_x_int_from_square(bstate_get_move_to());
      to_row = get_y_int_from_square(bstate_get_move_to());

      board->squares[to_row][to_col] = board->squares[from_row][from_col];
      board->squares[from_row][from_col] = EMPTY_SQUARE;
      gen_legal_moves(bstate_get_move_to()); 

      if (board->squares[to_row][to_col] == WHITE)
	{
	 bstate_set_just_finished(UNDO);
	 for (i=0; i<4; i++) //update queen to square mapping
	   {
	    if (board->square_to_queen_map[WHITE][i] == to_row*10 + to_col)
	      {
	       board->square_to_queen_map[WHITE][i] = from_row*10 + from_col;
	       break;
	      }
	   }
	}
      else
	{
	 bstate_set_just_finished(UNDO);
	 for (i=0; i<4; i++) //update queen to square mapping
	   {
	    if (board->square_to_queen_map[BLACK][i] == to_row*10 + to_col)
	      {
	       board->square_to_queen_map[BLACK][i] = from_row*10 + from_col;
	       break;
	      }
	   }
	}

     }
}


/*==============================================================================
 * on_BT_FORCEMOVE_clicked
 *
 * This sets the global timeout variable, forcing the AI to take the best move
 * currently available.
 */
void
on_BT_FORCEMOVE_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
   if (bstate_get_moving_piece())
      return;
   if (bstate_get_open_dialog())
      return;
   bstate_set_just_finished(FORCE_MOVE);
   ok = 0;
}


/*==============================================================================
 * on_BT_AUTOFINISH_clicked               
 *
 * This starts the auto-finish process if allowed.  This can't be done inbetween
 * a human player moving & firing an arrow, or while an AI is moving.
 *
 */
void
on_BT_AUTOFINISH_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
   if (bstate_get_moving_piece())
      return;
   if (bstate_get_open_dialog())
      return;
   options.white_player = AI;
   options.black_player = AI;
   options.engine.timeout = 1;
   options.engine.maxdepth = 5;
   options.engine.maxwidth = 15;
   bstate_set_just_finished(AUTO_FINISH);
   while (bstate_get_what_next() == WAIT_FOR_AI) 
      move_ai();  //if both players are AI, keeps on running

}


void
on_GamazonsMain_destroy                (GtkObject       *object,
                                        gpointer         user_data)
{
   state_hash = 0;
   bstate_set_just_finished(QUIT_GAME);
   gtk_main_quit(); 
}



/*==============================================================================
 * board_press_cb
 *
 * This is the main board handling routine.  Determines what the user is clicking
 * on and what a click means.  
 */
int board_press_cb (GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
   double drop_x, drop_y;
   double new_x, new_y;
   int from_row, from_col, to_row, to_col;
   int i,j,k;
   GtkWidget *auto_button, *undo_button;

   static Square to;
   static Square from;

   if (bstate_get_moving_piece())
     {
      //printf("Can't do that while a piece is moving\n");
      return;
     }
   if (bstate_get_open_dialog())
     {
      //printf("Can't do that while another window is open\n");
      return;
     }
   if (bstate_get_what_next() == NEW_GAME)
     {
      //printf("Can't do that while I'm expecting a new game to start\n");
      return;
     }
   if (bstate_get_replay_mode())
      return 0;

   switch (event->type) {
       case GDK_BUTTON_PRESS:
	   if (event->button.button != 1)
	      break;	

#ifdef DEBUG
	   printf("what_next = %d\n", bstate_get_what_next());
#endif
	   //Set where a piece is coming from
	   board->orig_x = board->curr_x = event->button.x;
	   board->orig_y = board->curr_y = event->button.y;
	   from = get_square (board->curr_x, board->curr_y);
	   from_col = get_x_int_from_square(from);
	   from_row = get_y_int_from_square(from);

#ifdef DEBUG
	   board_print(board);
#endif

	   //Make sure it's time to move a queen before letting the user do it,
	   //as well as make sure he's moving his own queen.  Also make sure it's
   	   //a human's turn!
#ifdef DEBUG
	   printf("checking row %d, col %d\n", from_row, from_col);
	   square_contains(from);
	   printf("board->squares = %d\n",board->squares[from_row][from_col]);
#endif
	   if ((board->squares[from_row][from_col] == WHITE && 
		       bstate_get_what_next() == MOVE_WHITE_QUEEN && 
		       options.white_player == HUMAN) ||
	       (board->squares[from_row][from_col] == BLACK && 
			bstate_get_what_next() == MOVE_BLACK_QUEEN && 
			options.black_player == HUMAN))
	     {
	      //Human is moving a queen
#ifdef DEBUG
	      printf("board->squares = %d, what_next = %d\n", board->squares[from_row][from_col], bstate_get_what_next());
#endif
	      grabbed_queen = TRUE;
	      gen_legal_moves(from);
#ifdef DEBUG
	      printf("grabbing queen\n");
#endif
	      gnome_canvas_item_raise_to_top (item);
	      gnome_canvas_item_grab (item,
		      GDK_POINTER_MOTION_MASK | 
		      GDK_BUTTON_RELEASE_MASK,
		      NULL,
		      event->button.time);

	      board->selected_queen = item;
	     }
	   from = get_square (board->curr_x, board->curr_y);
	   to = from;
#ifdef DEBUG
	   printf("GDK_BUTTON_PRESS: to = %d  from = %d\n", to, from);
#endif
	   break;

       case GDK_MOTION_NOTIFY:
	   if (event && (event->motion.state & GDK_BUTTON1_MASK)) 
	     {
	      if (!grabbed_queen)
		 break;
	      new_x = event->motion.x;
	      new_y = event->motion.y;
	      gnome_canvas_item_raise_to_top (item);
	      gnome_canvas_item_move (item, 
		      new_x - board->curr_x,
		      new_y - board->curr_y);


	      board->curr_x = new_x;
	      board->curr_y = new_y;
	     }
	   break;

       case GDK_BUTTON_RELEASE:
	   if (event->button.button != 1)
	      break;
#ifdef DEBUG
	   printf("what_next = %d\n", bstate_get_what_next());
#endif
	   if (grabbed_queen == FALSE)
	      break;
	   else
	      grabbed_queen = FALSE;

	   if (item != NULL)
	      gnome_canvas_item_ungrab (item, event->button.time);


#ifdef DEBUG
	   printf("GDK_BUTTON_RELEASE: to = %d  from = %d\n", to, from);
#endif
	   drop_x = event->button.x;
	   drop_y = event->button.y;

	   //something funny here?
	   if (is_queen_square(get_square (drop_x, drop_y)))
	     {
	      move_piece_to (to, item);
	      break;
	     }

#ifdef DEBUG
	   printf("Queen is dropped at coords %f, %f\n", drop_x, drop_y);
#endif
	   to = get_square (drop_x, drop_y);
#ifdef DEBUG
	   printf("new square is %d\n", to);
#endif
	
	   /* Register the queen move with the board */
	   from_col = get_x_int_from_square(from);
	   from_row = get_y_int_from_square(from);

	   to_col = get_x_int_from_square(to);
	   to_row = get_y_int_from_square(to);
#ifdef DEBUG
	   printf("setting square %d%d to hold %d\n", to_row, to_col, board->squares[from_row][from_col]);
#endif

	   //Queen is being moved to a different square
	   if (to != from) 
	     {
	      //put the piece back if it's not a legal move
	      if (!is_move_legal(to))
		{
		 //bstate_set_move_to(from);
		 move_piece_to(from, item);
		 break;
		}
	      else
		{
		 bstate_set_move_to(to);
		 bstate_set_move_from(from);
		 bstate_set_just_finished(MOVE_WHITE_QUEEN);

		 board->squares[to_row][to_col] = board->squares[from_row][from_col];
		 for (k=0; k<4; k++) //update square to queen mapping
		   {
		    if (board->square_to_queen_map[WHITE][k] == from_row*10 + from_col)
		      {
		       board->square_to_queen_map[WHITE][k] = to_row*10 + to_col;
		       break;
		      }
		    if (board->square_to_queen_map[BLACK][k] == from_row*10 + from_col)
		      {
		       board->square_to_queen_map[BLACK][k] = to_row*10 + to_col;
		       break;
		      }
		   }
		 board->squares[from_row][from_col] = EMPTY_SQUARE;
		 gen_legal_moves(to); //prepare moves for arrow
		}
	     }

#ifdef DEBUG
	   square_contains(to);
	   square_contains(from);
#endif


	   move_piece_to(to, item);
#ifdef DEBUG
	   printf("what_next = %d\n", bstate_get_what_next());
#endif
	   break;
       //default:
   }

   return 1;
}

/*==============================================================================
 * arrow_fire_cb
 *
 * Determines the square the arrow is fired at and marks it as such.
 * Also checks that this is the appropriate time to fire an arrow and that the
 * given square is in the list of legal moves.
 */
int arrow_fire_cb(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
   double x_coord, y_coord;
   int sq;
   int row, col;
   State *s;
   //GtkWidget *auto_button, *undo_button;

   if (bstate_get_moving_piece())
      return 0;
   if (bstate_get_open_dialog())
      return 0;
   if (bstate_get_what_next() == NEW_GAME)
      return 0;
   if (bstate_get_replay_mode())
      return 0;

   count_queens();
   switch (event->type) 
     {
       case GDK_BUTTON_PRESS:
	   if (event->button.button != 1)
	      break;	
	   if (bstate_get_what_next() != FIRE_ARROW)
	      break;

	   x_coord = event->button.x-4;
	   y_coord = event->button.y-4;

	   sq = get_square(x_coord, y_coord);
	   if (!is_move_legal(sq))
	      break;

	   /* Make sure the square is empty first */
	   col = get_x_int_from_square(sq);
	   row = get_y_int_from_square(sq);

#ifdef DEBUG
	   printf("what_next = %d\n", bstate_get_what_next());
	   printf("arrow fire:  ");
	   square_contains(sq);
#endif
	   if (board->squares[row][col] != EMPTY_SQUARE)
	      break;

	   fire_arrow(sq);
	   s = states.states[states.current_state];
	   register_move_with_engine(sq);

	   bstate_set_just_finished(FIRE_ARROW);

	   
	   //check for gameover
	   if (game_over())
	      break;

	   state_copy(s, states.states[++(states.current_state)]);
	   if (bstate_get_what_next() == WAIT_FOR_AI)
	      move_ai();
       //default:
     }

   return 0;
}


/*==============================================================================
 * on_PlayerOKButton_clicked
 *
 * This processes the information from the Player settings window.  If a change
 * is made, sometimes the state of the game must reflect this change.  For 
 * instance, if it was a humans turn, and this player was set to AI, the game
 * state must stop waiting for the human to move and must instead instruct the
 * AI to start thinking.
 */
void
on_PlayerOKButton_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
   GtkWidget *time, *width, *depth;
   GtkWidget *white_ai, *white_h, *black_ai, *black_h;

   load_values_from_file();

   /* grab all the widgets */
   time     = lookup_widget(PlayerSettingsWindow, "TimeSpinner");
   width    = lookup_widget(PlayerSettingsWindow, "WidthSpinner");
   depth    = lookup_widget(PlayerSettingsWindow, "DepthSpinner");
   white_ai = lookup_widget(PlayerSettingsWindow, "WhiteAIRadio");
   white_h  = lookup_widget(PlayerSettingsWindow, "WhiteHumanRadio");
   black_ai = lookup_widget(PlayerSettingsWindow, "BlackAIRadio");
   black_h  = lookup_widget(PlayerSettingsWindow, "BlackHumanRadio");


   /* Store new values */
   options.engine.timeout = gtk_spin_button_get_value_as_int( (GtkSpinButton *)time);
   options.engine.maxwidth = gtk_spin_button_get_value_as_int( (GtkSpinButton *)width);
   options.engine.maxdepth = gtk_spin_button_get_value_as_int( (GtkSpinButton *)depth);

   if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(white_h)))
      options.white_player = HUMAN;
   else
      options.white_player = AI;

   if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(black_h)))
      options.black_player = HUMAN;
   else
      options.black_player = AI;

   store_values_in_file();
   gtk_widget_destroy(PlayerSettingsWindow);

   bstate_update_player_settings();
   bstate_set_open_dialog(FALSE);
   //checks to see if the AI is supposed to move after changes to the player have been made
   //if the ai is already moving, don't muck things up by calling it again.
   if (!bstate_get_moving_ai())
     {
      while (bstate_get_what_next() == WAIT_FOR_AI) 
       	{
	 move_ai();
       	}
     }

}


void
on_PlayerCancelButton_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
   gtk_widget_destroy(PlayerSettingsWindow);
   bstate_set_open_dialog(FALSE);
}


void
on_theme1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

   if (bstate_get_open_dialog())
      return;
   bstate_set_open_dialog(TRUE);
   file_selector = gtk_file_selection_new ("Pick your theme file.");
   
   gtk_file_selection_set_filename (GTK_FILE_SELECTION (file_selector), PACKAGE_DATA_DIR "/gamazons/");
   gtk_file_selection_complete (GTK_FILE_SELECTION (file_selector), "*.theme");
   g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                     "clicked",
                     G_CALLBACK (load_new_theme),
                     NULL);
   g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
                     "clicked",
                     G_CALLBACK (on_ThemeCancelButton_clicked),
                     NULL);
   			   
   /* Ensure that the dialog box is destroyed when the user clicks a button. */
   
   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy), 
                             (gpointer) file_selector); 

   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) file_selector); 
   
   /* Display that dialog */
   
   gtk_widget_show (file_selector);


}

void load_new_theme (GtkFileSelection *selector, gpointer user_data)
{
   if (bstate_get_moving_piece())
     {
      gnome_warning_dialog("Warning: Theme can't be updated while a game piece is moving.  Please try again.");
      bstate_set_open_dialog(FALSE);
      return;
     }

   selected_filename = (gchar *) gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector));
   load_values_from_file(); //Get current settings
   if (!load_images_from_theme (selected_filename)) //load new theme settings
     {
      gnome_error_dialog("Theme file doesn't exist!\n");
      bstate_set_open_dialog(FALSE);
      return;
     }
   store_values_in_file();  //store new settings
   board_destroy();
   board_draw();
   bstate_set_open_dialog(FALSE);
   
}

void on_ThemeCancelButton_clicked(GtkButton *button, gpointer user_data)
{
   bstate_set_open_dialog(FALSE);

}

void
on_save_as1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   //char home[256];
   char *home_env;

   if (bstate_get_open_dialog())
      return;
   bstate_set_open_dialog(TRUE);

   file_selector = gtk_file_selection_new ("Save your move history to a file");

   gtk_file_selection_set_filename (GTK_FILE_SELECTION (file_selector), options.hist_dir);
   g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                     "clicked",
                     G_CALLBACK (save_game_history),
                     NULL);
   g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
                     "clicked",
                     G_CALLBACK (on_save_as1CancelButton_clicked),
                     NULL);
   			   
   /* Ensure that the dialog box is destroyed when the user clicks a button. */
   
   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy), 
                             (gpointer) file_selector); 

   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) file_selector); 
   
   /* Display that dialog */
   
   gtk_widget_show (file_selector);


}

void save_game_history(GtkFileSelection *selector, gpointer user_data)
{
   FILE *history_fd;
   GtkTextView *view; 
   GtkTextBuffer *buffer;
   GtkTextIter *iter_start = (GtkTextIter *) malloc(sizeof(GtkTextIter));
   GtkTextIter *iter_end = (GtkTextIter *) malloc(sizeof(GtkTextIter));
   gchar *text;
   char *temp_dir;

   selected_filename = (gchar *)gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector));
   if (strcmp(options.hist_dir, selected_filename) == 0)
     {
      bstate_set_open_dialog(FALSE);
      return;
     }

   load_values_from_file(); //Get current settings
   strcpy(options.hist_dir, selected_filename);
   temp_dir = dirname(options.hist_dir);
   strcpy(options.hist_dir, temp_dir);
   options.hist_dir[strlen(options.hist_dir)+1] = '\0';
   options.hist_dir[strlen(options.hist_dir)] = '/';
   store_values_in_file(); //save new dir
   
   view = (GtkTextView *) lookup_widget(main_window, "textview1");
   buffer = gtk_text_view_get_buffer (view);
   gtk_text_buffer_get_start_iter(buffer, iter_start);
   gtk_text_buffer_get_end_iter(buffer, iter_end);

   text = gtk_text_buffer_get_text(buffer, iter_start, iter_end, FALSE);
   history_fd = fopen(selected_filename, "w");

   if (history_fd == NULL)
     {
      gnome_error_dialog("Unable to save move history");
      bstate_set_open_dialog(FALSE);
      return;
     }

   fprintf(history_fd, "%s", text);
   fclose(history_fd);
   bstate_set_open_dialog(FALSE);
}

void on_save_as1CancelButton_clicked(GtkButton *button, gpointer user_data)
{
   bstate_set_open_dialog(FALSE);
}


void
on_open_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   //char home[256];
   char *home_env;

   if (bstate_get_open_dialog())
      return;
   bstate_set_open_dialog(TRUE);

   file_selector = gtk_file_selection_new ("Load saved Gamazons history file you wish to continue");

   gtk_file_selection_set_filename (GTK_FILE_SELECTION (file_selector), options.hist_dir);
   g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                     "clicked",
                     G_CALLBACK (load_saved_game),
                     NULL);
   g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
                     "clicked",
                     G_CALLBACK (on_OpenCancelButton_clicked),
                     NULL);
   			   
   /* Ensure that the dialog box is destroyed when the user clicks a button. */
   
   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy), 
                             (gpointer) file_selector); 

   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) file_selector); 
   
   /* Display that dialog */
   
   gtk_widget_show (file_selector);


}

void load_saved_game(GtkFileSelection *selector, gpointer user_data)
{

   FILE *history_fd;
   char *temp_dir;

   selected_filename = (gchar *)gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector));
   if (strcmp(options.hist_dir, selected_filename) == 0)
     {
      bstate_set_open_dialog(FALSE);
      return;
     }

   load_values_from_file(); //Get current settings
   strcpy(options.hist_dir, selected_filename);
   temp_dir = dirname(options.hist_dir);
   strcpy(options.hist_dir, temp_dir);
   options.hist_dir[strlen(options.hist_dir)+1] = '\0';
   options.hist_dir[strlen(options.hist_dir)] = '/';
   store_values_in_file();

   history_fd = fopen(selected_filename, "ro");
   if (history_fd == NULL)
     {
      gnome_error_dialog("Error opening history file\n");
      bstate_set_open_dialog(FALSE);
      return;
     }
   gtk_widget_destroy(file_selector);
   bstate_set_open_dialog(FALSE);
   bstate_set_replay_mode(FALSE);

   free_all_memory();
   init_engine();
   board_destroy();
   board_init_game(main_window);

   if (read_in_moves(history_fd))
      bstate_set_just_finished(LOAD_GAME);
   else
      gnome_error_dialog("Error: Corrupted history file\n");
      

   //printf("finished loading history file\n");
   fclose(history_fd);

   if (bstate_get_what_next() == NEW_GAME) //game over
      return;
   while (bstate_get_what_next() == WAIT_FOR_AI)
      move_ai();  //if both players are AI, keeps on running

}

void on_OpenCancelButton_clicked(GtkButton *button, gpointer user_data)
{
   bstate_set_open_dialog(FALSE);
}

void
on_BT_REPLAY_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
   //char home[256];
   char *home_env;

   if (bstate_get_open_dialog())
      return;
   bstate_set_open_dialog(TRUE);

   file_selector = gtk_file_selection_new ("Load saved Gamazons history file you wish to replay");

   gtk_file_selection_set_filename (GTK_FILE_SELECTION (file_selector), options.hist_dir);
   g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                     "clicked",
                     G_CALLBACK (replay_saved_game),
                     NULL);
   g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
                     "clicked",
                     G_CALLBACK (on_OpenCancelButton_clicked),
                     NULL);
   			   
   /* Ensure that the dialog box is destroyed when the user clicks a button. */
   
   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy), 
                             (gpointer) file_selector); 

   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) file_selector); 
   
   /* Display that dialog */
   
   gtk_widget_show (file_selector);


}

void replay_saved_game(GtkFileSelection *selector, gpointer user_data)
{

   FILE *history_fd;
   char *temp_dir;
   int game_life = bstate_get_game_life();

   selected_filename = (gchar *)gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector));
   if (strcmp(options.hist_dir, selected_filename) == 0)
     {
      bstate_set_open_dialog(FALSE);
      return;
     }

   load_values_from_file(); //Get current settings
   strcpy(options.hist_dir, selected_filename);
   temp_dir = dirname(options.hist_dir);
   strcpy(options.hist_dir, temp_dir);
   options.hist_dir[strlen(options.hist_dir)+1] = '\0';
   options.hist_dir[strlen(options.hist_dir)] = '/';
   store_values_in_file();

   history_fd = fopen(selected_filename, "ro");
   if (history_fd == NULL)
     {
      gnome_error_dialog("Error opening history file\n");
      bstate_set_open_dialog(FALSE);
      return;
     }
   gtk_widget_destroy(file_selector);
   bstate_set_open_dialog(FALSE);
   bstate_set_just_finished(START_REPLAY);

   free_all_memory();
   init_engine();
   board_destroy();
   board_init_game(main_window);

   //bstate_set_replay_mode(TRUE);
   if (read_in_moves(history_fd))
     {
     }
   else if (bstate_get_replay_mode()) //if still in replay mode, there must have been an error.  Otherwise
     {				      //the stop button was pressed and all is well.
      gnome_error_dialog("Error: Corrupted history file\n");
     }
   else if (game_life != bstate_get_game_life())
     {
      return;
     }

   bstate_set_just_finished(STOP_REPLAY); //cleans up replay mode if successfully finished
   //bstate_set_replay_mode(FALSE);
      

   //printf("finished loading history file\n");
   fclose(history_fd);

   if (bstate_get_what_next() == NEW_GAME) //game over
      return;
   while (bstate_get_what_next() == WAIT_FOR_AI)
      move_ai();  //if both players are AI, keeps on running

}


void
on_BT_REPLAY_STOP_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
   //bstate_set_replay_mode(FALSE);
   bstate_set_just_finished(STOP_REPLAY);

}

