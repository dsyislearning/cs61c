#include "state.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

/* Helper function definitions */
static void set_board_at(game_state_t *state, unsigned int row, unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t *state, unsigned int snum);
static char next_square(game_state_t *state, unsigned int snum);
static void update_tail(game_state_t *state, unsigned int snum);
static void update_head(game_state_t *state, unsigned int snum);

/* Task 1 */
game_state_t *create_default_state() {
  char *row_of_walls = "####################\n";
  char *row_of_space = "#                  #\n";
  char *row_of_snake = "# d>D    *         #\n";

  /* Initialize the game state */
  game_state_t *retval = malloc(sizeof(game_state_t));
  if (retval == NULL)
  {
    fprintf(stderr, "Game state malloc error");
    return NULL;
  }
  /* Initialize the board */
  retval->num_rows = 18;
  retval->board = malloc(retval->num_rows * sizeof(char *));
  if (retval->board == NULL)
  {
    fprintf(stderr, "Board malloc error");
    free(retval);
    return NULL;
  }
  long unsigned int num_cols = strlen(row_of_walls);
  for (int i = 0; i < retval->num_rows; i++)
  {
    char *row_ptr = malloc(num_cols * sizeof(char));
    if (row_ptr == NULL)
    {
      fprintf(stderr, "Board row malloc error");
      free(retval->board);
      free(retval);
      return NULL;
    }
    if (i == 0 || i == retval->num_rows - 1)
    {
      strcpy(row_ptr, row_of_walls);
    }
    else if (i == 2)
    {
      strcpy(row_ptr, row_of_snake);
    }
    else
    {
      strcpy(row_ptr, row_of_space);
    }
    retval->board[i] = row_ptr; // set the pointer to the row
  }

  /* Initialize the snakes */
  retval->num_snakes = 1;
  snake_t *snakes_ptr = malloc(retval->num_snakes * sizeof(snake_t));
  if (snakes_ptr == NULL)
  {
    fprintf(stderr, "Snake malloc error");
    for (int i = 0; i < retval->num_rows; i++)
    {
      free(retval->board[i]);
    }
    free(retval->board);
    free(retval);
    return NULL;
  }
  snakes_ptr->tail_row = 2;
  snakes_ptr->tail_col = 2;
  snakes_ptr->head_row = 2;
  snakes_ptr->head_col = 4;
  snakes_ptr->live = true;
  retval->snakes = snakes_ptr; // set the pointer to the snakes

  return retval;
}

/* Task 2 */
void free_state(game_state_t *state) {
  for (int i = 0; i < state->num_rows; i++)
  {
    free(state->board[i]);
  }
  free(state->board);
  free(state->snakes);
  free(state);
  return;
}

/* Task 3 */
void print_board(game_state_t *state, FILE *fp) {
  for (int i = 0; i < state->num_rows; i++)
  {
    fprintf(fp, "%s", state->board[i]);
  }
  return;
}

/*
  Saves the current state into filename. Does not modify the state object.
  (already implemented for you).
*/
void save_board(game_state_t *state, char *filename) {
  FILE *f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_state_t *state, unsigned int row, unsigned int col) { return state->board[row][col]; }

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_state_t *state, unsigned int row, unsigned int col, char ch) {
  state->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
  return strchr("wasd", c) != NULL;
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
  return strchr("WASDx", c) != NULL;
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
  return strchr("wasd^<v>WASDx", c) != NULL;
}

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {
  char retval = '?';
  switch (c)
  {
  case '^':
    retval = 'w';
    break;
  case '<':
    retval = 'a';
    break;
  case 'v':
    retval = 's';
    break;
  case '>':
    retval = 'd';
    break;
  }
  return retval;
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {
  char retval = '?';
  switch (c)
  {
  case 'W':
    retval = '^';
    break;
  case 'A':
    retval = '<';
    break;
  case 'S':
    retval = 'v';
    break;
  case 'D':
    retval = '>';
    break;
  }
  return retval;
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
  if (c == 'v' || c == 's' || c == 'S')
  {
    return cur_row + 1;
  }
  else if (c == '^' || c == 'w' || c == 'W')
  {
    return cur_row - 1;
  }
  return cur_row;
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  if (c == '>' || c == 'd' || c == 'D')
  {
    return cur_col + 1;
  }
  else if (c == '<' || c == 'a' || c == 'A')
  {
    return cur_col - 1;
  }
  return cur_col;
}

/*
  Task 4.2

  Helper function for update_state. Return the character in the cell the snake is moving into.

  This function should not modify anything.
*/
static char next_square(game_state_t *state, unsigned int snum) {
  unsigned int cur_row = state->snakes[snum].head_row;
  unsigned int cur_col = state->snakes[snum].head_col;
  char cur_head = state->board[cur_row][cur_col];
  unsigned int next_row = get_next_row(cur_row, cur_head);
  unsigned int next_col = get_next_col(cur_col, cur_head);
  return state->board[next_row][next_col];
}

/*
  Task 4.3

  Helper function for update_state. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the head.
*/
static void update_head(game_state_t *state, unsigned int snum) {
  unsigned int cur_row = state->snakes[snum].head_row;
  unsigned int cur_col = state->snakes[snum].head_col;
  char cur_head = state->board[cur_row][cur_col];
  unsigned int next_row = get_next_row(cur_row, cur_head);
  unsigned int next_col = get_next_col(cur_col, cur_head);
  /* update on the board */
  set_board_at(state, cur_row, cur_col, head_to_body(cur_head));
  set_board_at(state, next_row, next_col, cur_head);
  /* update in the snake struct */
  state->snakes[snum].head_row = next_row;
  state->snakes[snum].head_col = next_col;
  return;
}

/*
  Task 4.4

  Helper function for update_state. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_state_t *state, unsigned int snum) {
  unsigned int cur_row = state->snakes[snum].tail_row;
  unsigned int cur_col = state->snakes[snum].tail_col;
  char cur_tail = state->board[cur_row][cur_col];
  unsigned int next_row = get_next_row(cur_row, cur_tail);
  unsigned int next_col = get_next_col(cur_col, cur_tail);
  char next_square = state->board[next_row][next_col];
  /* update on the board */
  set_board_at(state, cur_row, cur_col, ' ');
  set_board_at(state, next_row, next_col, body_to_tail(next_square));
  /* update in the snake struct */
  state->snakes[snum].tail_row = next_row;
  state->snakes[snum].tail_col = next_col;
  return;
}

