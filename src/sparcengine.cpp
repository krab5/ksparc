/*
 * sparcengine.cpp -- implement the SparcEngine class
 */
#include "sparcengine.h"

// Cstr
SparcEngine::SparcEngine(
    AbstractMemory* mem,
    AbstractALU* alu,
    /*AbstractFPU* fpu,*/
    WindowRegisters* registers,
    SpecialRegister* psr,
    SpecialRegister* wim,
    SpecialRegister* tbr,
    SpecialRegister* y,
    SpecialRegister* pc,
    SpecialRegister* npc,
    SpecialRegister* fsr
) : AbstractSparcEngine(mem, alu/*, fpu*/, registers, psr, wim, tbr, y, pc, npc, fsr) {
}

// Dstr
SparcEngine::~SparcEngine() {
}

// (Re-)initialize the engine
void SparcEngine::init() {
  psr()->setField(PSR_IMPL, SE_IMPL); // impl 1
  psr()->setField(PSR_VERS, SE_VERS); // vers 1
  psr()->setField(PSR_ICC, 0);
  psr()->setField(PSR_EC, 0);         // coproc disabled
  psr()->setField(PSR_EF, 0);         // fpu disabled
  psr()->setField(PSR_PIL, 0);
  psr()->setField(PSR_S, 0);          // supervisor disabled
  psr()->setField(PSR_PS, 0);
  psr()->setField(PSR_ET, 0);         // traps disabled
  psr()->setField(PSR_CWP, 0);        // current windows : 0

  wim()->write(0);
  tbr()->write(SE_TRAPS_BASE_ADDR);
  pc()->write(0xFFFFFFFF);
  npc()->write(0);

  // TODO: initialize fsr
 
  _branch = false;
  _isdcti = false;
}

