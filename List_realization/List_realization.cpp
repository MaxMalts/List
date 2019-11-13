#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _DEBUG
#define PrintList_OK(list) ListDump(&list, __FILE__, __LINE__, __FUNCTION__, "just looking");
#define PrintList_NOK(list) ListDump(&list, __FILE__, __LINE__, __FUNCTION__, "list has an error");
#else
#define PrintList_OK(list) ;
#define PrintList_NOK(list) ;
#endif

const int listMaxSize = 100;

typedef int list_elem_t;

#pragma pack(1)
struct list_t {
#ifdef _DEBUG
	int secureVarBeg = 0;                    ///<Первая канарейка
#endif

	list_elem_t data[listMaxSize] = {};
	int next[listMaxSize] = {};
	int prev[listMaxSize] = {};
	int head = 0;
	int tail = 0;
	int free = 1;
	list_elem_t emptyelem = -2147483647;    ///<Элемент, соответствующий пустому
	int size = 0;

#ifdef _DEBUG
	char name[30] = "";                      ///<Имя списка
	int err = 0;                             ///<Код ошибки, содержащейся в списке:\n
	                                         ///0 - нет ошибок\n
	                                         ///1, 2 - выход соответственно за вержнюю или нижнюю границу списка\n
											 ///3, 4 - испорчена соответственно левая или правая канарейка\n
											 ///5 - неверная хэш-сумма\n
											 ///6 - Проблемы с массивом next\n
											 ///7 - Проблемы с массивом prev\n
											 ///8 - Проблемы со свободными элементами
	unsigned int hash = 0;                   ///<Хэш-сумма
	int secureVarEnd = 0;                    ///<Вторая канарейка
#endif
};
#pragma pack()


/**
*	Преобразует list_elem_t в строку
*
*	@param[in] elem Элемент
*
*	@return Указатель на строку, не заюудьте освободить память по этому указателю!
*/

char* list_elem_tToStr(list_elem_t elem) {

	const int elem_tMaxStrSize = 100;

	char* str = (char*)calloc(elem_tMaxStrSize, sizeof(char));
	itoa(elem, str, 10);
	return str;
}


/**
*	Выводит информацию о списке
*
*	@param[in] list Список
*	@param[in] file Название файла, окуда вызвали функцию
*	@param[in] line Номер строки, из которой вызвали функцию
*	@param[in] function Имя функции, из которой вызвали функцию
*	@param[in] reason Причина, по которой вызвали функцию
*/

#ifdef _DEBUG
void ListDump(list_t* list, const char* file, const int line, const char* function, const char* reason) {
	char status[10] = "ok";
	if (list->err != 0) {
		strcpy(status, "ERR");
	}

	printf("\nInfo about a list from file: %s, function: %s, line: %d, reason: %s:\n", file, function, line, reason);
	printf("list_t \"%s\" (%p):    (%s)\n", list->name, list, status);
	printf("\tsecureVarBeg = %d\n", list->secureVarBeg);
	printf("\tsize = %d\n\n", list->size);

	char tempStr1[50] = "";
	char tempStr2[50] = "";
	char tempStr3[50] = "";
	sprintf(tempStr1, "data[%d] (%p):", listMaxSize, &list->data);
	sprintf(tempStr2, "next[%d] (%p):", listMaxSize, &list->next);
	sprintf(tempStr3, "prev[%d] (%p):", listMaxSize, &list->prev);

	printf("\t%-32s %-32s %-32s\n", tempStr1, tempStr2, tempStr3);
	for (int i = 0; i < listMaxSize; i++) {
		char* elemStr = list_elem_tToStr(list->data[i]);
		sprintf(tempStr1, "[%d] = %s,", i, elemStr);
		sprintf(tempStr2, "[%d] = %d,", i, list->next[i]);
		if (list->data[i] == list->emptyelem) {
			sprintf(tempStr3, "[%d] = %d (poison);", i, list->prev[i]);
		}
		else {
			sprintf(tempStr3, "[%d] = %d;", i, list->prev[i]);
		}
		printf("\t%-32s %-32s %-32s\n", tempStr1, tempStr2, tempStr3);
		free(elemStr);
	}

	printf("\thash = %u\n", list->hash);
	printf("\tsecureVarEnd = %d\n", list->secureVarEnd);
	printf("err = %d\n\n\n", list->err);
}
#endif


/**
*	Вычисляет хэш-сумму списка
*
*	@param[in] list Список
*
*	@return Значение хэш-суммы
*/

