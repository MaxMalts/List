#pragma once

#ifdef _DEBUG
#define PrintList_OK(list) ListDump(&list, __FILE__, __LINE__, __FUNCTION__, "just looking");
#define PrintList_NOK(list) ListDump(&list, __FILE__, __LINE__, __FUNCTION__, "list has an error");
#else
#define PrintList_OK(list) ;
#define PrintList_NOK(list) ;
#endif

typedef int list_elem_t;

#ifdef _DEBUG
enum err_type {              ///<Код ошибки, содержащейся в списке
	no_err,                  ///<0 - нет ошибок
	underflow,               ///<1, 2 - выход соответственно за вержнюю или нижнюю границу списка
	overflow,
	first_canary_spoiled,    ///<3, 4 - испорчена соответственно левая или правая канарейка
	second_canary_spoiled,
	invalid_hash,            ///<5 - неверная хэш-сумма
	next_err,                ///<6 - Проблемы с массивом next
	prev_err,                ///<7 - Проблемы с массивом prev
	free_err,                ///<8 - Проблемы со свободными элементами
};
#endif

#pragma pack(1)
struct list_t {
#ifdef _DEBUG
	int secureVarBeg = 0;                    ///<Первая канарейка
#endif

	list_elem_t* data = {};
	int* next = {};
	int* prev = {};
	int head = 0;
	int tail = 0;
	int free = 1;
	list_elem_t emptyelem = -2147483647;    ///<Элемент, соответствующий пустому
	int size = 0;
	int curMaxSize = 0;
	const int delta = 5;

#ifdef _DEBUG
	char name[30] = "";                      ///<Имя списка
	err_type err = no_err;                   ///<Код ошибки, содержащейся в списке:\n
	                                         
	unsigned int hash = 0;                   ///<Хэш-сумма
	int secureVarEnd = 0;                    ///<Вторая канарейка
#endif
};
#pragma pack()




list_t ListConstructor(const char name[] = "list");

int InsertBeg(list_t* list, list_elem_t elem);

int InsertEnd(list_t* list, list_elem_t elem);

int Insert(list_t* list, int phIndex, list_elem_t elem);

int DeleteBeg(list_t* list);

int DeleteEnd(list_t* list);

int Delete(list_t* list, int phIndex);

int ListDestructor(list_t* list);