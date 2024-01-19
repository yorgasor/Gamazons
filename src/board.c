#include <time.h>
#include <assert.h>
#include <gnome.h>

#include "amazons.h"
#include "board.h"
#include "bstate.h"
#include "callbacks.h"

/* local Prototypes */


static void fill_a_square(GnomeCanvasGroup *group,
			  double x1, double y1, double x2, double y2, 
			  char *color);
static void get_square_color(int square, char *color);
static void draw_a_line(GnomeCanvasGroup *group, 
			int x1, int y1, int x2, int y2, char *color);
static void draw_grid();


/* Globals */
extern Board  *board;
extern struct options options;
extern Game_states    states;
extern int ok;
extern time_t start;
extern GtkWidget *main_window;
Square legal_moves[100];
int state_hash;


/* ================================================================ */


/* Initialize the board for a new game. 
 */

void board_init_game(GtkWidget *GamazonsMain)
{
   char color[256];
   GtkWidget *w = (GtkWidget *) lookup_widget(GamazonsMain, BOARD_NAME);
   GtkWidget *force_button, *undo_button;
   GtkTextView *view;
   GtkTextBuffer *buffer;
   GtkWidget *speed, *delay;
   static int first_run = TRUE;
   int i,j;
   
   if (w == NULL)
      printf("Couldn't find board!!!!!!!!\n");

   board = (Board *) malloc(sizeof(Board));
   board->canvas = GNOME_CANVAS(w);
   board->root = GNOME_CANVAS_GROUP(gnome_canvas_item_new(gnome_canvas_root(board->canvas),
	       gnome_canvas_group_get_type(),
	       NULL));
   
   gnome_canvas_set_scroll_region(board->canvas, 0.0, 0.0,
	   400.0,
	   400.0);


   /* initialize pieces */
   for (i=0; i<BOARD_SIZE; i++)
     {
      for (j=0; j<BOARD_SIZE; j++)
	{
	 board->squares[i][j] = EMPTY_SQUARE;
	}
     }
   gtk_signal_connect(GTK_OBJECT(board->canvas), "event",
	   GTK_SIGNAL_FUNC(arrow_fire_cb), NULL);

   //Place amazon queens on the board
   board->squares[9][3] = WHITE_SQUARE;
   board->squares[9][6] = WHITE_SQUARE;
   board->squares[6][0] = WHITE_SQUARE;
   board->squares[6][9] = WHITE_SQUARE;

   board->squares[0][3] = BLACK_SQUARE;
   board->squares[0][6] = BLACK_SQUARE;
   board->squares[3][0] = BLACK_SQUARE;
   board->squares[3][9] = BLACK_SQUARE;

   board->square_to_queen_map[WHITE][0] = 96;
   board->square_to_queen_map[WHITE][1] = 93;
   board->square_to_queen_map[WHITE][2] = 60;
   board->square_to_queen_map[WHITE][3] = 69;

   board->square_to_queen_map[BLACK][0] = 3;
   board->square_to_queen_map[BLACK][1] = 6;
   board->square_to_queen_map[BLACK][2] = 30;
   board->square_to_queen_map[BLACK][3] = 39;

   //Set up move history window
   view = (GtkTextView *) lookup_widget(main_window, "textview1");
   buffer = gtk_text_buffer_new(NULL);
   gtk_text_view_set_buffer(view, buffer);

   //Initialize buttons
   force_button = (GtkWidget *)lookup_widget(main_window, "BT_FORCEMOVE");
   gtk_widget_set_sensitive (force_button, FALSE);
   undo_button = (GtkWidget *)lookup_widget(main_window, "BT_UNDO");
   gtk_widget_set_sensitive (undo_button, FALSE);
   

   board_draw();
   if (first_run)
     {
      //Set spinner values
      delay = (GtkWidget *)lookup_widget(main_window, "ReplayDelaySpinner");
      gtk_spin_button_set_value((GtkSpinButton *)delay, options.replay_delay);
      speed = (GtkWidget *)lookup_widget(main_window, "MovementSpeedSpinner");
      gtk_spin_button_set_value((GtkSpinButton *)speed, options.movement_speed);

      first_run = FALSE;
      bstate_set_just_finished(START_GAME);
     }

}


void fill_a_square(GnomeCanvasGroup *group,
		   double x1, double y1, double x2, double y2, 
		   char *color)
{
   /* draw a box*/
   gnome_canvas_item_new(group,
	   gnome_canvas_rect_get_type(),
	   "x1", x1,
	   "y1", y1,
	   "x2", x2,
	   "y2", y2,
	   "outline_color", "black",
	   "fill_color", color,
	   "width_pixels", (double)THICKNESS,
	   NULL, NULL);

}

static void get_square_color(int square, char *color)
{
   if ((square % 2) == 0)
      strcpy(color, SQUARE_COLOR_1);
   else
      strcpy(color, SQUARE_COLOR_2);
}


static void draw_grid()
{
   int x,y;

   for (x=0; x<=10; x++)
     {
      for (y=0; y<=10; y++)
	{
	 //draw horiz
	 draw_a_line(board->root, //gnome_canvas_root(board->canvas),
		 0, y*CELL_SIZE, BOARD_SIZE*CELL_SIZE, y*CELL_SIZE, "black");

	 //draw vert
	 draw_a_line(board->root, //gnome_canvas_root(board->canvas),
		 x*CELL_SIZE, 0, x*CELL_SIZE, BOARD_SIZE*CELL_SIZE, "black");
	}
     }


}

