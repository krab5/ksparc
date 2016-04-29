/*
 * kdisasmmain.cpp -- very simple instruction by instruction disassembler
 * -----
 * This is a tool that has been create for debugging purpose; it allows you to
 * write in a command line an hexadecimal number and to get the disassembled code
 * from it.
 *
 * Author: krab
 * Version: 0.1
 */
#include <iostream>
#include <iomanip>
#include "disassembler.h"

using namespace std;

int main(int argc, char* argv[]) {
  uint32_t val = 1;
  cout << "==== KASM Disassembler ====" << endl;
  cout << "Type in an hex value for disassembling it." << endl;
  cout << "0 to exit." << endl;

  while (val != 0) {
    cout << "> ";
    // Read an hex value
    cin >> hex >> val;
    // Build an instruction
    Instruction i(val);
    // Disassemble
    string instr = disassemble(i, 0);
    // Show result
    cout << instr << std::endl;
  }

  return 0;
}


