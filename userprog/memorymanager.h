#ifdef CHANGED
#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "copyright.h"
#include "system.h"
#include "bitmap.h"

class  MemoryManager {
	public:

		MemoryManager(int numPages);
		~MemoryManager();

		int NewPage();
		void ClearPage(int page);
		int NumPagesFree();

		char ReadByte(int va);
		void WriteByte(int va, char byte);

	private:

		BitMap *memoryMap;
};

#endif //MEMORY_MANAGER_H
#endif //CHANGED
