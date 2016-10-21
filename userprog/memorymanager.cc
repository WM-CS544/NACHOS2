#ifdef CHANGED

#include "memorymanager.h"
#include <new>

MemoryManager::MemoryManager(int numPages)
{
	memoryMap = new(std::nothrow) BitMap(numPages);	
}

MemoryManager::~MemoryManager()
{
	delete memoryMap;
}

//TODO:Check if find returns -1
int
MemoryManager::NewPage()
{
	lock->Acquire();

	int pageNum = memoryMap->Find();

	lock->Acquire();

	return pageNum;
}

void
MemoryManager::ClearPage(int page)
{
	lock->Acquire();

	//page should not be empty if clearing it
	ASSERT(memoryMap->Test(page));

	memoryMap->Clear(page);

	lock->Release();
}

int
MemoryManager::NumPagesFree()
{
	lock->Acquire();

	int numClear = memoryMap->NumClear();

	lock->Release();

	return numClear;
}

//TODO: Add sanity checks
char
MemoryManager::ReadByte(int va)
{
	int virtPageNum = va / PageSize;
	int offset = va % PageSize;
	
	int physPageNum = currentThread->space->GetPhysPageNum(virtPageNum);

	return machine->mainMemory[(physPageNum*PageSize) + offset];
}

//TODO: Add sanity checks
void
MemoryManager::WriteByte(int va, char byte)
{
	int virtPageNum = va / PageSize;
	int offset = va % PageSize;
	
	int physPageNum = currentThread->space->GetPhysPageNum(virtPageNum);

	machine->mainMemory[(physPageNum*PageSize) + offset] = byte;
}
#endif
