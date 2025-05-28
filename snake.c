#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>

#define WIDTH 80
#define HEIGHT 24
#define MAX_SNAKE_LENGTH (WIDTH * HEIGHT)

/* Snake structure */
typedef struct
{
  int x, y;
} Position;

/* Global variables */
Position snake[MAX_SNAKE_LENGTH];
int snake_length = 3;
int head_x = 40;
int head_y = 12;
int x_dir = 1;
int y_dir = 0;
int food_x = 20;
int food_y = 10;
int game_over = 0;
int score = 0;
uint8_t bitmap[WIDTH][HEIGHT] = {
    0,
};

/* ARM Assembly optimized functions */

/* Fast bitmap clear using ARM assembly */
void clear_bitmap_asm()
{
#ifdef __arm__
  uint32_t *ptr = (uint32_t *)bitmap;
  int count = (WIDTH * HEIGHT) / 4; // 4 bytes at a time

  __asm__ volatile(
      "mov r0, #0          \n\t" // Clear value
      "1:                  \n\t" // Loop label
      "str r0, [%0], #4    \n\t" // Store and post-increment
      "subs %1, %1, #1     \n\t" // Decrement counter
      "bne 1b              \n\t" // Branch if not zero
      : "=&r"(ptr), "=&r"(count)
      : "0"(ptr), "1"(count)
      : "r0", "memory");
#else
  memset(bitmap, 0, WIDTH * HEIGHT);
#endif
}

/* Fast boundary wrapping using ARM assembly */
int wrap_coordinate_asm(int coord, int max_val)
{
#ifdef __arm__
  int result;
  __asm__ volatile(
      "cmp %1, #0          \n\t" // Compare coord with 0
      "movlt %0, %2        \n\t" // If < 0, set to max_val-1
      "sublt %0, %0, #1    \n\t"
      "blt 2f              \n\t" // Branch to end
      "cmp %1, %2          \n\t" // Compare coord with max_val
      "movge %0, #0        \n\t" // If >= max_val, set to 0
      "movlt %0, %1        \n\t" // Otherwise, keep original value
      "2:                  \n\t" // End label
      : "=&r"(result)
      : "r"(coord), "r"(max_val)
      : "cc");
  return result;
#else
  if (coord < 0)
    return max_val - 1;
  if (coord >= max_val)
    return 0;
  return coord;
#endif
}

/* Fast collision detection using ARM assembly */
int check_collision_asm(int x, int y, Position *snake_body, int length)
{
#ifdef __arm__
  int result = 0;
  __asm__ volatile(
      "mov %0, #0          \n\t" // Initialize result to 0
      "mov r3, #1          \n\t" // Start from index 1 (skip head)
      "1:                  \n\t" // Loop start
      "cmp r3, %3          \n\t" // Compare index with length
      "bge 2f              \n\t" // Branch if >= length
      "lsl r4, r3, #3      \n\t" // r4 = index * 8 (sizeof(Position))
      "ldr r5, [%2, r4]    \n\t" // Load snake[i].x
      "add r4, r4, #4      \n\t" // Offset for y coordinate
      "ldr r6, [%2, r4]    \n\t" // Load snake[i].y
      "cmp r5, %1          \n\t" // Compare snake[i].x with x
      "bne 3f              \n\t" // Branch if not equal
      "cmp r6, %4          \n\t" // Compare snake[i].y with y
      "moveq %0, #1        \n\t" // If equal, set result to 1
      "beq 2f              \n\t" // Branch to end
      "3:                  \n\t" // Continue label
      "add r3, r3, #1      \n\t" // Increment index
      "b 1b                \n\t" // Branch to loop start
      "2:                  \n\t" // End label
      : "=&r"(result)
      : "r"(x), "r"(snake_body), "r"(length), "r"(y)
      : "r3", "r4", "r5", "r6", "cc");
  return result;
#else
  for (int i = 1; i < length; i++)
  {
    if (snake_body[i].x == x && snake_body[i].y == y)
    {
      return 1;
    }
  }
  return 0;
#endif
}