// Execute next instruction
bool SparcEngine::next() {
  // Position the program counter
  pc()->write(npc()->read());
  
  // Read the instruction
  Instruction inst = memory()->readInstruction(pc()->read());

  // This special instruction (which correspond to "cbn 0x00000000" is simply ignored, as it is a basic state of the memory
  if (inst.getContent() == 0x00000000)
    return true;

  // Get the main operator
  uint32_t op = inst.getField(INST_OP);

  if (op == INST_OP_BR) {
    // Branches and SETHI
    uint32_t op2 = inst.getField(INST_OP2);

    if (op2 == INST_OP2_SETHI) {
      // Set high
      uint32_t rd = inst.getField(INST_RD);
      registers()->write(rd, 0);
      registers()->write(rd, inst.getField(INST_IMM22) << 10);
    } else if (op2 == INST_OP2_BICC) {
      // Branches
      uint32_t rawcond = inst.getField(INST_COND);
      bool neg = (rawcond >> 3) == 1;
      uint8_t cond = ((uint8_t)rawcond) & 0x07;
      bool a = inst.getField(INST_A) == 1;
      _dcti = pc()->read() + (signext(inst.getField(INST_DISP22), 22) << 2);
      Logger::log() << "dcti = " << pc()->read() << " - " << COMPL32((signext(inst.getField(INST_DISP22), 22) << 2)) << "\n";
      bool Z = (psr()->getField(PSR_ICC_Z) == 1),
           N = (psr()->getField(PSR_ICC_N) == 1),
           C = (psr()->getField(PSR_ICC_C) == 1),
           V = (psr()->getField(PSR_ICC_V) == 1);

      Logger::log() << "Branch ! Z=" << Z << ";N=" << N << ";C=" << C << ";V=" << V << "\n";

      // Calculate if we branch
      switch (cond) {
        case INST_COND_NEVER:
          _branch = false;
          break;
        case INST_COND_EQ:
          _branch = Z;
          break;
        case INST_COND_LET:
          _branch = Z || (N ^ V);
          break;
        case INST_COND_LT:
          _branch = N ^ V;
          break;
        case INST_COND_ULET:
          _branch = C || Z;
          break;
        case INST_COND_CSET:
          _branch = C;
          break;
        case INST_COND_NEG:
          _branch = N;
          break;
        case INST_COND_OSET:
          _branch = V;
          break;
        default:
          _dcti = 0;
          //nop
      }

      // adjust
      if (neg) _branch = !_branch;
      Logger::log() << "Will we branch ? " << (_branch ? "yes" : "no") << "\n";

      // Calculate if we need to DCTI
      _isdcti = (!a) || (_branch && !(cond == INST_COND_NEVER));

      /*if (_isdcti)
        _dcti -= 1;*/
      Logger::log() << "Will we dcti ? " << (_isdcti ? "yes" : "no") << "\n";

      Logger::log() << "Where will we branch ? " << _dcti << "\n";
    } else if (op2 == INST_OP2_FBFCC) {
      // unimplemented yet
    } else if (op2 == INST_OP2_CBCCC) {
      // unimplemented yet
    } else {
      // nop (or trap ?)
    }
  } else if (op == INST_OP_CALL) {
    // CALL
    _dcti = pc()->read() + (inst.getField(INST_DISP30) << 2);
    registers()->write(15, pc()->read() >> 2);
    _branch = true;
    _isdcti = false;
  } else if (op == INST_OP_OTHER) {
    // Arith, Logics, others
    uint32_t op3 = inst.getField(INST_OP3);
    uint32_t rd = inst.getField(INST_RD);
    uint32_t rs1 = inst.getField(INST_RS1);

    switch (op3) {
      // Read special registers
      case INST_OP3_RDY:
        registers()->write(rd, (!READING_PRIVILEGE | isSupervisor() ? y()->read() : 0));
        break;
      case INST_OP3_RDPSR:
        registers()->write(rd, (!READING_PRIVILEGE | isSupervisor() ? psr()->read() : 0));
        break;
      case INST_OP3_RDWIM:
        registers()->write(rd, (!READING_PRIVILEGE | isSupervisor() ? wim()->read() : 0));
        break;
      case INST_OP3_RDTBR:
        registers()->write(rd, (!READING_PRIVILEGE | isSupervisor() ? tbr()->read() : 0));
        break;
      // Write special registers
      case INST_OP3_WRY:
        if (isSupervisor())
          y()->write(registers()->read(rs1));
        break;
      case INST_OP3_WRPSR:
        if (isSupervisor())
          psr()->write(registers()->read(rs1));
        break;
      case INST_OP3_WRWIM:
        if (isSupervisor())
          wim()->write(registers()->read(rs1));
        break;
      case INST_OP3_WRTBR:
        if (isSupervisor())
          tbr()->write(registers()->read(rs1));
        break;
      // External instructions
      case INST_OP3_FPOP1:
      case INST_OP3_FPOP2:
      case INST_OP3_CPOP1:
      case INST_OP3_CPOP2:
        // Unimplemented yet
        break;
      // Jump and link
      case INST_OP3_JMPL:
        _dcti = registers()->read(rs1);
        if (inst.getField(INST_I) == 0)
          _dcti += registers()->read(inst.getField(INST_RS2));
        else
          _dcti += signext(inst.getField(INST_SIMM13), 13);
        _dcti = _dcti << 2;
        registers()->write(rd, pc()->read() >> 2);
        _isdcti = false;
        _branch = true;
        break;
      // Return from trap
      case INST_OP3_RETT:
        registers()->restore();
        break;
      // Trap if condition code
      case INST_OP3_TICC:
        break;
      // Flush
      case INST_OP3_FLUSH:
        break;
      // Save context
      case INST_OP3_SAVE: {
          Register* r1 = registers()->get(rs1);
          Register* r2 = (inst.getField(INST_I) == 0 ? registers()->get(inst.getField(INST_RS2)) : NULL);
          registers()->save();
        
          if (r2 == NULL)
            alu()->calc(ALU_OP_ADD, r1, inst.getField(INST_SIMM13), registers()->get(rd));
          else
            alu()->calc(ALU_OP_ADD, r1, r2, registers()->get(rd));
        }
        break;
      // Restore context
      case INST_OP3_REST:{
          Register* r1 = registers()->get(rs1);
          Register* r2 = (inst.getField(INST_I) == 0 ? registers()->get(inst.getField(INST_RS2)) : NULL);
          registers()->restore();
        
          if (r2 == NULL)
            alu()->calc(ALU_OP_ADD, r1, inst.getField(INST_SIMM13), registers()->get(rd));
          else
            alu()->calc(ALU_OP_ADD, r1, r2, registers()->get(rd));
        }
        break;
      default:
        if (inst.getField(INST_I) == 0) {
          // use register as src 2
          alu()->calc(op3,
              registers()->get(rs1),
              registers()->get(inst.getField(INST_RS2)),
              registers()->get(rd));
        } else {
          // use sign extension of simm13 (13-bit signed int) as src 2
          uint32_t extsimm = signext(inst.getField(INST_SIMM13), 13);
          alu()->calc(op3,
              registers()->get(rs1),
              extsimm,
              registers()->get(rd));
        }
    }
  } else {
    // Memory related insts
    uint32_t op3 = inst.getField(INST_OP3);
    uint32_t addr = registers()->read(inst.getField(INST_RS1));
    if (inst.getField(INST_I) == 0)
      addr += registers()->read(inst.getField(INST_RS2));
    else
      addr += signext(inst.getField(INST_SIMM13), 13);
    uint32_t rd = inst.getField(INST_RD);

    switch (op3) {
      // Load instruction
      case INST_OP3_LDSB:
        registers()->write(rd, signext(memory()->readByte(addr), 8));
        break;
      case INST_OP3_LDSH:
        registers()->write(rd, signext(memory()->readHalfword(addr), 16));
        break;
      case INST_OP3_LDUB:
        registers()->write(rd, memory()->readByte(addr));
        break;
      case INST_OP3_LDUH:
        registers()->write(rd, memory()->readHalfword(addr));
        break;
      case INST_OP3_LD:
        registers()->write(rd, memory()->readWord(addr));
        break;
      case INST_OP3_LDD:
        if (rd % 2 != 0) {
          // pb : rd is odd; we cannot write a double word in it !
          // trap ?
          registers()->write(rd, 0);
        } else {
          memory()->readDoubleword(addr, registers()->get(rd), registers()->get(rd+1));
        }
        break;
      // Store instruction
      case INST_OP3_STB:
        memory()->writeByte(addr, registers()->get(rd));
        break;
      case INST_OP3_STH:
        memory()->writeHalfword(addr, registers()->get(rd));
        break;
      case INST_OP3_ST:
        memory()->writeWord(addr, registers()->get(rd));
        break;
      case INST_OP3_STD:
        if (rd % 2 != 0) {
          // pb !
        } else {
          memory()->writeDoubleword(addr, registers()->get(rd), registers()->get(rd+1));
        }
        break;
      //default:
        // unknown load/store instruction
        // nop
    }
  } 

  // Position the next pc
  if (_branch) {
    if (_isdcti) {
      Logger::log("Executing DCTI...");
      alu()->calc(ALU_OP_ADD, pc(), 4, npc());
      _isdcti = false;
    } else {
      Logger::log("Execution Branch !");
      npc()->write(_dcti);
      _branch = false;
    }
  } else {
    alu()->calc(ALU_OP_ADD, pc(), 4, npc());
  }
  Logger::log() << "New nPC calculated : " << std::hex << npc()->read() << std::endl; 

  return true;
}

// Are we supervisor ?
bool SparcEngine::isSupervisor() {
  return psr()->getField(PSR_S) == 1;
}



