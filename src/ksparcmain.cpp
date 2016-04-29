/**
 * ksparc (main.cpp)
 * -----
 * Disassembler/debugger for the SPARC emulator engine (kSPARC)
 * Everything to know is shown in the ncurses graphical interface.
 *
 * Author: krab
 * Version: 0.1
 *
 */
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <vector>
#include <iterator>
#include <bitset>
#include "simplememory.h"
#include "simplealu.h"
#include "sparcengine.h"
#include "disassembler.h"

#include <ncurses.h>

// Predefined sizes for some zones
#define CMDS_HEIGHT 3
#define REG_HEIGHT  10

// Predefined colorpairs : selected/not selected
#define COL_SHALLOWSEL    2
#define COL_DEFAULT       3
#define COL_SELECTED      4
#define COL_NOTSELECTED   5
#define COL_CMDHL         6

// List of the windows
std::vector<WINDOW*> windows;

/*
 * refreshAll -- refresh every windows of the term
 */
void refreshAll() {
  for (auto it = windows.begin(); it < windows.end(); it++) {
    wrefresh(*it);
  }
}

/*
 * createWindow -- create a new window
 * height, width : size of the window
 * sy, sx : position of the window
 *
 * returns a window handler
 */
WINDOW* createWindow(int height, int width, int sy, int sx) {
  WINDOW* win;
  win = newwin(height, width, sy, sx);
  wrefresh(win);
  windows.push_back(win);
  return win;
}

/*
 * printMiddle -- print centered text
 * win : window on which we print
 * starty, startx : beginning of the area
 * width : size of the area
 * str : string to print
 * attr : attribute of the string
 *
 * printMiddle will basically center 'str' in a 1xwidth rectangle with (startx, starty) as 
 * upper left-hand corner
 *
 * Rq: if win == null, just uses stdscr
 */
void printMiddle(WINDOW *win, int starty, int startx, int width, std::string str, int attr = A_NORMAL) {
  if(win == NULL)
    win = stdscr;

  wattron(win, attr);
  mvwprintw(win, starty, startx+(width-str.size())/2, str.c_str());
  wattroff(win, attr);
}

/**
 * escape -- determine if a character is an escape one (and thus should be printed as a .)
 * ch : character to consider
 */
bool escape(char ch) {
  return ch <= 0x1F || ch == 0x7F;
}

/**
 * printCommands -- print a summary of the commands
 * win : window on which to print
 * starty, startx : position
 */
void printCommands(WINDOW* win, int starty, int startx) {
  wattron(win, COLOR_PAIR(COL_CMDHL));
  mvwprintw(win, starty, startx, " n ");
  wattroff(win, COLOR_PAIR(COL_CMDHL));
  wprintw(win, "  Next instruction/data selection");

  wattron(win, COLOR_PAIR(COL_CMDHL));
  mvwprintw(win, starty, startx+50, " + ");
  wattroff(win, COLOR_PAIR(COL_CMDHL));
  wprintw(win, "  Extend selection size");

  wattron(win, COLOR_PAIR(COL_CMDHL));
  mvwprintw(win, starty, startx+100, "<F1>");
  wattroff(win, COLOR_PAIR(COL_CMDHL));
  wprintw(win, " Switch mode");

  wattron(win, COLOR_PAIR(COL_CMDHL));
  mvwprintw(win, starty, startx+150, " q ");
  wattroff(win, COLOR_PAIR(COL_CMDHL));
  wprintw(win, "  Quit");

  wattron(win, COLOR_PAIR(COL_CMDHL));
  mvwprintw(win, starty+1, startx, " p ");
  wattroff(win, COLOR_PAIR(COL_CMDHL));
  wprintw(win, "  Previous instruction/data selection");

  wattron(win, COLOR_PAIR(COL_CMDHL));
  mvwprintw(win, starty+1, startx+50, " - ");
  wattroff(win, COLOR_PAIR(COL_CMDHL));
  wprintw(win, "  Shrink selection size");
}

/**
 * printMemory -- print a piece of memory
 * win : window on which we print the memory
 * mem : the memory to be printed
 * from : address from where we start to print data
 * rows, cols : how many rows and how many cols do we print
 * starty, startx : base coordinates for printing
 *
 * printMemory will present data under two form : hex and characters
 * on each row, there is the base address (so that the first column is the value
 * at this address) and the first column present offset address)
 */
