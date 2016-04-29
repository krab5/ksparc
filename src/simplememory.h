/*
 * simplememory.h -- a simple memory device
 */
#ifndef SIMPLEMEMORY_H
#define SIMPLEMEMORY_H

#include "abstractmemory.h"

/**
 * This class is a simple memory device for use with the kSPARC engine.
 * It is not reallistic as every access is done in 1 cycle and there is no alignment verification.
 * This is a good base though
 */
class SimpleMemory : public AbstractMemory {
	public:
    /**
     * Constructor
     * @param size size of the memory device
     */
		SimpleMemory(uint32_t size);
    /**
     * Destructor
     */
		~SimpleMemory();

    /**
     * Read function
     * @see AbstractMemory::read()
     */
    void read(uint32_t address, uint32_t size, uint8_t* data) const;
    /**
     * Write function
     * @see AbstractMemory::write()
     */
    void write(uint32_t address, uint8_t* data, uint32_t size);

	private:
    uint8_t* _content;
};

#endif // SIMPLEMEMORY_H

