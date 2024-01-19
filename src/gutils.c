#include <stdlib.h>
#include <stdio.h>

#if 1
#include <gnome.h>
#endif

#include "amazons.h"


extern struct options  options;
extern Game_states     states;

extern int             state_hash;
extern GtkWidget      *main_window;


void load_values_from_file(void);


/*==============================================================================
 * init_engine
 *
 * 
 * 
 */
void init_engine()
{
   State  *s;
   Move    temp;
   int     i;
   char   *home_env;

   srand(time(NULL));

   // Initialize game states
   for (i=0; i<100; i++)
      states.s[i] = (State *) malloc(sizeof(State));
   states.current_state = 0;
   states.max_state = 0;

   s = states.s[0];
   s->turn = WHITE;
   s->winner = 0;
   state_hash = state_create_hash(s);


   state_init(s);
   state_copy(s, states.s[++(states.current_state)]);

   /* set default options */
   options.engine.maxdepth=20;
   options.engine.maxwidth=3000;
   options.engine.timeout=1;
   options.white_player=HUMAN;
   options.black_player=HUMAN;
   options.print_statistics=FALSE;

   home_env = getenv("HOME");
   strcpy(options.hist_dir, home_env);
   strcat(options.hist_dir, "/");


   if (!load_images_from_theme(PACKAGE_DATA_DIR "/gamazons/default.theme"))
     {
      fprintf(stderr, "Cannot find theme file %s\n", PACKAGE_DATA_DIR "/gamazons/default.theme");
      exit(1);
     }

#ifdef DEBUG
   printf("white piece image = %s\n", options.images.white_piece);
#endif
   load_values_from_file();

}


/*==============================================================================
 * load_values_from_file
 *
 * If a .gamazons file is found in the user's home directory, it will attempt to
 * load the values stored therein.  Any variables it doesn't recognize will be
 * silently ignored.
 */
void load_values_from_file()
{
   char *home, file[256];
   FILE *rc_fd;
   char variable[256];
   char buffer[256];
   int value;
   char ch;


   if (!(home = getenv("HOME")))
      return;

   strcpy(file, home);
   strcat(file, "/.gamazons");
#ifdef DEBUG
   printf("looking for the file %s\n", file);
#endif

   rc_fd = fopen(file, "r");
   if(rc_fd == NULL)
      return;

   while (fscanf(rc_fd, "%s", variable) != EOF)
     {
      while (ch = fgetc(rc_fd))
	{
	 if (ch == EOF)
	    return;
	 if (ch == '=')
	    break;
	}

      if (strcmp(variable, "WHITE_PLAYER") == 0)
	{
	 fscanf(rc_fd, "%d", &value);
	 options.white_player = value;
	}
      else if (strcmp(variable, "BLACK_PLAYER") == 0)
	{
	 fscanf(rc_fd, "%d", &value);
	 options.black_player = value;
	}
      else if (strcmp(variable, "TIMEOUT") == 0)
	{
	 fscanf(rc_fd, "%d", &value);
	 if (value > 0)
	    options.engine.timeout = value;
	 else
	    options.engine.timeout = 1;
	}
      else if (strcmp(variable, "MAXWIDTH") == 0)
	{
	 fscanf(rc_fd, "%d", &value);
	 if (value > 0)
	    options.engine.maxwidth = value;
	 else
	    options.engine.maxwidth = 1;
	}
      else if (strcmp(variable, "MAXDEPTH") == 0)
	{
	 fscanf(rc_fd, "%d", &value);
	 if (value > 0)
	    options.engine.maxdepth = value;
	 else
	    options.engine.maxdepth = 1;
	}
      else if (strcmp(variable, "REPLAY_DELAY") == 0)
	{
	 fscanf(rc_fd, "%d", &value);
	 if (value > 0)
	    options.replay_delay = value;
	 else
	    options.replay_delay = 1;
	}
      else if (strcmp(variable, "MOVEMENT_SPEED") == 0)
	{
	 fscanf(rc_fd, "%d", &value);
	 if (value > 0 && value <= 10)
	    options.movement_speed = value;
	 else
	    options.movement_speed = 1;
	}
      else if (strcmp(variable, "WHITE_PIECE") == 0)
	{
	 fscanf(rc_fd, "%s", buffer);
	 strcpy(options.images.white_piece, buffer);
	}
      else if (strcmp(variable, "BLACK_PIECE") == 0)
	{
	 fscanf(rc_fd, "%s", buffer);
	 strcpy(options.images.black_piece, buffer);
	}
      else if (strcmp(variable, "WHITE_SQUARE") == 0)
	{
	 fscanf(rc_fd, "%s", buffer);
	 strcpy(options.images.white_sq, buffer);
	}
      else if (strcmp(variable, "GREY_SQUARE") == 0)
	{
	 fscanf(rc_fd, "%s", buffer);
	 strcpy(options.images.grey_sq, buffer);
	}
      else if (strcmp(variable, "ARROW_SQUARE") == 0)
	{
	 fscanf(rc_fd, "%s", buffer);
	 strcpy(options.images.arrow_sq, buffer);
	}
      else if (strcmp(variable, "HIST_DIR") == 0)
	{
	 fscanf(rc_fd, "%s", buffer);
	 strcpy(options.hist_dir, buffer);
	}
      else if (strcmp(variable, "DRAW_GRID") == 0)
	{
	 fscanf(rc_fd, "%s", buffer);
	 if (strcmp(buffer, "TRUE") == 0)
	    options.images.grid = TRUE;
	 else
	    options.images.grid = FALSE;
	}


     }

   fclose(rc_fd);

}

