#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>

// 화면 WIDTH * HEIGHT == 80 * 24
// Snake Length 제한 = 화면 크기
#define WIDTH 80
#define HEIGHT 24
#define MAX_SNAKE_LENGTH (WIDTH * HEIGHT)

/* Snake structure */
typedef struct
{
  int x, y;
} Position;

/* Snake body */
Position snake[MAX_SNAKE_LENGTH];
int snake_length = 3;

/* Snake head position and direction */
// default: x = 40, y = 20 즉 UI 중앙값
int head_x = 40;
int head_y = 12;
int x_dir = 1;
int y_dir = 0;

/* Food position */
int food_x = 20;
int food_y = 10;

/* Level and Velocity */
int level = 1;                   
int sleep_duration = 150000;     

/* Game state Variable*/
int game_over = 0;
int score = 0;

/* bitmap to draw at the screen */
// WIDTH = 80, HEIGHT = 24
uint8_t bitmap[WIDTH][HEIGHT] = {
    0,
};

/* clear bitmap array */
void clear_bitmap()
{
  memset(bitmap, 0, WIDTH * HEIGHT);
}

/* draw bitmap array on the screen */
void draw_bitmap()
{
  move(0, 0); // move cursor to the top left corner
  for (int i = 0; i < HEIGHT; i++)
  {
    move(i, 0);
    for (int j = 0; j < WIDTH; j++)
    {
      if (bitmap[j][i] == 1)
      {
        addch(ACS_CKBOARD); // snake body
      }
      else if (bitmap[j][i] == 2)
      {
        addch('*'); // food
      }
      else
      {
        addch(' '); // empty space
      }
    }
  }

  // Display score
  move(HEIGHT, 0);
  printw("Score: %d | Length: %d | Level: %d | Press 'q' to quit", score, snake_length, level);

  if (game_over)
  {
    move(HEIGHT / 2, WIDTH / 2 - 5);
    printw("GAME OVER!");
    move(HEIGHT / 2 + 1, WIDTH / 2 - 8);
    printw("Press 'r' to restart");
  }

  refresh();
}

/* generate new food at random position */
void generate_food()
{
  int valid_position = 0;

  while (!valid_position)
  {
    // random num make by rand()
    // modulo 연산자 사용 -> inside the bitmap
    food_x = rand() % WIDTH;
    food_y = rand() % HEIGHT;

    // Check if food position conflicts with snake body
    // if conflict -> then lose the game && break generate_food()
    // break generate_food() -> stop making the food at random position
    valid_position = 1;
    for (int i = 0; i < snake_length; i++)
    {
      if (snake[i].x == food_x && snake[i].y == food_y)
      {
        valid_position = 0;
        break;
      }
    }
  }
}

/* process keyboard inputs */
// default set: prevent reverse direction
void process_input()
{
  int ch = getch();

  if (ch == ERR)
    return; // no input

  /* special character starting with 'ESC' */
  if (ch == '\033')
  {
    getch(); // skip '['
    switch (getch())
    {
    case 'A': // arrow up
      if (y_dir != 1)
      { // prevent reverse direction
        x_dir = 0;
        y_dir = -1;
      }
      break;
    case 'B': // arrow down
      if (y_dir != -1)
      { // prevent reverse direction
        x_dir = 0;
        y_dir = 1;
      }
      break;
    case 'C': // arrow right
      if (x_dir != -1)
      { // prevent reverse direction
        x_dir = 1;
        y_dir = 0;
      }
      break;
    case 'D': // arrow left
      if (x_dir != 1)
      { // prevent reverse direction
        x_dir = -1;
        y_dir = 0;
      }
      break;
    }
  }
  else if (ch == 'w' && y_dir != 1)
  { // 'w' key
    x_dir = 0;
    y_dir = -1;
  }
  else if (ch == 's' && y_dir != -1)
  { // 's' key
    x_dir = 0;
    y_dir = 1;
  }
  else if (ch == 'a' && x_dir != 1)
  { // 'a' key
    x_dir = -1;
    y_dir = 0;
  }
  else if (ch == 'd' && x_dir != -1)
  { // 'd' key
    x_dir = 1;
    y_dir = 0;
  }
  else if (ch == 'q')
  { // 'q' key to quit
    endwin();
    exit(0);
  }
  else if (ch == 'r' && game_over)
  { // 'r' key to restart
    // Reset game state
    // but only if this 분기처리 possible in game over
    snake_length = 3;
    head_x = 40;
    head_y = 12;
    x_dir = 1;
    y_dir = 0;
    score = 0;
    game_over = 0;

    extern void reset_level_tracking();

    reset_level_tracking();

    // Initialize snake
    for (int i = 0; i < snake_length; i++)
    {
      snake[i].x = head_x - i;
      snake[i].y = head_y;
    }

    generate_food();
  }
}

