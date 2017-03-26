#include "ff.h"

struct List
{
	FILINFO file;
	struct List *next;
	struct List *previous;
};
struct List *add_last(struct List *last, FILINFO data);