/*==============================================================================
 * store_values_in_file
 *
 * Creates/overwrites a .gamazons file in the user's home directory.  Stores
 * the values in the options struct in it.
 */
void store_values_in_file()
{
   char *home, file[256];
   FILE *rc_fd;
#ifdef GAMAZONS
   GtkWidget *delay = (GtkWidget *)lookup_widget(main_window, "ReplayDelaySpinner");
   GtkWidget *speed = (GtkWidget *)lookup_widget(main_window, "MovementSpeedSpinner");
#endif

   if (!(home = getenv("HOME")))
      return;

   strcpy(file, home);
   strcat(file, "/.gamazons");
#ifdef DEBUG
   printf("looking for the file %s\n", file);
#endif

   rc_fd = fopen(file, "w");
   if (rc_fd == NULL)
      return;
   
   fprintf(rc_fd, "WHITE_PLAYER = ");
   fprintf(rc_fd, "%d\n", options.white_player);

   fprintf(rc_fd, "BLACK_PLAYER = ");
   fprintf(rc_fd, "%d\n", options.black_player);

   fprintf(rc_fd, "TIMEOUT = ");
   fprintf(rc_fd, "%d\n", options.engine.timeout);

   fprintf(rc_fd, "MAXWIDTH = ");
   fprintf(rc_fd, "%d\n", options.engine.maxwidth);

   fprintf(rc_fd, "MAXDEPTH = ");
   fprintf(rc_fd, "%d\n", options.engine.maxdepth);

   fprintf(rc_fd, "WHITE_PIECE = ");
   fprintf(rc_fd, "%s\n", options.images.white_piece);

   fprintf(rc_fd, "BLACK_PIECE = ");
   fprintf(rc_fd, "%s\n", options.images.black_piece);

   fprintf(rc_fd, "WHITE_SQUARE = ");
   fprintf(rc_fd, "%s\n", options.images.white_sq);

   fprintf(rc_fd, "GREY_SQUARE = ");
   fprintf(rc_fd, "%s\n", options.images.grey_sq);

   fprintf(rc_fd, "ARROW_SQUARE = ");
   fprintf(rc_fd, "%s\n", options.images.arrow_sq);

   fprintf(rc_fd, "HIST_DIR = ");
   fprintf(rc_fd, "%s\n", options.hist_dir);

   fprintf(rc_fd, "DRAW_GRID = ");
   if (options.images.grid == TRUE)
      fprintf(rc_fd, "%s\n", "TRUE");
   else
      fprintf(rc_fd, "%s\n", "FALSE");

#ifdef GAMAZONS
   fprintf(rc_fd, "REPLAY_DELAY = ");
   options.replay_delay = gtk_spin_button_get_value_as_int((GtkSpinButton *)delay);
   fprintf(rc_fd, "%d\n", options.replay_delay);

   fprintf(rc_fd, "MOVEMENT_SPEED = ");
   options.movement_speed = gtk_spin_button_get_value_as_int((GtkSpinButton *)speed);
   fprintf(rc_fd, "%d\n", options.movement_speed);
#endif

   fclose(rc_fd);
}

/*==============================================================================
 * load_images_from_theme
 *
 * This file will read image paths from a theme file.  These values get loaded 
 * into the options struct, and subsequently used to draw the board.
 */
int load_images_from_theme(char *theme)
{
   char *home, file[256];
   FILE *theme_fd;
   char variable[256];
   char buffer[256];
   int value;
   char ch;


   theme_fd = fopen(theme, "r");
   if(theme_fd == NULL)
     {
      fprintf(stderr, "Can't open theme file %s\n", theme);
      return FALSE;
     }

   while (fscanf(theme_fd, "%s", variable) != EOF)
     {
      while (ch = fgetc(theme_fd))
	{
	 if (ch == EOF)
	    return TRUE;
	 if (ch == '=')
	    break;
	}

      if (strcmp(variable, "WHITE_PIECE") == 0)
	{
	 fscanf(theme_fd, "%s", buffer);
	 strcpy(options.images.white_piece, PACKAGE_DATA_DIR "/pixmaps/gamazons/");
	 strcat(options.images.white_piece, buffer);
	}
      else if (strcmp(variable, "BLACK_PIECE") == 0)
	{
	 fscanf(theme_fd, "%s", buffer);
	 strcpy(options.images.black_piece, PACKAGE_DATA_DIR "/pixmaps/gamazons/");
	 strcat(options.images.black_piece, buffer);
	}
      else if (strcmp(variable, "WHITE_SQUARE") == 0)
	{
	 fscanf(theme_fd, "%s", buffer);
	 strcpy(options.images.white_sq, PACKAGE_DATA_DIR "/pixmaps/gamazons/");
	 strcat(options.images.white_sq, buffer);
	}
      else if (strcmp(variable, "GREY_SQUARE") == 0)
	{
	 fscanf(theme_fd, "%s", buffer);
	 strcpy(options.images.grey_sq, PACKAGE_DATA_DIR "/pixmaps/gamazons/");
	 strcat(options.images.grey_sq, buffer);
	}
      else if (strcmp(variable, "ARROW_SQUARE") == 0)
	{
	 fscanf(theme_fd, "%s", buffer);
	 strcpy(options.images.arrow_sq, PACKAGE_DATA_DIR "/pixmaps/gamazons/");
	 strcat(options.images.arrow_sq, buffer);
	}
      else if (strcmp(variable, "DRAW_GRID") == 0)
	{
	 fscanf(theme_fd, "%s", buffer);
	 if (strcmp(buffer, "TRUE") == 0)
	    options.images.grid = TRUE;
	 else
	    options.images.grid = FALSE;
	}


     }

   fclose(theme_fd);
   return TRUE;
}