static void draw_a_line(GnomeCanvasGroup *group,
       	int x1, int y1, int x2, int y2, char *color)
{
   GnomeCanvasPoints *points;

   /* allocate a new points array */
   points = gnome_canvas_points_new (2);

   /* fill out the points */
   points->coords[0] = x1;
   points->coords[1] = y1;
   points->coords[2] = x2;
   points->coords[3] = y2;
   /* draw the line */
   gnome_canvas_item_new(group,
	   gnome_canvas_line_get_type(),
	   "points", points,
	   "fill_color", color,
	   "width_units", (double)THICKNESS,
	   NULL);

   /* free the points array */
   gnome_canvas_points_free(points);
}

void board_draw()
{
   int i,j,k;
   int black_i = 0;
   GdkPixbuf *white_pb, *black_pb;
   GdkPixbuf *white_sq, *grey_sq, *arrow_sq;
   char color[256];
   GnomeCanvasItem *image;
   GnomeCanvasGroup *root = GNOME_CANVAS_GROUP(gnome_canvas_root (GNOME_CANVAS (board->canvas)));
   static int first_game = 1;
   


   /* Find images */
   white_pb = gdk_pixbuf_new_from_file(options.images.white_piece, NULL);
   if (white_pb == NULL)
     {
      fprintf(stderr, "Cannot find white piece image: %s\n", options.images.white_piece);
      exit(1);
     }

   black_pb = gdk_pixbuf_new_from_file(options.images.black_piece, NULL);
   if (black_pb == NULL)
     {
      fprintf(stderr, "Cannot find black piece image: %s\n", options.images.black_piece);
      exit(1);
     }

   white_sq = gdk_pixbuf_new_from_file(options.images.white_sq, NULL);
   if (white_sq == NULL)
     {
      fprintf(stderr, "Cannot find white square image: %s\n", options.images.white_sq);
      exit(1);
     }

   grey_sq = gdk_pixbuf_new_from_file(options.images.grey_sq, NULL);
   if (grey_sq == NULL)
     {
      fprintf(stderr, "Cannot find grey square image: %s\n", options.images.grey_sq);
      exit(1);
     }
   arrow_sq = gdk_pixbuf_new_from_file(options.images.arrow_sq, NULL);
   if (arrow_sq == NULL)
     {
      fprintf(stderr, "Cannot find arrow square image: %s\n", options.images.arrow_sq);
      exit(1);
     }



   /* fill alternate squares */
   for(j=0;j<BOARD_SIZE;j++) 
     {
      for(i=0;i<BOARD_SIZE;i++) 
	{
	 board->square_items[j*10+i] = NULL;
	 if ((i + j) % 2)
	   {
	    board->square_items[j*10+i] = gnome_canvas_item_new (board->root,
		    gnome_canvas_pixbuf_get_type (),
		    "x", i*CELL_SIZE+QUEEN_OFFSET, "y", j*CELL_SIZE+QUEEN_OFFSET,
		    "width", CELL_SIZE, "height", CELL_SIZE,
		    "width_set", TRUE, "height_set", TRUE,
		    "pixbuf", grey_sq,
		    NULL);
	   }
	 else
	   {
	    board->square_items[j*10+i] = gnome_canvas_item_new (board->root,
		    gnome_canvas_pixbuf_get_type (),
		    "x", i*CELL_SIZE+QUEEN_OFFSET, "y", j*CELL_SIZE+QUEEN_OFFSET,
		    "width", CELL_SIZE, "height", CELL_SIZE,
		    "width_set", TRUE, "height_set", TRUE,
		    "pixbuf", white_sq,
		    NULL);
	   }



	 //Place the queen images on the board in the right order
	 if (board->squares[j][i] == WHITE_SQUARE)
	   {
#ifdef DEBUG
	    printf("Square %c%c contains a white queen\n",i+'a',10-j+'0');
#endif
	    image = gnome_canvas_item_new (board->root,
		    gnome_canvas_pixbuf_get_type (),
		    "x", i*CELL_SIZE+QUEEN_OFFSET, "y", j*CELL_SIZE+QUEEN_OFFSET,
		    "width", CELL_SIZE, "height", CELL_SIZE,
		    "width_set", TRUE, "height_set", TRUE,
		    "pixbuf", white_pb,
		    NULL);
	    //We need to do some funky checking to make sure board->white_queens[] matches
	    //up exactly with state->white_q_x[], state->white_q_y[]
	    for (k=0; k<4; k++)
	      {
	       if(j*10 +i == board->square_to_queen_map[WHITE][k])
		 {
		  board->queen_images[WHITE][k] = image;
#ifdef DEBUG
		  printf("registering queen %d\n", k);
#endif
		  break;
		 }
	      }

#ifdef DEBUG
	    printf("connecting signal to queen\n");
#endif
	    gtk_signal_connect(GTK_OBJECT(image), "event",
		    GTK_SIGNAL_FUNC(board_press_cb), NULL);
	   }
	 else if (board->squares[j][i] == BLACK_SQUARE)
	   {
#ifdef DEBUG
	    printf("Square %c%d contains a black queen\n",i+'a',10-j);
#endif
	    image = gnome_canvas_item_new (board->root,
		    gnome_canvas_pixbuf_get_type (),
		    "x", i*CELL_SIZE+QUEEN_OFFSET, "y", j*CELL_SIZE+QUEEN_OFFSET,
		    "width", CELL_SIZE, "height", CELL_SIZE,
		    "width_set", TRUE, "height_set", TRUE,
		    "pixbuf", black_pb,
		    NULL);
	    for (k=0; k<4; k++)
	      {
	       if(j*10 +i == board->square_to_queen_map[BLACK][k])
		 {
		  board->queen_images[BLACK][k] = image;
#ifdef DEBUG
		  printf("registering queen %d\n", k);
#endif
		  break;
		 }
	      }

#ifdef DEBUG
	    printf("connecting signal to queen\n");
#endif
	    gtk_signal_connect(GTK_OBJECT(image), "event",
		    GTK_SIGNAL_FUNC(board_press_cb), NULL);
	   }
	 else if (board->squares[j][i] == ARROW_SQUARE)
	   {
	    image = gnome_canvas_item_new (board->root,
		    gnome_canvas_pixbuf_get_type (),
		    "x", i*CELL_SIZE+QUEEN_OFFSET, "y", j*CELL_SIZE+QUEEN_OFFSET,
		    "width", CELL_SIZE, "height", CELL_SIZE,
		    "width_set", TRUE, "height_set", TRUE,
		    "pixbuf", arrow_sq,
		    NULL);
	   }
	    

	}
     }

   if (options.images.grid == TRUE)
      draw_grid();

   /*
   image = gnome_canvas_item_new (root,
	   gnome_canvas_pixbuf_get_type (),
	   "x", 10.0, "y", 10.0,
	   "width", 40.0, "height", 40.0,
	   "width_set", TRUE, "height_set", TRUE,
	   "pixbuf", white_pb,
	   NULL);
	   */


   gtk_widget_show_now(main_window);
   gtk_widget_queue_draw ((GtkWidget *) board->canvas);
   gtk_widget_show_now((GtkWidget *) board->canvas);
}