/* check collision with snake body */
int check_self_collision()
{
  for (int i = 1; i < snake_length; i++)
  {
    if (snake[0].x == snake[i].x && snake[0].y == snake[i].y)
    {
      return 1;
    }
  }
  return 0;
}

/* move snake */
void move_snake()
{
  if (game_over)
    return;

  // Calculate new head position
  head_x += x_dir;
  head_y += y_dir;

  // Handle boundary wrapping
  if (head_x < 0)
    head_x = WIDTH - 1;
  if (head_x >= WIDTH)
    head_x = 0;
  if (head_y < 0)
    head_y = HEIGHT - 1;
  if (head_y >= HEIGHT)
    head_y = 0;

  // Move body segments
  for (int i = snake_length - 1; i > 0; i--)
  {
    snake[i] = snake[i - 1];
  }

  // Set new head position
  snake[0].x = head_x;
  snake[0].y = head_y;

  // Check if snake ate food
  if (head_x == food_x && head_y == food_y)
  {
    snake_length++;
    score += 10;
    generate_food();
  }

  // Check self collision
  if (check_self_collision())
  {
    game_over = 1;
  }
}

/* mark snake and food in bitmap */
void mark_objects()
{
  // Mark snake body
  for (int i = 0; i < snake_length; i++)
  {
    bitmap[snake[i].x][snake[i].y] = 1;
  }

  // Mark food
  bitmap[food_x][food_y] = 2;
}

/* initialize snake */
void init_snake()
{
  for (int i = 0; i < snake_length; i++)
  {
    snake[i].x = head_x - i;
    snake[i].y = head_y;
  }
}

int previous_level = 1;

void reset_level_tracking() {
  previous_level = 1;
  level = 1;
  sleep_duration = 150000; // 초기 속도로 다시 설정!
}



void update_level()
{
  
  // 점수에 따라 레벨 계산 (50점마다 1레벨씩 상승)
  level = score / 50 + 1;

  // 이전보다 레벨이 올라갔을 경우에만 처리
  if (level > previous_level)
  {
    previous_level = level;

    if (sleep_duration > 50000)
    {
      sleep_duration -= 20000; // 0.01초 줄이기
    }

    // 레벨업 메시지 잠깐 출력
    attron(A_BLINK); // 깜빡이 기능!
    move(HEIGHT / 2 - 1, WIDTH / 2 - 5);
    printw("LEVEL UP!");
    attroff(A_BLINK); 

    move(HEIGHT / 2 + 1, WIDTH / 2 - 8);
    printw("YOUR LEVEL IS %d", level);

    refresh();
    usleep(3000000);  // 메시지 잠깐 보여주고 다시 시작
  }
}

int main()
{
  srand(time(NULL)); // seed random number generator

  initscr();             // initialize ncurse screen
  cbreak();              // disable the line break buffer
  nodelay(stdscr, TRUE); // disable delay to wait keyboard inputs
  noecho();              // disable input character echos
  curs_set(0);           // disable cursor visibility

  init_snake();
  generate_food();

  while (1)
  {
    clear_bitmap(); // memset으로 계속 화면 다시 부름 (비효율적인 듯)
    process_input();
    move_snake();
    update_level();
    mark_objects();
    draw_bitmap();
    usleep(sleep_duration); // 레벨에 따라 속도 반영
  }

  endwin(); // end the ncurses screen
  return 0;
}