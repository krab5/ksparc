/*
 * assembler.cpp -- main function for building an assembled binary file
 * -----
 * This file contains a bunch of function; most of them are just helpers,
 * the main functions are parseForLabels and readi
 *
 * Most of the functions accept and ErrorList and a size_t arguments that
 * allow for precise error treatments.
 * 
 * Author: krab
 * Version : 0.1
 */
#include <algorithm>
#include <sstream>
#include <iterator>
#include <cmath>
#include <iostream>

#include "logger.h"
#include "assembler.h"

/// UAL opcodes
// Calculate
#define UAL_OP_ADD    0x00    // addition
#define UAL_OP_AND    0x01    // bitwise and
#define UAL_OP_OR     0x02    // bitwise or
#define UAL_OP_XOR    0x03    // bitwise xor
#define UAL_OP_SUB    0x04    // substraction
#define UAL_OP_ANDN   0x05    // bitwise and with ~right
#define UAL_OP_ORN    0x06    // birwise or with ~right
#define UAL_OP_XNOR   0x07    // bitwise xor with ~right
#define UAL_OP_ADDX   0x08    // extended add : add two operands + carry
#define UAL_OP_UMUL   0x0A    // unsigned mult
#define UAL_OP_SMUL   0x0B    // signed mult
#define UAL_OP_SUBX   0x0C    // extended sub : sub two operands + carry
#define UAL_OP_UDIV   0x0E    // unsigned div
#define UAL_OP_SDIV   0x0F    // signed div

// Calc and change ICC flags
#define UAL_OP_ADDcc  0x10
#define UAL_OP_ANDcc  0x11
#define UAL_OP_ORcc   0x12
#define UAL_OP_XORcc  0x13
#define UAL_OP_SUBcc  0x14
#define UAL_OP_ANDNcc 0x15
#define UAL_OP_ORNcc  0x16
#define UAL_OP_XNORcc 0x17
#define UAL_OP_ADDXcc 0x18
#define UAL_OP_UMULcc 0x1A
#define UAL_OP_SMULcc 0x1B
#define UAL_OP_SUBXcc 0x1C
#define UAL_OP_UDIVcc 0x1E
#define UAL_OP_SDIVcc 0x1F

#define UAL_OP_SLL    0x25
#define UAL_OP_SRL    0x26
#define UAL_OP_SRA    0x27

// Used quite often, represents typical parameters of an UAL opcode
#define UAL_PARAMETERS "source register 1", "source register 2 or constant", "destination register"

// Define a not found result by *fromOpcode
const uint32_t NOTFOUND = 0xFFFFFFFF;
const uint32_t MNEMO = 0xEEEEEEEE;

using namespace std;
using namespace ASMError;

