// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#include <new>

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable)
{
    NoffHeader noffH;
    unsigned int size;
#ifndef USE_TLB
    unsigned int i;
#endif

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);
#ifndef USE_TLB
// first, set up the translation 
    pageTable = new(std::nothrow) TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
	pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
#ifndef CHANGED
	pageTable[i].physicalPage = i;
#else
	pageTable[i].physicalPage = memoryManager->NewPage();
#endif
	pageTable[i].valid = true;
	pageTable[i].use = false;
	pageTable[i].dirty = false;
	pageTable[i].readOnly = false;  // if the code segment was entirely on 
					// a separate page, we could set its 
					// pages to be read-only
    }
#endif    

// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment
#ifndef CHANGED
    bzero(machine->mainMemory, size);
#else
		for (i = 0; i < numPages; i++) {
			bzero(&(machine->mainMemory[GetPhysPageNum(i)*PageSize]), PageSize);
		}
#endif

// then, copy in the code and data segments into memory
    if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
			noffH.code.virtualAddr, noffH.code.size);
#ifndef CHANGED
        executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]),
			noffH.code.size, noffH.code.inFileAddr);
#else
			//TODO: Maybe clean this up a little
			for (i=(noffH.code.virtualAddr % PageSize); i < (unsigned int)noffH.code.size; i+=(PageSize - (i % PageSize))) {
				int physAddress = GetPhysAddress((noffH.code.virtualAddr + i) - (noffH.code.virtualAddr % PageSize));
				unsigned int writtenSoFar = i - (noffH.code.virtualAddr % PageSize);
				int location = noffH.code.inFileAddr + writtenSoFar;
				/*fprintf(stderr, "i = %d\n", i);
				fprintf(stderr, "phys = %d\n", physAddress);
				fprintf(stderr, "writtenSoFar = %d\n", writtenSoFar);
				fprintf(stderr, "location = %d\n", location);*/

				//not at beginning of page
				if (i % PageSize != 0) { //more data than can fit on page
					if ((noffH.code.size - writtenSoFar) > (PageSize - (i % PageSize))) {
						executable->ReadAt(&(machine->mainMemory[physAddress]),
								(PageSize - (i % PageSize)), location);
					} else {	//all data can fit on current page
						executable->ReadAt(&(machine->mainMemory[physAddress]),
								(noffH.code.size - writtenSoFar), location);
					}

				} else { //starting at beginning of page
					if ((noffH.code.size - writtenSoFar) > PageSize) {	//more data than can fit on page
						executable->ReadAt(&(machine->mainMemory[physAddress]),
								PageSize, (noffH.code.inFileAddr + writtenSoFar));
					} else {	//all data can fit on current page
						executable->ReadAt(&(machine->mainMemory[physAddress]),
								(noffH.code.size - writtenSoFar), location);
					}
				}
			}
#endif
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", 
			noffH.initData.virtualAddr, noffH.initData.size);
#ifndef CHANGED
        executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]),
			noffH.initData.size, noffH.initData.inFileAddr);
#else
			//TODO: Maybe clean this up a little
			for (i=(noffH.initData.virtualAddr % PageSize); i < (unsigned int)noffH.initData.size; i+=(PageSize - (i % PageSize))) {
				int physAddress = GetPhysAddress((noffH.initData.virtualAddr + i) - (noffH.initData.virtualAddr % PageSize));
				unsigned int writtenSoFar = i - (noffH.initData.virtualAddr % PageSize);
				int location = noffH.initData.inFileAddr + writtenSoFar;
				/*fprintf(stderr, "i = %d\n", i);
				fprintf(stderr, "phys = %d\n", physAddress);
				fprintf(stderr, "writtenSoFar = %d\n", writtenSoFar);
				fprintf(stderr, "location = %d\n", location);*/

				//not at beginning of page
				if (i % PageSize != 0) { //more data than can fit on page
					if ((noffH.initData.size - writtenSoFar) > (PageSize - (i % PageSize))) {
						executable->ReadAt(&(machine->mainMemory[physAddress]),
								(PageSize - (i % PageSize)), location);
					} else {	//all data can fit on current page
						executable->ReadAt(&(machine->mainMemory[physAddress]),
								(noffH.initData.size - writtenSoFar), location);
					}

				} else { //starting at beginning of page
					if ((noffH.initData.size - writtenSoFar) > PageSize) {	//more data than can fit on page
						executable->ReadAt(&(machine->mainMemory[physAddress]),
								PageSize, location);
					} else {	//all data can fit on current page
						executable->ReadAt(&(machine->mainMemory[physAddress]),
								(noffH.initData.size - writtenSoFar), location);
					}
				}
			}
#endif
    }

#ifdef CHANGED
	for (std::size_t x=0; x < (sizeof(fdArray)/sizeof(fdArray[0])); x++) {
		if (x == 0 || x == 1) {
			fdArray[x] = (OpenFile *)1;	//could be a bad idea not sure
		} else {
			fdArray[x] = NULL;	//initialize fd array
		}
	}
#endif

}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
#ifndef USE_TLB
   delete pageTable;
#endif
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table,
//      IF address translation is done with a page table instead
//      of a hardware TLB.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
#ifndef USE_TLB
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
#endif
}

#ifdef CHANGED
int
AddrSpace::AddFD(OpenFile *file)
{
	int fd = -1;
	int size = sizeof(fdArray)/sizeof(fdArray[0]);
	for (int i=0; i < size; i++) {
		if (fdArray[i] == NULL) {
			fdArray[i] = file;
			fd = i;
			break;
		}
	}
	return fd;
}

OpenFile*
AddrSpace::GetFile(int fd)
{
	OpenFile *file = NULL;
	int size = sizeof(fdArray)/sizeof(fdArray[0]);
	if (fd < size) {
		file = fdArray[fd];
	}

	return file;
}

int
AddrSpace::DeleteFD(int fd)
{
	OpenFile *file;
	int size = sizeof(fdArray)/sizeof(fdArray[0]);
	if (fd < size) {
		if ((file = fdArray[fd]) != NULL) {
			delete file;
			fdArray[fd] = NULL;
			return 1;
		}
	}
	return 0;
}

int
AddrSpace::GetPhysPageNum(int virtPageNum)
{
	return pageTable[virtPageNum].physicalPage;
}

int AddrSpace::GetPhysAddress(int va)
{
	int virtPageNum = va / PageSize;
	int offset = va % PageSize;
	int physPageNum = GetPhysPageNum(virtPageNum);

	return (physPageNum * PageSize) + offset;
}
#endif
