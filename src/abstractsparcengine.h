/*
 * abstractsparcengine.h -- abstract class that defines a SPARC emulation engine
 * -------
 * Author: krab
 * Version: 0.1
 */
#ifndef ABSTRACTSPARCENGINE_H
#define ABSTRACTSPARCENGINE_H

#include "abstractmemory.h"
#include "abstractalu.h"
//#include "abstractfpu.h"

#include "register.h"
#include "windowregisters.h"
#include "specialregister.h"

/**
 * Represents an abstract SPARC engine.
 *
 * This is the core class for emulating SPARC; it defines all the important function
 * to emulate a SPARC processor.
 *
 * This engine is made of 4 main components :
 * - the memory, for storing and loading data
 * - the arithmetic and logic unit, for making calculus
 * - the window registers, which are the main registers of the unit
 * - a whole set of "special" registers that stores various informations about the state of the device
 *
 * @see AbstractMemory
 * @see AbstractALU
 * @see WindowRegister
 * @see SpecialRegister
 */
class AbstractSparcEngine {
	public:
    /**
     * Constructor
     *
     * @param mem the memory device
     * @param alu the arithmetic and logic unit
     * @param registers main registers of the engine
     * @param psr processor state register
     * @param wim window invalid mask register
     * @param tbr trap base register
     * @param y multiplication and division auxiliary register
     * @param pc program counter register
     * @param npc next program counter register
     * @param fsr FPU state register
     */
		AbstractSparcEngine(
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
		~AbstractSparcEngine();
    
    /**
     * (Re-)initialize the sparc engine, setting registers, etc.
     */
    virtual void init() = 0;

    /**
     * Do a cycle : this usually means
     * - reading the instruction from the memory at adress given by the pc register
     * - decoding the instruction
     * - executing the instruction
     * - update registers and memory
     */
    virtual bool next() = 0;

	protected:
    /**
     * Get the memory device of the engine
     * @returns the memory device
     */
    AbstractMemory* memory();
    /**
     * Get the alu device of the engine
     * @returns the alu device
     */
    AbstractALU* alu();
    //AbstractFPU* fpu();
    /**
     * Get the window register of the engine
     * @returns the window register
     */
    WindowRegisters* registers();
    /**
     * Get the psr register of the engine
     * @returns the psr register
     */
    SpecialRegister* psr();
    /**
     * Get the wim register of the engine
     * @returns the wim register
     */
    SpecialRegister* wim();
    /**
     * Get the tbr register of the engine
     * @returns the tbr register
     */
    SpecialRegister* tbr();
    /**
     * Get the y register of the engine
     * @returns the y register
     */
    SpecialRegister* y();
    /**
     * Get the pc register of the engine
     * @returns the pc register
     */
    SpecialRegister* pc();
    /**
     * Get the npc register of the engine
     * @returns the npc register
     */
    SpecialRegister* npc();
    /**
     * Get the fsr register of the engine
     * @returns the fsr register
     */
    SpecialRegister* fsr();


	private:
    /**
     * The memory device of the engine
     */
    AbstractMemory* _mem;

    /**
     * The ALU device of the engine
     */
    AbstractALU* _alu;
    //AbstractFPU* _fpu;
    
    /**
     * The window register of the engine
     */
    WindowRegisters* _reg;

    /**
     * The special registers of the device
     */
    SpecialRegister *_psr, *_wim, *_tbr, *_y, *_pc, *_npc, *_fsr;

};

#endif // ABSTRACTSPARCENGINE_H