void mark_square (GnomeCanvasItem *square)
{
   gnome_canvas_item_set (square, "outline_color", "red", NULL);
}

Square get_square (double x, double y)
{
   Square from;
   int x_square;
   int y_square;

   /*
   x -= (BOARD_BORDER - CELL_PAD);
   y -= (BOARD_BORDER - CELL_PAD);
   */

   if (x < 0)
      x = 0.0;
   else if (x > ((BOARD_SIZE-1) * CELL_SIZE))
      x = (BOARD_SIZE-1) * CELL_SIZE;
   if (y < 0)
      y = 0.0;
   else if (y > ((BOARD_SIZE-1) * CELL_SIZE))
      y = (BOARD_SIZE-1) * CELL_SIZE;

   x_square = x / CELL_SIZE;
   y_square = y / CELL_SIZE;

#ifdef DEBUG
   printf("x coord = %f   y coord = %f\n", x, y);
   printf("x coord = %d   y coord = %d\n", x_square, y_square);
#endif

   from = x_square + y_square * 10;

   return from;
}

void clear_square (GnomeCanvasItem **square)
{
   gnome_canvas_item_set (*square, "outline_color", NULL, NULL);
   *square = NULL;
}

/*==============================================================================
 * move_piece_to
 *
 * This moves the given piece to the given square.  This is a very tricky function
 * because the user can do many things to upset the state of the board while a 
 * piece is sliding, so various checks must take place throughout the movement 
 * process to make sure the board still exists as it thinks it does.
 *
 * The piece can also move at different speeds.  Originally, it moved one pixel at
 * a time, but it now gets its value from the MovementSpeedSpinner widget.  This
 * makes it so the pieces can move at acceptable speeds on slower computers, but 
 * adds more complexity in trying to calculate the distance a piece should move.
 */
void move_piece_to(Square to, GnomeCanvasItem *item)
{
   double Lx, Uy, Rx, By;
   double to_Lx, to_Uy;
   int x, y;
   int i,j;
   int game_life;
   GtkWidget *speed = (GtkWidget *)lookup_widget(main_window, "MovementSpeedSpinner");
   int inc = gtk_spin_button_get_value_as_int((GtkSpinButton *)speed);
   int count = 0;

   game_life = bstate_get_game_life();
   to_Lx = get_x_from_square(to);
   to_Uy = get_y_from_square(to);
      
#ifdef DEBUG
   printf("We want the queen at coords %f, %f\n", to_Lx, to_Uy);
#endif
   gnome_canvas_item_get_bounds(item, &Lx, &Uy, &Rx, &By);
#ifdef DEBUG
   printf("The queen is at coords %f, %f\n", Lx, Uy);
#endif
   if (bstate_get_dont_slide())
     {
      gnome_canvas_item_get_bounds(item, &Lx, &Uy, &Rx, &By);
      gnome_canvas_item_move (item, 
	      get_x_from_square(to) - Lx,
	      get_y_from_square(to) - Uy);
      gnome_canvas_item_raise_to_top (item);
     }
   else
     {
      while (Lx != to_Lx || Uy != to_Uy)
       	{
#ifdef DEBUG
	 if (count++ < 20)
	    printf("Lx = %g;  to_Lx = %g; Uy = %g; to_Uy = %g\n", Lx, to_Lx, Uy, to_Uy);
#endif
	 bstate_set_moving_piece(TRUE);
	 if ((int)Lx == (int)to_Lx)
	    x = 0;
	 else if (Lx < to_Lx && (to_Lx - Lx) >= inc)
	    x = inc;
	 else if (Lx < to_Lx && (to_Lx - Lx) <= inc)
	    x = to_Lx - Lx;
	 else if (Lx > to_Lx && (Lx - to_Lx) >= inc)
	    x = -inc;
	 else if (Lx > to_Lx && (Lx - to_Lx) <= inc)
	    x = to_Lx - Lx;

	 if ((int)Uy == (int)to_Uy)
	    y = 0;
	 else if (Uy < to_Uy && (to_Uy - Uy) >= inc)
	    y = inc;
	 else if (Uy < to_Uy && (to_Uy - Uy) <= inc)
	    y = to_Uy - Uy;
	 else if (Uy > to_Uy && (Uy - to_Uy) >= inc)
	    y = -inc;
	 else if (Uy > to_Uy && (Uy - to_Uy) <= inc)
	    y = to_Uy - Uy;

#ifdef DEBUG
	 if (count < 20)
	    printf("x = %d, y = %d\n", x, y);
#endif

	 if (bstate_get_game_life() != game_life)
	   {
	    bstate_set_moving_piece(FALSE);
	    return;
	   }
	 gnome_canvas_item_move (item, (double)x, (double)y);
	 gnome_canvas_item_raise_to_top (item);

	 //update the board
         while (gtk_events_pending())
	    gtk_main_iteration();

	 if (bstate_get_quit_game())
	    exit(0);
	 if (bstate_get_game_life() != game_life)
	   {
	    bstate_set_moving_piece(FALSE);
	    return;
	   }
	 gnome_canvas_item_get_bounds(item, &Lx, &Uy, &Rx, &By);
       	}
     }

   if (bstate_get_game_life() != game_life)
     {
      bstate_set_moving_piece(FALSE);
      return;
     }
   gnome_canvas_item_raise_to_top (item);
   bstate_set_moving_piece(FALSE);
	   //see where it landed
   gnome_canvas_item_get_bounds(item, &Lx, &Uy, &Rx, &By);
#ifdef DEBUG
   printf("The queen landed at coords %f, %f\n", Lx, Uy);
   printf("this time the queen is on square %d (%c%d)\n", 
	  to, 'a' + to % 10, 10 - (to / 10));
#endif
}


   

