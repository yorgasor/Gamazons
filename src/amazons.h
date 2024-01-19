#define ushort  unsigned short
#define uchar   unsigned char
#define ull     unsigned long long
#define ulong   unsigned long


#define TT 0x3d0925
#define MAX_TT_SEARCH 30

//#define TRUE 1
//#define FALSE 0

#ifndef	FALSE
#define	FALSE	(0)
#endif

#ifndef	TRUE
#define	TRUE	(!FALSE)
#endif

#define WHITE  0  //player 1
#define BLACK  1  //player 2

#define OTHER_COLOR(c) (WHITE + BLACK - (c))


#define NOTHINK 0
#define THINK   1

#undef DEBUG
//#define DEBUG

//#define DEBUG_HEVAL 
//#define DEBUG_WEB


typedef struct
{
    uchar queen;
    uchar tocol, torow;
    uchar wallcol, wallrow;
    int val;
    int depth;
} Move;


#define QUEENS 4

//Basic game state at one point in time
typedef struct
{
    ull  white_bd[2];		/* Bitmap with white queens. */
    ull  black_bd[2];		/* Bitmap with black queens */
    ull  blocks_bd[2];		/* Bitmap with arrows */

    uchar queens_x[2][QUEENS];	/* Col for [WHITE, BLACK] */
    uchar queens_y[2][QUEENS];	/* Row for [WHITE, BLACK] */

    uchar turn;			/* WHITE or BLACK */

    short value;
    uchar depth;
    uchar winner;
} State;


//state stored in the transposition table
struct state_t
{
    ull  white_bd[2];
    ull  black_bd[2];
    ull  blocks_bd[2];

    uchar turn;
    uchar depth;
    short value;
    short alpha;
    short beta;
};

typedef struct state_t state_t;

state_t **tt;

//Collection of all game states
typedef struct
{
   State  *states[100];
   int     current_state;
   int     max_state;
} Game_states ;


//Global game options
struct engine 
{
   int timeout;
   int maxdepth;
   int maxwidth;
};

//paths to game pieces & squares
struct images
{
   char  white_piece[255];
   char  black_piece[255];
   char  white_sq[255];
   char  grey_sq[255];
   char  arrow_sq[255];
   int grid;
};

struct options
{
   int   white_player;		/* AI or HUMAN */
   int   black_player;		/* AI or HUMAN */

   int   print_statistics;
   int   replay_delay;
   int   movement_speed;
   char  hist_dir[512];

   struct engine engine;
   struct images images;
};

// Different player choices
enum {
   AI,
   HUMAN,
};

/* intersting statistic globals */
int tt_overwrite;
int tt_stores;
int tt_updates;
int tt_lookups;
int tt_lookup_finds;

int heval_calls;


//convert x,y coords to pos 0-99
#define XY_TO_POS(x,y) y*10 + x

/* Bit board stuff */
#define GET_ROW(board, row) (board >> (row%5) * 10) & 0x3ff
#define GET_COL(board_l, board_u, col) (GET_HALF_COL(board_l, col) | (GET_HALF_COL(board_u, col) << 5)) 

#define GET_HALF_COL(board, col) (((board >> col) &0x1) | ((board >> (col + 9)) &0x2) | ((board >> (col + 18)) &0x4) | ((board >> (col + 27)) &0x8) | ((board >> (col + 36)) &0x10))

/* These defines require a board number between 0-99 */
//Tells you where in a stream of 10 bits the position (x,y) would be (0-9)
#define GET_COL_POS(y) (int) y / 10
#define GET_ROW_POS(x) x % 10
//Tells you where in a stream of up to 10 bits the position (f,b) would be (0-9)
#define GET_FDIAG_POS(f) (f%10 > f/10) ? f/10 : f%10
#define GET_BDIAG_POS(b) (b/10 < (10 - b%10)) ? b/10 : 9 - b%10