/* Fast array copy for snake movement using ARM assembly */
void move_snake_body_asm(Position *snake_body, int length)
{
#ifdef __arm__
  if (length <= 1)
    return;

  __asm__ volatile(
      "sub r2, %1, #1      \n\t" // r2 = length - 1
      "1:                  \n\t" // Loop start
      "cmp r2, #0          \n\t" // Compare with 0
      "ble 2f              \n\t" // Branch if <= 0
      "lsl r3, r2, #3      \n\t" // r3 = r2 * 8 (current index offset)
      "sub r4, r3, #8      \n\t" // r4 = (r2-1) * 8 (previous index offset)
      "ldr r5, [%0, r4]    \n\t" // Load snake[r2-1].x
      "str r5, [%0, r3]    \n\t" // Store to snake[r2].x
      "add r4, r4, #4      \n\t" // Offset for y coordinate
      "add r3, r3, #4      \n\t" // Offset for y coordinate
      "ldr r5, [%0, r4]    \n\t" // Load snake[r2-1].y
      "str r5, [%0, r3]    \n\t" // Store to snake[r2].y
      "sub r2, r2, #1      \n\t" // Decrement index
      "b 1b                \n\t" // Branch to loop start
      "2:                  \n\t" // End label
      :
      : "r"(snake_body), "r"(length)
      : "r2", "r3", "r4", "r5", "memory");
#else
  for (int i = length - 1; i > 0; i--)
  {
    snake_body[i] = snake_body[i - 1];
  }
#endif
}

/* Regular C functions */
void clear_bitmap()
{
  clear_bitmap_asm();
}

void draw_bitmap()
{
  move(0, 0);
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

  move(HEIGHT, 0);
  printw("Score: %d | Length: %d | Press 'q' to quit", score, snake_length);

  if (game_over)
  {
    move(HEIGHT / 2, WIDTH / 2 - 5);
    printw("GAME OVER!");
    move(HEIGHT / 2 + 1, WIDTH / 2 - 8);
    printw("Press 'r' to restart");
  }

  refresh();
}

void generate_food()
{
  int valid_position = 0;

  while (!valid_position)
  {
    food_x = rand() % WIDTH;
    food_y = rand() % HEIGHT;

    // Use optimized collision check
    valid_position = !check_collision_asm(food_x, food_y, snake, snake_length);
  }
}

void process_input()
{
  int ch = getch();

  if (ch == ERR)
    return;

  if (ch == '\033')
  {
    getch();
    switch (getch())
    {
    case 'A': // arrow up
      if (y_dir != 1)
      {
        x_dir = 0;
        y_dir = -1;
      }
      break;
    case 'B': // arrow down
      if (y_dir != -1)
      {
        x_dir = 0;
        y_dir = 1;
      }
      break;
    case 'C': // arrow right
      if (x_dir != -1)
      {
        x_dir = 1;
        y_dir = 0;
      }
      break;
    case 'D': // arrow left
      if (x_dir != 1)
      {
        x_dir = -1;
        y_dir = 0;
      }
      break;
    }
  }
  else if (ch == 'w' && y_dir != 1)
  {
    x_dir = 0;
    y_dir = -1;
  }
  else if (ch == 's' && y_dir != -1)
  {
    x_dir = 0;
    y_dir = 1;
  }
  else if (ch == 'a' && x_dir != 1)
  {
    x_dir = -1;
    y_dir = 0;
  }
  else if (ch == 'd' && x_dir != -1)
  {
    x_dir = 1;
    y_dir = 0;
  }
  else if (ch == 'q')
  {
    endwin();
    exit(0);
  }
  else if (ch == 'r' && game_over)
  {
    snake_length = 3;
    head_x = 40;
    head_y = 12;
    x_dir = 1;
    y_dir = 0;
    score = 0;
    game_over = 0;

    for (int i = 0; i < snake_length; i++)
    {
      snake[i].x = head_x - i;
      snake[i].y = head_y;
    }

    generate_food();
  }
}

int check_self_collision()
{
  return check_collision_asm(snake[0].x, snake[0].y, snake, snake_length);
}

void move_snake()
{
  if (game_over)
    return;

  // Calculate new head position with optimized boundary wrapping
  head_x += x_dir;
  head_y += y_dir;

  head_x = wrap_coordinate_asm(head_x, WIDTH);
  head_y = wrap_coordinate_asm(head_y, HEIGHT);

  // Use optimized snake body movement
  move_snake_body_asm(snake, snake_length);

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

void init_snake()
{
  for (int i = 0; i < snake_length; i++)
  {
    snake[i].x = head_x - i;
    snake[i].y = head_y;
  }
}

int main()
{
  srand(time(NULL));

  initscr();
  cbreak();
  nodelay(stdscr, TRUE);
  noecho();
  curs_set(0);

  init_snake();
  generate_food();

  while (1)
  {
    clear_bitmap();
    process_input();
    move_snake();
    mark_objects();
    draw_bitmap();
    usleep(150000);
  }

  endwin();
  return 0;
}