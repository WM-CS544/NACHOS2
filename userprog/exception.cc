// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

#ifdef USE_TLB

//----------------------------------------------------------------------
// HandleTLBFault
//      Called on TLB fault. Note that this is not necessarily a page
//      fault. Referenced page may be in memory.
//
//      If free slot in TLB, fill in translation info for page referenced.
//
//      Otherwise, select TLB slot at random and overwrite with translation
//      info for page referenced.
//
//----------------------------------------------------------------------

void
HandleTLBFault(int vaddr)
{
  int vpn = vaddr / PageSize;
  int victim = Random() % TLBSize;
  int i;

  stats->numTLBFaults++;

  // First, see if free TLB slot
  for (i=0; i<TLBSize; i++)
    if (machine->tlb[i].valid == false) {
      victim = i;
      break;
    }

  // Otherwise clobber random slot in TLB

  machine->tlb[victim].virtualPage = vpn;
  machine->tlb[victim].physicalPage = vpn; // Explicitly assumes 1-1 mapping
  machine->tlb[victim].valid = true;
  machine->tlb[victim].dirty = false;
  machine->tlb[victim].use = false;
  machine->tlb[victim].readOnly = false;
}

#endif

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
#ifdef CHANGED
void getDataFromUser(int va, char *buffer, int length);
void getStringFromUser(int va, char *string, int length);
void increasePC();
void SysCreate();
void SysOpen();
void SysRead();
void SysWrite();
void SysClose();
#endif

void
ExceptionHandler(ExceptionType which)
{
	int type = machine->ReadRegister(2);

	switch (which) {
		case SyscallException:
			switch (type) {
				case SC_Halt:
					DEBUG('a', "Shutdown, initiated by user program.\n");
					interrupt->Halt();
#ifdef CHANGED
					break;
				case SC_Create:
					DEBUG('a', "Create file, initiated by user program.\n");
					SysCreate();
					break;
				case SC_Open:
					DEBUG('a', "Open file, initiated by user program.\n");
					SysOpen();
					break;
				case SC_Read:
					DEBUG('a', "Read, initiated by user program.\n");
					SysRead();
					break;
				case SC_Write:
					DEBUG('a', "Write, initiated by user program.\n");
					SysWrite();
					break;
				case SC_Close:
					DEBUG('a', "Close, initiated by user program.\n");
					SysClose();
					break;
#endif
				default:
					printf("Undefined SYSCALL %d\n", type);
					ASSERT(false);
			}
#ifdef USE_TLB
    case PageFaultException:
			HandleTLBFault(machine->ReadRegister(BadVAddrReg));
			break;
#endif
    default: ;
  }
}

#ifdef  CHANGED
void
getStringFromUser(int va, char *string, int length)
{
	for (int i=0; i<length; i++) {
		string[i] = machine->mainMemory[va++];	//no translation for now
		if (string[i] == '\0') {
			break;
		}
	}

	string[length-1] = '\0';
}

void
getDataFromUser(int va, char *buffer, int length)
{
	for (int i=0; i<length; i++) {
		buffer[i] = machine->mainMemory[va++];	//no translation for now
	}	
}

void
increasePC()
{
	int tmp = machine->ReadRegister(PCReg);
	machine->WriteRegister(PrevPCReg, tmp);
	tmp = machine->ReadRegister(NextPCReg);
	machine->WriteRegister(PCReg, tmp);
	tmp+=4;
	machine->WriteRegister(NextPCReg, tmp);
}

//void Create(char *name)
void
SysCreate()
{
	char *name = new(std::nothrow) char[42];
	int va = machine->ReadRegister(4);	//virtual address of name string

	getStringFromUser(va, name, 42);

	if (!fileSystem->Create(name, 0)) {
		DEBUG('a', "File could not be created");
	}

	increasePC();
	delete name;
}

//OpenFileId Open(char *name)
void
SysOpen()
{
	OpenFile *file;
	char *name = new(std::nothrow) char[42];
	int va = machine->ReadRegister(4);	//virtual address of name string
	int fd = -1;
	
	getStringFromUser(va, name, 42);

	if ((file = fileSystem->Open(name)) != NULL) {
		DEBUG('a', "File opened");
		fd = currentThread->space->AddFD(file);
		if (fd == -1) {
			DEBUG('a', "File could not be added to FDArray");
		}
	}

	machine->WriteRegister(2, fd);
	increasePC();
	delete name;
}

//TODO: Does size include NULL and do we check if writing too much to user and check if console open
//void Read(char *buffer, int size, OpenFileId id)
void
SysRead()
{
	OpenFile *file;
	int bytesRead = -1;
	int va = machine->ReadRegister(4);		//virtual address of buffer
	int size = machine->ReadRegister(5);	//size
	int fd = machine->ReadRegister(6);		//file descriptor
	char *buffer = new(std::nothrow) char[size];

	if ((file = currentThread->space->GetFile(fd)) != NULL) {

		if (fd == ConsoleInput) {
			for (int i=0; i < size; i++) {
				bytesRead = size;
				machine->mainMemory[va++] = synchConsole->GetChar();	//no translation for now
			}
		} else if (fd == ConsoleOutput) {
			//can't read from output
		} else {
			for (int i=0; i < size; i++) {
				bytesRead = file->Read(buffer, size);
				machine->mainMemory[va++] = buffer[i];	//no translation for now
			}
		}

	} else {
		//fd doesn't exist
	}

	machine->WriteRegister(2, bytesRead);
	increasePC();
	delete buffer;
}
//TODO: Check if console open
//void Write(char *buffer, int size, OpenFileId id)
void
SysWrite()
{
	OpenFile *file;
	int va = machine->ReadRegister(4);		//virtual address of buffer
	int size = machine->ReadRegister(5);	//size
	int fd = machine->ReadRegister(6);		//file descriptor
	char *buffer = new(std::nothrow) char[size];

	getDataFromUser(va, buffer, size);

	if ((file = currentThread->space->GetFile(fd)) != NULL) {
		if (fd == ConsoleInput) {
			//can't read from input
		} else if (fd == ConsoleOutput) {
			for (int i=0; i < size; i++) {
				synchConsole->PutChar(buffer[i]);
			}
		} else {
			file->Write(buffer, size);
		}
	} else {
		//fd doesn't exist
	}
	
	increasePC();
	delete buffer;
}

//void Close(OpenFileId id)
void
SysClose()
{
	int fd = machine->ReadRegister(4);	//file descriptor

	if (!currentThread->space->DeleteFD(fd)) {
		//fd doesn't exit
	}

	increasePC();
}
#endif


