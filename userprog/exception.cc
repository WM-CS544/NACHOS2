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
void SysFork();
void SysJoin();
void SysExit();
void SysExec();
void SysDup();
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
				case SC_Fork:
					DEBUG('a', "Fork, initiated by user program.\n");
					SysFork();
					break;
				case SC_Exec:
					DEBUG('a', "Exec, initiated by user program.\n");
					SysExec();
					break;
				case SC_Join:
					DEBUG('a', "Join, initiated by user program.\n");
					SysJoin();
					break;
				case SC_Exit:
					DEBUG('a', "Exit, initiated by user program.\n");
					SysExit();
					break;
				case SC_Dup:
					DEBUG('a', "Dup, initiated by user program.\n");
					SysDup();
					break;
#endif
				default:
					printf("Undefined SYSCALL %d\n", type);
					ASSERT(false);
			}
#ifdef CHANGED
			break;
#endif
#ifdef USE_TLB
    case PageFaultException:
			HandleTLBFault(machine->ReadRegister(BadVAddrReg));
			break;
#endif
#ifndef CHANGED
    default: ;
#else
		default:
			//exit with value of -1
			machine->WriteRegister(4, -1);
			SysExit();
#endif
  }
}

#ifdef  CHANGED
void
getStringFromUser(int va, char *string, int length)
{
	// iterate through length of the string until a null byte is reached, then break
	for (int i=0; i<length; i++) {
		string[i] = currentThread->space->ReadByte(va++);	//translation done in addrspace
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
		buffer[i] = currentThread->space->ReadByte(va++); //translation done in addrspace
	}	
}

void
writeDataToUser(int va, char *buffer, int length)
{
	for (int i=0; i < length; i++) {
		currentThread->space->WriteByte(va++, buffer[i]);
	}
}

void
writeCharToUser(int va, char ch) {
	currentThread->space->WriteByte(va, ch);
}