double get_x_from_square(int sq)
{
   double x;

   x = (double) ((sq % 10) * CELL_SIZE+QUEEN_OFFSET);

   return x;

}

double get_y_from_square(int sq)
{
   double y;

   y = (double) ((sq / 10) * CELL_SIZE+QUEEN_OFFSET);

   return y;


}

int get_x_int_from_square(int sq)
{
   return(sq % 10);
}

int get_y_int_from_square(int sq)
{
   return(sq / 10);
}

int get_grid_num_from_square(int sq)
{
   return(10 - sq/10);
}

char get_grid_alpha_from_square(int sq)
{
   return('a' + sq % 10);
}

int engine_x_to_board_x(int eng_x)
{
   return(eng_x);//hey, these are the same, no conversion necessary
}

int engine_y_to_board_y(int eng_y)
{
   return(9 - eng_y);
}

int board_x_to_engine_x(int brd_x)
{
   return(brd_x);
}

int board_y_to_engine_y(int brd_y)
{
   return(9 - brd_y);
}

int get_square_from_engine(int x, int y)
{
   return((9 - y) * 10 + x);
}


void fire_arrow(Square sq)
{
   int x,y;
   GdkPixbuf *arrow_sq;
   GnomeCanvasItem *image;
   GnomeCanvasGroup *root = GNOME_CANVAS_GROUP(gnome_canvas_root (GNOME_CANVAS (board->canvas)));

   x = sq % 10;
   y = sq / 10;

   board->squares[y][x] = ARROW_SQUARE;

   arrow_sq = gdk_pixbuf_new_from_file(options.images.arrow_sq, NULL);
   if (arrow_sq == NULL)
     {
      fprintf(stderr, "Cannot find arrow image: %s\n", options.images.arrow_sq);
      exit(1);
     }

   image = gnome_canvas_item_new (board->root,
	   gnome_canvas_pixbuf_get_type (),
	   "x", x*CELL_SIZE+QUEEN_OFFSET, "y", y*CELL_SIZE+QUEEN_OFFSET,
	   "width", CELL_SIZE, "height", CELL_SIZE,
	   "width_set", TRUE, "height_set", TRUE,
	   "pixbuf", arrow_sq,
	   NULL);
}

void square_contains(Square sq)
{
   int row, col;

   col = get_x_int_from_square(sq);
   row = get_y_int_from_square(sq);

   if (board->squares[row][col] == EMPTY_SQUARE)
      printf("Nothing is found at square %d\n", sq);
   else if (board->squares[row][col] == WHITE_SQUARE)
      printf("A White Queen is found at square %d\n", sq);
   else if (board->squares[row][col] == BLACK_SQUARE)
      printf("A Black Queen is found at square %d\n", sq);
   else if (board->squares[row][col] == ARROW_SQUARE)
      printf("An arrow is found at square %d\n", sq);
   else
      printf("Whoa, I don't know _what_ is on square %d\n", sq);
}


void
handle_events()
{
    //Make the game responsive while the AI is thinking
    while (gtk_events_pending())
	gtk_main_iteration();
}


/* ===========================================================================
 * move_ai
 *
 * Checks to see if an AI oppenent is next to move.  If it is, it
 * starts the move process, and checks for a win afterwards.  If not,
 * it just checks for the win.  Returns TRUE if an AI opponent moves
 * next.  False if human.
 *
 * NOTE: If you have 2 AI players, this function would just keep
 * thinking and never let the board update or respond.  So there are
 * several checks to see if any new events occured that should be
 * handled, or critical changes have been made (like starting a new
 * game, etc..).  If something critical has changed the function will
 * exit after learning about it.
 */
