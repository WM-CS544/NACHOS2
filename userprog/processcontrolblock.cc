#ifdef CHANGED

#include "processcontrolblock.h"
#include <new>

ProcessControlBlock::ProcessControlBlock(ProcessControlBlock *parent, FDSet *parentSet, int newPID)
{
	pid = newPID;
	parentBlock = parent;
	fdSet = new(std::nothrow) FDSet(parentSet);
}

ProcessControlBlock::~ProcessControlBlock()
{
	delete fdSet;
}

FDSet*
ProcessControlBlock::GetFDSet()
{
	return fdSet;
}

int
ProcessControlBlock::GetPID()
{
	return pid;
}

void
ProcessControlBlock::SetFDSet(FDSet *parentSet)
{
	delete fdSet;
	fdSet = parentSet;
} 

void
ProcessControlBlock::AddChild(ProcessControlBlock *childBlock, int childPID, Semaphore *childSem)
{
	ChildNode *newNode = new(std::nothrow) ChildNode;
	newNode->retval = -1;
	newNode->pid = childPID;
	newNode->childDone = childSem;
	newNode->child = childBlock;
	newNode->next = NULL;

	ChildNode *curNode = childListHead;
	ChildNode *prevNode = NULL;

	while (curNode != NULL) {
		prevNode = curNode;
		curNode = curNode->next;
	}
	
	if (prevNode == NULL) { //empty list
		childListHead = newNode;
	} else {
		prevNode->next = newNode;
	}
}

void
ProcessControlBlock::DeleteChild(ProcessControlBlock *child)
{

}

ChildNode*
ProcessControlBlock::GetChild(int childPID)
{
	ChildNode *curNode = childListHead;

	while (curNode != NULL) {
		if (curNode->pid == pid) {
			return curNode;
		}
	}
	return curNode;	//will be NULL
}

#endif