int
getPointerFromUser(int va) {
	//assumes 4 byte pointer little endian
	char newVA[4];
	for (int i=0; i < 4; i++) {
		newVA[i] = currentThread->space->ReadByte(va++); //translation done in addrspace
	}
	unsigned int result = ((newVA[3] & 255) << 24) | ((newVA[2] & 255) << 16) | ((newVA[1] & 255) << 8) | (newVA[0] & 255);
	return result;
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
		fd = currentThread->space->GetProcessControlBlock()->GetFDSet()->AddFD(file); // Add a file descriptor to the array
		if (fd == -1) {
			DEBUG('a', "File could not be added to FDArray");
		} else {
			//if added to fdSet add to global open file list
			fileManager->OpenFile(name);
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
	if ((file = currentThread->space->GetProcessControlBlock()->GetFDSet()->GetFile(fd)) != NULL) {
		if ((int) file == 1) {	//1 = cookie for console input
			bytesRead = synchConsole->Read(buffer, size);
			writeDataToUser(va, buffer, size);
		} else if ((int) file == 2) {	//2 = cookie for console output
			//can't read from output - do nothing
		} else {
			//get file lock then read from file
			Lock *fileLock = fileManager->GetLock(file->GetName());
			ASSERT(fileLock != NULL);
			fileLock->Acquire();
			bytesRead = file->Read(buffer, size);
			fileLock->Release();
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
	if ((file = currentThread->space->GetProcessControlBlock()->GetFDSet()->GetFile(fd)) != NULL) {
		if ((int) file == 1) { //1 = cookie for console input
			//can't write to input
		} else if ((int) file == 2) {	//2 = cookie for console output
			synchConsole->Write(buffer, size);
		} else {
			//get file lock then write to file
			Lock *fileLock = fileManager->GetLock(file->GetName());
			ASSERT(fileLock != NULL);
			fileLock->Acquire();
			file->Write(buffer, size);
			fileLock->Release();
		}
	} else {
		// In this case we dont return anything since Write() is a void
		// Just remember to increment PC at end
		//fd doesn't exist
	}
	
	// No return value for the Write() syscall
	increasePC();
	delete [] buffer; // Avoid memory leaks
}

//void Close(OpenFileId id)
void
SysClose()
{
	int fd = machine->ReadRegister(4);	//file descriptor

	FDSet *fdSet = currentThread->space->GetProcessControlBlock()->GetFDSet();
	OpenFile *toBeDeleted = fdSet->GetFile(fd);
	char *nameToBeDeleted = new(std::nothrow) char[strlen(toBeDeleted->GetName())+1];
	strncpy(nameToBeDeleted, toBeDeleted->GetName(), strlen(toBeDeleted->GetName())+1);

	//fd exists
	if (toBeDeleted != NULL) {
		// Call helper DeleteFD function to remove the file descriptor from the array
		if (fdSet->DeleteFD(fd) == 1) {
			// Remove from global open file list
			fileManager->CloseFile(nameToBeDeleted);	
		}
	}
	
	delete nameToBeDeleted;
	increasePC(); // Remember to increment the PC
}

void
StartForked(int arg)
{
	currentThread->RestoreRegsForFork();
	currentThread->space->RestoreState();
	machine->WriteRegister(2, 0);	//child gets 0
	machine->Run();
}

//SpaceId Fork()
void
SysFork()
{
	//create new thread
	SpaceId newPID = processManager->NewProcess();
	Thread *newThread = new(std::nothrow) Thread("forked thread");		
	
	AddrSpace *newSpace = new(std::nothrow) AddrSpace(currentThread->space, newPID);
	newThread->space = newSpace;

	//add new thread to child list
	ProcessControlBlock *newBlock = newSpace->GetProcessControlBlock();
	Semaphore *newSem = new(std::nothrow) Semaphore("childDone sem", 0);
	currentThread->space->GetProcessControlBlock()->AddChild(newBlock, newPID, newSem);

	//increase PC before saving registers to new thread
	increasePC();
	newThread->SaveRegsForFork();
	newThread->Fork(StartForked, (int) newThread);	//fork off new thread

	machine->WriteRegister(2, newPID);	//parent gets child PID
}

//int Join(SpaceId id)
void
SysJoin()
{
	SpaceId pid = machine->ReadRegister(4);
	int retval = -1;
	ChildNode *child = currentThread->space->GetProcessControlBlock()->GetChild(pid);
	if (child != NULL) {
		child->childDone->P();	//wait until child is done

		retval = child->retval;

		//remove child from list
		currentThread->space->GetProcessControlBlock()->DeleteChild(pid);
	}
	machine->WriteRegister(2, retval);
	increasePC();
}

//TODO: check if parent is alive
//void Exit(int status)
void
SysExit()
{
	int status = machine->ReadRegister(4);

	ProcessControlBlock *myBlock = currentThread->space->GetProcessControlBlock();
	ProcessControlBlock *parentBlock = myBlock->GetParent();

	if (parentBlock != NULL) {	//parent still alive
		parentBlock->GetChild(myBlock->GetPID())->retval = status;	//tell parent return value
		parentBlock->GetChild(myBlock->GetPID())->childDone->V();	//tell parent we are done
	}

	//TODO:clean up everything

	currentThread->Finish();
}

//int Exec(char *name, char *args[]);
void
SysExec()
{

	int retval = -1;
	OpenFile *executable;
	char *fileName = new(std::nothrow) char[42];
	int nameVA = machine->ReadRegister(4);	//virtual address of name string

	getStringFromUser(nameVA, fileName, 42); //get filename string

	char *args[5];
	for (unsigned int i=0; i < sizeof(args)/sizeof(args[0]); i++) {
		args[i] = new(std::nothrow) char[42];
	}
	int argsVA = machine->ReadRegister(5);	//virtual address of arg array
	int numArgs = 0;

	//get arguments
	int stringVA = getPointerFromUser(argsVA);
	while(stringVA != '\0' && numArgs < 5) {
		getStringFromUser(stringVA, args[numArgs++], 42);
		argsVA+=4;
		stringVA = getPointerFromUser(argsVA);
	}

	//can open file
	if ((executable = fileSystem->Open(fileName)) != NULL) {
		currentThread->space->Exec(executable);
		currentThread->space->InitRegisters();
		currentThread->space->RestoreState();

		//hide args "above" stack
		int sp = machine->ReadRegister(StackReg);
		int argv[numArgs];
		for (int i=0; i < numArgs; i++) {
			int len = strlen(args[i]) + 1;
			sp -= len;
			//write string
			for (int x=0; x < len; x++) {
				writeCharToUser((sp+x), args[i][x]);			
			}
			//store pointer
			argv[i] = sp;
		}
		//align pointer on 4 byte boundrary	
		sp = sp &~3;	//stack goes top down

		//write pointers
		sp -= sizeof(int) * numArgs;
		for (int i=0; i < numArgs; i++) {
			*(unsigned int *) &machine->mainMemory[currentThread->space->GetPhysAddress(sp + i*4)] = WordToMachine((unsigned int) argv[i]);
		}

		//write values to registers
		machine->WriteRegister(4, numArgs);
		machine->WriteRegister(5, sp);
		machine->WriteRegister(StackReg, sp-8);

		//cleanup
		for (unsigned int i=0; i < sizeof(args)/sizeof(args[0]); i++) {
			delete args[i];
		}
		delete executable;
		delete fileName;

		//start running user program
		machine->Run();
	}

	//cleanup
	for (unsigned int i=0; i < sizeof(args)/sizeof(args[0]); i++) {
		delete args[i];
	}
	delete executable;
	delete fileName;
	//return if failure
	machine->WriteRegister(2, retval);
	increasePC();
}

//OpenFileId Dup(OpenFileId fd);
void
SysDup()
{
	int retval;
	OpenFileId fd = machine->ReadRegister(4);
	
	retval = currentThread->space->GetProcessControlBlock()->GetFDSet()->Dup(fd);

	machine->WriteRegister(2, retval);
	increasePC();
}
#endif