#ifdef _DEBUG
int CalcHash(list_t* list) {
	assert(list != NULL);

	int hash = 0;
	for (int i = 0; i < sizeof(list_t); i++) {
		char* curByteP = (char*)list + i;

		if (curByteP == (char*)&list->hash) {
			i = i + sizeof(list->hash) - 1;
			continue;
		}

		char curByte = *curByteP;
		hash = hash ^ (curByte * 2 + hash / 2);
	}

	return hash;
}
#endif


/**
*	Проверяет массив next в списке
*
*	@param[in] list Список
*
*	@return 1 - позиция выходит за верхнюю границу списка;\n
*2 - позиция выходит за нижнюю границу списка;\n
*3 - позиция зациклилась;\n
*4 - последняя позиция не соответствукт tail;\n
*0 - массив в порядке
*/

int ListNextOk(list_t* list) {
	assert(list != NULL);

	int curPos = list->head;
	int prevPos = 0;
	int i = 0;
	while (curPos != 0) {
		if (curPos >= listMaxSize) {
			return 1;
		}
		if (curPos < 0) {
			return 2;
		}
		if (i > list->size) {
			return 3;
		}
		prevPos = curPos;
		curPos = list->next[curPos];
		i++;
	}
	if (prevPos != list->tail) {
		return 4;
	}

	return 0;
}


/**
*	Проверяет массив prev в списке
*
*	@param[in] list Список
*
*	@return 1 - позиция выходит за верхнюю границу списка;\n
*2 - позиция выходит за нижнюю границу списка;\n
*3 - позиция зациклилась;\n
*4 - последняя позиция не соответствукт head;\n
*0 - массив в порядке
*/

int ListPrevOk(list_t* list) {
	assert(list != NULL);

	int curPos = list->tail;
	int prevPos = 0;
	int i = 0;
	while (curPos != 0) {
		if (curPos >= listMaxSize) {
			return 1;
		}
		if (curPos < 0) {
			return 2;
		}
		if (i > list->size) {
			return 3;
		}
		prevPos = curPos;
		curPos = list->prev[curPos];
		i++;
	}
	if (prevPos != list->head) {
		return 4;
	}

	return 0;
}


/**
*	Проверяет свободные элементы в списке
*
*	@param[in] list Список
*
*	@return 1 - позиция выходит за верхнюю границу списка;\n
*2 - позиция выходит за нижнюю границу списка;\n
*3 - позиция зациклилась;\n
*4 - значение свободного элементы не равно emptyelem;\n
*5 - значение prev в свободной позиции не равно -1;\n
*0 - все в порядке
*/

int ListFreeOk(list_t* list) {
	assert(list != NULL);

	int curPos = list->free;
	int i = 0;
	while (curPos != 0) {
		if (curPos >= listMaxSize) {
			return 1;
		}
		if (curPos < 0) {
			return 2;
		}
		if (i > listMaxSize) {
			return 3;
		}
		if (list->data[curPos] != list->emptyelem) {
			return 4;
		}
		if (list->prev[curPos] != -1) {
			return 5;
		}
		curPos = list->next[curPos];
		i++;
	}

	return 0;
}


/**
*	Проверяет массивы в списке
*
*	@param[in] list Список
*
*	@return Если в разряде десятков:\n
*1 - проблема в массиве next;\n
*2 - проблема в массиве prev;\n
*3 - проблема с свободными элементами.\n
*В разряде единиц стоит конкретная ошибка. Ее значение см. в возращаемых значениях \
функции ListNextOk, ListPrevOk или ListFreeOk в соответствии с разрядом десятков.
*/

int ListArraysOk(list_t* list) {
	assert(list != NULL);

	int err = ListNextOk(list);
	if (err != 0) {
		return err;
	}

	err = ListPrevOk(list);
	if (err != 0) {
		return 10 + err;
	}

	err = ListFreeOk(list);
	if (err != 0) {
		return 20 + err;
	}

	return 0;
}


/**
*	Проверяет список и записывает в него код ошибки
*
*	@param[in] list Список
*
*	@return 0 - список некорректный; 1 - список корректный
*/

#ifdef _DEBUG
int ListOk(list_t* list) {
	assert(list != NULL);

	if (list->size > listMaxSize) {
		list->err = 1;
		return 0;
	}
	if (list->size < 0) {
		list->err = 2;
		return 0;
	}
	if (list->secureVarBeg != 0) {
		list->err = 3;
		return 0;
	}
	if (list->secureVarEnd != 0) {
		list->err = 4;
		return 0;
	}
	if (list->hash != CalcHash(list)) {
		list->err = 5;
		return 0;
	}

	int arrErr = ListArraysOk(list);
	if (arrErr != 0) {
		switch (arrErr / 10) {
		case 0:
			list->err = 6;
			return 0;
		case 1:
			list->err = 7;
			return 0;
		case 2:
			list->err = 8;
			return 0;
		}
	}

	list->err = 0;
	return 1;
}
#endif


