#ifdef CHANGED

#include "processmanager.h"

ProcessManager::ProcessManager()
{
	numProcesses = 1;
}

ProcessManager::~ProcessManager()
{
}

int
ProcessManager::NewProcess()
{
	numProcesses++;
	return numProcesses;
}

#endif
