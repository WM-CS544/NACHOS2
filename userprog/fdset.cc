#ifdef CHANGED

#include "fdset.h"

FDSet::FDSet(FDSet *copySet)
{

	if (copySet == NULL) {

		FDSetEntry *consoleIn = new(std::nothrow) FDSetEntry;
		consoleIn->numOpen = 1;
		consoleIn->file = (OpenFile *)1;
		FDSetEntry *consoleOut = new(std::nothrow) FDSetEntry;
		consoleOut->numOpen = 1;
		consoleOut->file = (OpenFile *)2;

		for (std::size_t i=0; i < (sizeof(fdArray)/sizeof(fdArray[0])); i++) {
			if (i == 0) {
				fdArray[i] = consoleIn;
			} else if (i == 1) {
				fdArray[i] = consoleOut;
			} else {
				fdArray[i] = NULL;
			}
		}

	} else {
		FDSetEntry **copyArray = copySet->GetArray();
		for (std::size_t i=0; i < (sizeof(fdArray)/sizeof(fdArray[0])); i++) {
			if (copyArray[i] != NULL) {
				copyArray[i]->numOpen++;
			}
			fdArray[i] = copyArray[i];
		}
	}
}

FDSet::~FDSet()
{
	for (std::size_t i=0; i < (sizeof(fdArray)/sizeof(fdArray[0])); i++) {
		if (fdArray[i] != NULL) {
			delete fdArray[i]->file;
			delete fdArray[i];
		}
	}	
}

FDSetEntry**
FDSet::GetArray()
{
	return fdArray;
}

int
FDSet::AddFD(OpenFile *file)
{
	int fd = -1;
	for (int i=0; i < (int)(sizeof(fdArray)/sizeof(fdArray[0])); i++) {
		if (fdArray[i] == NULL) {
			FDSetEntry *newEntry = new(std::nothrow) FDSetEntry;
			newEntry->numOpen = 1;
			newEntry->file = file;
			fdArray[i] = newEntry;
			fd = i;
			break;
		}
	}
	return fd;
}

OpenFile*
FDSet::GetFile(int fd)
{
	OpenFile *file = NULL;
	if (fd < (int)(sizeof(fdArray)/sizeof(fdArray[0])) && fd >= 0) {
		if (fdArray[fd] != NULL) {
			file = fdArray[fd]->file;
		}
	}
	return file;
}

int
FDSet::DeleteFD(int fd)
{
	if (fd < (int)(sizeof(fdArray)/sizeof(fdArray[0])) && fd >= 0) {
			if (fdArray[fd] != NULL) {
				if (fdArray[fd]->numOpen == 1) { //delete file
					delete fdArray[fd]->file;
					delete fdArray[fd];
					fdArray[fd] = NULL;
				} else { //don't delete, decrement numOpen
					ASSERT(fdArray[fd] > 0);
					fdArray[fd]->numOpen--;
					fdArray[fd] = NULL;
				}
				return 1;
			}
	}
	return 0;
}

#endif