//Gets diag numbers, for passing into get_forw_diag() & get_back_diag()
#define GET_FDIAG(f) (f%10 > f/10) ? f - ((f/10) * 11) : f - ((f%10) * 11)
#define GET_BDIAG(b) (b/10 < (10 - b%10)) ? b - ((b/10) * 9) : b - ((9 - (b%10)) * 9)

//Calculates length of a diagonal, pass in value from GET_FDIAG/GET_BDIAG
//Note: Don't call GET_[F,B]DIAG within GET_[F,B]DIAG_LEN, you get really weird results
//eg GET_FDIAG_LEN(GET_FDIAG(pos)).  Instead, store it into a variable first:
//eg diag = GET_FDIAG(pos); len = GET_FDIAG_LEN(diag);
#define GET_FDIAG_LEN(fdiag) (fdiag < 10) ? (10 - fdiag) : (10 - fdiag/10)
#define GET_BDIAG_LEN(bdiag) (bdiag < 10) ? bdiag + 1 : (10 - bdiag/10)

#define PUT_ROW(board, row, stream) board |= ((ull) stream << ((row % 5) * 10))
#define PUT_COL(board_l, board_u, col, stream) PUT_HALF_COL(board_l, col, stream); PUT_HALF_COL(board_u, col, stream >> 5)
#define PUT_HALF_COL(board, col, stream) board |= ((((ull)stream & 0x1) << col) | \
					 (((ull)stream & 0x2) << (col + 9)) | \
					 (((ull)stream & 0x4) << (col + 18)) | \
					 (((ull)stream & 0x8) << (col + 27)) | \
					 (((ull)stream & 0x10) << (col + 36)))


/* Prototypes */

/* moves.c */
int sean_heval(State *s);

int  state_init(State *s);
void state_copy(State *s_old, State *s_new);
int  state_create_hash(State *s);
int  state_gen_moves(State *s, Move movelist[]);
int  state_is_legal_move(State *state, int color, Move *move);
void state_print(State *s);

Move isearch(State *s, int think, void (*func)(void));
Move search(State *s, int depth, int alpha, int beta, int tdepth, int think);

Move savemove(State *s, Move m);
int  makemove(State *s, Move m);
int  undomove(State *s, Move m);

int  pvec(ull v);
int  string_to_square(char *str, int *col, int *row);
int  string_to_move(State *state, char *str, Move *move);


void pmove(State *state, int color, Move m);
void print_stats();

int  test(State *s, uchar col, uchar row);
void bitmap_clear(ull bd[2]);
void bitmap_print(FILE *outfile, ull bd[2]);
void bitmap_copy(ull bd_old[2], ull bd_new[2]);
void bitmap_set(ull bd[2], uchar col, uchar row);
void bitmap_reset(ull bd[2], uchar col, uchar row);
int  bitmap_isset(ull bd[2], uchar col, uchar row);
void bitmap_flip(ull bd[2], uchar col, uchar row);
void bitmap_or(ull bd_1[2], ull bd_2[2]);

/* eval.c */
int get_forward_diag(ull board_l, ull board_u, int diag);
int get_back_diag(ull board_l, ull board_u, int diag);
void put_forward_diag(ull *board_l, ull *board_u, ushort stream, int diag);
void put_back_diag(ull *board_l, ull *board_u, ushort stream, int diag);
int heval(State *s);

int calc_stream_moves(ushort stream, ushort pos, ushort len);
int calc_moves(ull board_l, ull board_u, int pos);
int count_contig_bits(ushort stream, int len);
ushort gen_web_stream(ushort stream, int pos, int len);
ushort gen_web_stream_plus(ushort stream, int pos, int len);
void gen_web_board(ull *web_l, ull *web_u, ull board_l, ull board_u, int pos);
int gen_web_board_count(ull *web_l, ull *web_u, ull board_l, ull board_u, int pos);

int pbvec(ull l, ull u);

state_t *tt_lookup(State *s);
int tt_compare(State *s, state_t *t);
void tt_store(State *s, short alpha, short beta);
void tt_update(State *s, short alpha, short beta);

void gen_dirs_board(ull *board_l, ull *board_u, int pos);
int count_bits(ull board_l, ull board_u);


