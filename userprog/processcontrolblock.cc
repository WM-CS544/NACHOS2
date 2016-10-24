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
	
	ChildNode *curNode = childListHead;
	ChildNode *prevNode = NULL;

	while(curNode != NULL) {
		curNode->child->SetParent(NULL); //tell child we left them
		prevNode = curNode;
		curNode = curNode->next;
		//delete prevNode->child;	//should already be deleted
		//delete prevNode->childDone; //can't delete this right now
		delete prevNode;
	}
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
ProcessControlBlock::SetParent(ProcessControlBlock *newParent) {
	parentBlock = newParent;
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
ProcessControlBlock::DeleteChild(int childPID)
{
	ChildNode *curNode = childListHead;
	ChildNode *prevNode = NULL;

	while (curNode != NULL) {
		if (curNode->pid == childPID) {
			if (prevNode == NULL) {	//head of list
				childListHead = curNode->next;
			} else {
				prevNode->next = curNode->next;
			}
			//delete curNode->child;	//should already be deleted
			//delete curNode->childDone; //can't delete this right now
			delete curNode;
			break;
		}
		prevNode = curNode;
		curNode = curNode->next;
	}
}

ChildNode*
ProcessControlBlock::GetChild(int childPID)
{
	ChildNode *curNode = childListHead;

	while (curNode != NULL) {
		if (curNode->pid == childPID) {
			return curNode;
		}
		curNode = curNode->next;
	}
	return NULL;
}

#endif