// List of every opcode known by the assembler, as well as their code and parameters
// Sometimes, it is precised that there is a minimal amount of parameters, or that
// opcode is not implemented yet
vector<OpCode> opcodes {
  // Nop
  OpCode(0, "nop"),
    // Call
    OpCode(0, "call", "label"),
    // Sethi
    OpCode(0, "sethi", "32-bit number", "destination register"),
    // Branch
    OpCode(INST_COND_ALWAYS , "ba"     , "label"),
    OpCode(INST_COND_NEVER  , "bn"     , "label"),
    OpCode(INST_COND_NEQ    , "bne"    , "label"),
    OpCode(INST_COND_EQ     , "be"     , "label"),
    OpCode(INST_COND_GT     , "bg"     , "label"),
    OpCode(INST_COND_LET    , "ble"    , "label"),
    OpCode(INST_COND_GET    , "bge"    , "label"),
    OpCode(INST_COND_LT     , "blt"    , "label"),
    OpCode(INST_COND_UGT    , "bgu"    , "label"),
    OpCode(INST_COND_ULET   , "bleu"   , "label"),
    OpCode(INST_COND_CCLR   , "bcc"    , "label"),
    OpCode(INST_COND_CSET   , "bcs"    , "label"),
    OpCode(INST_COND_POS    , "bpos"   , "label"),
    OpCode(INST_COND_NEG    , "bneg"   , "label"),
    OpCode(INST_COND_OCLR   , "boc"    , "label"),
    OpCode(INST_COND_OSET   , "bos"    , "label"),
    OpCode(INST_FCOND_ALWAYS, "fba"    , "label"),
    OpCode(INST_FCOND_NEVER , "fbn"    , "label"),
    OpCode(INST_FCOND_U     , "fbu"    , "label"),
    OpCode(INST_FCOND_G     , "fbg"    , "label"),
    OpCode(INST_FCOND_UG    , "fbug"   , "label"),
    OpCode(INST_FCOND_L     , "fbl"    , "label"),
    OpCode(INST_FCOND_UL    , "fbul"   , "label"),
    OpCode(INST_FCOND_LG    , "fblg"   , "label"),
    OpCode(INST_FCOND_NE    , "fbne"   , "label"),
    OpCode(INST_FCOND_E     , "fbe"    , "label"),
    OpCode(INST_FCOND_UE    , "fbue"   , "label"),
    OpCode(INST_FCOND_GE    , "fbge"   , "label"),
    OpCode(INST_FCOND_UGE   , "fbuge"  , "label"),
    OpCode(INST_FCOND_LE    , "fble"   , "label"),
    OpCode(INST_FCOND_ULE   , "fbule"  , "label"),
    OpCode(INST_FCOND_O     , "fbo"    , "label"),
    OpCode(INST_CCOND_ALWAYS, "cba"    , "label"),
    OpCode(INST_CCOND_NEVER , "cbn"    , "label"),
    OpCode(INST_CCOND_3     , "cb3"    , "label"),
    OpCode(INST_CCOND_2     , "cb2"    , "label"),
    OpCode(INST_CCOND_23    , "cb23"   , "label"),
    OpCode(INST_CCOND_1     , "cb1"    , "label"),
    OpCode(INST_CCOND_13    , "cb13"   , "label"),
    OpCode(INST_CCOND_12    , "cb12"   , "label"),
    OpCode(INST_CCOND_123   , "cb123"  , "label"),
    OpCode(INST_CCOND_0     , "cb0"    , "label"),
    OpCode(INST_CCOND_03    , "cb03"   , "label"),
    OpCode(INST_CCOND_02    , "cb02"   , "label"),
    OpCode(INST_CCOND_023   , "cb023"  , "label"),
    OpCode(INST_CCOND_01    , "cb01"   , "label"),
    OpCode(INST_CCOND_013   , "cb013"  , "label"),
    OpCode(INST_CCOND_012   , "cb012"  , "label"),
    // Branches with annulation
    OpCode(INST_COND_ALWAYS , "baa"    , "label"),
    OpCode(INST_COND_NEVER  , "bna"    , "label"),
    OpCode(INST_COND_NEQ    , "bnea"   , "label"),
    OpCode(INST_COND_EQ     , "bea"    , "label"),
    OpCode(INST_COND_GT     , "bga"    , "label"),
    OpCode(INST_COND_LET    , "blea"   , "label"),
    OpCode(INST_COND_GET    , "bgea"   , "label"),
    OpCode(INST_COND_LT     , "blta"   , "label"),
    OpCode(INST_COND_UGT    , "bgua"   , "label"),
    OpCode(INST_COND_ULET   , "bleua"  , "label"),
    OpCode(INST_COND_CCLR   , "bcca"   , "label"),
    OpCode(INST_COND_CSET   , "bcsa"   , "label"),
    OpCode(INST_COND_POS    , "bposa"  , "label"),
    OpCode(INST_COND_NEG    , "bnega"  , "label"),
    OpCode(INST_COND_OCLR   , "boca"   , "label"),
    OpCode(INST_COND_OSET   , "bosa"   , "label"),
    OpCode(INST_FCOND_ALWAYS, "fbaa"   , "label"),
    OpCode(INST_FCOND_NEVER , "fbna"   , "label"),
    OpCode(INST_FCOND_U     , "fbua"   , "label"),
    OpCode(INST_FCOND_G     , "fbga"   , "label"),
    OpCode(INST_FCOND_UG    , "fbuga"  , "label"),
    OpCode(INST_FCOND_L     , "fbla"   , "label"),
    OpCode(INST_FCOND_UL    , "fbula"  , "label"),
    OpCode(INST_FCOND_LG    , "fblga"  , "label"),
    OpCode(INST_FCOND_NE    , "fbnea"  , "label"),
    OpCode(INST_FCOND_E     , "fbea"   , "label"),
    OpCode(INST_FCOND_UE    , "fbuea"  , "label"),
    OpCode(INST_FCOND_GE    , "fbgea"  , "label"),
    OpCode(INST_FCOND_UGE   , "fbugea" , "label"),
    OpCode(INST_FCOND_LE    , "fblea"  , "label"),
    OpCode(INST_FCOND_ULE   , "fbulea" , "label"),
    OpCode(INST_FCOND_O     , "fboa"   , "label"),
    OpCode(INST_CCOND_ALWAYS, "cbaa"   , "label"),
    OpCode(INST_CCOND_NEVER , "cbna"   , "label"),
    OpCode(INST_CCOND_3     , "cb3a"   , "label"),
    OpCode(INST_CCOND_2     , "cb2a"   , "label"),
    OpCode(INST_CCOND_23    , "cb23a"  , "label"),
    OpCode(INST_CCOND_1     , "cb1a"   , "label"),
    OpCode(INST_CCOND_13    , "cb13a"  , "label"),
    OpCode(INST_CCOND_12    , "cb12a"  , "label"),
    OpCode(INST_CCOND_123   , "cb123a" , "label"),
    OpCode(INST_CCOND_0     , "cb0a"   , "label"),
    OpCode(INST_CCOND_03    , "cb03a"  , "label"),
    OpCode(INST_CCOND_02    , "cb02a"  , "label"),
    OpCode(INST_CCOND_023   , "cb023a" , "label"),
    OpCode(INST_CCOND_01    , "cb01a"  , "label"),
    OpCode(INST_CCOND_013   , "cb013a" , "label"),
    OpCode(INST_CCOND_012   , "cb012a" , "label"),
    // Load instr
    OpCode(INST_OP3_LDSB   , "ldsb" , "[address]", "destination register"), 
    OpCode(INST_OP3_LDSH   , "ldsh" , "[address]", "destination register"), 
    OpCode(INST_OP3_LDUB   , "ldub" , "[address]", "destination register"), 
    OpCode(INST_OP3_LDUH   , "lduh" , "[address]", "destination register"), 
    OpCode(INST_OP3_LD     , "ld"   , "[address]", "destination register"), 
    OpCode(INST_OP3_LDD    , "ldd"  , "[address]", "destination register"), 
    OpCode(INST_OP3_LDF    , "ldf"  , "[address]", "destination register", 2, false), 
    OpCode(INST_OP3_LDDF   , "lddf" , "[address]", "destination register", 2, false), 
    OpCode(INST_OP3_LDFSR  , "ldfsr", "[address]", "destination register", 2, false), 
    OpCode(INST_OP3_LDC    , "ldc"  , "[address]", "destination register", 2, false), 
    OpCode(INST_OP3_LDDC   , "lddc" , "[address]", "destination register", 2, false), 
    OpCode(INST_OP3_LDCSR  , "ldcsr", "[address]", "destination register", 2, false), 
    OpCode(INST_OP3_STB    , "stb"  , "[address]", "destination register"), 
    OpCode(INST_OP3_STH    , "sth"  , "[address]", "destination register"), 
    OpCode(INST_OP3_ST     , "st"   , "[address]", "destination register"), 
    OpCode(INST_OP3_STD    , "std"  , "[address]", "destination register"), 
    OpCode(INST_OP3_STF    , "stf"  , "[address]", "destination register", 2, false), 
    OpCode(INST_OP3_STDF   , "stdf" , "[address]", "destination register", 2, false), 
    OpCode(INST_OP3_STFSR  , "stfsr", "[address]", "destination register", 2, false), 
    OpCode(INST_OP3_STC    , "stc"  , "[address]", "destination register", 2, false), 
    OpCode(INST_OP3_STDC   , "stdc" , "[address]", "destination register", 2, false), 
    OpCode(INST_OP3_STCSR  , "stcsr", "[address]", "destination register", 2, false),
    // rd and wr
    OpCode(0, "rd", "source special register", "destination register"),
    OpCode(0, "wr", UAL_PARAMETERS),
    // JMPL
    OpCode(INST_OP3_JMPL, "jmpl", "address", "destination register"),
    // Save and Restore
    OpCode(INST_OP3_SAVE, "save", UAL_PARAMETERS, 0),
    OpCode(INST_OP3_REST, "restore", UAL_PARAMETERS, 0),
    // UAL instr
    OpCode(UAL_OP_ADD,    "add",     UAL_PARAMETERS),
    OpCode(UAL_OP_AND,    "and",     UAL_PARAMETERS),
    OpCode(UAL_OP_OR ,    "or",      UAL_PARAMETERS),
    OpCode(UAL_OP_XOR,    "xor",     UAL_PARAMETERS),
    OpCode(UAL_OP_SUB,    "sub",     UAL_PARAMETERS),
    OpCode(UAL_OP_ANDN,   "andn",    UAL_PARAMETERS),
    OpCode(UAL_OP_ORN,    "orn",     UAL_PARAMETERS),
    OpCode(UAL_OP_XNOR,   "xnor",    UAL_PARAMETERS),
    OpCode(UAL_OP_ADDX,   "addx",    UAL_PARAMETERS),
    OpCode(UAL_OP_UMUL,   "umul",    UAL_PARAMETERS),
    OpCode(UAL_OP_SMUL,   "smul",    UAL_PARAMETERS),
    OpCode(UAL_OP_SUBX,   "subx",    UAL_PARAMETERS),
    OpCode(UAL_OP_UDIV,   "udiv",    UAL_PARAMETERS),
    OpCode(UAL_OP_SDIV,   "sdiv",    UAL_PARAMETERS),
    OpCode(UAL_OP_ADDcc,  "addcc",   UAL_PARAMETERS), 
    OpCode(UAL_OP_ANDcc,  "andcc",   UAL_PARAMETERS), 
    OpCode(UAL_OP_ORcc,   "orcc",    UAL_PARAMETERS), 
    OpCode(UAL_OP_XORcc,  "xorcc ",  UAL_PARAMETERS), 
    OpCode(UAL_OP_SUBcc,  "subcc ",  UAL_PARAMETERS), 
    OpCode(UAL_OP_ANDNcc, "andncc",  UAL_PARAMETERS), 
    OpCode(UAL_OP_ORNcc,  "orncc ",  UAL_PARAMETERS), 
    OpCode(UAL_OP_XNORcc, "xnorcc",  UAL_PARAMETERS), 
    OpCode(UAL_OP_ADDXcc, "addxcc",  UAL_PARAMETERS), 
    OpCode(UAL_OP_UMULcc, "umulcc",  UAL_PARAMETERS), 
    OpCode(UAL_OP_SMULcc, "smulcc",  UAL_PARAMETERS), 
    OpCode(UAL_OP_SUBXcc, "subxcc",  UAL_PARAMETERS), 
    OpCode(UAL_OP_UDIVcc, "udivcc",  UAL_PARAMETERS), 
    OpCode(UAL_OP_SDIVcc, "sdivcc",  UAL_PARAMETERS), 
    OpCode(UAL_OP_SLL,    "sll",     UAL_PARAMETERS), 
    OpCode(UAL_OP_SRL,    "srl",     UAL_PARAMETERS), 
    OpCode(UAL_OP_SRA,    "sra",     UAL_PARAMETERS),
    // Mnemonics
    OpCode(MNEMO, "cmp",      "source register 1", "source register 2 or constant"),
    OpCode(MNEMO, "jmp",      "address"),
    OpCode(MNEMO, "tst",      "source/destination register"),
    OpCode(MNEMO, "ret"),
    OpCode(MNEMO, "retl"),
    OpCode(MNEMO, "set",      "32-bit constant", "destination register"),
    OpCode(MNEMO, "not",      "source register", "destination register", 1),
    OpCode(MNEMO, "neg",      "source register", "destination register", 1),
    OpCode(MNEMO, "inc",      "source/destination register", "constant", 1),
    OpCode(MNEMO, "inccc",    "source/destination register", "constant", 1),
    OpCode(MNEMO, "dec",      "source/destination register", "constant", 1),
    OpCode(MNEMO, "deccc",    "source/destination register", "constant", 1),
    OpCode(MNEMO, "clrr",     "destination register"),
    OpCode(MNEMO, "clrb",     "[address]"),
    OpCode(MNEMO, "clrh",     "[address]"),
    OpCode(MNEMO, "clr",      "[address]"),
    OpCode(MNEMO, "mov",     "source register or constant", "destination register")
};

