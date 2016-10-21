#ifdef CHANGED

#include "childlist.h"

ChildList::ChildList()
{
	head = NULL;
}

ChildList::~ChildList()
{
	ChildNode *curNode = head;
	while (curNode != NULL) {
		ChildNode *tmp = curNode->next;
		delete curNode->child;
		delete curNode;
		curNode = tmp;
	}
}

#endif
