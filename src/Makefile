# Makefile for project kSPARC
#
# kSparc is a SPARC architecture processor emulator create for debugging, educationnal and
# testing purposes.
#
# kSparc consists in three main tools :
# 	kasm			a command line kSparc assembler
# 	kdisasm		a instruction by instructionkSparc disassembler
# 	ksparc		the emulator in console GUI
#
# This file is the Makefile for all the tools
#
# Author: krab
# Version: 0.1
#

### General variables ###
# Compiler and linker
CXX=g++
LD=g++

# Compiling and linking flags
CXXFLAGS=--std=c++11 -D_DEBUG
LDFLAGS=

# Library for ksparc
LIBS=-lncurses

### Objects and target ###
OUTPUTDIR=../bin
OBJDIR=../obj

# Objects common to every targets
BASEOBJECTS=$(addprefix $(OBJDIR)/, instruction.o utils.o logger.o)

# kasm : the assembler
KASM=$(OUTPUTDIR)/kasm
KASMOBJECTS=$(addprefix $(OBJDIR)/, kasmmain.o assembler.o asmerror.o)

# kdisasm : the disassembler
KDISASM=$(OUTPUTDIR)/kdisasm
KDISASMOBJECTS=$(addprefix $(OBJDIR)/, kdisasmmain.o disassembler.o)

# ksparc the emulator
KSPARC=$(OUTPUTDIR)/ksparc
KSPARCOBJECTS=$(addprefix $(OBJDIR)/, ksparcmain.o register.o specialregister.o windowregisters.o abstractmemory.o simplememory.o abstractalu.o simplealu.o abstractsparcengine.o sparcengine.o disassembler.o)

# Targets
TARGETS=$(KSPARC) $(KASM) $(KDISASM)

### Production rules ###
all: $(OBJDIR) $(OUTPUTDIR) $(TARGETS)

$(KSPARC): $(BASEOBJECTS) $(KSPARCOBJECTS)
	$(LD) $(LDFLAGS) -o $(KSPARC) $(BASEOBJECTS) $(KSPARCOBJECTS) $(LIBS)

$(KASM): $(BASEOBJECTS) $(KASMOBJECTS)
	$(LD) $(LDFLAGS) -o $(KASM) $(BASEOBJECTS) $(KASMOBJECTS)

$(KDISASM): $(BASEOBJECTS) $(KDISASMOBJECTS)
	$(LD) $(LDFLAGS) -o $(KDISASM) $(BASEOBJECTS) $(KDISASMOBJECTS)

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OUTPUTDIR):
	@mkdir -p $@

$(OBJDIR):
	@mkdir -p $@

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(KASM)
	rm -f $(KSPARC)
	rm -f $(KDISASM)



