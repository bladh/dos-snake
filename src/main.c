#include <allegro.h>
#include <stdio.h>

/*
 * TODO: Some bug with the tail.
 * useful links
 * double buffering:
 * https://cboard.cprogramming.com/cplusplus-programming/24721-double-buffering-allegro.html
 */
#define PLAYING_BOARD_OFFSET 6
#define TILES 32
#define TILE_SIZE 6
#define MAXSNAKE TILES*TILES
#define SIDEBOARD PLAYING_BOARD_OFFSET*2+(TILES*TILE_SIZE)
struct position {
    int x;
    int y;
} position;

volatile long counter = 0;
struct position snake[MAXSNAKE];
int score = 0;

void Increment() {
    counter++;
}

void draw_square(int x, int y, int color) {
    rectfill(screen, PLAYING_BOARD_OFFSET + (x*TILE_SIZE), PLAYING_BOARD_OFFSET + (y*TILE_SIZE),
        PLAYING_BOARD_OFFSET + (x*TILE_SIZE)+TILE_SIZE, PLAYING_BOARD_OFFSET + (y*TILE_SIZE)+TILE_SIZE,
        color);
}

void draw_pos(struct position pos, int color) {
    draw_square(pos.x, pos.y, color);
}

int collision_snake(struct position pos) {
    for(int i = score; i > 0; i--) {
        if(pos.x==snake[i].x && pos.y==snake[i].y) {
            return 1;
        }
    }
    return 0;
}

int collision_points(struct position pos1, struct position pos2) {
    return (pos1.x == pos2.x && pos1.y == pos2.y);
}

struct position get_random_position() {
    struct position newpos;
    do {
        newpos.x = rand()%TILES;
        newpos.y = rand()%TILES;
    } while (collision_snake(newpos)==1);
    return newpos;
}


int main(int argc, const char **argv) {
    if (allegro_init() != 0) {
        printf("Could not initialize allegro");
        return 1;
    }
    srand(time(NULL));

    for(int i = 0; i<MAXSNAKE;i++){
        snake[i].x = -1;
        snake[i].y = -1;
    }

    install_keyboard();
    install_timer();
    
    LOCK_VARIABLE(counter);
    LOCK_VARIABLE(Increment);
    install_int_ex(Increment, BPS_TO_TIMER(15));

    // Switch to graphics mode, 320x200.
    if (set_gfx_mode(GFX_AUTODETECT, 320, 200, 0, 0) != 0) {
        set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
        allegro_message("Cannot set graphics mode:\r\n%s\r\n", allegro_error);
        return 1;
    }

    // set up colors
    set_palette(desktop_palette);
    int BLACK = makecol(0,0,0);
    int BACKGROUND = makecol(255,255,255);
    int playing = 1;
    
    // set up snake
    snake[0].x = 3;
    snake[0].y = 3;
    
    struct position candy;
    candy.x = 6;
    candy.y = 3;

    while(playing) {
        while(counter > 0) {        
            if (key[KEY_ESC]) {
                playing = 0;
            }
            
            // move tail
            for(int i = score; i > 0; i--) {
                snake[i] = snake[i-1];
            }

            // autoplay
	    if (candy.x > snake[0].x) {
               snake[0].x++;
	    }
	    else if (candy.x < snake[0].x) {
               snake[0].x--;
	    }
	    else if (candy.y > snake[0].y) {
               snake[0].y++;
	    }
	    else if (candy.y < snake[0].y) {
               snake[0].y--;
	    }
            
            // wraparound
            if (snake[0].x >= TILES) {
                snake[0].x = 0;
            }
            if (snake[0].x < 0) {
                snake[0].x = TILES-1;
            }
            if (snake[0].y >= TILES) {
                snake[0].y = 0;
            }
            if (snake[0].y < 0) {
                snake[0].y = TILES-1;
            }
            
            // eat candy and grow
            if(collision_points(snake[0], candy)) {
                candy = get_random_position();
                score++;
            }
            counter--;
        }

        vsync();
        clear_to_color(screen, BACKGROUND);    
        textout_ex(screen, font, "SNAKE FOR DOS", SIDEBOARD, PLAYING_BOARD_OFFSET, BLACK, -1);

        textout_ex(screen, font, "HEAD POS", SIDEBOARD, 40, BLACK, -1);
	textprintf_ex(screen, font, SIDEBOARD, 60, BLACK, -1, "X: %d Y: %d", snake[0].x, snake[0].y);
        textout_ex(screen, font, "CANDY POS", SIDEBOARD, 80, BLACK, -1);
	textprintf_ex(screen, font, SIDEBOARD, 100, BLACK, -1, "X: %d Y: %d", candy.x, candy.y);

        rect(screen, PLAYING_BOARD_OFFSET-1, PLAYING_BOARD_OFFSET-1, PLAYING_BOARD_OFFSET+TILES*TILE_SIZE, PLAYING_BOARD_OFFSET+TILES*TILE_SIZE, BLACK);
        for(int i = score; i > 0; i--) {
            draw_pos(snake[i], BLACK);
        }
        draw_pos(candy, BLACK);
    }

    return 0;
}
END_OF_MAIN()