void printMemory(WINDOW* win, AbstractMemory* mem, uint32_t from, uint32_t rows, uint32_t cols, int starty, int startx) {
  // Declarations
  uint32_t addr;

  // Print first line : columns offset address
  wmove(win, starty, startx+12);
  wattron(win, A_BOLD);
  for (uint32_t k = 0; k < cols; k++) {
    wprintw(win, "%02x ", k);
  }
  wattroff(win, A_BOLD);

  // Print the memory
  for (uint32_t i = 0; i < rows; i++) {
    // Newline
    wmove(win, starty+1+i, startx);

    // Print the row's address (0xXXXXXXXX)
    wattron(win, A_BOLD);
    wprintw(win, "0x%08x  ", (from+i)*cols);
    wattroff(win, A_BOLD);
    
    // Print *cols*xvalues from the mem
    for (uint32_t j = 0; j < cols; j++) {
      addr = (from+i)*cols+j; // effective address
      if (addr < mem->getSize()) {
        Logger::log() << std::hex << std::setfill('0') << std::setw(8) << addr << ": " << mem->readByte(addr) << "\n";
        wprintw(win, "%02x ", mem->readByte(addr)); // value is just in hex (XX)
      } else {
        wprintw(win, "## "); // if there is no value (memory size exceeded)
      }
    }

    wprintw(win, "   ");

    // Print values with characters
    for (uint32_t j = 0; j < cols; j++) {
      addr = (from+i)*cols+j;
      if (addr < mem->getSize()) {
        uint8_t c = mem->readByte(addr);
        wprintw(win, "%c", (escape(c) ? '.' : c)); // if value = 0, by convention, we put '.'
      } else {
        wprintw(win, "#");
      }
    }
  }
}

/**
 * printInstructions -- print instructions relative to a piece of memory
 * win : window on which to print instructions
 * mem : memory from which to take instructions
 * from : base address
 * cols, rows : dimensions of the "widget"
 * starty, startx : position
 * width : width of the "widget"
 */
void printInstructions(WINDOW* win, AbstractMemory* mem, uint32_t from, uint32_t rows, uint32_t cols, int starty, int startx, int width) {
  uint32_t addr, ninst = cols/4;
  int instwidth = width/(int)ninst;

  for (uint32_t i = 0; i < rows; i++) {
    for (uint32_t j = 0; j < ninst; j ++) {
      addr = (from+i)*cols+j*4;
      mvwprintw(win, starty+1+i, startx+j*instwidth, "%s", disassemble(mem->readInstruction(addr), addr).c_str());

      if (j != ninst - 1)
        mvwprintw(win, starty+1+i, startx+(j+1)*instwidth-2, ";");
    }
  }
}

/**
 * printRegisters -- print window registers
 * win : on what window to print them
 * regs : registers to print
 * starty, startx : where to print them
 */
void printRegisters(WINDOW* win, WindowRegisters* regs, int starty, int startx) {
  uint32_t i;

  wattron(win, A_BOLD);
  for (i = 0; i < 8; i++)
    mvwprintw(win, starty+i+1, startx, "%d", i);
  wattroff(win, A_BOLD);
  
  // Print globals
  wattron(win, A_BOLD | A_UNDERLINE);
  mvwprintw(win, starty, startx+2+2, "GLOBALS");
  wattroff(win, A_BOLD | A_UNDERLINE);
  for (i = 0; i < NREGGLOB; i++)
    mvwprintw(win, starty+i+1, startx+2, "0x%08x", regs->read(i));

  // Print locals
  wattron(win, A_BOLD | A_UNDERLINE);
  mvwprintw(win, starty, startx+13+2, "LOCALS");
  wattroff(win, A_BOLD | A_UNDERLINE);
  for (i = 0; i < NREGLOC; i++)
    mvwprintw(win, starty+i+1, startx+13, "0x%08x", regs->read(REG_LOC(i)));

  // Print ins
  wattron(win, A_BOLD | A_UNDERLINE);
  mvwprintw(win, starty, startx+24+2, "INPUTS");
  wattroff(win, A_BOLD | A_UNDERLINE);
  for (i = 0; i < NREGIO; i++)
    mvwprintw(win, starty+i+1, startx+24, "0x%08x", regs->read(REG_IN(i)));

  // Print outs
  wattron(win, A_BOLD | A_UNDERLINE);
  mvwprintw(win, starty, startx+35+2, "OUTPUTS");
  wattroff(win, A_BOLD | A_UNDERLINE);
  for (i = 0; i < NREGIO; i++)
    mvwprintw(win, starty+i+1, startx+35, "0x%08x", regs->read(REG_OUT(i)));
}

