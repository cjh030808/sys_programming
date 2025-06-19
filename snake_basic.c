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
} Obstacle;

/* Snake body */
Position snake[MAX_SNAKE_LENGTH];
int snake_length = 3;

Obstacle obstacles[MAX_OBSTACLES];
int obstacle_count = 0;

/* Snake head position and direction */
// default: x = 40, y = 24 즉 UI 중앙값
int head_x = 20;
int head_y = 12;
int x_dir = 1;
int y_dir = 0;

/* Food position */
int food_x = 10;
int food_y = 10;

/* Level and Velocity */
int previous_level = 1;
int level = 1;
int sleep_duration = 150000;

/* Game state Variable*/
int game_over = 0;
int score = 0;

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
  obstacle_count = 0;
}

/* initialize level */
void init_level()
{
  previous_level = 1;
  level = 1;
  sleep_duration = 150000; // 초기 속도로 다시 설정!
}

/* 전체 화면 그리기 (게임 시작/재시작시만 사용) */
void draw_full_screen()
{
  clear();

  // 뱀 그리기
  for (int i = 0; i < snake_length; i++)
  {
    move(snake[i].y, snake[i].x);
    addch(ACS_CKBOARD);
  }

  // 음식 그리기
  move(food_y, food_x);
  addch('*');

  // 장애물 그리기
  for (int i = 0; i < obstacle_count; i++)
  {
    move(obstacles[i].y, obstacles[i].x);
    addch('#');
  }

  // 상태 정보 표시
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
  food_x = rand() % WIDTH;
  food_y = rand() % HEIGHT;
}

/* generate obstacle */
void generate_obstacle()
{
  // limit obstacle < MAX_obstacles
  if (obstacle_count >= MAX_OBSTACLES)
    return;
  // random make obstacle in start point
  int edge = rand() % 4; // 0: 좌, 1: 우, 2: 상, 3: 하

  switch (edge)
  {
  case 0: // left -> right
    obstacles[obstacle_count].x = 0;
    obstacles[obstacle_count].y = rand() % HEIGHT;
    obstacles[obstacle_count].dx = 1;
    obstacles[obstacle_count].dy = 0;
    break;
  case 1: // right -> left
    obstacles[obstacle_count].x = WIDTH - 1;
    obstacles[obstacle_count].y = rand() % HEIGHT;
    obstacles[obstacle_count].dx = -1;
    obstacles[obstacle_count].dy = 0;
    break;
  case 2: // top -> bottom
    obstacles[obstacle_count].x = rand() % WIDTH;
    obstacles[obstacle_count].y = 0;
    obstacles[obstacle_count].dx = 0;
    obstacles[obstacle_count].dy = 1;
    break;
  case 3: // bottom -> top
    obstacles[obstacle_count].x = rand() % WIDTH;
    obstacles[obstacle_count].y = HEIGHT - 1;
    obstacles[obstacle_count].dx = 0;
    obstacles[obstacle_count].dy = -1;
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
    case 'A': // arrow down
      if (y_dir != 1)
      { // prevent reverse direction
        x_dir = 0;
        y_dir = -1;
      }
      break;
    case 'B': // arrow up
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
void check_self_collision()
{
  for (int i = 1; i < snake_length; i++)
  {
    if (snake[0].x == snake[i].x && snake[0].y == snake[i].y)
    {
      game_over = 1;
    }
  }
}

/* check collision between snake and obstacle */
void check_obstacle_collision()
{
  for (int i = 0; i < obstacle_count; i++)
  {
    // 뱀의 모든 몸통 부분과 충돌 검사
    if (obstacles[i].x == snake[0].x && obstacles[i].y == snake[0].y)
    {
      game_over = 1;
    }
  }
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
  check_self_collision();
  check_obstacle_collision();
}

/* move obstacle */
void move_obstacles()
{
  for (int i = 0; i < obstacle_count; i++)
  {
    obstacle[i].x += obstacle[i].dx;
    obstacle[i].y += obstacle[i].dy;

    // when obstacle over terminal WIDTH or HEIGHT then remove
    if (obstacle[i].x < 0 || obstacle[i].x >= WIDTH || obstacle[i].y < 0 || obstacle[i].y >= HEIGHT)
    {
      // 해당 장애물을 마지막으로 이동시키고 카운트 감소
      obstacles[i] = obstacles[obstacle_count - 1];
      obstacle_count--;
      i--;
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
      sleep_duration -= 20000; // 0.02초 줄이기
    }

    // 레벨업 메시지 잠깐 출력
    move(HEIGHT / 2 - 1, WIDTH / 2 - 5);
    printw("LEVEL UP!");

    move(HEIGHT / 2 + 1, WIDTH / 2 - 8);
    printw("YOUR LEVEL IS %d", level);

    refresh();
    usleep(1000000); // 1초로 단축
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
  init_obstacles();
  init_level();
  generate_food();

  while (1)
  {
    process_input();
    move_snake();
    move_obstacles();

    // random obstacle generate (possibility: 2%)
    if (rand() % 50 == 0)
    {
      generate_obstacle();
    }

    update_level();
    draw_full_screen(); // 부분 렌더링 사용
    usleep(sleep_duration);
  }

  endwin(); // end the ncurses screen
  return 0;
}