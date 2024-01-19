; /* Definitions */

/* Widget Names */
#define BOARD_NAME "CNVS_GAMEBOARD"

/* the board size */
#define BOARD_SIZE 10
/* border around the board itself */
#define BOARD_BORDER 0
/* cell size on the board */
#define CELL_SIZE 40.0 
/* cell padding */
#define CELL_PAD 6.0
/* square colors */
#define SQUARE_COLOR_1 "white"
#define SQUARE_COLOR_2 "grey"

#define THICKNESS 1.0
#define QUEEN_OFFSET 0

#define INVALID_SQUARE_VALUE 101

/* Data structures */
typedef gushort Square;
typedef gchar   Piece;


typedef struct {

   // Gnome canvas counts coordinates w/ origin starting in top/left corner
   Square squares[BOARD_SIZE][BOARD_SIZE];

   /* The AI engine queens are managed in an array w/ elements 0-3.
    * This mapping is meant to keep track of which queen is on which
    * square.
    */
   Square square_to_queen_map[2][4];

   GnomeCanvasItem *selected_queen;
   GnomeCanvasItem *queen_images[2][4];

   double orig_x;
   double orig_y;
   double curr_x;
   double curr_y;

   // char db[120];
   GnomeCanvas       *canvas;
   GnomeCanvasGroup  *root;
   GnomeCanvasItem   *square_items[100];
} Board;


/* Possible square occupants */
enum {
   WHITE_SQUARE,
   BLACK_SQUARE,
   EMPTY_SQUARE,
   ARROW_SQUARE
};

//GUI states
enum {
   MOVE_BLACK_QUEEN,
   MOVE_WHITE_QUEEN,
   UNDO,
   FIRE_ARROW,

   WAIT_FOR_AI,
   FORCE_MOVE,

   START_GAME,
   NEW_GAME,
   LOAD_GAME,
   AUTO_FINISH,
   START_REPLAY,
   STOP_REPLAY,
   GAME_OVER,
   QUIT_GAME,

   CONFUSED
};

/* Prototypes */

void board_init_game(GtkWidget *GamazonsMain);
void board_draw();
void mark_square (GnomeCanvasItem *square);


Square  get_square (double x, double y);
void    clear_square (GnomeCanvasItem **square);
void    move_piece_to (Square to, GnomeCanvasItem *item);

void  move_piece(Move m);
int   move_ai();
void  register_move_with_engine(Square arrow_sq);
void  gen_legal_moves(Square sq);
int   is_move_legal(Square sq);
int   is_queen_square(Square sq);
void  free_all_memory();
void  board_print(Board *board);
void  board_destroy();
void  print_move_in_text_window(Move *m);
int   read_in_moves(FILE *history_fd);
void  get_move_str(Move *m, char move_str[]);
int   get_move_from_str(Move *m, char move_str[]);
void  rest(int duration);


// Coordinate conversion routines
double get_x_from_square(int sq);
double get_y_from_square(int sq);
int get_x_int_from_square(int sq);
int get_y_int_from_square(int sq);
int engine_x_to_board_x(int eng_x);
int engine_y_to_board_y(int eng_y);
int board_x_to_engine_x(int brd_x);
int board_y_to_engine_y(int brd_y);
int get_square_from_engine(int x, int y);


void fire_arrow(Square sq);
void square_contains(Square sq);
void count_queens();
int game_over();