/**
 * printPSR -- print the processor state register
 * win : window on which to print
 * psr : the register
 * starty, startx : position
 */
void printPSR(WINDOW* win, SpecialRegister* psr, int starty, int startx) {
  mvwprintw(win, starty  , startx, "PSR:");
  mvwprintw(win, starty  , startx+5, "impl:0x%x vers:0x%x EC:%d EF:%d ET:%x",
      psr->getField(PSR_IMPL), psr->getField(PSR_VERS), psr->getField(PSR_EC), psr->getField(PSR_EF), psr->getField(PSR_ET));
  mvwprintw(win, starty+1, startx+5, "N=%d Z=%d V=%d C=%d  S=%d  CWP=0x%02x",
      psr->getField(PSR_ICC_N), psr->getField(PSR_ICC_Z), psr->getField(PSR_ICC_V), psr->getField(PSR_ICC_C), psr->getField(PSR_S), psr->getField(PSR_CWP));
}

/**
 * printFSR -- print the fpu state register
 * win : window on which to print
 * fsr : the register
 * starty, startx : position
 */
char fcc[] = { '=', '<', '>', 'U' };
void printFSR(WINDOW* win, SpecialRegister* fsr, int starty, int startx) {
  mvwprintw(win, starty  , startx, "FSR:");
  mvwprintw(win, starty  , startx+5, "res:%d vers:0x%x RD:%d NS:%d ftt:%d FCC:%c",
      fsr->getField(FPU_IMPL), fsr->getField(FPU_VERS), fsr->getField(FPU_ROUND), fsr->getField(FPU_NS), fsr->getField(FPU_FTT), fcc[fsr->getField(FPU_FCC)]);
  
  std::ostringstream masks;
  masks << "TEM=" << std::bitset<5>(fsr->getField(FPU_TEM)) << " "
        << "aexc=" << std::bitset<5>(fsr->getField(FPU_AEXC)) << " "
        << "cexc=" << std::bitset<5>(fsr->getField(FPU_CEXC));
  mvwprintw(win, starty+1, startx+5, masks.str().c_str());
}

/**
 * printWIM -- print the window invalid mask register
 * win : window on which to print
 * wim : the register
 * starty, startx : position
 */
void printWIM(WINDOW* win, SpecialRegister* wim, int starty, int startx) {
  uint32_t wimval = wim->read();
  uint32_t bit;

  mvwprintw(win, starty, startx, "WIM: ");
  for (uint32_t i = 0; i < 32; i++) {
    bit = (wimval >> (31-i)) & 0x00000001;
    if (bit == 1)
      wattron(win, COLOR_PAIR(COL_NOTSELECTED));
    else
      wattron(win, COLOR_PAIR(COL_SELECTED));
    wprintw(win, "%d", bit);
    if (bit == 1)
      wattroff(win, COLOR_PAIR(COL_NOTSELECTED));
    else
      wattroff(win, COLOR_PAIR(COL_SELECTED));
  }
}

/**
 * highlightInstruction -- highlight an area corresponding to an instruction
 * pwin, dwin : windows on which we highlight the area
 * addr : address of the instruction (it should be 4-byte aligned)
 * rows, cols : dimensions of the memory "widget"
 * from : base address of the memory "widget"
 * startx, starty : coordinates of the memory "widget"
 * width : width of the "widget"
 * color : color of the selection
 */