int move_ai()
{
   State *s = states.states[states.current_state];
   Move temp;
   int ai = FALSE;
   int current_hash;
   int game_life = bstate_get_game_life();
   //GtkWidget *auto_button, *force_button;

   current_hash = state_hash = state_create_hash(s);

   //quit this function if a 'New Game' option was selected since this
   //function was started.
   /*
   if (bstate_get_new_game())
     {
      bstate_set_new_game(FALSE);
      return FALSE;
     }
     */
   ok = 1;
   ai = TRUE;
   start = time(NULL);
   temp = isearch(s, NOTHINK, handle_events);
   if (s->winner) //XXX does this if statement do any good?  I thot winner was handled elsewhere
      return FALSE;
   makemove(s,temp);

   //update the board before drawing the new move
   //the program would segfault if you closed it while it was thinking
   while (gtk_events_pending())
      gtk_main_iteration();
   if (current_hash != state_hash)
      return FALSE;

   /*
   if (bstate_get_new_game())
     {
      bstate_set_new_game(FALSE);
      return FALSE;
     }
     */

   if (bstate_get_quit_game())
      exit(0);

   if (bstate_get_game_life() != game_life)
      return FALSE;


   //register move on graphical board
   move_piece(temp);
   if (bstate_get_game_life() != game_life)
      return FALSE;
   print_move_in_text_window(&temp);
   state_copy(s, states.states[++(states.current_state)]);
   bstate_set_just_finished(WAIT_FOR_AI);


#ifdef DEBUG
   if (options.white_player == AI)
      printf("White is AI\n");
   if (options.black_player == AI)
      printf("Black is AI\n");
   printf("Turn is %d\n", states.states[states.current_state]->turn );
#endif

   if (game_over())
     {
      //XXX should I set ai = FALSE?
      ai = FALSE;
      return (ai);
     }

   return ai;
}

/*==============================================================================
 * move_piece
 *
 * Takes a move struct generated by the engine, and extracts the necessary info
 * to update the GUI data structs and have the GUI reflect the move.
 */
void move_piece(Move m)
{
   GnomeCanvasItem *item;
   int from_row, from_col;
   int to_row, to_col;
   int game_life = bstate_get_game_life();
   int  turn;

   bstate_set_move_to(get_square_from_engine(m.tocol, m.torow));
   to_col = get_x_int_from_square(bstate_get_move_to());
   to_row = get_y_int_from_square(bstate_get_move_to());
   //Note: by the time the state gets here, it's signaled the other player's turn
   //so if it says it's white's turn, black just moved and we need to move black's
   //piece.
   turn = states.states[states.current_state]->turn;
   if (turn == WHITE)
     {
      bstate_set_move_from(get_square_from_engine(states.states[states.current_state -1]->queens_x[BLACK][m.queen],
						  states.states[states.current_state -1]->queens_y[BLACK][m.queen]));
      
      board->squares[to_row][to_col] = BLACK_SQUARE;
#ifdef DEBUG
      printf("Moving black queen\n");
#endif
      item = board->queen_images[BLACK][m.queen];
      board->square_to_queen_map[BLACK][m.queen] = to_row*10 +to_col;
     }
   else
     {
      bstate_set_move_from(get_square_from_engine(states.states[states.current_state -1]->queens_x[WHITE][m.queen],
	      states.states[states.current_state -1]->queens_y[WHITE][m.queen]));
      board->squares[to_row][to_col] = WHITE_SQUARE;
#ifdef DEBUG
      printf("Moving white queen\n");
#endif
      item = board->queen_images[WHITE][m.queen];
      board->square_to_queen_map[WHITE][m.queen] = to_row*10 +to_col;
     }


   from_col = get_x_int_from_square(bstate_get_move_from());
   from_row = get_y_int_from_square(bstate_get_move_from());
   board->squares[from_row][from_col] = EMPTY_SQUARE;

   move_piece_to(bstate_get_move_to(), item);
#ifdef DEBUG
   printf("Engine coords for arrow: %d, %d\n", m.wallcol, m.wallrow);
#endif
   if (game_life != bstate_get_game_life())
      return;
   fire_arrow(get_square_from_engine(m.wallcol, m.wallrow));
#ifdef DEBUG
   printf("fired arrow to square %d\n", get_square_from_engine(m.wallcol, m.wallrow));
   pmove(states.states[states.current_state - 1], OTHER_COLOR(turn), m);
#endif
}

/*==============================================================================
 * register_move_with_engine
 *
 * Fills out a move struct with the human's move and calls makemove() to update
 * the state struct, so the AI engine knows what happened.
 */
void register_move_with_engine(Square arrow_sq)
{
   State *s = states.states[states.current_state];
   Move m;
   Move movelist[3000];
   int move_count;
   int i;
   int tocol, torow;
   int found = FALSE;
   char move_str[32];
   char err_msg[256];

   //find out the index of the queen that was just moved.  
   //weird things will happen if it's not found.
   for (i=0; i<4; i++)
     {
      if (board->selected_queen == board->queen_images[WHITE][i]
	  || board->selected_queen == board->queen_images[BLACK][i])
	{
	 found = TRUE;
	 break;
	}
     }

   if (!found)
      fprintf(stderr, "Error registering move w/ AI engine!  Game play will now be weird.\n");

   m.queen = i;
   m.torow = board_y_to_engine_y(get_y_int_from_square(bstate_get_move_to()));
   m.tocol = board_x_to_engine_x(get_x_int_from_square(bstate_get_move_to()));

   m.wallrow = board_y_to_engine_y(get_y_int_from_square(arrow_sq));
   m.wallcol = board_x_to_engine_x(get_x_int_from_square(arrow_sq));

   //make sure it's a valid move
   move_count = state_gen_moves(s, movelist);
   if (move_count == 0)
     {
      gnome_error_dialog("The AI engine thinks the game is over.  Please use the menu item Game->New to start a new game");
      return;
     }
   if (!state_is_legal_move(s, s->turn, &m))
     {
	 printf("This move was not OK:\n");
	 pmove(s, s->turn, m);

      get_move_str(&m, move_str);
      strcpy(err_msg, "You've hit a bug.  The following illegal move was just attempted: ");
      strcat(err_msg, move_str);
      gnome_error_dialog(err_msg);
      return;
     }

   makemove(s, m);
   print_move_in_text_window(&m);
}


