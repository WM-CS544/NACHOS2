#ifdef CHANGED

#ifndef PROCESS_CONTROL_BLOCK_H
#define PROCESS_CONTROL_BLOCK_H

#include "copyright.h"
#include "fdset.h"

class Semaphore;
struct ChildNode;

class ProcessControlBlock {

	public:

		ProcessControlBlock(ProcessControlBlock *parent, FDSet *parentSet, int newPID);
		~ProcessControlBlock();

		FDSet *GetFDSet();
		int GetPID();
		ProcessControlBlock *GetParent() { return parentBlock; };

		void SetFDSet(FDSet *parentSet);

		void AddChild(ProcessControlBlock *childBlock, int childPID, Semaphore *childSem);
		void DeleteChild(ProcessControlBlock *child);
		ChildNode *GetChild(int childPID);

	private:

		int pid;
		FDSet *fdSet;
		ProcessControlBlock *parentBlock;
		ChildNode *childListHead;
};

struct ChildNode {
	int retval;
	int pid;
	Semaphore *childDone;
	ProcessControlBlock *child;
	ChildNode *next;
};

#endif	//PROCESS_CONTROL_BLOCK_H
#endif	//CHANGED
