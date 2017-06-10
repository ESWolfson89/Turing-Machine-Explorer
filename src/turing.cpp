#include "turing.h"

// print chtype to screen
// chtype consists of a bitmap containing a char, a color value, and
// various flags:
// i.e.
// ch equaling 'X'|COLOR_PAIR(4)|A_BOLD, x equaling 0 and y equaling 0
// prints a bright red X at position 0,0 on the terminal window.
void addChar(int x, int y, chtype ch)
{
    // Note: PDCurses functions take y before x in API calls.
    // We execute this function in 2 steps.

    // Step 1: Delete character at position 0,0 (this moves each character at position (x,y = 0) to (x-1,y = 0) for
    // x > 0
    // Example in 4 rows 10 columns terminal window:
    // ..........
    // ..........
    // ..........
    // ..........

    // becomes

    // .........
    // ..........
    // ..........
    // ..........
    // Notice the blank character at the end of the first line

    // Step 2:
    // Now we insert the X at the original position yielding:
    // X.........
    // ..........
    // ..........
    // ..........
    // Reminder: the cursor will be off throughout the entire duration of the program's lifetime.
    mvdelch(y,x);     // step 1
    mvinsch(y,x,ch);  // step 2
}

//
// tape head class implementation
//

tape_head::tape_head()
{
    // Initialize the location of the tape head to the center of the tape.
    // This tape doesn't represent the tape in a "theoretical" one-tape Turing Machine
    // (I believe..., although I might be wrong on this), since the size of the tape is finite
    loc = TAPESIZE / 2;
    // initialize the current state of the turing machine tape head to the first enum state
    curr_state = STATE_QA;
    // Has no effect on program execution, since during the first iteration of the simulation this value will be overwritten
    // to a new value based on a rule cell in the transition table.
    curr_dir = LEFT;
}

// Move the tape head left or right one unit
void tape_head::moveTapeHead()
{
    if (curr_dir == LEFT)
        loc--;
    else
        loc++;

    // One limitation of this simulation is that the length of the tape is finite.
    // By default the machine wraps around from one end to another to prevent an out of bounds
    // access violation.
    if (loc > TAPESIZE - 1)
        loc = 0;
    if (loc < 0)
        loc = TAPESIZE - 1;
}

// Setter for the current state of the tape head
void tape_head::setCurrentState(state s)
{
    curr_state = s;
}

// Setter for the current direction that the tape head
// should move at the next iteration.
void tape_head::setCurrentDirection(direction d)
{
    curr_dir = d;
}

// Setter for the tape head location:
// Manually sets the tape head location.
void tape_head::setTapeHeadLoc(int l)
{
    loc = l;
}

// Getter for the tape head location
int tape_head::getTapeHeadLoc()
{
    return loc;
}

// Getter for the current direction
direction tape_head::getCurrentDirection()
{
    return curr_dir;
}

// Getter for the current state of the tape head
state tape_head::getCurrentState()
{
    return curr_state;
}

//
// tape class implementation
//

tape::tape()
{
}

// Initalizes all tape cells to the zeroth
// enum value for "symbol" (default start values)
// When simulation is initialized, this method is called.
// When simulation is reset, this method is called.
void tape::setupTape()
{
    for (int i = 0; i < TAPESIZE; ++i)
    {
        values[i] = (symbol)0;
    }
}

// Setter for a tape cell at a given position.
void tape::setTapeCell(symbol new_val, int position)
{
    values[position] = new_val;
}

// Getter for a tape cell at a given position
symbol tape::getTapeCell(int position)
{
    return values[position];
}


// primary simulation class
sim_obj::sim_obj()
{
    th_obj = tape_head();
    tape_obj = tape();
}

// reset all simulation statistics, rules, tape cells, etc.. and
// redisplay everything
void sim_obj::reInitializeEverything(bool rnd)
{
    // simulation not running
    halt = false;
    // number of ticks for current simulation
    ticks = 0;
    // set current tape head state to first (default) state
    // (only needed when resetting simulation; This is done in tape head
    // constructor as well)
    th_obj.setCurrentState(STATE_QA);
    // set current tape head direction to left (default)
    // (only needed when resetting simulation; This is done in tape head
    // constructor as well)
    th_obj.setCurrentDirection(LEFT);
    // set machine head to middle of tape
    // (only needed when resetting simulation; This is done in tape head
    // constructor as well)
    th_obj.setTapeHeadLoc(TAPESIZE / 2);
    // initialize TM rules
    setupTransitionTable(rnd);
    // initialize tape
    tape_obj.setupTape();
    // print every component: (ruleset, tape, etc...)
    reDisplay();
}