/**
*	Пересчитывает хэш-сумму списка
*
*	@param[in] list Список
*
*	@return 1 - ошибка в списка после пересчитывания хэша; 0 - все прошло нормально
*/

#ifdef _DEBUG
int RecalcHash(list_t* list) {
	assert(list != NULL);

	list->hash = CalcHash(list);

#ifdef _DEBUG
	if (!ListOk(list)) {
		PrintList_NOK(*list);
		return 1;
	}
#endif

	return 0;
}
#endif


/**
*	Создает новый список, заполняя его "пустыми" элементами
*
*	@param[in] name Имя списка
*
*	@return Указатель на созданный список
*/

list_t ListConstructor(const char name[]) {
	list_t list = {};
#ifdef _DEBUG
	strcpy(list.name, name);
	list.err = 0;
#endif

	list_elem_t emptyelem = list.emptyelem;
	for (int i = 0; i < listMaxSize; i++) {
		list.data[i] = emptyelem;
		list.prev[i] = -1;
	}

	list.next[0] = 0;
	for (int i = 1; i < listMaxSize-1; i++) {
		list.next[i] = i + 1;
	}
	list.next[listMaxSize - 1] = 0;


	list.size = 0;

#ifdef _DEBUG
	RecalcHash(&list);
	if (ListOk(&list)) {
		PrintList_OK(list);
	}
	else {
		PrintList_NOK(list);
	}
#endif

	return list;
}

//int InsertFirst(list_t* list, int phIndex, list_elem_t elem) {
//	assert(list != NULL);
//	assert(phIndex > 0);
//
//	int head = phIndex + 1;
//	
//	if (list->prev[head] == 0) {
//		assert(list->free == head);
//
//		list->free = list->next[head];
//		list->prev[list->free] = 0;
//		list->data[head] = elem;
//		list->next[head] = 0;
//		list->prev[head] = 0;
//	}
//	else {
//		assert(list->prev[head] > 0);
//		
//		list->next[list->prev[head]] = list->next[head];
//		list->prev[list->next[head]] = list->prev[head];
//		list->data[head] = elem;
//		list->next[head] = 0;
//		list->prev[head] = 0;
//	}
//
//	list->head = head;
//
//	return 0;
//}


/**
*	Вставляет элемент в начало списка
*
*	@param list Список
*	@param[in] elem Элемент
*
*	@return Позиция, по которой вставился элемент; -1 - нет свободного места в списке; \
-2 - на вход подался список с ошибкой (только в режиме отладки)
*/

int InsertBeg(list_t* list, list_elem_t elem) {
	assert(list != NULL);

#ifdef _DEBUG
	if (!ListOk(list)) {
		PrintList_NOK(*list);
		return -1;
	}
#endif

	if (list->size >= listMaxSize) {
		return -1;
	}

	int newHead = list->free;

	list->free = list->next[newHead];

	list->data[newHead] = elem;
	if (list->size > 0) {
		list->prev[list->head] = newHead;
	}
	else {
		list->tail = newHead;
	}
	list->prev[newHead] = 0;
	list->next[newHead] = list->head;

	list->head = newHead;
	list->size++;

#ifdef _DEBUG
	RecalcHash(list);
	if (ListOk(list)) {
		PrintList_OK(*list);
	}
	else {
		PrintList_NOK(*list);
	}
#endif

	return newHead;
}


/**
*	Вставляет элемент в конец списка
*
*	@param list Список
*	@param[in] elem Элемент
*
*	@return Позиция, по которой вставился элемент; -1 - нет свободного места в списке; \
-2 - на вход подался список с ошибкой (только в режиме отладки)
*/

int InsertEnd(list_t* list, list_elem_t elem) {
	assert(list != NULL);

#ifdef _DEBUG
	if (!ListOk(list)) {
		PrintList_NOK(*list);
		return -2;
	}
#endif

	if (list->size >= listMaxSize) {
		return -1;
	}

	int newTail = list->free;

	list->free = list->next[newTail];

	list->data[newTail] = elem;
	list->next[list->tail] = newTail;
	list->next[newTail] = 0;
	list->prev[newTail] = list->tail;

	list->tail = newTail;
	list->size++;

#ifdef _DEBUG
	RecalcHash(list);
	if (ListOk(list)) {
		PrintList_OK(*list);
	}
	else {
		PrintList_NOK(*list);
	}
#endif

	return newTail;
}


/**
*	Вставляет элемент в списка
*
*	@param list Список
*	@param[in] phIndex Позиция, после которой надо вставить элемент
*	@param[in] elem Элемент
*
*	@return Позиция, по которой вставился элемент; -1 - нет свободного места в списке; \
-2 - на вход подался список с ошибкой (только в режиме отладки)
*/

