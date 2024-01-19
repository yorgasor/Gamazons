struct board_state {
   int turn;	     //Who's turn it is
   int what_next;    //What's expected to happen next
   int moving_ai;    //TRUE if AI is moving
   int moving_piece; //TRUE if a game piece is sliding
   int new_game;     //TRUE if a new game has been started
   int open_dialog;  //TRUE if a dialog window has been opened
   int quit_game;    //TRUE if the game has been quit
   int dont_slide;   //TRUE if game pieces shouldn't slide on board
   int game_life;    //Each game has a number.  At critical points in the
   		     //program, it can check if this number has changed. If
    		     //so, it's a different game and the prev version should end
   int replay_mode;  //TRUE when a game is being replayed
   Square from;
   Square to;
   Square last_arrow;
};
typedef struct board_state Board_State;


/* Prototypes */

int bstate_set_just_finished(int finished);
//void bstate_store_what_next(int next);
int bstate_get_what_next();
void bstate_set_move_from(Square from);
void bstate_set_move_to(Square to);
Square bstate_get_move_from();
Square bstate_get_move_to();

int  bstate_get_moving_ai();
//void bstate_set_moving_ai(int moving);
int  bstate_get_new_game();
void bstate_set_new_game(int new_game);

int  bstate_get_moving_piece();
void bstate_set_moving_piece(int moving);

void bstate_update_player_settings();
int bstate_get_turn();
int bstate_get_open_dialog();
void bstate_set_open_dialog(int dialog);
int bstate_get_quit_game();
void bstate_set_dont_slide(int slide);
int bstate_get_dont_slide();
int bstate_get_game_life();
void bstate_set_replay_mode(int mode);
int bstate_get_replay_mode();
void print_bstate();

