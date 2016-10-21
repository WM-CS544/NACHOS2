#ifdef CHANGED

#ifndef PROCESS_CONTROL_BLOCK_H
#define PROCESS_CONTROL_BLOCK_H

#include "copyright.h"
#include "fdset.h"

class Thread;

class ProcessControlBlock {

	public:

		ProcessControlBlock();
		~ProcessControlBlock();

		FDSet *GetFDSet();

	private:

		FDSet *fdSet;

		Thread *parent;
};

#endif	//PROCESS_CONTROL_BLOCK_H
#endif	//CHANGED
