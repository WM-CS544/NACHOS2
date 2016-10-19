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

int
MemoryManager::NewPage()
{
	return memoryMap->Find();	
}

void
MemoryManager::ClearPage(int page)
{
	//page should not be empty if clearing it
	ASSERT(memoryMap->Test(page));

	memoryMap->Clear(page);
}

int
MemoryManager::NumPagesFree()
{
	return memoryMap->NumClear();
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