/*
 * trim -- remove front spaces of a string iterator
 * it : the iterator
 */
void trim(string::iterator& it) {
  while (isspace(*it)) it++;
}

/*
 * toNum -- parse a string into a number
 * str : string to parse
 * errs, line : error list
 */
uint32_t toNum(string str, ErrorList& errs, size_t line) {
  uint32_t a;
  try {
    a = stoul(str, nullptr);
  } catch (std::invalid_argument& e) {
    errs.push_back(ASMError::WrongNumberFormatError(str, line));
  }
  return a;
}

/*
 * split -- split a string by a delimiter
 * str: string to split
 * del: delimiter
 */
vector<string> split(string str, char del) {
  vector<string> res;

  if (str.size() > 0) {
    stringstream ss(str);
    string item;
    while (getline(ss, item, ',')) {
      res.push_back(item);
    }
  }

  return res;
}

/*
 * opcodeFromOp -- get an OpCode object from a code name
 * code : code to get the OpCode
 */
OpCode opcodeFromOp(string code) {
  for (auto it = opcodes.begin(); it != opcodes.end(); it++) {
    if ((*it).name == code)
      return (*it);
  }

  return OpCode(NOTFOUND, "not found");
}

/*
 * valid -- determines if an opcode is valid
 */
bool valid(string code) {
  return opcodeFromOp(code).code != NOTFOUND;
}

