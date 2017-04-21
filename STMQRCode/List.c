#include "List.h"

struct List *add_last(struct List *last, FILINFO data)
{
	struct List *pointer;
	pointer = (struct List*)malloc(sizeof(struct List));
	pointer->file = data;
	pointer->next = 0;
	pointer->previous = last;
	if(last != 0)
	{
		last->next = pointer;
	}
	return pointer;
}
struct List* remove_all(struct List *first, struct List *last)
{
	struct List *pointer1;
	while(first!=0)
	{
		pointer1=first->next;
		if(pointer1==0)
		{
			break;
		}
		first->next=0;
		free(first);
		first=pointer1;

	}

	return 0;
}