/*==============================================================================
 * is_queen_square
 *
 * Compares the given square with the information stored in board->squares[][]
 * to see if a queen is there.
 */
int is_queen_square(Square sq)
{
   int row, col;

   col = get_x_int_from_square(sq);
   row = get_y_int_from_square(sq);

   if ((board->squares[row][col] == BLACK_SQUARE) ||
       (board->squares[row][col] == WHITE_SQUARE))
      return TRUE;
   else
      return FALSE;
}


/*==============================================================================
 * gen_legal_moves
 *
 * Given a square, this generates an array of all legal moves that can be made
 * from it.  This works for both queen movement, as well as firing an arrow, 
 * since both move the same way.
 *
 * The array terminator is 100
 */
void gen_legal_moves(Square sq)
{
   int arr_i = 0;
   int i;
   int row, col, sq_row, sq_col;
   int scanning;

   sq_col = get_x_int_from_square(sq);
   sq_row = get_y_int_from_square(sq);

   //make sure the player can drop the piece on the same square if he changes
   //his mind
   legal_moves[arr_i++] = sq;

   //get vertical moves
   row = sq_row + 1;
   col = sq_col;
   while (board->squares[row][col] == EMPTY_SQUARE && row < 10)
     {
      legal_moves[arr_i++] = (row*10) + col;
      row++;
     }

   row = sq_row - 1;
   while (board->squares[row][col] == EMPTY_SQUARE && row >= 0)
     {
      legal_moves[arr_i++] = (row*10) + col;
      row--;
     }

   //get horizontal moves
   row = sq_row;
   col = sq_col + 1;
   while (board->squares[row][col] == EMPTY_SQUARE && col < 10)
     {
      legal_moves[arr_i++] = (row*10) + col;
      col++;
     }

   col = sq_col - 1;
   while (board->squares[row][col] == EMPTY_SQUARE && col >= 0)
     {
      legal_moves[arr_i++] = (row*10) + col;
      col--;
     }

   //get forward diagonal moves
   row = sq_row + 1;
   col = sq_col + 1;
   while (board->squares[row][col] == EMPTY_SQUARE && col < 10 && row < 10)
     {
      legal_moves[arr_i++] = (row*10) + col;
      col++;
      row++;
     }

   row = sq_row - 1;
   col = sq_col - 1;
   while (board->squares[row][col] == EMPTY_SQUARE && col >= 0 && row >= 0)
     {
      legal_moves[arr_i++] = (row*10) + col;
      col--;
      row--;
     }

   //get backward diagonal moves
   row = sq_row + 1;
   col = sq_col - 1;
   while (board->squares[row][col] == EMPTY_SQUARE && col >= 0 && row < 10)
     {
      legal_moves[arr_i++] = (row*10) + col;
      col--;
      row++;
     }

   row = sq_row - 1;
   col = sq_col + 1;
   while (board->squares[row][col] == EMPTY_SQUARE && col < 10 && row >= 0)
     {
      legal_moves[arr_i++] = (row*10) + col;
      col++;
      row--;
     }


   legal_moves[arr_i] = 100;
#ifdef DEBUG
   printf("legal move list for %d of length %d: \n", sq, arr_i);
   i = 0;
   while (legal_moves[i] < 100)
      printf(" %d", legal_moves[i++]);
   printf("\n");
#endif

}

/*==============================================================================
 * is_move_legal
 *
 * Looks up the given square in the current legal_moves array.  If it doesn't 
 * exist, it's not a legal move and returns FALSE.  Returns TRUE if it is there.
 */
int is_move_legal(Square sq)
{
   int i=0;

#ifdef DEBUG
   printf("checking to see if a move is legal\n");
#endif
   while (legal_moves[i] < 100)
     {
      if (sq == legal_moves[i++])
	{
#ifdef DEBUG
	 printf("%d is a legal move\n", sq);
#endif
	 return TRUE;
	}
     }

#ifdef DEBUG
   printf("Can't move to square.  Legal moves are: %d\n", sq);
   i=0;
   while (legal_moves[i] < 100)
      printf(" %d", legal_moves[i++]);
   printf("\n");
#endif
   return FALSE;
}

/*==============================================================================
 * count_queens
 *
 * DEBUG - counts the number of queens on the GUI representation and prints
 * out a bunch of XXXX's whenever there are not 4 of both kinds.
 */ 
void count_queens()
{
   int black=0, white=0;
   int i,j;

   for(i=0; i<10; i++)
     {
      for(j=0; j<10; j++)
	{
	 if (board->squares[i][j] == WHITE_SQUARE)
	    white++;
	 if (board->squares[i][j] == BLACK_SQUARE)
	    black++;
	}
     }
   if (black > 4) 
      printf("YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY Gained a black queen\n");
   if (white > 4)
      printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX Gained a white queen\n");
   if (black < 4)
      printf("XYXYXYXYXYXYXYXYXYXYXYXYXYXYXYXYXYXYXYXYXXYXYX Lost a black queen\n");
   if (white < 4)
      printf("ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ Lost a white queen\n");
}



/*==============================================================================
 * free_all_memory
 *
 * When a new game is started, we want to free all the memory we've allocated
 * so we can start all over without leaking all the memory.
 */
