#pragma once

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
	int secureVarBeg = 0;                    ///<������ ���������
#endif

	list_elem_t data[listMaxSize] = {};
	int next[listMaxSize] = {};
	int prev[listMaxSize] = {};
	int head = 0;
	int tail = 0;
	int free = 1;
	list_elem_t emptyelem = -2147483647;    ///<�������, ��������������� �������
	int size = 0;

#ifdef _DEBUG
	char name[30] = "";                      ///<��� ������
	int err = 0;                             ///<��� ������, ������������ � ������:\n
											 ///0 - ��� ������\n
											 ///1, 2 - ����� �������������� �� ������� ��� ������ ������� ������\n
											 ///3, 4 - ��������� �������������� ����� ��� ������ ���������\n
											 ///5 - �������� ���-�����\n
											 ///6 - �������� � �������� next\n
											 ///7 - �������� � �������� prev\n
											 ///8 - �������� �� ���������� ����������
	unsigned int hash = 0;                   ///<���-�����
	int secureVarEnd = 0;                    ///<������ ���������
#endif
};
#pragma pack()




list_t ListConstructor(const char name[]);

int InsertBeg(list_t* list, list_elem_t elem);

int InsertEnd(list_t* list, list_elem_t elem);

int Insert(list_t* list, int phIndex, list_elem_t elem);

int DeleteBeg(list_t* list);

int DeleteEnd(list_t* list);

int Delete(list_t* list, int phIndex);

int ListDestructor(list_t* list);