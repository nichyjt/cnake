#include <stdlib.h>
#include <ncurses.h>
#include <locale.h>
#include <string.h>
#include <time.h>

// Prototype Fn Declarations
WINDOW* init_window();
void debug_fill_window();
struct SnakeCell *init_snake();
struct SnakeCell *add_snake_cell();
int updateAndValidateSnake();
void remove_snake_block();
void add_snake_block();
void generate_food();
int parseInput();
int start_game();


// Constant game parameters
const int DXN_UP = 3, DXN_DOWN = 5, DXN_LEFT = 2, DXN_RIGHT = 4;
const int GAME_HEIGHT = 15; //y
const int GAME_WIDTH = 42; //x
const int LEVEL_MAX = 30;
const int LEVEL_SPEED = 20;
time_t t;

// Doubly Linked List
struct SnakeCell {
    int x;
    int y;
    struct SnakeCell* next;
    struct SnakeCell* prev;
};

struct Food {
    int x;
    int y;
};

// Driver function
int main(){
    setlocale(LC_CTYPE,"C-UTF-8");
    // TODO terminate early if the terminal scr is too small

    // Start ncurses with ctrlC acting as sigterm and noecho output to screen
    initscr();
    int initx = getmaxx(stdscr);
    int inity = getmaxy(stdscr);
    cbreak();
    noecho();
    keypad(stdscr, TRUE); //allow usage of fn keys
    refresh();
    curs_set(0);

    //starty = (LINES - height)/2;
    int startx = (initx-50)/2;

    // Main Windows Init
    WINDOW *WINDOW_TOP;
    WINDOW *WINDOW_GAME;
    WINDOW *WINDOW_BOTTOM;
    WINDOW_TOP = init_window(3, GAME_WIDTH, 1, startx, 0);
    WINDOW_GAME = init_window(GAME_HEIGHT, GAME_WIDTH, getmaxy(WINDOW_TOP)+1, startx, 1);
    WINDOW_BOTTOM = init_window(5, GAME_WIDTH, getmaxy(WINDOW_GAME) + getmaxy(WINDOW_TOP)+1, startx, 1);

    // Pre-Game UI Initialisation
    mvwaddstr(WINDOW_TOP, 0, 1, "CNAKE: A Ripoff Game by nichyjt");
    mvwaddstr(WINDOW_BOTTOM, 1, 1, "- WASD/Arrow Keys to move");
    mvwaddstr(WINDOW_BOTTOM, 2, 1, "- Press 'q' to quit");
    mvwaddstr(WINDOW_BOTTOM, 3, 1, "- Collect diamonds for points!");
    wrefresh(WINDOW_TOP);
    wrefresh(WINDOW_BOTTOM);
    if(getch() == 'q'){
        endwin();
        return 0;
    }

    // Dangerous & spicy infinite loop
    int retry = 1;
    while(retry){
        retry = start_game(WINDOW_TOP, WINDOW_GAME);
    }
    // Properly end ncurses mode
    endwin();
    return 0;
}

