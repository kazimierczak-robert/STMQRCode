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
