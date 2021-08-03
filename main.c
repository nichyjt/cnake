#include <stdlib.h>
#include <ncurses.h>
#include <locale.h>


// Prototype Fn Declarations
WINDOW* init_window(int,int,int,int);
void debug_fill_window();
void init_bottom();
struct SnakeCell *init_snake();
int updateAndValidateSnake();
void remove_snake_block();
void add_snake_block();


const int DXN_UP = 1, DXN_DOWN = 2, DXN_LEFT = 3, DXN_RIGHT = 4;

// Linked List-esque
struct SnakeCell {
    int x;
    int y;
    // Allow iteration from back to front
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

    // Variables to help out with the game
    int refresh_ms = 100;

    // Start ncurses with ctrlC acting as sigterm and noecho output to screen
    initscr();
    int initx = getmaxx(stdscr);
    int inity = getmaxy(stdscr);
    cbreak();
    noecho();
    keypad(stdscr, TRUE); //allow usage of fn keys
    refresh();

    //starty = (LINES - height)/2;
    int startx = (initx-50)/2;
    //startx = 1;
    //starty = 1;

    // Main Windows Init
    WINDOW *WINDOW_TOP;
    WINDOW *WINDOW_GAME;
    WINDOW *WINDOW_BOTTOM;
    WINDOW_TOP = init_window(4,50,1,startx);
    WINDOW_GAME = init_window(20, 50, getmaxy(WINDOW_TOP)+1, startx);
    WINDOW_BOTTOM = init_window(5,50, getmaxy(WINDOW_GAME) + getmaxy(WINDOW_TOP)+1, startx);
    
    // Init UI
    init_bottom(WINDOW_BOTTOM);
    // Init Snake and get its head and tail
    struct SnakeCell head;
    head.x = getmaxx(WINDOW_GAME)/2;
    head.y = getmaxy(WINDOW_GAME)/2;
    head.next = NULL;
    
    struct SnakeCell* tail = init_snake(WINDOW_GAME, &head, 5);
    // Main Loop
    int input = DXN_RIGHT;
    int alive = 1;
    int DXN = DXN_RIGHT;
    // Main game loop driver
    while(alive){
        // Get an input and validate correctness
        if(input = getchar() == 'q') break;
        updateAndValidateSnake(WINDOW_GAME, &head, tail, DXN_RIGHT);
    }
    
    // Properly end ncurses mode
    endwin();
    return 0;
}

WINDOW* init_window(int height, int width, int starty, int startx){
    WINDOW* win;
    win = newwin(height, width, starty, startx);
    box(win, 0, 0);
    wrefresh(win);
    return win;
}

struct SnakeCell* init_snake(WINDOW* window, struct SnakeCell *head, int initialSize){
    // Returns the tail of the snake
    struct SnakeCell *cell = head;
    // Populate the linked list
    while(initialSize-- > 0){
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
    if(newx <= xmin || newx >= xmax || newy <= ymin || newy >= ymax){
        printf("ded");
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


void init_bottom(WINDOW* window){
    // Starts up the text for the bottom window
    int mx, my;
    mx = getmaxx(window);
    my = getmaxy(window);
    mvwaddstr(window, 1,1, "Control: WASD/Arrow Keys");
    wrefresh(window);
}

void debug_fill_window(WINDOW* window){
    int mx, my;
    getmaxyx(window, my, mx);
    // Playing field starts from:
    // Rows 1 to mx-1
    // Columns 1 to my-1
    for(int i=1; i<mx-1; ++i){
        for(int j=1; j<my-1; ++j){
            char ch = ACS_BULLET;
            mvwaddch(window, j, i, ch);
            wrefresh(window);
        }
        printf("\n");
    } 
}