#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>

// 화면 WIDTH * HEIGHT == 40 * 24
// Snake Length 제한 = 화면 크기
#define WIDTH 40
#define HEIGHT 24
#define MAX_SNAKE_LENGTH (WIDTH * HEIGHT)
#define MAX_OBSTACLES 10

/* Snake structure */
typedef struct
{
  int x, y;
} Position;

typedef struct
{
  int x, y;
  int dx, dy; // 이동 방향
  int active; // 활성 상태 (1: 활성, 0: 비활성)
} Obstacle;

/* Snake body */
Position snake[MAX_SNAKE_LENGTH];
int snake_length = 3;

Obstacle obstacles[MAX_OBSTACLES];
int obstacle_count = 0;

/* Snake head position and direction */
// default: x = 40, y = 20 즉 UI 중앙값
int head_x = 20;
int head_y = 12;
int x_dir = 1;
int y_dir = 0;

/* Food position */
int food_x = 10;
int food_y = 10;

/* Level and Velocity */
int level = 1;                   
int sleep_duration = 150000;     

/* Game state Variable*/
int game_over = 0;
int score = 0;
int previous_level = 1;

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
      else if (bitmap[j][i] == 3)
      {
        addch('#'); // 장애물 표시
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

/* initialize snake */
void init_snake()
{
  for (int i = 0; i < snake_length; i++)
  {
    snake[i].x = head_x - i;
    snake[i].y = head_y;
  }
}

/* initialize obstacle */
void init_obstacles()
{
  for (int i = 0; i < MAX_OBSTACLES; i++)
  {
    obstacles[i].active = 0;
  }
  obstacle_count = 0;
}

/* initialize level */
void init_level() {
  previous_level = 1;
  level = 1;
  sleep_duration = 150000; // 초기 속도로 다시 설정!
}

/* generate new food at random position */
void generate_food()
{
  int valid_position = 0;

  while (!valid_position)
  {
    // random num make by rand() inside the bitmap
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
    for (int i = 0; i < MAX_OBSTACLES; i++)
    {
      if (obstacles[i].active &&
          obstacles[i].x == food_x && obstacles[i].y == food_y)
      {
        valid_position = 0;
        break;
      }
    }
  }
}

/* generate obstacle */
void generate_obstacle()
{
  // limit obstacle < MAX_obstacles
  if (obstacle_count == MAX_OBSTACLES)
    return;

  // find not using obstacle in array
  int slot = -1;
  for (int i = 0; i < MAX_OBSTACLES; i++)
  {
    if (!obstacles[i].active)
    {
      slot = i;
      break;
    }
  }

  Obstacle *obs = &obstacles[slot];
  obs->active = 1;

  // random make obstacle in start point
  int edge = rand() % 4; // 0: 좌, 1: 우, 2: 상, 3: 하

  switch (edge)
  {
  case 0: // left -> right
    obs->x = 0;
    obs->y = rand() % HEIGHT;
    obs->dx = 1;
    obs->dy = 0;
    break;
  case 1: // right -> left
    obs->x = WIDTH - 1;
    obs->y = rand() % HEIGHT;
    obs->dx = -1;
    obs->dy = 0;
    break;
  case 2: // top -> bottom
    obs->x = rand() % WIDTH;
    obs->y = 0;
    obs->dx = 0;
    obs->dy = 1;
    break;
  case 3: // bottom -> top
    obs->x = rand() % WIDTH;
    obs->y = HEIGHT - 1;
    obs->dx = 0;
    obs->dy = -1;
    break;
  }

  obstacle_count++;
}

/* process keyboard inputs */
// default set: prevent reverse direction
void process_input()
{
  int ch = getch();

  if (ch == ERR)
    return; // no input

  // special character starting with 'ESC'
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
    head_x = 20;
    head_y = 12;
    x_dir = 1;
    y_dir = 0;
    score = 0;
    game_over = 0;

    init_snake();
    init_obstacles();
    init_level();
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

/* check collision between snake and obstacle */
int check_obstacle_collision()
{
  for (int i = 0; i < MAX_OBSTACLES; i++)
  {
    if (!obstacles[i].active)
      continue;

    // 뱀의 모든 몸통 부분과 충돌 검사
    for (int j = 0; j < snake_length; j++)
    {
      if (obstacles[i].x == snake[j].x && obstacles[i].y == snake[j].y)
      {
        return 1; // 몸통 어느 부분이라도 충돌하면 게임 오버
      }
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

  // Check collision
  if (check_self_collision() || check_obstacle_collision())
    game_over = 1;
}

/* move obstacle */
void move_obstacles()
{
  for (int i = 0; i < MAX_OBSTACLES; i++)
  {
    if (!obstacles[i].active)
      continue;

    Obstacle *obs = &obstacles[i];

    // obstacle moving (by this dircetion variable)
    obs->x += obs->dx;
    obs->y += obs->dy;

    // when obstacle over terminal WIDTH or HEIGHT then remove
    if (obs->x < 0 || obs->x >= WIDTH || obs->y < 0 || obs->y >= HEIGHT)
    {
      obs->active = 0;
      obstacle_count--;
    }
  }
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

/* mark snake and food in bitmap */
void mark_objects()
{
  // Mark snake body
  for (int i = 0; i < snake_length; i++)
  {
    bitmap[snake[i].x][snake[i].y] = 1;
  }
  // Mark obstacles
  for (int i = 0; i < MAX_OBSTACLES; i++)
  {
    if (!obstacles[i].active)
      continue;
    bitmap[obstacles[i].x][obstacles[i].y] = 3;
  }

  // Mark food
  bitmap[food_x][food_y] = 2;
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
  init_obstacles();
  generate_food();

  while (1)
  {
    clear_bitmap();
    process_input();
    move_snake();
    move_obstacles();

    // random obstacle generate (possibility: 2%)
    // todo: relate level_process -> then more possibility generate obstacle :), so we should change 50 to any variable(ex. obstacle_possibility?)  jojo
    if (rand() % 50 == 0)
    {
      generate_obstacle();
    }
    
    update_level();
    mark_objects();
    draw_bitmap();
    usleep(sleep_duration);
  }

  endwin(); // end the ncurses screen
  return 0;
}