void free_all_memory()
{
   int i;

   //free states
   for (i=0; i< states.max_state; i++)
      free (states.states[i]);

   //Oh, this is ugly!  Do I really want to do this?
   /*
   if(tt)
     {
      for (i=0; i<TT; i++)
       	{
	 if (tt[i])
	    free(tt[i]);
       	}
     }
     */



}


/*==============================================================================
 * game_over
 *
 * Checks to see if the game is over and generates a pop-up stating who the 
 * winner is.  Returns TRUE if the game is over.
 */
int game_over()
{
   State *s = states.states[states.current_state];
   Move movelist[3000];
//   GtkWidget *auto_button, *force_button;

   if (state_gen_moves(s, movelist) == 0)
     {
      s->winner = OTHER_COLOR(s->turn);
      if (s->winner == BLACK)
	 gnome_ok_dialog("Black wins!");
      else
	 gnome_ok_dialog("White wins!");

      bstate_set_just_finished(GAME_OVER);

      /*
      auto_button = (GtkWidget *) lookup_widget(main_window, "BT_AUTOFINISH");
      force_button = (GtkWidget *) lookup_widget(main_window, "BT_FORCEMOVE");
      gtk_widget_set_sensitive (auto_button, FALSE);
      gtk_widget_set_sensitive (force_button, FALSE);
      */

      return TRUE;
     }

   return FALSE;
}


/*==============================================================================
 * update_status_bar
 *
 * Updates the status bar based on the current value of what_next
 */
update_status_bar()
{
   GtkStatusbar *status = (GtkStatusbar *) lookup_widget(main_window, "statusbar1");
   guint context_id = bstate_get_what_next();

   gtk_statusbar_pop(status, context_id);

   //printf("Updating status bar for state %d\n", bstate_get_what_next());
   switch (bstate_get_what_next()) 
     {
       case FIRE_ARROW:
	   gtk_statusbar_push(status, context_id, "Fire Arrow");
	   break;
       case MOVE_BLACK_QUEEN:
	   gtk_statusbar_push(status, context_id, "Move Black Amazon");
	   break;
       case MOVE_WHITE_QUEEN:
	   gtk_statusbar_push(status, context_id, "Move White Amazon");
	   break;
       case WAIT_FOR_AI:
	   gtk_statusbar_push(status, context_id, "AI is thinking...");
	   break;
       case NEW_GAME:
	   gtk_statusbar_push(status, context_id, "Select Game->New to start game");
	   break;
       case STOP_REPLAY:
	   gtk_statusbar_push(status, context_id, "Replaying a game...");
	   break;

       default:
	   gtk_statusbar_push(status, context_id, "I have no idea what to do next!");

     }

}


/* board_print
 *
 * prints out an ascii version of the board, useful for ensuring the board shows
 * what it should show.
 */
void board_print(Board *board)
{
   int i,j;

   for (i=0; i<BOARD_SIZE; i++)
     {
      for (j=0; j<BOARD_SIZE; j++)
	{
	 if (board->squares[i][j] == 0)
	    printf(" 0");
	 if (board->squares[i][j] == 1)
	    printf(" B");
	 if (board->squares[i][j] == 2)
	    printf(" W");
	 if (board->squares[i][j] == 3)
	    printf(" x");
	}
      printf("\n");
     }
}

/* ===========================================================================
 * board_destroy
 *
 * Destroys the main board canvas group which contains all the square
 * and piece images, and takes them with it.  Once done, it creates a
 * new group that board_draw() can take advantage of.
 */
void board_destroy()
{
   int i;
   GtkWidget *CNVS_GAMEBOARD, *w, *scrolledwindow4, *table1;


#ifdef DEBUG
   printf("Destroying board now\n");
#endif


   gtk_object_destroy(GTK_OBJECT(board->root));
   board->root = GNOME_CANVAS_GROUP(gnome_canvas_item_new(gnome_canvas_root(board->canvas),
	       gnome_canvas_group_get_type(),
	       NULL));
}



/* ============================================================================
 * print_move_in_text_window
 *
 * Prints the given move in official amazon notation.  The move struct
 * doesn't contain 'from' information, so that must be retrieved from
 * the previous state
 */
void print_move_in_text_window(Move *m)
{
   GtkTextView *view; 
   GtkTextBuffer *buffer;
   GtkTextIter *iter = (GtkTextIter *) malloc(sizeof(GtkTextIter));
   char string_buf[32];
   GtkScrolledWindow *w;
   GtkAdjustment *adj;

   view = (GtkTextView *) lookup_widget(main_window, "textview1");
   buffer = gtk_text_view_get_buffer (view);
   gtk_text_buffer_get_end_iter(buffer, iter);

   get_move_str(m, string_buf);

#ifdef DEBUG
   printf("%s", string_buf);
#endif
   gtk_text_buffer_insert(buffer, iter, string_buf, -1);

   w = (GtkScrolledWindow *) lookup_widget(main_window, "scrolledwindow6");
   adj = gtk_scrolled_window_get_vadjustment(w);
   gtk_adjustment_set_value(adj, adj->upper);
   gtk_scrolled_window_set_vadjustment(w, adj);
   

   free (iter);
}


/* ===========================================================================
 * get_move_str
 *
 * Creates a move string from a move struct
 */