/*
 * implemented -- determines if an opcode has been implemented
 */
bool implemented(string code) {
  OpCode op = opcodeFromOp(code);
  return op.code != NOTFOUND && op.implemented;
}

/*
 * is*** -- determines if an opcode is of a special type
 */
bool isBranch(string st) {
  if (st.size() == 0)
    return false;
  return st[0] == 'b';
}

bool isFBranch(string st) {
  if (st.size() < 2)
    return false;
  return st[0] == 'f' && st[1] == 'b';
}

bool isCBranch(string st) {
  if (st.size() < 2)
    return false;
  return st[0] == 'c' && st[1] == 'b';
}

bool isLoadInstr(string st) {
  if (st.size() < 2)
    return false;
  return (st[0] == 'l' && st[1] == 'd');
}

bool isStoreInstr(string st) {
  if (st.size() < 2)
    return false;
  return (st[0] == 's' && st[1] == 't');
}

bool isArithLog(string str) {
  uint32_t code = opcodeFromOp(str).code;
  return (code <= 0x0F && code != 0x09 && code != 0x0D) ||
    (code >= 0x10 && code <= 0x1F && code != 0x19 && code != 0x1D) ||
    (code == 0x25 || code == 0x26 || code == 0x27);
}

bool isMnemonic(string str) {
  return opcodeFromOp(str).code == MNEMO;
}

