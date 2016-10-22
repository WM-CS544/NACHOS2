#ifdef CHANGED

#ifndef FD_SET_H
#define FD_SET_H

#include "copyright.h"
#include "filesys.h"

struct FDSetEntry {
	
	int numOpen;
	OpenFile *file;
};

class FDSet {

	public:

		FDSet(FDSet *copySet);
		~FDSet();
			
		FDSetEntry **GetArray();
		int AddFD(OpenFile *file);
		OpenFile *GetFile(int fd);
		int DeleteFD(int fd);
		FDSet *CopyFDSet();

	private:
		
		FDSetEntry *fdArray[42];

};

#endif	//FD_SET_H
#endif	//CHANGED
