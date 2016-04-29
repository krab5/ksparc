/*
 * disassembler.h -- header for the disassembler function
 * ------
 * Author: krab
 * Version: 0.1
 */
#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include "instruction.h"
#include <string>

/**
 * Main function : disassemble a binary encoded instruction stored in a Instruction object.
 * @param instruction instruction to decode
 * @param addr address of the instruction to calculated branches, calls, jmpl
 * @returns the code
 */
std::string disassemble(Instruction instruction, uint32_t addr);

#endif // DISASSEMBLER_H
