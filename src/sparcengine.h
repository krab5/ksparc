/*
 * sparcengine.h -- defines a simple sparc engine
 */
#ifndef SPARCENGINE_H
#define SPARCENGINE_H

#include "abstractsparcengine.h"

// Implementation and version of the engine
#define SE_IMPL 0x01
#define SE_VERS 0x01

// Value of the TBR
#define SE_TRAPS_BASE_ADDR  0x00000000

// do we need privilege to read internal registers ?
#define READING_PRIVILEGE true

/**
 * This defines a really simple sparc enfine, sufficient for most of the application we could do with.
 * There is no system of cycle, every instruction is executed one following the other, no pipeline, no trap, no FPU, etc.
 */
class SparcEngine : public AbstractSparcEngine {
	public:
    /**
     * Constructor.
     * @see AbstractSparcEngine::AbstractSparcEngine()
     */
		SparcEngine(
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
    );
    /**
     * Destructor
     */
		~SparcEngine();

    /**
     * (Re-)initialize sparc engine
     * @see AbstractSparcEngine::init()
     */
    void init();
    /**
     * Execute next instruction
     * @see AbstractSparcEngine::next()
     */
    bool next();

	protected:
    /**
     * Determines if the CPU is in supervisor mode
     */
    bool isSupervisor();

	private:
    /**
     * This attribute is set to true when a branch as been encountered and taken.
     * When this is the case, at the next instruction, we execute the branch (if there is no DCTI) or we execute the DCTI and then, at the next instruction we execute the branch (and set the attribute to false).
     */
    bool _branch;
    /**
     * Determines if we must execute the delayed constrol transfer instruction; that is, the instruction right after the branch/jmpl/call/etc.
     * In general, the DCTI is executed only if :
     * - a branch is responsible for delayed transfer
     * - the annulment bit of this branch is set to false
     * - the annulment bit of this branch is set to true AND the branch is conditionnal and has been taken
     * In other words, a unconditionnal branch (branch always or branch never) always executes the dcti only if the annulment bit equals to 0; a conditionnal branch executes it when its annulment bit equals to 0 or if the condition is true.
     */
    bool _isdcti;
    /**
     * When there is a branch, indicate the next instruction (and maybe the next, next instruction, if we must execute dcti).
     */
    uint32_t _dcti;

};

#endif // SPARCENGINE_H