int _lsx = -1, _lsx2 = 0, _lsx3 = 0, _lsy = 0;
void highlightInstruction(WINDOW* pwin, WINDOW* dwin, uint32_t addr, uint32_t rows, uint32_t cols, uint32_t from, int starty, int startx, int width, int color) {
  // How many instruction can we put in one line ?
  uint32_t numinstr = cols/4;
  
  // As this function is called uppon update, we first clear the last highlighted zone
  // Such zone is stored into "private" global variables _ls*
  if (_lsx != -1) { // do not update anything if it is the first time
    mvwchgat(pwin, _lsy, _lsx, 11, A_NORMAL, COL_DEFAULT, NULL); 
    mvwchgat(pwin, _lsy, _lsx2, 4, A_NORMAL, COL_DEFAULT, NULL);
    mvwchgat(dwin, _lsy, _lsx3, width/numinstr-startx-2, A_NORMAL, COL_DEFAULT, NULL);
  }

  // If the address of the instruction is out of range, we don't highlight anything !
  if (addr < from*cols || addr >= (from+rows)*cols)
    return;

  // Get the number of the line corresponding to the setup
  uint32_t relpos = addr/cols - from;
  
  // Calculate offsets
  _lsx = startx+12+(addr % (numinstr*4) == 0 ? 0 : 12); // each line contains *2* instructions
  _lsy = starty+1+relpos;
  _lsx2 = startx+39+(addr % (numinstr*4));
  _lsx3 = startx+1+(width/numinstr)*((addr%(numinstr*4))/4);

  // Update the program window
  mvwchgat(pwin, _lsy, _lsx, 11, A_NORMAL, color, NULL);
  mvwchgat(pwin, _lsy, _lsx2, 4, A_NORMAL, color, NULL);

  // Update the disasembler window
  mvwchgat(dwin, _lsy, _lsx3, width/numinstr-startx-2, A_NORMAL, color, NULL);
}

/**
 * highlightSelection -- highlight a selection in the memory
 * win : windows on which we highlight the area
 * addr : address of the data to be highlighted (it should be *size*-aligned)
 * size : size of the data to be highlighted
 * rows, cols : dimensions of the memory "widget"
 * from : base address of the memory "widget"
 * startx, starty : coordinates of the memory "widget"
 */
int _dlsx = -1, _dlsx2 = 0, _dlsy = 0, _dlsize = 0;
void highlightSelection(WINDOW* win, uint32_t addr, uint32_t size, uint32_t rows, uint32_t cols, uint32_t from, int starty, int startx) {
  // As this function is called uppon update, we first clear the last highlighted zone
  // Such zone is stored into "private" global variables _ls*
  if (_lsx != -1) { // do not update anything if it is the first time
    mvwchgat(win, _dlsy, _dlsx, 3*_dlsize-1, A_NORMAL, 3, NULL); 
    mvwchgat(win, _dlsy, _dlsx2, _dlsize, A_NORMAL, 3, NULL);
  }

  // If the address of the data is out of range, we don't highlight anything !
  if (addr < from*cols || addr >= (from+rows)*cols)
    return;

  // Get the number of the line corresponding to the setup
  uint32_t relpos = addr/cols - from;
  
  // Calculate offsets
  _dlsx = startx+12+(addr%cols)*3;
  _dlsy = starty+1+relpos;
  _dlsx2 = startx+39+(addr%cols);
  _dlsize = size;

  // Update the memory window
  mvwchgat(win, _dlsy, _dlsx, 3*_dlsize-1, A_NORMAL, 2, NULL);
  mvwchgat(win, _dlsy, _dlsx2, _dlsize, A_NORMAL, 2, NULL);
}

/**
 * loadFile -- load a file into memory
 * mem : the memory
 * filename : name of the file
 */
void loadFile(AbstractMemory* mem, std::string filename, uint32_t baseaddr = 0) {
  std::ifstream fs;
  char ch;
  uint32_t inst;
  uint32_t addr = baseaddr;

  try {
    fs.open(filename, std::ios::in | std::ios::binary);
  } catch (std::ifstream::failure e) {
    Logger::log() << "Cannot open file " << filename << ": " << e.what() << "\n";
  }

  fs.seekg(0, std::ios::beg);
  while (!fs.eof()) {
    fs.read(&ch, sizeof(char));
    Logger::log() << "Writing char " << std::hex << (int32_t)(ch) << " into memory\n";
    mem->writeByte(addr, ch);
    Logger::log() << "=> " << std::hex << std::setfill('0') << std::setw(8) << addr << " = " << (int32_t)(mem->readByte(addr)) << "\n";
    addr++;
  }
  fs.close();
}

/**
 * main function
 */
