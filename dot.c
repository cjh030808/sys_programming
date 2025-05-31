#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>

#define WIDTH 80
#define HEIGHT 24

/* Position of the ball */
int8_t ball_x = 40;
int8_t ball_y = 12;

/* Moving directions of the ball */
int8_t x_dir = 1;
int8_t y_dir = 1;

/* bitmap to draw at the screen */
uint8_t bitmap[WIDTH][HEIGHT] = {0, };

/* clear bitmap array */
void clear_bitmap()
{
  memset(bitmap, 0, WIDTH * HEIGHT);
}

/* draw bitmap array on the screen */
void draw_bitmap()
{
  move(0, 0); // move cursor to the top left corner
  for( int i = 0; i < HEIGHT; i++ ) {
    move(i, 0);
    for( int j = 0; j < WIDTH; j++ ) {
      if( bitmap[j][i] ) {
	addch(ACS_CKBOARD); // black
      } else {
	addch(' '); // white
      }
    }
  }

  refresh();
}

/* process keyboard inputs */
void process_input()
{
  char ch = getch();

  /* special character starting with 'ESC' */
  if( ch == '\033') {
    getch(); // skip '['
    switch(getch()) {
    case 'A': // arrow up
      y_dir = -1;
      break;
    case 'B': // arrow down
      y_dir = 1;
      break;
    case 'C': // arrow right
      x_dir = 1;
      break;
    case 'D': // arrow left
      x_dir = -1;
      break;
    }
  } else if( ch == 'w' ) y_dir = -1; // 'w' key
  else if( ch == 's' ) y_dir = 1;    // 's' key
  else if( ch == 'a' ) x_dir = -1;   // 'a' key
  else if( ch == 'd' ) x_dir = 1;    // 'd' key
  else if( ch == 'q' ) {             // 'q' key to quit
    endwin();
    exit(0);
  }
}

/* for every cycle, move the ball position diagonally */
void move_ball()
{
  ball_x += x_dir;
  ball_y += y_dir;
  
  // at the boundary, the ball goes to the other end
  if( ball_x < 0 ) ball_x = WIDTH - 1;
  if( ball_x > WIDTH ) ball_x = 0;
  if( ball_y < 0 ) ball_y = HEIGHT - 1;
  if( ball_y > HEIGHT ) ball_y = 0;
}

/* draw the ball in the bitmap */
void mark_ball()
{
  bitmap[ball_x][ball_y] = 1;
}

int main()
{
  initscr(); // initialize ncurse screen
  cbreak();  // disable the line break buffer
  nodelay(stdscr, TRUE); // disable delay to wait keyboard inputs
  noecho();  // disable input character echos
  curs_set(0); // disable cursor visibility
  //printf("\e[1J");

  while(1) {
    clear_bitmap();
    process_input();
    move_ball();
    mark_ball();
    draw_bitmap();
    usleep(100000); // sleep 100 ms
  }

  endwin(); // end the ncures screen
}
