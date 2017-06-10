#include "curses.h"
#include <stdlib.h>
#include <time.h>
#include <iostream>

// window width and height
static const int HGT = 24;
static const int WID = 80;

// fixed TM tape size
#define TAPESIZE 1024

// There are 6*16 possible rules
#define NUMSYM 6
#define NUMSTT 16 // num non halting states

// Direction the TM tape head will go
enum direction
{
	LEFT,RIGHT
};

// A TM tape symbol
enum symbol
{
	BLANK, CROSS, ASTERISK, AMPERSAND, ZERO, ONE
};

// A TM tape state (3 are halting states)
enum state
{
	STATE_QA, STATE_QB, STATE_QC, STATE_QD,
	STATE_QE, STATE_QF, STATE_QG, STATE_QH,
	STATE_QI, STATE_QJ, STATE_QK, STATE_QL,
	STATE_QM, STATE_QN, STATE_QO, STATE_QP,
	STATE_QHALT,
	STATE_QACCEPT, STATE_QREJECT
};

// Display characters for a state
static const chtype state_ch[NUMSTT + 3] =
{
	'a'|COLOR_PAIR(7)|A_DIM,
	'b'|COLOR_PAIR(7)|A_DIM,
	'c'|COLOR_PAIR(7)|A_DIM,
	'd'|COLOR_PAIR(7)|A_DIM,
	'e'|COLOR_PAIR(7)|A_DIM,
	'f'|COLOR_PAIR(7)|A_DIM,
	'g'|COLOR_PAIR(7)|A_DIM,
	'h'|COLOR_PAIR(7)|A_DIM,
	'i'|COLOR_PAIR(7)|A_DIM,
	'j'|COLOR_PAIR(7)|A_DIM,
	'k'|COLOR_PAIR(7)|A_DIM,
	'l'|COLOR_PAIR(7)|A_DIM,
	'm'|COLOR_PAIR(7)|A_DIM,
	'n'|COLOR_PAIR(7)|A_DIM,
	'o'|COLOR_PAIR(7)|A_DIM,
	'p'|COLOR_PAIR(7)|A_DIM,
	'H'|COLOR_PAIR(5)|A_BOLD,
	'A'|COLOR_PAIR(2)|A_BOLD,
	'R'|COLOR_PAIR(4)|A_BOLD
};

// Display characters for a symbol
static const chtype symbol_ch[NUMSYM] =
{
	'.'|COLOR_PAIR(7)|A_BOLD,
	'X'|COLOR_PAIR(7)|A_BOLD,
	'$'|COLOR_PAIR(7)|A_BOLD,
	'&'|COLOR_PAIR(7)|A_BOLD,
	'0'|COLOR_PAIR(7)|A_BOLD,
	'1'|COLOR_PAIR(7)|A_BOLD
};

// Display characters for a direction
// 'l' for left, 'r' for right
static const chtype dir_ch[2] =
{
	'l'|COLOR_PAIR(8)|A_BOLD,
	'r'|COLOR_PAIR(8)|A_BOLD
};

// Adds a character on the Curses window
void addChar(int, int, chtype);

// Represents a transition for a rule in the modifiable rule-set
// of a TM
struct transition
{
    state next_state;
    symbol write_symbol;
    direction move_head;
    state curr_state;
    symbol curr_symbol;
};

// Class declarations for the TM below...

class tape_head
{
    public:
        tape_head();
        void moveTapeHead();
        void setCurrentDirection(direction);
        void setCurrentState(state);
        void setTapeHeadLoc(int);
        int getTapeHeadLoc();
        direction getCurrentDirection();
        state getCurrentState();
    private:
        int loc;
        direction curr_dir;
        state curr_state;
};

class tape
{
    public:
        tape();
        void setupTape();
        void setTapeCell(symbol,int);
        symbol getTapeCell(int);
    private:
        // Fixed TM tape size
        symbol values[TAPESIZE];
};

// Main program class below

class sim_obj
{
    public:
        sim_obj();
        void runApp();
        void reDisplay();
        void reDisplayMachine();
        void printTapeHead();
        void printTape();
        void printTransitionTable();
        void reInitializeEverything(bool);
        void printStats();
        void simulate();
        void applyTransition();
        void setupTransitionTable(bool);
        void checkClick(MEVENT);
        void checkTransitionTableClick(int,int);
        void checkTapeCellAreaClick(int,int);
        void checkTapeHeadAreaClick(int,int);
        state getNextRuleState(int,int);
        symbol getNextRuleSymbol(int,int);
        direction getNextRuleDirection(int,int);
    private:
        // This class contains a tape head object and tape object
        tape_head th_obj;
        tape tape_obj;
        // Instance of an 2d array of rules representing A TM
        transition ruleset[NUMSTT][NUMSYM];
        bool halt;
        int ticks;
        int num_symbols;
        int num_states;
};