// primary program loop method
void sim_obj::runApp()
{
    // initialize everything before beginning main program loop
    reInitializeEverything(false);

    // initializing all flags
    // (bstate,id,x,y,z to 0 to avoid debug errors "variables not properly initialized")
    // for some compiler settings
    MEVENT minput = {0,0,0,0,0};

    // last key pressed
    int keyp = 'a';

    // loop until user quits
    do
    {
        // check mouse input for click on transition table
        // (only when simulation is not running)
        if (keyp == KEY_MOUSE)
        {
            checkClick(minput);
        }
        // Run simulation until reject, accept or generic halt state reached.
        // Once one of these states is reached the user must reinitialize everything
        // for a new simulation run. Otherwise, if the user presses space, the simulation
        // is paused and may be continued once space is pressed again (here, in the code)
        if (keyp == ' ' && !halt)
        {
            simulate();
        }
        // reset rules and clear tape
        if (keyp == 'i')
        {
            reInitializeEverything(false);
        }
        // setup random rules
        if (keyp == 'r')
        {
            reInitializeEverything(true);
        }

        // when the simulation is not running, and the user is changing the tape cells
        // or transition table, the key input can be blocking, waiting for the next user input.
        timeout(-1);

    // when the user presses "q" and the simulation is paused or has ended,
    // the program exits
    } while ((keyp = getch()) != 'q');
}

// Run the simulation given the tape cells up until this point and the transition table (ruleset).
void sim_obj::simulate()
{
    // unblock key input for simulation so that is runs continuously (with tick delay)
    timeout(0);

    do
    {
        // See 2 points below
        applyTransition();

        // redisplay only the TM tape and tape head
        reDisplayMachine();

        // 50 milliseconds is the time for each simulation tick
        // one tick consists of:
        // 1) changing the tape head state to the appropriate state based on its state and the tape's
        //    cell beneath it
        // 2) changing the tape's cell based on the initial tape state and the cell beneath it.
        napms(50);
        // needed after napms call according to PDCurses documentation
        refresh();

        // if a halting state has been reached (Reject,Accept,or Halt setting this flag to true)
        // break out of the simulation and force the user to reset. This current run has permanently
        // ended...
        if (halt)
            break;

        // Move the tape head left or right depending on the ruleset and current states
        th_obj.moveTapeHead();

        // reDisplay to update the transition
        reDisplay();

        // This entire loop consists of one simulation tick
        ticks++;

        // Delay for one millisecond (I think this is for display
        // synchronization purposes, but I can't remember exactly why I put it in.
        napms(1);
        refresh();

      // Loop until simulation is paused
    } while (getch() != ' ');
}

// apply one step of the transition table rule-set onto the TM
void sim_obj::applyTransition()
{
    // What is the index of the tape cell that the tape head is currently located at?
    int tape_head_loc = th_obj.getTapeHeadLoc();

    // What is the current symbol of the tape cell above the tape head?
    symbol current_symbol = tape_obj.getTapeCell(tape_head_loc);

    // What is the current state of the tape head?
    state current_state = th_obj.getCurrentState();

    // Set the current state of the tape head based on current_symbol and current_state
    th_obj.setCurrentState(ruleset[(int)current_state][(int)current_symbol].next_state);

    // The tape head is in a non-halting state...
    if (th_obj.getCurrentState() != STATE_QACCEPT && th_obj.getCurrentState() != STATE_QREJECT && th_obj.getCurrentState() != STATE_QHALT)
    {
        // then set the tape head's next direction
        th_obj.setCurrentDirection(ruleset[(int)current_state][(int)current_symbol].move_head);
        // and set the tape cell's next symbol value
        tape_obj.setTapeCell(ruleset[(int)current_state][(int)current_symbol].write_symbol,tape_head_loc);
    }
    else
    {
        // otherwise the current simulation run should come to an end (as signified by this flag)
        halt = true;
    }
}

