/*
 * disassembler.cpp -- implements the disassembler algorithm
 * -----
 * Author: krab
 * Version: 0.1
 */
#include "disassembler.h"

#include <sstream>
#include <iomanip>

using namespace std;

// Instruction names
string branchname[] = {
  "n",
  "e",
  "le",
  "l",
  "leu",
  "cs",
  "neg",
  "vs",
  "a",
  "ne",
  "g",
  "ge",
  "gu",
  "cc",
  "pos",
  "vc"
};

string fbranchname[] = {
  "n",
  "ne",
  "lg",
  "ul",
  "l",
  "ug",
  "g",
  "u",
  "a",
  "e",
  "ue",
  "ge",
  "uge",
  "le",
  "ule",
  "o"
};

string cbranchname[] = {
  "n",
  "123",
  "12",
  "13",
  "1",
  "23",
  "2",
  "3",
  "a",
  "0",
  "03",
  "02",
  "023",
  "01",
  "013",
  "012"
};

string meminstname[] = {
  "ld", "ldub", "lduh", "ldd", "st", "stb", "sth", "std", "", "ldsb", "ldsh", "", "", "", "", "", // 0x0B -> 0x0F
  "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", // 0x10 -> 0x1F
  "ldf", "ldfsr", "", "lddf", "", "", "", "", "", "", "", "", "", "", "", "",
  "ldc", "ldcsr", "", "lddc", "", "", "", "", "", "", "", "", "", "", "", "",
  "", "", "", "", "stf", "stfsr", "", "stdf", "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
  "", "", "", "", "stc", "stcsr", "", "stdc", "", "", "", "", "", "", "", ""
    // nothing beyond
};

string aluinstrname[] = {
  "add",
  "and",
  "or",
  "xor",
  "sub",
  "andn",
  "orn",
  "xnor",
  "addx",
  "",
  "umul",
  "smul",
  "subx",
  "",
  "udiv",
  "sdiv",
  "addcc",
  "andcc",
  "orcc",
  "xorcc",
  "subcc",
  "andncc",
  "orncc",
  "xnorcc",
  "addxcc",
  "",
  "umulcc",
  "smulcc",
  "subxcc",
  "",
  "udivcc",
  "sdivcc",
  "", "", "", "", "", // 0x20 -> 0x24
  "sll",
  "srl",
  "sra"
};

string op3name[] = {
  "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "", "rdy", "rdpsr", "rdwim", "rdtbr", "", "", "", "",
  "wry", "wrpsr", "wrwim", "wrtbr", "fpop1", "fpop2", "cpop1", "cpop2", "jmpl", "rett", "ticc", "flush", "save", "rest", "", ""
};

/*
 * Determines if a instruction is ALU
 */
bool isALU(uint32_t op) {
  return (         op <= 0x0F && op != 0x09 && op != 0x0D) ||
    (op >= 0x10 && op <= 0x1F && op != 0x19 && op != 0x1D) ||
    (op >= 0x25 && op <= 0x27);
}

/*
 * Get the name of a register from its number
 */
string registerName(uint32_t rnum) {
  ostringstream res;
  if (rnum < 8)
    res << "%g";
  else if (rnum < 16)
    res << "%o";
  else if (rnum < 24)
    res << "%l";
  else
    res << "%i";

  res << (rnum % 8);
  return res.str();
}

/*
 * Main function : disassemble a binary encoded instruction
 */
