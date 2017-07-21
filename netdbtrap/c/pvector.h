#ifndef pvector_H_included
#define pvector_H_included 0

/*==============================================================
// PVector - специализированный вектор указателей.
// Указатели хранятся в виде void*; преобразование из целевого
// типа в тип для хранения и обратно - проблема использующего.
*/

enum { PVECTOR_MAGIC = 0xCAFEDAFA };

typedef struct {
   unsigned int Magic;
   void** Data;
   int Size, Count, Delta;
} PVector;

void PVec_Zero( PVector* );
void PVec_Clear( PVector*, void ( *clrfunc )( void* ) );
int PVec_Init( PVector* V, int BegSize, int Delta );
int PVec_Add( PVector* V, void* Data );
int PVec_AddZ( PVector* V );
void* PVec_GetAt( PVector* V, int ind );
void* PVec_GetLast( PVector* V );
int PVec_SetAt( PVector* V, int I, void* PP );

#undef  pvector_H_included
#define pvector_H_included 1
#endif
