/*
 * windowregisters.h -- defines the WindowRegisters class
 */
#ifndef WINDOWREGISTERS_H
#define WINDOWREGISTERS_H

#include "specialregister.h"
#include "register.h"

// Number of register of each type
#define NREGIO      8 // Input/output
#define NREGLOC     8 // Local
#define NREGGLOB    8 // Global

// Transform a specific register into a general register (e.g: %i6 => %r30)
#define REG_GLOB(n) n
#define REG_IN(n)   n+NREGIO+NREGLOC+NREGGLOB
#define REG_OUT(n)  n+NREGGLOB
#define REG_LOC(n)  n+NREGGLOB+NREGIO

/**
 * This class defines the general windowed register system of a SPARC architecture.
 * Basically, each context can access 4 types of registers : globals, which are the same for every context; locals, which are specific to each context and input and outputs, which are shared from adjacent context.
 * Indeed, the inputs of context N is the outputs of context N+1, which makes easy argument and result transfer between context.
 * The context number is given by the CWP field of the PSR. When CWPmax is reached and we increment it (or when 0 is reached and we decrement it), the WIM register is positionned to indicate the incriminated register that caused an over/underflow.
 */
class WindowRegisters {
	public:
    /**
     * Constructor
     * @param wsize size of the window
     * @param psr processor state register
     * @param wim window invalid mask register
     */
		WindowRegisters(const uint32_t wsize, SpecialRegister* psr, SpecialRegister* wim);
    /**
     * Destructor
     */
		~WindowRegisters();

    /**
     * Get the window size
     * @returns window size
     */
    uint32_t getWindowSize() const;

    /**
     * Get a pointer to a register, depending on the context
     * @param nb number of the register to access
     * @returns pointer to the requested register
     */
    Register* get(uint32_t nb);

    /**
     * Read from a register, depending on the context
     * @param nb number of the register to access
     * @returns value of the requested register
     */
    uint32_t read(uint32_t nb) const;
    /**
     * Write to a register, depending on the context
     * @param nb number of the register to access
     * @param data data to write to the register
     */
    void write(uint32_t nb, uint32_t data);
    /**
     * Save context. This basically means CWP++
     */
    void save();
    /**
     * Restore context. This basically means CWP--
     */
    void restore();

	private:
    const uint32_t _wsize;
    Register* _registers;
    SpecialRegister *_psr, *_wim;
};

#endif // WINDOWREGISTERS_H

