#ifdef CHANGED

#ifndef SYNCH_CONSOLE_H
#define SYNCH_CONSOLE_H

#include "copyright.h"
#include "console.h"
#include "synch.h"

class SynchConsole {
	
	public:
		SynchConsole(char *in, char *out);
		~SynchConsole();

		void PutChar(char ch);
		char GetChar();
		inline void ReadAvail(){ readAvail->V(); };
		inline void WriteDone(){ writeDone->V(); };

	private:
		Console *console;
		Semaphore *readAvail;
		Semaphore *writeDone;
		Lock *readLock;
		Lock *writeLock;
};

#endif
#endif
