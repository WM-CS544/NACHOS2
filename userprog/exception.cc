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
void getStringFromUser(int va, char *string, int length);
void getDataFromUser(int va, char *buffer, int length);
void writeDataToUser(int va, char *buffer, int length);
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
	// iterate through length of the string until a null byte is reached, then break
	for (int i=0; i<length; i++) {
		string[i] = memoryManager->ReadByte(va++);	//translation done in memoryManager
		if (string[i] == '\0') {
			break;
		}
	}

	// Place a null byte at end of char * to terminate string
	string[length-1] = '\0';
}

void
getDataFromUser(int va, char *buffer, int length)
{
	for (int i=0; i<length; i++) {
		buffer[i] = memoryManager->ReadByte(va++); //translation done in memoryManager
	}	
}

void
writeDataToUser(int va, char *buffer, int length)
{
	for (int i=0; i < length; i++) {
		memoryManager->WriteByte(va++, buffer[i]);
	}
}

// increments the program counter by 4 bytes
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

// Helper function to create a file.  To do this we need to fetch the filename 
// string from user-memory and bring it into the kernel memory.  From there, 
// we can use the fileSystem->Create method to create the empty file.  
//void Create(char *name)
void
SysCreate()
{
	char *name = new(std::nothrow) char[42]; // 42 chosen semi-arbitrarily.  Might want to change this later.
	int va = machine->ReadRegister(4);	//virtual address of name string

	getStringFromUser(va, name, 42);

	if (!fileSystem->Create(name, 0)) {
		DEBUG('a', "File could not be created");
		// On failure do nothing, since the syscall is a void function
	}

	increasePC(); // Remember to increment the program counter
	delete name; // Delete the name string to avoid memory leaks
}

// Helper function to open a file and return the OpenFileId associated with it. 
// On error, return a -1.  
//OpenFileId Open(char *name)
void
SysOpen()
{
	OpenFile *file;
	char *name = new(std::nothrow) char[42];
	int va = machine->ReadRegister(4);	//virtual address of name string
	int fd = -1; // Initialize file descriptor to -1 in case of error
	
	// Fetch the filename string from user-mem
	getStringFromUser(va, name, 42);

	if ((file = fileSystem->Open(name)) != NULL) {
		DEBUG('a', "File opened");
		fd = currentThread->space->AddFD(file); // Add a file descriptor to the array
		if (fd == -1) {
			DEBUG('a', "File could not be added to FDArray");
		}
	}

	// Place the file descriptor back in R2, to return
	machine->WriteRegister(2, fd);
	increasePC(); // Remember to increment the program counter
	delete name; // Delete the name string to avoid memory leaks
}

//TODO: Does size include NULL and do we check if writing too much to user and check if console open
//int Read(char *buffer, int size, OpenFileId id)
void
SysRead()
{
	OpenFile *file;
	int bytesRead = -1; // Return -1 if file could not be read
	int va = machine->ReadRegister(4);		//virtual address of buffer
	int size = machine->ReadRegister(5);	//size
	int fd = machine->ReadRegister(6);		//file descriptor
	char *buffer = new(std::nothrow) char[size];

	// GetFile should return null on error.  In this case do nothing
	if ((file = currentThread->space->GetFile(fd)) != NULL) {

		if (fd == ConsoleInput) {
			bytesRead = synchConsole->Read(buffer, size);
			writeDataToUser(va, buffer, size);
		} else if (fd == ConsoleOutput) {
			//can't read from output - do nothing
		} else {
			bytesRead = file->Read(buffer, size);
			writeDataToUser(va, buffer, size);
		}

	} else {
		// In this case we just keep bytesRead set to -1
		// and remember to increment PC
		//fd doesn't exist
	}

	// Place the number of bytes read into R2 to return
	machine->WriteRegister(2, bytesRead);
	increasePC();
	delete buffer; // Avoid memory leaks
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

	// Fetch string from user-mem
	getDataFromUser(va, buffer, size);

	// GetFile should return null on error.  In this case do nothing
	if ((file = currentThread->space->GetFile(fd)) != NULL) {
		if (fd == ConsoleInput) {
			//can't write to input
		} else if (fd == ConsoleOutput) {
			synchConsole->Write(buffer, size);
		} else {
			file->Write(buffer, size);
		}
	} else {
		// In this case we dont return anything since Write() is a void
		// Just remember to increment PC at end
		//fd doesn't exist
	}
	
	// No return value for the Write() syscall
	increasePC();
	delete buffer; // Avoid memory leaks
}

//void Close(OpenFileId id)
void
SysClose()
{
	int fd = machine->ReadRegister(4);	//file descriptor

	// Call helper DeleteFD function to remove the file descriptor from the array
	if (!currentThread->space->DeleteFD(fd)) {
		//fd doesn't exist
	}

	increasePC(); // Remember to increment the PC
}
#endif


