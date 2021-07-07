#include <allegro.h>
#include <stdio.h>

#define PLAYING_BOARD_OFFSET 6
#define TILES 32
#define TILE_SIZE 6
#define MAXSNAKE TILES *TILES
#define SIDEBOARD PLAYING_BOARD_OFFSET * 2 + (TILES * TILE_SIZE)

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

#define DIR_R 0
#define DIR_L 1
#define DIR_U 2
#define DIR_D 3

struct position {
  int x;
  int y;
} position;

volatile long counter = 0;
struct position snake[MAXSNAKE];
int score = 0;

BITMAP *buffer;

void Increment() { counter++; }

void draw_square(int x, int y, int color) {
  rectfill(buffer, PLAYING_BOARD_OFFSET + (x * TILE_SIZE),
           PLAYING_BOARD_OFFSET + (y * TILE_SIZE),
           PLAYING_BOARD_OFFSET + (x * TILE_SIZE) + TILE_SIZE,
           PLAYING_BOARD_OFFSET + (y * TILE_SIZE) + TILE_SIZE, color);
}

void draw_pos(struct position pos, int color) {
  draw_square(pos.x, pos.y, color);
}

int collision_snake(struct position pos) {
  for (int i = score; i > 0; i--) {
    if (pos.x == snake[i].x && pos.y == snake[i].y) {
      return 1;
    }
  }
  return 0;
}

int collision_points(struct position pos1, struct position pos2) {
  return (pos1.x == pos2.x && pos1.y == pos2.y);
}

void move_snake(int direction) {
  if (direction == DIR_U) {
    snake[0].y--;
  } else if (direction == DIR_D) {
    snake[0].y++;
  } else if (direction == DIR_R) {
    snake[0].x++;
  } else if (direction == DIR_L) {
    snake[0].x--;
  }
}

struct position get_random_position() {
  struct position newpos;
  do {
    newpos.x = rand() % TILES;
    newpos.y = rand() % TILES;
  } while (collision_snake(newpos) == 1);
  return newpos;
}

int main(int argc, const char **argv) {
  if (allegro_init() != 0) {
    printf("Could not initialize allegro");
    return 1;
  }
  srand(time(NULL));
  int autoplay = 0;

  for (int i = 0; i < MAXSNAKE; i++) {
    snake[i].x = -1;
    snake[i].y = -1;
  }

  install_keyboard();
  install_timer();

  LOCK_VARIABLE(counter);
  LOCK_VARIABLE(Increment);
  install_int_ex(Increment, BPS_TO_TIMER(16));

  if (set_gfx_mode(GFX_AUTODETECT, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0) != 0) {
    set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
    allegro_message("Cannot set graphics mode:\r\n%s\r\n", allegro_error);
    return 1;
  }

  // set up colors
  set_palette(desktop_palette);
  int BLACK = makecol(0, 0, 0);
  int BACKGROUND = makecol(255, 255, 255);
  int RED = makecol(255, 0, 0);
  int GREY = makecol(125, 125, 125);
  int playing = 1;
  int paused = 0;
  int pausePressed = 0;

  // set up drawing buffer
  buffer = create_bitmap(SCREEN_WIDTH, SCREEN_HEIGHT);

  // set up snake
  snake[0].x = 3;
  snake[0].y = 3;

  struct position candy;
  candy.x = 4;
  candy.y = 8;

  int direction = DIR_R;
  int dead = 0;
  int snakecol = BLACK;

  while (playing) {
    while (counter > 0 && dead == 0 && paused == 0) {
      if (key[KEY_UP] && direction != DIR_D) {
        direction = DIR_U;
      } else if (key[KEY_DOWN] && direction != DIR_U) {
        direction = DIR_D;
      } else if (key[KEY_RIGHT] && direction != DIR_L) {
        direction = DIR_R;
      } else if (key[KEY_LEFT] && direction != DIR_R) {
        direction = DIR_L;
      }
      if (key[KEY_SPACE]) {
        if (pausePressed == 0) {
          paused = 1;
          pausePressed == 1;
          snakecol = GREY;
          goto drawing;
        }
      } else {
        pausePressed = 0;
      }

      // move tail
      for (int i = score; i > 0; i--) {
        snake[i] = snake[i - 1];
      }

      // eat candy and grow
      if (collision_points(snake[0], candy)) {
        candy = get_random_position();
        score++;
        snake[score] = snake[score - 1];
      }

      // autoplay
      if (autoplay) {
        if (candy.x > snake[0].x) {
          snake[0].x++;
        } else if (candy.x < snake[0].x) {
          snake[0].x--;
        } else if (candy.y > snake[0].y) {
          snake[0].y++;
        } else if (candy.y < snake[0].y) {
          snake[0].y--;
        }
      } else {
        move_snake(direction);
      }

      // wraparound
      if (snake[0].x >= TILES) {
        snake[0].x = 0;
      }
      if (snake[0].x < 0) {
        snake[0].x = TILES - 1;
      }
      if (snake[0].y >= TILES) {
        snake[0].y = 0;
      }
      if (snake[0].y < 0) {
        snake[0].y = TILES - 1;
      }

      // check death
      for (int i = score; i > 0; i--) {
        if (snake[i].x == snake[0].x && snake[i].y == snake[0].y) {
          dead = 1;
          snakecol = RED;
        }
      }
      counter--;
    }
    if (key[KEY_SPACE]) {
      if (pausePressed == 0) {
        paused = 0;
        counter = 0;
        snakecol = BLACK;
        pausePressed = 1;
      }
    } else {
      pausePressed = 0;
    }
  drawing:
    if (key[KEY_ESC]) {
      playing = 0;
    }
    clear_to_color(buffer, BACKGROUND);
    textout_ex(buffer, font, "SNAKE FOR DOS", SIDEBOARD, PLAYING_BOARD_OFFSET,
               BLACK, -1);

    textprintf_ex(buffer, font, SIDEBOARD, 60, BLACK, -1, "SCORE: %d", score);

    if (dead == 1)
      textout_ex(buffer, font, "GAME OVER", SIDEBOARD, 30, RED, -1);

    rect(buffer, PLAYING_BOARD_OFFSET - 1, PLAYING_BOARD_OFFSET - 1,
         PLAYING_BOARD_OFFSET + TILES * TILE_SIZE,
         PLAYING_BOARD_OFFSET + TILES * TILE_SIZE, BLACK);
    for (int i = score; i >= 0; i--) {
      draw_pos(snake[i], snakecol);
    }
    draw_pos(candy, BLACK);
    vsync();
    blit(buffer, screen, 0, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  }

  destroy_bitmap(buffer);
  return 0;
}
END_OF_MAIN()