int Insert(list_t* list, int phIndex, list_elem_t elem) {
	assert(list != NULL);
	assert(phIndex >= 0);

#ifdef _DEBUG
	if (!ListOk(list)) {
		PrintList_NOK(*list);
		return -2;
	}
#endif

	if (list->size >= listMaxSize) {
		return -1;
	}

	int newPos = 0;
	if (phIndex == 0) {
		newPos = InsertBeg(list, elem);
	}
	else if (phIndex == list->tail) {
		newPos = InsertEnd(list, elem);
	}
	else {
		newPos = list->free;
		
		list->free = list->next[newPos];

		list->data[newPos] = elem;

		list->prev[newPos] = phIndex;
		list->prev[list->next[phIndex]] = newPos;

		list->next[newPos] = list->next[phIndex];
		list->next[phIndex] = newPos;

		list->size++;
	}

#ifdef _DEBUG
	RecalcHash(list);
	if (ListOk(list)) {
		PrintList_OK(*list);
	}
	else {
		PrintList_NOK(*list);
	}
#endif

	return newPos;
}


/**
*	Удаляет элемент из начала списка
*
*	@param list Список
*
*	@return 1 - список пуст; 2 - на вход подался список с ошибкой (только в режиме отладки); \
0 - все прошло нормально
*/

int DeleteBeg(list_t* list) {
	assert(list != NULL);

#ifdef _DEBUG
	if (!ListOk(list)) {
		PrintList_NOK(*list);
		return 2;
	}
#endif

	if (list->size == 0) {
		return 1;
	 }

	int newHead = list->next[list->head];

	if (list->size > 1) {
		list->prev[newHead] = 0;
	}
	else {
		list->tail = 0;
	}

	list->data[list->head] = list->emptyelem;
	list->next[list->head] = list->free;
	list->prev[list->head] = -1;

	list->free = list->head;
	list->head = newHead;
	list->size--;

#ifdef _DEBUG
	RecalcHash(list);
	if (ListOk(list)) {
		PrintList_OK(*list);
	}
	else {
		PrintList_NOK(*list);
	}
#endif

	return 0;
}


/**
*	Удаляет элемент из конца списка
*
*	@param list Список
*
*	@return 1 - список пуст; 2 - на вход подался список с ошибкой (только в режиме отладки); \
0 - все прошло нормально
*/

int DeleteEnd(list_t* list) {
	assert(list != NULL);

#ifdef _DEBUG
	if (!ListOk(list)) {
		PrintList_NOK(*list);
		return 2;
	}
#endif

	if (list->size == 0) {
		return 1;
	}

	int newTail = list->prev[list->tail];

	if (list->size > 1) {
		list->next[newTail] = 0;
	}
	else {
		list->head = 0;
	}

	list->data[list->tail] = list->emptyelem;
	list->next[list->tail] = list->free;
	list->prev[list->tail] = -1;

	list->free = list->tail;
	list->tail = newTail;
	list->size--;

#ifdef _DEBUG
	RecalcHash(list);
	if (ListOk(list)) {
		PrintList_OK(*list);
	}
	else {
		PrintList_NOK(*list);
	}
#endif

	return 0;
}


/**
*	Удаляет элемент из списка
*
*	@param list Список
	@param[in] phIndex Позиция элемента, который нужно удалить
*
*	@return 1 - на вход подался список с ошибкой (только в режиме отладки); 0 - все прошло нормально
*/

int Delete(list_t* list, int phIndex) {
	assert(list != NULL);

#ifdef _DEBUG
	if (!ListOk(list)) {
		PrintList_NOK(*list);
		return 1;
	}
#endif

	if (phIndex == list->head) {
		DeleteBeg(list);
	}
	else if (phIndex == list->tail) {
		DeleteEnd(list);
	}
	else {
		list->data[phIndex] = list->emptyelem;
		list->next[list->prev[phIndex]] = list->next[phIndex];
		list->prev[list->next[phIndex]] = list->prev[phIndex];

		list->next[phIndex] = list->free;
		list->prev[phIndex] = -1;

		list->free = phIndex;
		list->size--;
	}

#ifdef _DEBUG
	RecalcHash(list);
	if (ListOk(list)) {
		PrintList_OK(*list);
	}
	else {
		PrintList_NOK(*list);
	}
#endif

	return 0;
}

/**
*	Удаляет список
*
*	@param[in] list Список
*
*	@return 1 - проблемы со списком; 0 - все прошло нормально
*/

int ListDestructor(list_t* list) {
	assert(list != NULL);

#ifdef _DEBUG
	if (!ListOk(list)) {
		PrintList_NOK(*list);
		return 1;
	}
#endif

	return 0;
}


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