// initialize the rule-set
// To start all combinations of states and symbols should yield:
// goto state a, print symbol . on tape, move left
void sim_obj::setupTransitionTable(bool rnd)
{
    for (int i = 0; i < NUMSTT; ++i)
    {
        for (int j = 0; j < NUMSYM; ++j)
        {
            // if i == 3 and j == 5, for instance, then
            // state "c" and symbol "1" on the transition table should produce
            // a/./left (as should every other combination to start out)
            ruleset[i][j].curr_state = (state)i;
            ruleset[i][j].curr_symbol = (symbol)j;
            ruleset[i][j].next_state = (rnd == false ? (state)0 : (state)(rand() % (NUMSTT + 3)));
            ruleset[i][j].write_symbol = (rnd == false ? (symbol)0 : (symbol)(rand() % NUMSYM));
            ruleset[i][j].move_head = (rnd == false ? (direction)0 : (direction)(rand() % 2));
        }
    }
}

// Draw everything: the machine, rule-set and stats parts of the window.
void sim_obj::reDisplay()
{
    clear();
    printTape();
    printTapeHead();
    printTransitionTable();
    printStats();
}

void sim_obj::reDisplayMachine()
{
    // Completely redraw machine (tape head and tape)
    // Not necessary to combine with reDisplay as we don't need
    // to print the machine when we just want to update the display
    // of the transition table

    // Clear machine output
    // We need to do this so that overwriting does not produce remnants/artifacts
    for (int i = 0; i < WID; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            addChar(i,j,' ');
        }
    }

    // Once clear, print the state of the new tape and tape head
    printTape();
    printTapeHead();
}

// Check to see if the user clicked on the rule-set table.
void sim_obj::checkTransitionTableClick(int x, int y)
{
     // Innermost array index value for ruleset
     int state_int = 0;
     // Outermost array index value for ruleset
     int symbol_int = 0;

     // The transition table occurs on values 10,12,14,16,18 and 20 along the y axis:
     // Check if the y cursor is within these bounds
     if (y % 2 == 0 && y >= 10 && y <= 20)
     {
         // The symbol index is either 0,1,2,3,4 or 5 corresponding to one of the
         // (BLANK, CROSS, ASTERISK, AMPERSAND, ZERO, ONE) enumeration values
         symbol_int = (int)((y - 10) / 2);

         // The user clicked on a state character in the rule-set table
         if (x % 5 == 2)
         {
             // Increment this rule's next state
             state_int = (int)((x - 2) / 5);
             ruleset[state_int][symbol_int].next_state = getNextRuleState(state_int,symbol_int);
         }

         // The user clicked on a symbol character in the rule-set table
         if (x % 5 == 3)
         {
             // Increment this rule's next symbol
             state_int = (int)((x - 3) / 5);
             ruleset[state_int][symbol_int].write_symbol = getNextRuleSymbol(state_int,symbol_int);
         }

         // The user clicked on a direction character ('l' or 'r') in the rule-set table
         if (x % 5 == 4)
         {
             // Increment this rule's next direction
             state_int = (int)((x - 4) / 5);
             ruleset[state_int][symbol_int].move_head = getNextRuleDirection(state_int,symbol_int);
         }

         // Redraw the rule-set table to reflect the latest change
         printTransitionTable();
     }
}

// Check to see if the user clicked on the TM tape.
void sim_obj::checkTapeCellAreaClick(int x, int y)
{
     // Index of the tape array
     int curr_symbol_int = 0;

     // The y value of the tape on the window.
     if (y == 3)
     {
         // Offset symbol based on positioning of the TM tape head with respect to the window
         curr_symbol_int = x + th_obj.getTapeHeadLoc() - (WID / 2);
         // Make sure the user didn't click off the edge of the tape (otherwise segfault)
         if (curr_symbol_int >= 0 && curr_symbol_int <= TAPESIZE - 1)
         {
             // Increment the enum value of that tape cell manually
             tape_obj.setTapeCell((symbol)(((int)tape_obj.getTapeCell(curr_symbol_int) + 1) % NUMSYM),curr_symbol_int);
             // Redraw rule-set to reflect latest change (We need to call this since the current transition may have been
             // been changed to reflect the latest modification to the tape)
             printTransitionTable();
             // And then redraw tape to reflect latest change
             printTape();
             printTapeHead();
         }
     }
}

