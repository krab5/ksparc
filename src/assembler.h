/*
 * assembler.h -- define an assembler
 * ----
 * Author: krab
 * Version: 0.1
 */
#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <vector>
#include <fstream>
#include <iomanip>
#include <string>
#include <map>

#include "instruction.h"
#include "asmerror.h"

/**
 * This class defines an assembler made for reading kasm code and convert it to ksparc compatible instructions.
 *
 * The assembly is made in two steps : first, we read the file to find labels (that will be called by branches, jmpl and so on) and then we read each opcodes one by one and build binary codes from them.
 */
class Assembler {
	public:
    typedef std::vector<Instruction> InstructionList;

    /**
     * Constructor
     */
		Assembler();
    /**
     * Destructor
     */
		~Assembler();

    /**
     * Get the current instruction offset.
     * Instruction offset is used for calculating branches displacements. Sometimes, we need to set it (e.g.: if we read several files)
     * @returns the offset
     */
    uint32_t getInstructionOffset() const;
    /**
     * Set the current instruction offset
     * @param io new instruction offset
     * @see getInstructionOffset()
     */
    void setInstructionOffset(uint32_t io);

    /**
     * Read a line of kasm code and transform it into a list of isntructions.
     * The only case where the code is converted into several instructions is with the SET mnemonics, which does a sethi and a or
     * @param str instruction to parse
     * @returns the list of instruction corresponding to the line
     */
    InstructionList read(std::string instr);
    /**
     * Manually add a label to the label database
     * @param label label's name
     * @param instaddr address pointed by the label
     */
    void addLabel(std::string label, uint32_t instaddr);
    /**
     * Parse a line to detect and add (if needed) a label.
     * Labels are prefix to the code and takes the form of a non-space string terminated by a colon (for example : "begin:").
     * Labels are typically used in calls, branch and other delayed control transfer instructions (DCTI).
     */
    void parseForLabels(std::string instr);

    /**
     * Read a list of lines and transform it into a list of instructions.
     * One should be warned that if there has been error, the resulting instruction cannot be right and should not be used.
     * @param insts list of lines to transform
     * @param ioffset instruction offset : where to start for instruction address
     * @param loffset line offset : where to start for lines (useful for error reporting)
     * @returns the converted instruction list
     */
    InstructionList readAll(std::vector<std::string> insts, uint32_t ioffset = 0, size_t loffset = 1);
    /**
     * Read a list of lines and create the label database.
     * @param insts list of lines to parse
     * @param ioffset instruction offset
     * @param loffset line offset
     */
    void parseForLabels(std::vector<std::string> insts, uint32_t ioffset = 0, size_t loffset = 1);

    /**
     * Indicate that there are errors
     * @returns true if there are errors
     */
    bool hasErrors();
    /**
     * List of errors encountered by the program
     * @returns a list of ASMError::Error objects
     */
    ASMError::ErrorList errors();
    /**
     * Indicate that there are warnings
     * @returns true if there are warnings
     */
    bool hasWarnings();
    /**
     * List of warnings raised by the program
     * @returns a list of ASMError::Warning objects
     */
    ASMError::WarningList warnings();


	private:
    /**
     * The instruction address : this count the number of instructions and is used when storing labels and calculating displacement for DCTI
     */
    uint32_t _instAddr;
    /**
     * The current line being read. This is used for error reporting
     */
    size_t _line;
    /**
     * The label database. Each label is stored with its instruction adress for calculating the displacement
     */
    std::map<std::string, uint32_t> _labels;
    /**
     * List of errors
     */
    ASMError::ErrorList _errors;
    /**
     * List of warnings
     */
    ASMError::WarningList _warnings;
};

#endif // ASSEMBLER_H