// TODO retry-able game logic
int start_game(WINDOW* WINDOW_TOP, WINDOW* WINDOW_GAME){
    // Init Snake, its params and get its head and tail
    struct SnakeCell* head = malloc(sizeof(struct SnakeCell));
    int snakelen = 5;
    head->x = getmaxx(WINDOW_GAME)/2;
    head->y = getmaxy(WINDOW_GAME)/2;
    head->next = NULL;
    struct SnakeCell* tail = init_snake(WINDOW_GAME, head, 5);

    // Init the first food coordinates
    struct Food food;
    food.x = 2*getmaxx(WINDOW_GAME)/4;
    food.y = 3*getmaxy(WINDOW_GAME)/4;
    mvwaddch(WINDOW_GAME, food.y, food.x, ACS_DIAMOND);
    wrefresh(WINDOW_GAME);
    
    // Main Loop and game variables
    int score = 0;
    char scoretext[20] = "Score: 0";
    char leveltext[20];
    sprintf(leveltext, "Level: 1/%d", LEVEL_MAX);
    // Level up every 2 pieces
    // Level 1-5 Size Increase
    // Level 6-10 Timeout duration decrease by 50ms
    int level = 1;
    mvwaddstr(WINDOW_TOP, 1, 1, leveltext);
    mvwaddstr(WINDOW_TOP, 2, 1, scoretext);
    wrefresh(WINDOW_TOP);
    int input = DXN_RIGHT;
    int inertia = DXN_RIGHT;
    int addCell = 0;
    int DXN = DXN_RIGHT;
    // Main game loop driver
    int timeout_duration = 400;
    timeout(timeout_duration);
    // Add length to snake body if applicable
    while(1){
        if( (input = getch()) == ERR) input = inertia;
        inertia = parseInput(input, inertia);
        if(inertia < 0) break;
        if(addCell){
            //printf("addcell");
            addCell = 0;
            head = add_snake_cell(WINDOW_GAME, head, inertia);
        }
        // Get an input and validate correctness
        if(updateAndValidateSnake(WINDOW_GAME, head, tail, inertia)){
            // Check if head coincides with the food.
            // Update everything as necessary
            if(head->x == food.x && head->y == food.y){
                sprintf(scoretext, "Score: %d", ++score);
                mvwaddstr(WINDOW_TOP, 2,1, scoretext);
                generate_food(WINDOW_GAME, head, &food);
                // Handle lvl up logic
                if(!(score%2) && level < LEVEL_MAX){
                    ++level;
                    if(level<LEVEL_SPEED){
                        addCell = 1;
                    }else if(level<LEVEL_MAX){
                        timeout_duration-=50;
                        timeout(timeout_duration);
                    }
                }
                sprintf(leveltext, "Level: %d/%d", level, LEVEL_MAX);
                mvwaddstr(WINDOW_TOP, 1,1, leveltext);
                wrefresh(WINDOW_TOP);
            }
            if(mvwinch(WINDOW_GAME, food.y, food.x) != ACS_DIAMOND){
                mvwaddch(WINDOW_GAME, food.y, food.x, ACS_DIAMOND);  
                wrefresh(WINDOW_GAME);
                refresh();
                redrawwin(WINDOW_GAME);
            }
        }else{
            //TODO add color and game over logic
            mvwaddstr(WINDOW_TOP, 2, 12, "GAME OVER! Press r to retry...");
            wrefresh(WINDOW_TOP);
            timeout(-1);
            while(input = getch()){
                if(input == 'r'){
                    werase(WINDOW_GAME);
                    mvwaddstr(WINDOW_TOP, 2, 12, "                              "); //lol
                    box(WINDOW_GAME, 0, 0);
                    return 1;
                }else if(input == 'q'){
                    return 0;
                }
            }
            return 0;
        }
    }
    return 0;
}


WINDOW* init_window(int height, int width, int starty, int startx, int border){
    WINDOW* win;
    win = newwin(height, width, starty, startx);
    if(border) box(win, 0, 0);
    wrefresh(win);
    return win;
}

struct SnakeCell* init_snake(WINDOW* window, struct SnakeCell *head, int initialSize){
    // Returns the tail of the snake
    struct SnakeCell *cell = head;
    // Populate the linked list
    while(initialSize-- > 1){
        struct SnakeCell* newcell = malloc(sizeof(struct SnakeCell));
        cell->prev = newcell;
        newcell->next = cell;
        newcell->x = cell->x-1;
        newcell->y = cell->y;
        cell = newcell;
    }
    // Initialise the final node's prev pointer
    cell->prev = NULL;
    struct SnakeCell * tail = cell;
    // Populate the game window
    while(cell != NULL){
        mvwaddch(window, cell->y, cell->x, '#');
        cell = cell->next;
    }
    wrefresh(window);
    return tail;
}

struct SnakeCell *add_snake_cell(WINDOW* window, struct SnakeCell *head, int inertia){
    // Returns the new head
    struct SnakeCell *newhead = malloc(sizeof(struct SnakeCell));
    int newx, newy;
    if(inertia == DXN_UP){
        newhead->x = head->x;
        newhead->y = head->y - 1;
    } else if(inertia == DXN_DOWN){
        newhead->x = head->x;
        newhead->y = head->y + 1;
    } else if(inertia == DXN_LEFT){
        newhead->x = head->x - 1;
        newhead->y = head->y;
    } else if(inertia == DXN_RIGHT){
        newhead->x = head->x + 1;
        newhead->y = head->y;
    }
    head->next = newhead;
    newhead->prev = head;
    mvwaddch(window, newhead->y, newhead->x, '#');
    wrefresh(window);
    return newhead;
}