// Check to see if the user clicked on the same x-axis of the tape head on the window.
void sim_obj::checkTapeHeadAreaClick(int x, int y)
{
     // Index of the tape array
     int curr_symbol_int = 0;

     // Only the top 2 window tiles make up the area that the tape head can move.
     if (y == 0 || y == 1)
     {
         // Offset symbol based on positioning of the TM tape head with respect to the window
         curr_symbol_int = x + th_obj.getTapeHeadLoc() - (WID / 2);
         // Make sure the user didn't click off the edge of the tape (otherwise segfault)
         if (curr_symbol_int >= 0 && curr_symbol_int <= TAPESIZE - 1)
         {
            // Set the tape head x location along TM to be where the user's relative click position.
            th_obj.setTapeHeadLoc(curr_symbol_int);
            // Redraw everything
            reDisplay();
         }
     }
}

// Check various areas on the window to see if a click event occurred there...
void sim_obj::checkClick(MEVENT minput)
{
     // Update minput to contain mouse information (button clicked,location,etc...).
     // Here, we just initialized it earlier to recognize (in main.cpp) left button clicks.
     nc_getmouse(&minput);

     // Get current mouse position
     int y = minput.y;
     int x = minput.x;

     // Area on the transition table was clicked
     checkTransitionTableClick(x,y);
     // Area on the TM Tape was clicked
     checkTapeCellAreaClick(x,y);
     // Area above the TM tape was clicked
     checkTapeHeadAreaClick(x,y);
}

// Get the next rule direction (which can only be one of 2 values) of rule <state = state_int symbol = symbol_int>
direction sim_obj::getNextRuleDirection(int state_int, int symbol_int)
{
   // The 2 values are 'l' and 'r'
   return (direction)(((int)ruleset[state_int][symbol_int].move_head + 1) % 2);
}

// Get the next rule symbol of rule <state = state_int symbol = symbol_int>
symbol sim_obj::getNextRuleSymbol(int state_int, int symbol_int)
{
   // There are NUMSYM (6) possible values
   return (symbol)(((int)ruleset[state_int][symbol_int].write_symbol + 1) % NUMSYM);
}

// Get the next rule state of rule <state = state_int symbol = symbol_int>
state sim_obj::getNextRuleState(int state_int, int symbol_int)
{
   // NUMSTT corresponds to non-halting states (16) (+ 3 added to include the 3 halting states)
   return (state)(((int)ruleset[state_int][symbol_int].next_state + 1) % (NUMSTT + 3));
}

// Redraw the transition table
void sim_obj::printTransitionTable()
{
    // Current tape cell the tape head is above
    symbol current_symbol = tape_obj.getTapeCell(th_obj.getTapeHeadLoc());
    // Current state of the tape head
    state current_state = th_obj.getCurrentState();
    // Highlight the current rule on the rule-set
    chtype highlight;

    // Location of character to be drawn
    int charx = 0;
    int chary = 0;

    // Draw the symbol legend
    for (int i = 0; i < NUMSYM; ++i)
    {
        chary = 10+(i*2);
        addChar(0,chary,symbol_ch[i]);
    }

    // Draw the state legend
    for (int i = 0; i < NUMSTT; ++i)
    {
        charx = 3+(i*5);
        addChar(charx,8,state_ch[i]);
    }

    // Draw each possible rule
    for (int i = 0; i < NUMSTT; ++i)
    {
        for (int j = 0; j < NUMSYM; ++j)
        {
            // Note, on PDCurses A_BLINK will just highlight, which is the intended effect
            // On 'nix system terminals the highlighted region might blink (haven't tested)
            if (current_state == ruleset[i][j].curr_state &&
                current_symbol == ruleset[i][j].curr_symbol)
                highlight = A_BLINK;
            else
                highlight = 0;

            charx = 3+(i*5);
            chary = 10+(j*2);

            // Add state character (highlighted if current transition is in this rule) corresponding to the
            // next state at this rule.
            addChar(charx-1,chary,state_ch[(int)ruleset[i][j].next_state]|highlight);

            if ((int)ruleset[i][j].next_state < 16)
            {
                // Do the same for symbol
                addChar(charx,chary,symbol_ch[(int)ruleset[i][j].write_symbol]|highlight);
                // Do the same for the direction
                addChar(charx+1,chary,dir_ch[(int)ruleset[i][j].move_head]|highlight);
            }
            else
            {
                // There is no point in adding the 'next' symbol or current direction, since here
                // the TM will have halted and cannot transition further.
                addChar(charx,chary,' ');
                addChar(charx+1,chary,' ');
            }
        }
    }

}

