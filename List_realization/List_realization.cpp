﻿#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "list.h"


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
	if (list->err != no_err) {
		strcpy(status, "ERR");
	}

	printf("\nInfo about a list from file: %s, function: %s, line: %d, reason: %s:\n", file, function, line, reason);
	printf("list_t \"%s\" (%p):    (%s)\n", list->name, list, status);
	printf("\tsecureVarBeg = %d\n", list->secureVarBeg);
	printf("\tsize = %d\n", list->size);
	printf("\tcurMaxSize = %d\n", list->curMaxSize);
	printf("\tfree = %d\n\n", list->free);

	char tempStr1[50] = "";
	char tempStr2[50] = "";
	char tempStr3[50] = "";
	sprintf(tempStr1, "data[%d] (%p):", list->curMaxSize + 1, &list->data);
	sprintf(tempStr2, "next[%d] (%p):", list->curMaxSize + 1, &list->next);
	sprintf(tempStr3, "prev[%d] (%p):", list->curMaxSize + 1, &list->prev);

	printf("\t%-32s %-32s %-32s\n", tempStr1, tempStr2, tempStr3);
	for (int i = 0; i <= list->curMaxSize; i++) {
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
		char* tcurByteP = (char*)list + i;

		if (tcurByteP == (char*)&list->hash) {
			i = i + sizeof(list->hash) - 1;
			continue;
		}

		char curByte = *tcurByteP;
		hash = hash ^ (curByte * 2 + hash / 2);
	}

	for (int i = 0; i <= list->curMaxSize * sizeof(list_elem_t); i++) {
		char curByte = *((char*)list->data + i);
		hash = hash ^ (curByte * 2 + hash / 2);
	}
	for (int i = 0; i <= list->curMaxSize * sizeof(int); i++) {
		char curByte = *((char*)list->next + i);
		hash = hash ^ (curByte * 2 + hash / 2);
	}
	for (int i = 0; i <= list->curMaxSize * sizeof(int); i++) {
		char curByte = *((char*)list->prev + i);
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
		if (curPos > list->curMaxSize) {
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
		if (curPos > list->curMaxSize) {
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
		if (curPos > list->curMaxSize) {
			return 1;
		}
		if (curPos < 0) {
			return 2;
		}
		if (i > list->curMaxSize) {
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
 1 - проблема в массиве next;\n
 2 - проблема в массиве prev;\n
 3 - проблема с свободными элементами.\n
 В разряде единиц стоит конкретная ошибка. Ее значение см. в возращаемых значениях\
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

	if (list->size > list->curMaxSize) {
		list->err = overflow;
		return 0;
	}
	if (list->size < 0) {
		list->err = underflow;
		return 0;
	}
	if (list->secureVarBeg != 0) {
		list->err = first_canary_spoiled;
		return 0;
	}
	if (list->secureVarEnd != 0) {
		list->err = second_canary_spoiled;
		return 0;
	}
	if (list->hash != CalcHash(list)) {
		list->err = invalid_hash;
		return 0;
	}

	int arrErr = ListArraysOk(list);
	if (arrErr != 0) {
		switch (arrErr / 10) {
		case 0:
			list->err = next_err;
			return 0;
		case 1:
			list->err = prev_err;
			return 0;
		case 2:
			list->err = free_err;
			return 0;
		}
	}

	list->err = no_err;
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

list_t ListConstructor(const char* name) {
	assert(name != NULL);

	const int begMaxSize = 10;

	list_t list = {};

#ifdef _DEBUG
	strcpy(list.name, name);
	list.err = no_err;
#endif

	list.data = (list_elem_t*)calloc(begMaxSize + 1, sizeof(list_elem_t));
	list.next = (int*)calloc(begMaxSize + 1, sizeof(int));
	list.prev = (int*)calloc(begMaxSize + 1, sizeof(int));
	list.curMaxSize = begMaxSize;

	list_elem_t emptyelem = list.emptyelem;
	for (int i = 0; i <= begMaxSize; i++) {
		list.data[i] = emptyelem;
		list.prev[i] = -1;
	}

	list.next[0] = 0;
	for (int i = 1; i <= list.curMaxSize - 1; i++) {
		list.next[i] = i + 1;
	}
	list.next[list.curMaxSize] = 0;

	list.head = 0;
	list.tail = 0;
	list.free = 1;
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


/*  Не для пользователя
*	Увеличивает размер списка
*
*	@param list Список
*	@param[in] newSize Новый размер
*
*	@return 1 - новый размер меньше либо равен текущему; 2 - не удалось реаллоцировать;\
 0 - все прошло нормально
*/

int IncreaseList(list_t* list, int newSize) {
	assert(list != NULL);
	assert(newSize > 0);
	assert(list->free > 0);
	assert(list->next[list->free] == 0);
	
	if (newSize <= list->curMaxSize) {
		return 1;
	}

	list_elem_t* newDataMem = (list_elem_t*)realloc(list->data, (newSize + 1) * sizeof(list_elem_t));
	if (newDataMem == NULL) {
		return 2;
	}

	int* newNextMem = (int*)realloc(list->next, (newSize + 1) * sizeof(int));
	if (newNextMem == NULL) {
		return 2;
	}

	int* newPrevMem = (int*)realloc(list->prev, (newSize + 1) * sizeof(int));
	if (newPrevMem == NULL) {
		return 2;
	}

	list->data = newDataMem;
	list->next = newNextMem;
	list->prev = newPrevMem;

	list_elem_t emptyelem = list->emptyelem;
	for (int i = list->curMaxSize + 1; i <= newSize; i++) {
		list->data[i] = emptyelem;
		list->prev[i] = -1;
	}

	for (int i = list->curMaxSize + 1; i <= newSize - 1; i++) {
		list->next[i] = i + 1;
	}
	list->next[newSize] = 0;

	list->next[list->free] = list->curMaxSize + 1;
	list->curMaxSize = newSize;

	return 0;

#ifdef _DEBUG
	RecalcHash(list);
	if (ListOk(list)) {
		PrintList_OK(*list);
	}
	else {
		PrintList_NOK(*list);
	}
#endif
}


/**
*	Вставляет элемент в начало списка
*
*	@param list Список
*	@param[in] elem Элемент
*
*	@return Позиция, по которой вставился элемент; -1 - не удалось увеличить размер списка; \
-2 - на вход подался список с ошибкой (только в режиме отладки)
*/

int InsertBeg(list_t* list, list_elem_t elem) {
	assert(list != NULL);

#ifdef _DEBUG
	if (!ListOk(list)) {
		PrintList_NOK(*list);
		return -2;
	}
#endif

	if (list->size == list->curMaxSize - 1) {
		if (IncreaseList(list, list->curMaxSize * 2) != 0) {
			return -1;
		}
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
*	@return Позиция, по которой вставился элемент; -1 - не удалось увеличить размер списка; \
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

	if (list->size == list->curMaxSize - 1) {
		if (IncreaseList(list, list->curMaxSize * 2) != 0) {
			return -1;
		}
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
*	@return Позиция, по которой вставился элемент; -1 - не удалось увеличить размер списка; \
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

	if (list->size == list->curMaxSize - 1) {
		if (IncreaseList(list, list->curMaxSize * 2) != 0) {
			return -1;
		}
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

	free(list->data);

	return 0;
}