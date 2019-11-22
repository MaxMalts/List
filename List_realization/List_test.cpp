#include <stdio.h>
#include "list.h"

void CanaryTest() {
	list_t list1 = ListConstructor("list1");

	Insert(&list1, 0, 10);
	Insert(&list1, 1, 30);
	Insert(&list1, 0, 20);
	Insert(&list1, 1, 40);
	Insert(&list1, 1, 40);

	list1.secureVarBeg = 1;

	Delete(&list1, 2);
	Delete(&list1, 1);
	Delete(&list1, 4);
	Delete(&list1, 3);

	ListDestructor(&list1);
}

void ListTest() {
	list_t list1 = ListConstructor("list1");

	Insert(&list1, 0, 10);
	Insert(&list1, 1, 30);
	Insert(&list1, 0, 20);
	Insert(&list1, 1, 40);
	Insert(&list1, 1, 40);
	Insert(&list1, 1, 40);
	Insert(&list1, 1, 40);
	Insert(&list1, 1, 40);
	Insert(&list1, 1, 40);
	Insert(&list1, 1, 40);
	Insert(&list1, 1, 40);
	Insert(&list1, 1, 40);
	Insert(&list1, 1, 40);
	Insert(&list1, 1, 40);
	Insert(&list1, 1, 40);
	Insert(&list1, 1, 40);
	Insert(&list1, 1, 40);
	Insert(&list1, 1, 40);
	Insert(&list1, 1, 40);
	Insert(&list1, 1, 40);
	Insert(&list1, 1, 40);


	Delete(&list1, 2);
	Delete(&list1, 1);
	Delete(&list1, 4);
	Delete(&list1, 3);

	ListDestructor(&list1);
}

int main() {
	ListTest();

	getchar();
	return 0;
}