void sim_obj::printStats()
{
    // Print information about how to use program and simulation metrics
    mvprintw(HGT - 2,0,"Num non-halting states: %d", NUMSTT);
    mvprintw(HGT - 1,0,"Tape alphabet =      ");
    mvprintw(HGT - 2,28,"SPACE-pause/run i-reset q-quit");
    mvprintw(HGT - 1,28,"LCLICK-alter rule,cell/move head");
    mvprintw(HGT - 2,62,"Ticks -> %d",ticks);

    // Print at x positions 18, 20, 22, 24, 26 entire tape alphabet
    for (int i = 0; i < NUMSYM; ++i)
    {
        addChar(16+i,HGT - 1,symbol_ch[i]);
    }

    // Print border of info section of window
    for (int i = 0; i < WID; ++i)
    {
         addChar(i,HGT-3,'=');
    }

    // Print label for this section of the window
    attron(COLOR_PAIR(8)|A_DIM|A_BLINK);
    mvprintw(HGT-3,WID/2 - 8,"Simulation Info");
    attroff(COLOR_PAIR(8)|A_DIM|A_BLINK);
}

// output tape head (a '#' symbol and a symbol that denotes the state (enum))
void sim_obj::printTapeHead()
{
    addChar(40, 2,  '|' | COLOR_PAIR(8) | A_BOLD);
    addChar(40, 1,  '#' | COLOR_PAIR(6) | A_BOLD);
    addChar(40, 0, state_ch[(int)th_obj.getCurrentState()]);
}

// print tape to console screen
void sim_obj::printTape()
{
    int tape_head_loc = th_obj.getTapeHeadLoc();

    // Tape head is always visible on the x-axis center (of window).
    // That is, the tape moves with respect to the window leaving the tape head
    // to appear stationary relative to the window.
    int x_min = tape_head_loc - (WID / 2);

    for (int i = 0; i < WID; ++i)
    {
        // print "----" aesthetic borders to signify where the visible tape begins and ends
        // (its really just one character tall)
        // Only do this if the end of the tape is out of range of the window, otherwise print blank chtype
        // values.
        // For instance, for a window of width 15:
        // ---------------
        // .......H.......
        // ---------------
        // is printed if ends beyond window for both ends of the tape
        // or something like (although mathematically skewed probably by about a tile)...
        //      ----------
        //      ..H.......
        //      ----------
        // if left end is near head (and vice versa for right end)
        if (x_min + i >= 0 && x_min + i <= TAPESIZE - 1)
        {
            addChar(i, 2, '-'|COLOR_PAIR(7)|A_BOLD);
            addChar(i, 4, '-'|COLOR_PAIR(7)|A_BOLD);
            addChar(i, 3, symbol_ch[(int)tape_obj.getTapeCell(x_min + i)]);
        }
        else
        {
            addChar(i, 2, ' ');
            addChar(i, 4, ' ');
            addChar(i, 3, ' ');
        }
    }

    // print left center and right coordinates of tape with respect to tape window.
    // Numbers will appear like:
    // ----------------
    // ........H.......
    // (-8)---(0)---(7)
    // See notes about A_BLINK in documentation if you want to port to ncurses (I haven't tested any of this on Linux)
    attron(COLOR_PAIR(8)|A_DIM|A_BLINK);
    mvprintw(4,abs(std::min(0,x_min)),"%d",std::max(x_min,0)-(TAPESIZE/2));
    mvprintw(4,std::min(WID - 4,TAPESIZE - x_min - 5),"%d",std::min(x_min + WID - 1,TAPESIZE - 1)-(TAPESIZE/2));
    mvprintw(4,40,"%d",tape_head_loc-(TAPESIZE/2));
    attroff(COLOR_PAIR(8)|A_DIM|A_BLINK);
}