/*
 * getRegister -- get a formatted number from a string register
 * regname: name of the register to parse
 * this name is of the form %CX, where C is r, g, l, o or i, and
 * X is a number
 */
uint32_t getRegister(string regname, ErrorList& errors, size_t line) {
  if (regname.size() < 2) {
    errors.push_back(ASMError::InvalidRegisterNameError('?', line));
    return 0;
  }

  // Get first relevent character
  char c = regname.at(1);

  // Get register number
  uint32_t reg = toNum(regname.substr(2, regname.size()), errors, line);

  // Test if number is coherent : from 0 to 32 for r; 0 to 8 for the others
  if ((c == 'o' || c == 'l' || c == 'i' || c == 'g')
      && reg > 8) {
    errors.push_back(ASMError::InvalidOILGRegisterNumberError(reg, line));
  } else if (c == 'r' && reg > 32) {
    errors.push_back(ASMError::InvalidRRegisterNumberError(reg, line));
  }

  // Calculate effective register number
  switch (c) {
    case 'o':
      reg += 8;
      break;
    case 'l':
      reg += 16;
      break;
    case 'i':
      reg += 24;
      break;
    case 'g':
    case 'r':
      reg += 0;
      break;
    default: // the register name is not recognized !
      reg = 0;
      errors.push_back(ASMError::InvalidRegisterNameError(c, line));
  }

  return reg;
}

/*
 * isRegister -- test if a string is a register
 */
bool isRegister(string str) {
  if (str.size() == 0) return false;
  return str.at(0) == '%';
}

/*
 * parseAddress -- get an address from a formated string
 * str : string to parse; it should be of the form [reg1+reg2] or [reg1+cst], where reg1 and reg2
 * are registers and cst is a 13-bit signed number
 * rs1, s2, i : output -> source register 1, source register 2 or cste, i = 0 if we use reg2 (or 1 else)
 */
void parseAddress(string str, uint32_t* rs1, uint32_t* s2, uint32_t* i, ErrorList& errs, size_t line) {
  auto it = str.begin();
  it++;
  string ss1 = "", ss2 = "";
  while (*it != '+') {
    ss1 += *it;
    it++;
  }
  it++;
  while (*it != ']') {
    ss2 += *it;
    it++;
  }

  if (isRegister(ss1)) 
    *rs1 = getRegister(ss1, errs, line);
  else {
    errs.push_back(ASMError::WrongAddressFormatError(line));
    *rs1 = 0;
  }

  if (isRegister(ss2)) {
    *s2 = getRegister(ss2, errs, line);
    *i = 0;
  } else {
    *s2 = toNum(str, errs, line);
    *i = 1;
  }
}

/*
 * Cstr
 */
Assembler::Assembler() {
  _instAddr = 0;
  _line = 0;

#ifdef _DEBUG
  Logger::init();
#endif
}

/*
 * Dstr
 */
Assembler::~Assembler() {
#ifdef _DEBUG
  Logger::destroy();
#endif
}

/*
 * hasErrors -- indicate that the assembler encountered errors
 */
bool Assembler::hasErrors() {
  return _errors.size() > 0;
}

/*
 * hasWarnings -- indicate that the assembler encountered warnings
 */
bool Assembler::hasWarnings() {
  return _warnings.size() > 0;
}

/*
 * errors -- get errors encountered by the assembler
 */
ErrorList Assembler::errors() {
  return _errors;
}

/*
 * warnings -- get warnings encountered by the asssembler
 */
WarningList Assembler::warnings() {
  return _warnings;
}

/*
 * get/setInstructionOffset -- change or retrieve the instruction offset
 * instruction offset allow for creating labels and calculating
 * branches displacements.
 */
uint32_t Assembler::getInstructionOffset() const {
  return _instAddr;
}

void Assembler::setInstructionOffset(uint32_t io) {
  _instAddr = io;
}

/*
 * addLabel -- add a label to the label list
 * label : label name
 * instaddr : address of the label
 */
void Assembler::addLabel(string label, uint32_t instaddr) {
  if (_labels.find(label) == _labels.end())
    _labels[label] = instaddr;
  else
    _errors.push_back(ASMError::LabelAlreadyDefinedError(label, _labels[label], _line));
  Logger::log() << "Add " << label << " at address " << instaddr << "\n";
}