/* Task 4.5 */
void update_state(game_state_t *state, int (*add_food)(game_state_t *state)) {
  for (unsigned int i = 0; i < state->num_snakes; i++)
  {
    if (state->snakes[i].live)
    {
      char next = next_square(state, i);
      if (next == ' ')
      {
        update_head(state, i);
        update_tail(state, i);
      }
      else if (next == '*')
      {
        update_head(state, i);
        add_food(state);
      }
      else if (is_snake(next) || next == '#')
      {
        unsigned int cur_row = state->snakes[i].head_row;
        unsigned int cur_col = state->snakes[i].head_col;
        set_board_at(state, cur_row, cur_col, 'x');
        state->snakes[i].live = false;
      }
    }
  }
  return;
}

/* Task 5.1 */
char *read_line(FILE *fp) {
  char buffer[102400];
  char *ptr = fgets(buffer, sizeof(buffer), fp);
  if (ptr == NULL)
  {
    return NULL;
  }
  long unsigned int strlength = strlen(buffer);
  char *retval = malloc((strlength + 1) * sizeof(char));
  if (retval == NULL)
  {
    fprintf(stderr, "Malloc error");
    return NULL;
  }
  strcpy(retval, buffer);
  return retval;
}

/* Task 5.2 */
game_state_t *load_board(FILE *fp) {
  /* malloc for game_state_t */
  game_state_t *retval = malloc(sizeof(game_state_t));
  if (retval == NULL)
  {
    fprintf(stderr, "Game state malloc error");
    return NULL;
  }
  /* initialize snakes in task 6 */
  retval->num_snakes = 0;
  retval->snakes = NULL;

  /* read in boards */
  char *buffer_rows[102400];
  char *line;
  unsigned int i = 0;
  while ((line = read_line(fp)) != NULL)
  {
    buffer_rows[i] = line;
    i++;
  }
  retval->num_rows = i;
  /* malloc for board */
  retval->board = malloc(retval->num_rows * sizeof(char *));
  if (retval->board == NULL)
  {
    fprintf(stderr, "Board malloc error");
    free(retval);
    return NULL;
  }
  /* copy the board */
  for (unsigned int j = 0; j < retval->num_rows; j++)
  {
    long unsigned int strlength = strlen(buffer_rows[j]);
    char *row_ptr = malloc((strlength + 1) * sizeof(char));
    if (row_ptr == NULL)
    {
      fprintf(stderr, "Board row malloc error");
      for (unsigned int k = 0; k < j; k++)
      {
        free(retval->board[k]);
      }
      free(retval->board);
      free(retval);
      return NULL;
    }
    strcpy(row_ptr, buffer_rows[j]);
    retval->board[j] = row_ptr;
  }

  return retval;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_state_t *state, unsigned int snum) {
  /* tail as starter */
  unsigned int cur_row = state->snakes[snum].tail_row;
  unsigned int cur_col = state->snakes[snum].tail_col;
  char cur_char = state->board[cur_row][cur_col];
  /* trace through the snake body until the snake head */
  while (!is_head(cur_char)) {
    cur_row = get_next_row(cur_row, cur_char);
    cur_col = get_next_col(cur_col, cur_char);
    cur_char = state->board[cur_row][cur_col];
  }
  state->snakes[snum].head_row = cur_row;
  state->snakes[snum].head_col = cur_col;
  return;
}

/* Task 6.2 */
game_state_t *initialize_snakes(game_state_t *state) {
  unsigned int snakenum = 0;
  snake_t buffer_snakes[102400];
  for (unsigned int i = 0; i < state->num_rows; i++)
  {
    for (unsigned int j = 0; j < strlen(state->board[i]); j++)
    {
      if (is_tail(get_board_at(state, i, j)))
      {
        buffer_snakes[snakenum].tail_row = i;
        buffer_snakes[snakenum].tail_col = j;
        buffer_snakes[snakenum].live = true;
        snakenum++;
      }
    }
  }
  state->num_snakes = snakenum;
  state->snakes = malloc(state->num_snakes * sizeof(snake_t));
  if (state->snakes == NULL)
  {
    fprintf(stderr, "Snake malloc error");
    return NULL;
  }
  for (unsigned int i = 0; i < state->num_snakes; i++)
  {
    state->snakes[i] = buffer_snakes[i];
    find_head(state, i);
  }
  return state;
}