string disassemble(Instruction inst, uint32_t addr) {
  // This particular configuration (which, in fact corresponds to cbn 0x0), is considered to be useless and ignored
  if (inst.getContent() == 0x00000000)
    return ".";

  // The result
  ostringstream res;

  // Get the main operator and the destination register (used most of the time)
  uint32_t op = inst.getField(INST_OP);
  uint32_t rd = inst.getField(INST_RD);

  // Branches and SETHI
  if (op == INST_OP_BR) {
    // Get op2
    uint32_t op2 = inst.getField(INST_OP2);

    if (op2 == INST_OP2_SETHI) {
      uint32_t imm = inst.getField(INST_IMM22);
      if (imm == 0 && rd == 0)
        res << "nop";
      else
        res << "sethi 0x" << hex << setfill('0') << setw(8) << imm << ", " << registerName(rd); 
    } else if (op2 == INST_OP2_BICC) {
      string a = (inst.getField(INST_A) == 1 ? "a" : "");
      res << "b" << branchname[inst.getField(INST_COND)] << a << " inst" << hex << (addr + signext(inst.getField(INST_DISP22), 22)*4)/4; 
    } else if (op2 == INST_OP2_FBFCC) {
      string a = (inst.getField(INST_A) == 1 ? "a" : "");
      res << "fb" << fbranchname[inst.getField(INST_COND)] << a << " inst" << hex << (addr + signext(inst.getField(INST_DISP22), 22)*4)/4;
    } else { // CBCCC
      string a = (inst.getField(INST_A) == 1 ? "a" : "");
      res << "cb" << cbranchname[inst.getField(INST_COND)] << a << " inst" << hex << (addr + signext(inst.getField(INST_DISP22), 22)*4)/4;
    }
  }
  // Calls
  else if (op == INST_OP_CALL) {
    res << "call " << "inst" << hex << (addr + signext(inst.getField(INST_DISP30), 30)*4)/4;
  }
  // Others (logic, arithmetics)
  else if (op == INST_OP_OTHER) {
    uint32_t op3 = inst.getField(INST_OP3);
    uint32_t rs1 = inst.getField(INST_RS1);
    uint32_t rs2 = inst.getField(INST_RS2);
    uint32_t simm = signext(inst.getField(INST_SIMM13), 13);

    ostringstream saddr;
    saddr << registerName(rs1) << ", ";

    if (inst.getField(INST_I) == 1) {
      if (ISNEG(simm))
        saddr << "-0x" << hex << setfill('0') << setw(4) << COMPL32(simm);
      else
        saddr << "0x" << hex << setfill('0') << setw(4) << simm;
    } else {
      saddr << registerName(rs2);
    } 

    if (isALU(op3)) {
      res << aluinstrname[op3] << " " << saddr.str() << ", " << registerName(rd);
    } else {
      res << op3name[op3] << " "; 

      switch (op3) {
        case INST_OP3_WRY:
        case INST_OP3_WRPSR:
        case INST_OP3_WRWIM:
        case INST_OP3_WRTBR:
        case INST_OP3_RETT:
        case INST_OP3_FLUSH:
          res << saddr.str();
          break;
        case INST_OP3_SAVE:
        case INST_OP3_REST:
        case INST_OP3_JMPL:
          res << saddr.str() << ", " << registerName(rd);
          break;
        case INST_OP3_TICC:
          res << "#";
          break;
        case INST_OP3_RDY:
        case INST_OP3_RDPSR:
        case INST_OP3_RDWIM:
        case INST_OP3_RDTBR:
          res << registerName(rd);
          break;
        case INST_OP3_FPOP1:
        case INST_OP3_FPOP2:
        case INST_OP3_CPOP1:
        case INST_OP3_CPOP2:
          res << hex << inst.getField(INST_OPF) << ", " << registerName(rs1) << ", " << registerName(rs2) << ", " << registerName(rd);
          break;
      }
    }
  }
  // Memory
  else {
    uint32_t op3 = inst.getField(INST_OP3);
    uint32_t rs1 = inst.getField(INST_RS1);
    uint32_t rs2 = inst.getField(INST_RS2);
    uint32_t simm = signext(inst.getField(INST_SIMM13), 13);
    res << meminstname[op3] <<
      " [" << registerName(rs1);
    if (inst.getField(INST_I) == 1) {
      if (ISNEG(simm))
        res << "-0x" << hex << setfill('0') << setw(4)<< COMPL32(simm);
      else
        res << "+0x" << hex << setfill('0') << setw(4) << simm;
    } else {
      res << "+" << registerName(rs2);
    }

    res << "], " << registerName(rd);
  }

  return res.str();
}



