#ifdef CHANGED

#ifndef CHILD_LIST_H
#define CHILD_LIST_H

#include "copyright.h"
#include "thread.h"

struct ChildNode {
	int retval;
	Thread *child;
	ChildNode *next;
};

class ChildList {
		
	public:

		ChildList();
		~ChildList();
		int AddNode(Thread *child);
		int DeleteNode(Thread *child);

	private:
		
		ChildNode *head;
};

#endif	//CHILD_LIST_H
#endif	//CHANGED