/*
 * parseForLabel -- parse a string for retrieving a label
 * instr: string to parse
 */
void Assembler::parseForLabels(string instr) {
  string opcode = "";
  size_t pos, scpos;

  auto iit = instr.begin();

  trim(iit);

  // Get label (if there is one)
  pos = instr.find_first_of(':');
  scpos = instr.find_first_of(';');
  if (pos != string::npos &&
      (scpos == string::npos || pos < scpos)) {
    // We found a label !
    string label = "";
    while (*iit != ':') {
      label += *iit;
      iit++;
    }
    addLabel(label, _instAddr);
    trim(iit);
  }

  // Get opcode
  while (!isspace(*iit) && iit != instr.end()) {
    opcode += *iit;
    iit++;
  }

  if (opcode.size() == 0 && iit == instr.end())
    return;

  // Modify instraddr according to opcode
  if (opcode == "set") {
    _instAddr +=2;
  } else {
    _instAddr++;
  }
}

/*
 * parseForLabels -- parse several lines for labels
 * insts : list of instructions
 * ioff : instruction offset; consider to start from this instruction
 * loff : line offset; consider to start from this line
 */
void Assembler::parseForLabels(std::vector<std::string> insts, uint32_t ioff, size_t loff) {
  _instAddr = ioff;
  _line = loff;
  for (auto it = insts.begin(); it != insts.end(); it++) {
    parseForLabels((*it));
    _line++;
  }
}

/*
 * readAll -- read several lines
 * insts: list of instructions
 * ioff : instruction offset; consider to start from this instruction
 * loff : line offset; consider to start from this line
 */
Assembler::InstructionList Assembler::readAll(std::vector<std::string> insts, uint32_t ioff, size_t loff) {
  _instAddr = ioff;
  _line = loff;
  InstructionList il;
  for (auto it = insts.cbegin(); it != insts.cend(); it++) {
    if ((*it).size() > 0) {
      size_t p = (*it).find_first_not_of(' ');
      if (p != string::npos) {
        if ((*it)[p] != ';') {
          InstructionList tmp = read((*it));
          for (auto tmpit = tmp.begin(); tmpit != tmp.end(); tmpit++)
            il.push_back((*tmpit));
          _instAddr += tmp.size();
        }
      }
    }
    _line++;
  }
  return il;
}

/*
 * read -- read an instruction to extract a list of Instruction
 */