int updateAndValidateSnake(WINDOW * window, struct SnakeCell *head, struct SnakeCell *tail, int DXN){
    // Return 0 if snake dies
    // Find the playing field limits
    int xmin = 1, xmax = getmaxx(window);
    int ymin = 1, ymax = getmaxy(window);
    // Update the snake's position
    int newx, newy; //head
    int oldx, oldy; //tail
    if(DXN == DXN_UP){
        newx = head->x;
        newy = head->y - 1;
    } else if(DXN == DXN_DOWN){
        newx = head->x;
        newy = head->y + 1;
    } else if(DXN == DXN_LEFT){
        newx = head->x - 1;
        newy = head->y;
    } else if(DXN == DXN_RIGHT){
        newx = head->x + 1;
        newy = head->y;
    }
    // Check if the snake dies due to touching the border
    if(newx < xmin || newx > xmax-2 || newy < ymin || newy > ymax-2){
        return 0;
    }
    // Update ListCoords from tail to head
    struct SnakeCell* curr = tail;
    oldx = curr->x;
    curr->x = curr->next->x;
    oldy = curr->y;
    curr->y = curr->next->y;
    curr = curr->next;

    // Visit every non-head/tail cell and...
    // ...check if the next tick/mvmt will cause issues
    while(curr->next != NULL){
        // Check if the new position touches any cell
        if(newx == curr->x && newy == curr->y){
            return 0;
        }
        
        // No issue; update xy
        curr->x = curr->next->x;
        curr->y = curr->next->y;
        curr = curr->next;
    }
    // Snake has passed tick test (no self-collision)
    // Curr is now the head
    curr->x = newx;
    curr->y = newy;
    // Update the UI accordingly
    remove_snake_block(window, oldx, oldy);
    add_snake_block(window, newx, newy);
    return 1;
}

void add_snake_block(WINDOW* window, int newx, int newy){
    mvwaddch(window, newy, newx, '#');
    wrefresh(window);
}

void remove_snake_block(WINDOW* window, int oldx, int oldy){
    mvwaddch(window, oldy, oldx, ' ');
    wrefresh(window);
}

int parseInput(int input, int inertia){
    // Return -1 if quit
    // Return 0 if invalid (opposite dxn)
    // Return DXN_X if valid input
    switch(input){
        case 'q':
            return -1;
        break;
        case 'w':
        case KEY_UP:
            return (inertia%2 != 0)? inertia:DXN_UP;
            break;
        case 's':
        case KEY_DOWN:
            return (inertia%2 != 0)? inertia:DXN_DOWN;
            break;
        case 'a':
        case KEY_LEFT:
            return (inertia%2 == 0)? inertia:DXN_LEFT;
            break;
        case 'd':
        case KEY_RIGHT:
            return (inertia%2 == 0)? inertia:DXN_RIGHT;
            break;
        default:
            return inertia;
            break; 
    }
}

void generate_food(WINDOW* window, struct SnakeCell* head, struct Food *food){
    // head.x 1<=N<=GAME_HEIGHT-1
    // head.y 1<=N<=GAME_WIDTH-1

    // Find out what coordinates are a no-go
    int num_rows = GAME_HEIGHT-2;
    int num_cols = GAME_WIDTH-2;
    int matrix[(num_rows)*(num_cols)];
    // Contiguous index not working with the compiler.
    // So, using manual array instead to count appearances
    memset(matrix, 0, sizeof matrix);
    struct SnakeCell* curr = head;

    int n;
    while(curr != NULL){
        n = (curr->x-1)*(num_rows) + (curr->y-1)%(num_rows);
        ++matrix[n];
        curr = curr->prev;
    }
    // Choose the rnum-th valid square
    int rnum, i = 0;
    srand(time(&t));
    rnum = rand() % (num_rows*num_cols);    
    
    while(rnum>0){
        if(i>num_rows*num_cols) i = 0;
        if(matrix[i]==0) --rnum;
        ++i;
    }
  
    // Update food coords
    food->y = i/(num_cols)+1;
    food->x = i%(num_cols)+1;
    // printf("(%d:%d,%d)", i, food->x, food->y);
    mvwaddch(window, food->y, food->x, ACS_DIAMOND);
    wrefresh(window);
}