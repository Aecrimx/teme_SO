//https://www.viget.com/articles/game-programming-in-c-with-the-ncurses-library
#define _POSIX_C_SOURCE 199309L


#include <stdio.h>
#include <ncurses.h>
#include <time.h>
#include <unistd.h>


typedef struct {
    double x, y;
    double vx, vy;
    double radius;
} Ball;

typedef struct {
    int x, y;
    int width;
    int alive;
} Brick;

typedef struct {
    int x, y;
    int width;
} Paddle;


void render_game(void) {
    erase();

    //draw_bricks();
    draw_paddle();
    //draw_ball();
    //draw_hud();

    wnoutrefresh(stdscr);
    doupdate();
}


int main(int argc, char *argv[]) {
    
    int x = 0, y = 0;
    int max_x = 0, max_y = 0;

    int next_x = 0;
    int direction = 1;

    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    curs_set(FALSE);

        
    const double dt = 1.0 / 120.0; // 120 de update-uri pe secunda pt fizica
    const double render_dt = 1.0 / 60.0;

    double accumulator = 0.0;
    double render_accumulator = 0.0;

    double previous = get_time_seconds();
    int running = 1;

    while (running) {
        double now = get_time_seconds();
        double frame_time = now - previous;
        previous = now;

        if(frame_time > 0.25)
            frame_time = 0.25;

        accumulator += frame_time;
        render_accumulator += frame_time;

        handle_input();
    }

    return 0;
}