Assembler::InstructionList Assembler::read(string instr) {
  string opcode = "", args = "";
  vector<string> argl;
  InstructionList instructions;

  /// Parse
  auto iit = instr.begin();

  // Remove label
  size_t p = instr.find(':');
  if (p != string::npos)
    iit += p + 1;

  // Trim
  trim(iit);

  // Get opcode
  while (!isspace(*iit) && iit != instr.end()) {
    opcode += *iit;
    iit++;
  }

  // Remove space, trim out
  while (*iit != ';' && iit != instr.end()) {
    if (!isspace(*iit))
      args += *iit;
    iit++;
  }

  // Get argument list
  argl = split(args, ',');

  /// Parse opcode
  // Extract invalid opcode
  if (!valid(opcode)) {
    _errors.push_back(ASMError::UnexpectedOpcodeError(opcode, _line));
    return instructions;
  }

  if (!implemented(opcode)) {
    _errors.push_back(ASMError::UnimplementedOpcodeError(opcode, _line));
    return instructions;
  }

  // Verrify parameters
  OpCode oc = opcodeFromOp(opcode);

  if (argl.size() < oc.mnumparams) {
    _errors.push_back(ASMError::InvalidParamNumberError(oc, _line));
    return instructions;
  }


  if (argl.size() > oc.numparams) {
    _warnings.push_back(ASMError::TooMuchParametersWarning(oc, _line));
  }


  // Parse instructions
  if (opcode == "nop") {
    instructions.push_back(Instruction::makeInstruction(INST_OP_BR, 0, INST_OP2_SETHI, 0));
  } else if (opcode == "call") {
    uint32_t disp = 0;
    if (_labels.find(argl[0]) != _labels.end()) {
      disp = _labels[argl[0]] - _instAddr;
      instructions.push_back(Instruction::makeInstruction(
            INST_OP_CALL, disp
            ));
    } else {
      _errors.push_back(ASMError::LabelNotFoundError(argl[0], _line));
    }
  } else if (opcode == "sethi") {
    instructions.push_back(Instruction::makeInstruction(
          INST_OP_BR,
          getRegister(argl[0], _errors, _line),
          INST_OP2_SETHI,
          toNum(argl[1], _errors, _line)
          ));
  } else if (isBranch(opcode) || isFBranch(opcode) || isCBranch(opcode)) {
    uint32_t branchtype;

    if (isBranch(opcode))
      branchtype = INST_OP2_BICC;
    else if (isFBranch(opcode))
      branchtype = INST_OP2_FBFCC;
    else
      branchtype = INST_OP2_CBCCC;

    uint32_t disp = 0;
    if (_labels.find(argl[0]) != _labels.end())
      disp = _labels[argl[0]] - _instAddr;
    else
      _errors.push_back(ASMError::LabelNotFoundError(argl[0], _line));
    
    int a = (opcode[opcode.size()-1] == 'a' && opcode.size() > 2);
    if (a) opcode = opcode.substr(0, opcode.size()-1);

    instructions.push_back(Instruction::makeInstruction(
          INST_OP_BR,
          a,
          oc.code,
          branchtype,
          disp
          ));
  } else if (isLoadInstr(opcode)) {
    uint32_t s1, s2, i;
    parseAddress(argl[0], &s1, &s2, &i, _errors, _line);
    instructions.push_back(Instruction::makeInstruction(
          INST_OP_MEM,
          getRegister(argl[1], _errors, _line),
          oc.code,
          s1,
          i,
          0,
          s2
          ));
  } else if (isStoreInstr(opcode)) {
    uint32_t s1, s2, i;
    parseAddress(argl[1], &s1, &s2, &i, _errors, _line);
    instructions.push_back(Instruction::makeInstruction(
          INST_OP_MEM,
          getRegister(argl[0], _errors, _line),
          oc.code,
          s1,
          i,
          0,
          s2
          ));
  } else if (isArithLog(opcode)) {
    instructions.push_back(Instruction::makeInstruction(
          INST_OP_OTHER,
          getRegister(argl[2], _errors, _line),
          oc.code,
          getRegister(argl[0], _errors, _line),
          isRegister(argl[1]) ? 0 : 1,
          0,
          isRegister(argl[1]) ? getRegister(argl[1], _errors, _line) : toNum(argl[1], _errors, _line)
          ));
  } else if (opcode == "rd") {
    string reg = argl[0].substr(1, argl[0].size());
    uint32_t op3;
    if (reg == "y")
      op3 = INST_OP3_RDY;
    else if (reg == "psr")
      op3 = INST_OP3_RDPSR;
    else if (reg == "wim")
      op3 = INST_OP3_RDWIM;
    else if (reg == "tbr")
      op3 = INST_OP3_RDTBR;
    else {
      op3 = 0;
      _errors.push_back(ASMError::InvalidSpecialRegisterNameError(reg, _line));
    }
    instructions.push_back(Instruction::makeInstruction(
          INST_OP_OTHER,
          getRegister(argl[1], _errors, _line),
          op3,
          0,
          1,
          0,
          0
          ));
  } else if (opcode == "wr") {
    string reg = argl[2].substr(1, argl[2].size());
    uint32_t op3;
    if (reg == "y")
      op3 = INST_OP3_WRY;
    else if (reg == "psr")
      op3 = INST_OP3_WRPSR;
    else if (reg == "wim")
      op3 = INST_OP3_WRWIM;
    else if (reg == "tbr")
      op3 = INST_OP3_WRTBR;
    else {
      op3 = 0;
      _errors.push_back(ASMError::InvalidSpecialRegisterNameError(reg, _line));
    }
    instructions.push_back(Instruction::makeInstruction(
          INST_OP_OTHER,
          0,
          op3,
          getRegister(argl[0], _errors, _line),
          isRegister(argl[1]) ? 0 : 1,
          0,
          isRegister(argl[1]) ? getRegister(argl[1], _errors, _line) : toNum(argl[1], _errors, _line)
          ));
  } else if (opcode == "jmpl") {
    uint32_t s1, s2, i;
    parseAddress("[" + argl[0] + "]", &s1, &s2, &i, _errors, _line);
    instructions.push_back(Instruction::makeInstruction(
          INST_OP_OTHER,
          getRegister(argl[1], _errors, _line),
          INST_OP3_JMPL,
          s1,
          i,
          0,
          s2
          ));
  } else if (opcode == "save" || opcode == "restore") {
    uint32_t rd, s1, s2, i;
    if (argl.size() == 0) {
      rd = 0;
      s1 = 0;
      s2 = 0;
      i = 1;
    } else {
      rd = getRegister(argl[2], _errors, _line);
      s1 = getRegister(argl[0], _errors, _line);
      i = isRegister(argl[1]) ? 0 : 1;
      s2 = i == 0 ? getRegister(argl[1], _errors, _line) : toNum(argl[1], _errors, _line); 
    }
    instructions.push_back(Instruction::makeInstruction(
          INST_OP_OTHER,
          rd,
          (opcode == "save" ? INST_OP3_SAVE : INST_OP3_REST),
          s1,
          i,
          0,
          s2
          ));
  }
  // Menomnics
  else if (opcode == "cmp") {
    instructions.push_back(Instruction::makeInstruction(
          INST_OP_OTHER,
          0,
          UAL_OP_SUBcc,
          getRegister(argl[0], _errors, _line),
          isRegister(argl[1]) ? 0 : 1,
          0,
          isRegister(argl[1]) ? getRegister(argl[1], _errors, _line) : toNum(argl[1], _errors, _line)
          ));
  } else if (opcode == "jmp") {
    uint32_t s1, s2, i;
    parseAddress("[" + argl[0] + "]", &s1, &s2, &i, _errors, _line);
    instructions.push_back(Instruction::makeInstruction(
          INST_OP_OTHER,
          0,
          INST_OP3_JMPL,
          s1,
          i,
          0,
          s2
          ));
  } else if (opcode == "tst") {
    instructions.push_back(Instruction::makeInstruction(
          INST_OP_OTHER,
          0,
          UAL_OP_ORcc,
          getRegister(argl[0], _errors, _line),
          1,
          0,
          0
          ));
  } else if (opcode == "ret") {
    instructions.push_back(Instruction::makeInstruction(
          INST_OP_OTHER,
          0,
          INST_OP3_JMPL,
          getRegister("%o7", _errors, _line),
          1,
          0,
          1
          ));
  } else if (opcode == "retl") {
    instructions.push_back(Instruction::makeInstruction(
          INST_OP_OTHER,
          0,
          INST_OP3_JMPL,
          getRegister("%o7", _errors, _line),
          1,
          0,
          0
          ));
  } else if (opcode == "set") {
    uint32_t reg = getRegister(argl[1], _errors, _line), value = toNum(argl[0], _errors, _line);
    instructions.push_back(Instruction::makeInstruction(
          INST_OP_BR,
          reg,
          INST_OP2_SETHI,
          value >> 10
          ));
    instructions.push_back(Instruction::makeInstruction(
          INST_OP_OTHER,
          reg,
          UAL_OP_OR,
          reg,
          1,
          0,
          value & 0x000003FF
          ));
  } else if (opcode == "not") {
    uint32_t rd, rs;
    if (argl.size() == 1) {
      rd = getRegister(argl[0], _errors, _line);
      rs = rd;
    } else {
      rd = getRegister(argl[1], _errors, _line);
      rs = getRegister(argl[0], _errors, _line);
    }
    instructions.push_back(Instruction::makeInstruction(
          INST_OP_OTHER,
          rd,
          UAL_OP_XNOR,
          rs,
          1,
          0,
          0
          ));
  } else if (opcode == "neg") {
    uint32_t rd, rs;
    if (argl.size() == 1) {
      rd = getRegister(argl[0], _errors, _line);
      rs = rd;
    } else {
      rd = getRegister(argl[1], _errors, _line);
      rs = getRegister(argl[0], _errors, _line);
    }
    instructions.push_back(Instruction::makeInstruction(
          INST_OP_OTHER,
          rd,
          UAL_OP_SUB,
          0,
          0,
          0,
          rs
          ));
  } else if (opcode == "inc") {
    instructions.push_back(Instruction::makeInstruction(
          INST_OP_OTHER,
          getRegister(argl[0], _errors, _line),
          UAL_OP_ADD,
          getRegister(argl[0], _errors, _line),
          1,
          0,
          argl.size() > 1 ? toNum(argl[1], _errors, _line) : 1
          ));
  } else if (opcode == "inccc") {
    instructions.push_back(Instruction::makeInstruction(
          INST_OP_OTHER,
          getRegister(argl[0], _errors, _line),
          UAL_OP_ADDcc,
          getRegister(argl[0], _errors, _line),
          1,
          0,
          argl.size() > 1 ? toNum(argl[1], _errors, _line) : 1
          ));
  } else if (opcode == "dec") {
    instructions.push_back(Instruction::makeInstruction(
          INST_OP_OTHER,
          getRegister(argl[0], _errors, _line),
          UAL_OP_SUB,
          getRegister(argl[0], _errors, _line),
          1,
          0,
          argl.size() > 1 ? toNum(argl[1], _errors, _line) : 1
          ));
  } else if (opcode == "deccc") {
    instructions.push_back(Instruction::makeInstruction(
          INST_OP_OTHER,
          getRegister(argl[0], _errors, _line),
          UAL_OP_SUBcc,
          getRegister(argl[0], _errors, _line),
          1,
          0,
          argl.size() > 1 ? toNum(argl[1], _errors, _line) : 1
          ));
  } else if (opcode == "mov") {
    instructions.push_back(Instruction::makeInstruction(
          INST_OP_OTHER,
          getRegister(argl[1], _errors, _line),
          UAL_OP_OR,
          0,
          (isRegister(argl[0]) ? 0 : 1),
          0,
          (isRegister(argl[0]) ? getRegister(argl[0], _errors, _line) : toNum(argl[0], _errors, _line))
          ));
  }

  // End
  return instructions;
}



