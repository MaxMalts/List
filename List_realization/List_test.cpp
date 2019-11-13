#include <stdio.h>
#include "list.h"

int main() {
	list_t list1 = ListConstructor("list1");

	Insert(&list1, 0, 10);
	Insert(&list1, 1, 30);
	Insert(&list1, 0, 20);
	Insert(&list1, 1, 40);

	Delete(&list1, 2);
	Delete(&list1, 1);
	Delete(&list1, 4);
	Delete(&list1, 3);

	ListDestructor(&list1);

	getchar();
	return 0;
}