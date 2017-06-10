#include "turing.h"

void initColor()
{
    start_color();
    // map various integers to certain colors
    // to be used for display in PDCurses...
    init_pair(1,COLOR_BLUE,COLOR_BLACK);
    init_pair(2,COLOR_GREEN,COLOR_BLACK);
    init_pair(3,COLOR_CYAN,COLOR_BLACK);
    init_pair(4,COLOR_RED,COLOR_BLACK);
    init_pair(5,COLOR_MAGENTA,COLOR_BLACK);
    init_pair(6,COLOR_YELLOW,COLOR_BLACK);
    init_pair(7,COLOR_WHITE,COLOR_BLACK);
    init_pair(8,COLOR_BLACK,COLOR_BLACK);
}

// initialize pdcurses
void initCurses()
{
    initscr();
    resize_term(HGT,WID);     
    initColor();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr,true);
    mouseinterval(1);
    mousemask(BUTTON1_CLICKED | BUTTON1_PRESSED, NULL);
    PDC_set_title("Turing Machine Explorer");
}


int main(int argc, char* argv[])
{
    // init random number generator
    srand(time(NULL));

    // initialize display mechanism
    initCurses();

    // create main program class instance
    sim_obj simulation;

    // run everything
    simulation.runApp();

    // clear screen
    clear();

    // destroy window
    endwin();
    return 0;
}
