#ifdef CHANGED

#include "synchconsole.h"

//----------------------------------------------------------------------
// 	Need this to be a C routine, because 
//	C++ can't handle pointers to member functions.
//----------------------------------------------------------------------

static void
staticReadAvail(int arg)
{
	SynchConsole *synchConsole = (SynchConsole *)arg;
	synchConsole->ReadAvail();
}

static void
staticWriteDone(int arg)
{
	SynchConsole *synchConsole = (SynchConsole *)arg;
	synchConsole->WriteDone();
}

SynchConsole::SynchConsole(char *in, char *out)
{
	console = new(std::nothrow) Console(in, out, staticReadAvail, staticWriteDone, int(this));
	readAvail = new(std::nothrow) Semaphore("read avail", 0);
	writeDone = new(std::nothrow) Semaphore("write done", 0);
	readLock = new(std::nothrow) Lock("read lock");
	writeLock = new(std::nothrow) Lock("write lock");
}

SynchConsole::~SynchConsole()
{
	delete console;
	delete readAvail;
	delete writeDone;
	delete readLock;
	delete writeLock;
}

char
SynchConsole::GetChar()
{
	char ch;

	readLock->Acquire();			//get lock

	readAvail->P();						//wait for something to read
	ch = console->GetChar();	//read char

	readLock->Release();			//release lock

	return ch;

}

void
SynchConsole::PutChar(char ch)
{
	writeLock->Acquire();	//get lock

	console->PutChar(ch);	//write to console
	writeDone->P();				//wait for write to finish

	writeLock->Release();	//release lock
}

#endif