void get_move_str(Move *m, char move_str[])
{
   Square to_sq, from_sq, arrow_sq;
   int to_num, from_num, arrow_num;
   char to_alpha, from_alpha, arrow_alpha;
   int state_i = states.current_state;
   int turn;

   //Determine which side just moved:
   turn = OTHER_COLOR(states.states[state_i]->turn);
   from_sq  = get_square_from_engine(states.states[state_i-1]->queens_x[turn][m->queen],
				     states.states[state_i-1]->queens_y[turn][m->queen]);
   to_sq    = get_square_from_engine(m->tocol, m->torow);
   arrow_sq = get_square_from_engine(m->wallcol, m->wallrow);


   from_alpha  = get_grid_alpha_from_square(from_sq);
   from_num    = get_grid_num_from_square(from_sq);

   to_alpha    = get_grid_alpha_from_square(to_sq);
   to_num      = get_grid_num_from_square(to_sq);
   
   arrow_alpha = get_grid_alpha_from_square(arrow_sq);
   arrow_num   = get_grid_num_from_square(arrow_sq);
   
   sprintf(move_str, "%d. %c%d-%c%d, %c%d\n", state_i, from_alpha, from_num,
	   to_alpha, to_num, arrow_alpha, arrow_num);
}


/* ===========================================================================
 * get_move_from_str
 *
 * This generates a move struct from a move string of the format 1. a4-a6, b6
 * If the string is not in a valid format, the return value will be FALSE to 
 * inform the calling routine that the returned move struct does not contain
 * valid information.  If everything goes well, TRUE will be returned.
 */
int
get_move_from_str(Move *m, char move_str[])
{
   int i=0, j;
   int from_row, from_col;
   int to_row, to_col;
   int arrow_row, arrow_col;
   Square queen_sq;
   int queen_i = -1;

   //Ignore move number
   while (move_str[i++] != ' ')

   if (i > 4)
     {
      fprintf(stderr, "Space is in wrong place: %d\n", i);
      return FALSE;  //Ack! this can't be right!
     }
   
   from_col = move_str[i++] - 'a';
   from_row = move_str[i++] - '1';
   if (move_str[i] == '0')
     {
      from_row = 9;
      i++;
     }

   if (move_str[i++] != '-')
     {
      fprintf(stderr, "Expected a hyphen at index %d\n", i);
      return FALSE;  //Ack!!  I expected a '-' here, this is not a valid move string
     }

   to_col = move_str[i++] - 'a';
   to_row = move_str[i++] - '1';
   if (move_str[i] == '0')
     {
      to_row = 9;
      i++;
     }
   
   if (move_str[i++] != ',')
     {
      fprintf(stderr, "Expected a comma at index %d\n", i);
      return FALSE;  //Ack!!  I expected a ',' here, this is not a valid move string
     }
   if (move_str[i++] != ' ')
     {
      fprintf(stderr, "Expected a space at index %d\n", i);
      return FALSE;  //Ack!!  I expected a ' ' here, this is not a valid move string
     }

   arrow_col = move_str[i++] - 'a';
   arrow_row = move_str[i++] - '1';
   if (move_str[i++] == '0')
      arrow_row = 9;

   //Find the index to the appropriate queen
   queen_sq = get_square_from_engine(from_col, from_row);
   for(j=0; j<4; j++)
     {
      if (queen_sq == board->square_to_queen_map[WHITE][j])
	{
	 queen_i = j;
	 //printf("found move for white queen\n");
	}
      if (queen_sq == board->square_to_queen_map[BLACK][j])
	{
	 queen_i = j;
	 //printf("found move for black queen\n");
	}
     }
   if (queen_i == -1)
     {
      fprintf(stderr, "Couldn't find a queen at square %d\n", queen_sq);
      return FALSE;  //Couldn't find queen index!
     }

   m->queen = queen_i;
   m->tocol = to_col;
   m->torow = to_row;
   m->wallcol = arrow_col;
   m->wallrow = arrow_row;

   return TRUE;
}
   

int
read_in_moves(FILE *history_fd)
{
   char *buffer=NULL;
   size_t buf_size = 30;

   State *s;
   Move m;
   int replay_mode = bstate_get_replay_mode();
   int replay_delay = 0;
   int game_life = bstate_get_game_life();
   GtkWidget *delay;

   if (replay_mode)
     {
      delay = (GtkWidget *)lookup_widget(main_window, "ReplayDelaySpinner");
     }
   else
     {
      bstate_set_dont_slide(TRUE);
     }
   //printf("History contents:\n");
   while (getline(&buffer, &buf_size, history_fd) != -1)
     {
      if (replay_mode && !bstate_get_replay_mode()) //If not in replay mode anymore, drop out
	 return FALSE;
      s = states.states[states.current_state];
      /*
      if (buffer != NULL)
	 printf("%s", buffer);
      */
      if (get_move_from_str(&m, buffer))
	{
	 makemove(s,m);  //register w/ engine
	 move_piece(m); //register w/ board
	 if (game_life != bstate_get_game_life())
	    return FALSE;
	 print_move_in_text_window(&m);
	 state_copy(s, states.states[++(states.current_state)]);
	}
      else
	{
	 fprintf(stderr, "Move string is invalid\n");
	 bstate_set_dont_slide(FALSE);
	 return FALSE;
	}
      free(buffer);
      buffer = NULL;

      if (replay_mode) //Delay for appropriate amount of time
       	{
	 replay_delay = gtk_spin_button_get_value_as_int((GtkSpinButton *)delay);
	 rest(replay_delay); 
	 if (game_life != bstate_get_game_life())
	    return FALSE;
	}
     }
   

   bstate_set_dont_slide(FALSE);
   return TRUE;
}


void
rest(int duration)
{
   time_t start = time(NULL);
   time_t end = start + duration;

   while (time(NULL) < end)
     {
      usleep(10000);
      while (gtk_events_pending())
	 gtk_main_iteration();
     }
      
}
