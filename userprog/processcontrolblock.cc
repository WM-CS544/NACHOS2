#ifdef CHANGED

#include "processcontrolblock.h"
#include <new>

ProcessControlBlock::ProcessControlBlock()
{
	fdSet = new(std::nothrow) FDSet();
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

#endif