int main(int argc, char* argv[]) {
  Logger::init("output.log");
  Logger::log("Logger is set up !");

  /// Parse inputs
  if (argc < 2) {
    std::cerr << "No file specified !" << std::endl;
    return -1;
  }

  /// Declarations
  // GUI related : sizes of the window, of some areas, etc.
  int width, height, mainwidth, mainheight, mainy;
  uint32_t memrows, memcols, dmemrows; // Number of rows/cols of the memory "widget"
  bool executionmode = false;

  WINDOW *regw, *prgrmw, *disasmw, *memw, *cmdsw;
  
  // Variables for dynamism
  // Each variable have its old self, so we can detect changes and restore old areas
  int      currentwindow = 0, lastwindow = 1;
  uint32_t prgrmfrom = 0,     lastprgrmfrom = 1;
  uint32_t memfrom = 0,       lastmemfrom = 1;
  uint32_t instr = 0,         lastinstr = 1;
  uint32_t selection = 0,     lastselection = 1;
  uint32_t selsize = 1,       lastselsize = 0;
  bool next = true;
  
  // User input character
  int ch = 0;

  /// Sparc Engine stuff
  SpecialRegister psr, wim, tbr, y, pc, npc, fsr;
  WindowRegisters* registers = new WindowRegisters(4, &psr, &wim);
  SimpleALU* alu = new SimpleALU(&psr, &y);
  SimpleMemory* memory = new SimpleMemory(32768); // 32 ko
  loadFile(memory, std::string(argv[1]));

  SparcEngine* engine = new SparcEngine(memory, alu, registers, &psr, &wim, &tbr, &y, &pc, &npc, &fsr);

  /// Initialize GUI
  initscr();
  start_color(); // use color
  raw(); // do not wait for EOF/EOL to retrieve user input
  noecho(); // do not show what is typed
  keypad(stdscr, TRUE); // use arrow keys, functions, etc.
  curs_set(0); // no cursor

  // Some colors
  init_color(COLOR_WHITE, 700, 700, 700);
  init_color(COLOR_BLACK, 0, 0, 0);
  init_pair(2, COLOR_BLACK, COLOR_WHITE);
  init_pair(3, COLOR_WHITE, COLOR_BLACK);
  init_pair(4, COLOR_BLACK, COLOR_GREEN);
  init_pair(5, COLOR_BLACK, COLOR_RED);
  init_pair(6, COLOR_BLACK, COLOR_CYAN);

  // Set background
  bkgd(COLOR_PAIR(3));

  // Calculate dimensions
  // main* are for building the main windows (the three in the middle)
  getmaxyx(stdscr, height, width);
  mainheight = height - REG_HEIGHT - CMDS_HEIGHT;
  mainwidth = width/3;
  mainy = REG_HEIGHT;

  // Create UI
  // regw : registers
  // cmdsw : commands summary
  // prgrmw : memory for viewing instructions
  // disasmw : disassembled code
  // memw : memory view
  regw = createWindow(REG_HEIGHT, width, 0, 0);
  cmdsw = createWindow(CMDS_HEIGHT, width, height - CMDS_HEIGHT, 0);
  prgrmw = createWindow(mainheight, mainwidth, mainy, 0);
  disasmw = createWindow(mainheight, mainwidth, mainy, mainwidth);
  memw = createWindow(mainheight, width - 2*mainwidth, mainy, 2*mainwidth);

  // Create some borders
  mvwhline(regw, REG_HEIGHT-1, 0, ACS_HLINE, width);
  mvwvline(prgrmw, 0, mainwidth-1, '|', mainheight);
  mvwvline(disasmw, 0, mainwidth-1, '|', mainheight);
  mvwhline(prgrmw, mainheight-1, 0, ACS_HLINE, mainwidth);
  mvwhline(disasmw, mainheight-1, 0, ACS_HLINE, mainwidth);
  mvwhline(memw, mainheight-1, 0, ACS_HLINE, width-2*mainwidth);

  // Titles, content, etc.
  printMiddle(prgrmw, 0, 0, mainwidth-1, "*PROGRAM*");
  printMiddle(disasmw, 0, 0, mainwidth-1, "*DIASSEMBLED*");
  printMiddle(memw, 0, 0, width - 2*mainwidth, "*MEMORY*");

  mvwchgat(prgrmw, 0, 0, mainwidth-1, A_BOLD, COL_SELECTED, NULL);
  mvwchgat(disasmw, 0, 0, mainwidth-1, A_NORMAL, COL_NOTSELECTED, NULL);
  mvwchgat(memw, 0, 0, width - 2*mainwidth, A_NORMAL, COL_NOTSELECTED, NULL);

  printCommands(cmdsw, 0, 0);

  // Memory "widget" dimensions
  memrows = mainheight-6;
  dmemrows = memrows - 4;
  memcols = 8;

  // Refresh
  refresh();
  refreshAll();

  // Main loop
  while (ch != 'q') { // q for quit
    /// Interpret the character
    // Change window focus
    if (ch == KEY_LEFT) {
      currentwindow = (currentwindow + 2) % 3; // 3 + cw - 1
    } else if (ch == KEY_RIGHT) {
      currentwindow = (currentwindow + 1) % 3;
    }
    // Navigate in memory
    else if (ch == KEY_UP) {
      if (currentwindow == 2) // mem
        memfrom = (memfrom > 0 ? memfrom - 1 : 0);
      else
        prgrmfrom = (prgrmfrom > 0 ? prgrmfrom - 1 : 0);
    } else if (ch == KEY_DOWN) {
      if (currentwindow == 2) // mem
        memfrom = (memfrom < memory->getSize()/memcols ? memfrom + 1 : memory->getSize()/memcols - 1);
      else
        prgrmfrom = (prgrmfrom < memory->getSize()/memcols ? prgrmfrom + 1 : memory->getSize()/memcols - 1);
    } else if (ch == KEY_PPAGE) { // page down
      if (currentwindow == 2) // mem
        memfrom = (memfrom > memrows ? memfrom - memrows : 0);
      else
        prgrmfrom = (prgrmfrom > memrows ? prgrmfrom - memrows : 0);
    } else if (ch == KEY_NPAGE) {
      if (currentwindow == 2) // mem
        memfrom = (memfrom < memory->getSize()/memcols ? memfrom + memrows : memory->getSize()/memcols - 1);
      else
        prgrmfrom = (prgrmfrom < memory->getSize()/memcols ? prgrmfrom + memrows : memory->getSize()/memcols - 1);
    }
    // Navigate in instructions/data
    else if (ch == '+') {
      selsize = (selsize == 8 ? 8 : 2*selsize);
      if (selection % selsize != 0) // we are not aligned !
        selection = selsize * (selection / selsize);
    } else if (ch == '-') {
      selsize = (selsize == 1 ? 1 : selsize/2);
    } else if (ch == 'n') {
        if (currentwindow == 2)
          selection = (selection + selsize) % memory->getSize();
        else
          if (executionmode) {
            engine->next();
            next = true;
          } else
            instr = (instr + 4) % memory->getSize();
    } else if (ch == 'p') {
        if (currentwindow == 2)
          selection = (selection > selsize ? selection - selsize : 0);
        else
          if (!executionmode)
            instr = (instr >= 4 ? instr - 4 : 0);
    }
    // Switch modes
    else if (ch == KEY_F(1)) {
      engine->init();
      executionmode = !executionmode;
      next = true;
    }

    // Execution mode => no free instruction !
    if (executionmode)
      instr = pc.read();

    // We need to update !
    if (next) {
      printRegisters(regw, registers, 0, width-48);
      printPSR(regw, &psr, 1, 1);
      printFSR(regw, &fsr, 4, 1);
      printWIM(regw, &wim, 7, 1);
      mvwprintw(regw, 1, 46, " PC: 0x%08x", pc.read());
      mvwprintw(regw, 2, 46, "nPC: 0x%08x", npc.read());
      mvwprintw(regw, 3, 46, "TBR: (TBA) 0x%06x", tbr.getField(TBR_TBA));
      mvwprintw(regw, 4, 46, "      (tt) 0x%02x", tbr.getField(TBR_TT));
      mvwprintw(regw, 5, 46, "  Y: 0x%08x", y.read());

      wrefresh(regw);
    }

    // We have changed of window
    if (lastwindow != currentwindow) {
      switch (currentwindow) {
        case 0: // prgrm
          mvwchgat(prgrmw, 0, 0, mainwidth-1, A_BOLD, COL_SELECTED, NULL);
          wrefresh(prgrmw);
          break;
        case 1: // disasm
          mvwchgat(disasmw, 0, 0, mainwidth-1, A_BOLD, COL_SELECTED, NULL);
          wrefresh(disasmw);
          break;
        case 2:
          mvwchgat(memw, 0, 0, width - 2*mainwidth, A_BOLD, COL_SELECTED, NULL);
          wrefresh(memw);
          break;
      }
      switch (lastwindow) {
        case 0: // prgrm
          mvwchgat(prgrmw, 0, 0, mainwidth-1, A_NORMAL, COL_NOTSELECTED, NULL);
          wrefresh(prgrmw);
          break;
        case 1: // disasm
          mvwchgat(disasmw, 0, 0, mainwidth-1, A_NORMAL, COL_NOTSELECTED, NULL);
          wrefresh(disasmw);
          break;
        case 2:
          mvwchgat(memw, 0, 0, width - 2*mainwidth, A_NORMAL, COL_NOTSELECTED, NULL);
          wrefresh(memw);
          break;
      }
    }

    // We have changed of prgrmfrom
    if (lastprgrmfrom != prgrmfrom || next) {
      printMemory(prgrmw, memory, prgrmfrom, memrows, memcols, 2, 1);
      printInstructions(disasmw, memory, prgrmfrom, memrows, memcols, 2, 2, mainwidth-1);
      highlightInstruction(prgrmw, disasmw, instr, memrows, memcols, prgrmfrom, 2, 1, mainwidth-1, executionmode ? COL_SELECTED : COL_SHALLOWSEL);
      wrefresh(prgrmw);
      wrefresh(disasmw);
    }

    // We have changed of memfrom
    if (lastmemfrom != memfrom || next) {
      printMemory(memw, memory, memfrom, dmemrows, memcols, 2, 1);
      highlightSelection(memw, selection, selsize, dmemrows, memcols, memfrom, 2, 1);
      wrefresh(memw);
    }

    // We have changed of instr
    if (lastinstr != instr || next) {
      highlightInstruction(prgrmw, disasmw, instr, memrows, memcols, prgrmfrom, 2, 1, mainwidth-1, executionmode ? COL_SELECTED : COL_SHALLOWSEL);
      mvwprintw(prgrmw, memrows+4, 1, "ADDR:0x%08x", instr);
      wrefresh(prgrmw);
      wrefresh(disasmw);
    }

    // We have changed of data
    if (lastselection != selection || lastselsize != selsize) {
      // Highlight selection
      highlightSelection(memw, selection, selsize, dmemrows, memcols, memfrom, 2, 1);

      // Print address
      mvwprintw(memw, dmemrows+4, 1, "ADDR:0x%08x:0x%08x", selection, selection+selsize-1);
      
      // Build selection
      std::ostringstream toshow;
      toshow << "VALUE=0x" << std::hex << std::setfill('0') << std::setw(selsize*2);
      uint64_t value = 0;
      uint8_t temp[selsize];
      memory->read(selection, selsize, temp);
      for (uint32_t i = 0; i < selsize; i++)
        value = (value << 8) | temp[i];
      toshow << value;

      if (selsize > 4) toshow << std::endl << "     ";

      toshow << " (" << std::dec << value << " [";

      value = signext64(value, selsize*8);
      if ((value >> 63) == 1) // value is negative
        toshow << "-" << COMPL64(value);
      else
        toshow << "+" << value;
      toshow << "])";

      // Print value of selection
      mvwprintw(memw, dmemrows+5, 1, toshow.str().c_str());
      for (uint32_t i = toshow.str().size(); i < (lastselsize+1)*12; i++)
        wprintw(memw, " ");

      // Refresh
      wrefresh(memw);
    }

    // End of loop : refresh 
    ch = getch();

    // Updates
    lastwindow = currentwindow;
    lastprgrmfrom = prgrmfrom;
    lastmemfrom = memfrom;
    lastinstr = instr;
    lastselection = selection;
    lastselsize = selsize;
    next = false;
  }

  // End
  delwin(regw);
  delwin(cmdsw);
  delwin(prgrmw);
  delwin(disasmw);
  delwin(memw);
  endwin();

  delete engine;
  delete registers;
  delete memory;
  delete alu;

  Logger::destroy();

  return 0;
}


