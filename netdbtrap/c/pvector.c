/*
 * Copyright (c) 1999-2001 Valentin Nechayev <netch@netch.kiev.ua>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: pvector.c,v 1.1 2001/01/27 22:22:31 netch Exp $
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "pvector.h"

#define WLOG wlog( __LINE__, __FILE__ )

static void
wlog( int l, const char* f )
{
   fprintf( stderr, "netdbtrap: generic message from %s:%d\n", f, l );
}

/*==============================================================
//  PVector
*/

void PVec_Zero( PVector* V )
{
   if( !V )
      return;
   bzero( V, sizeof( *V ) );
   V->Magic = 0;
   V->Data = NULL;
   V->Size = V->Count = V->Delta = 0;
}

int PVec_Init( PVector* V, int BegSize, int Delta )
{
   if( !V ) { WLOG; return -1; }
   if( BegSize < 0 ) { WLOG; return -1; }
   PVec_Zero( V );
   V->Magic = PVECTOR_MAGIC;
   V->Data = NULL;
   V->Size = V->Count = 0;
   V->Delta = Delta;
   if( BegSize > 0 ) {
      size_t S = sizeof( void* ) * BegSize;
      V->Data = (void**) malloc( S );
      if( V->Data == NULL )
         return -1;
      V->Size = BegSize;
   }
   return 0;
}

void* PVec_GetAt( PVector* V, int I )
{
   if( !V ) { WLOG; return NULL; }
   if( V->Magic != PVECTOR_MAGIC ) { WLOG; return NULL; }
   if( I < 0 ) { WLOG; return NULL; }
   if( I >= V->Count ) { WLOG; return NULL; }
   if( V->Data == NULL ) { WLOG; return NULL; }
   return V->Data[ I ];
}

int PVec_SetAt( PVector* V, int I, void* PP )
{
   if( !V ) { WLOG; return -1; }
   if( V->Magic != PVECTOR_MAGIC ) { WLOG; return -1; }
   if( I < 0 ) { WLOG; return -1; }
   if( I >= V->Count ) { WLOG; return -1; }
   if( V->Data == NULL ) { WLOG; return -1; }
   V->Data[ I ] = PP;
   return 0;
}

int PVec_Expand( PVector* V, int ExpandBy )
{
   int NewSz;
   void* NewP = NULL;
   assert( V != NULL );
   if( V->Magic != PVECTOR_MAGIC ) { WLOG; return -1; }
   assert( ExpandBy > 0 );
   NewSz = ( V->Size + ExpandBy ) * sizeof( void* );
   if( !( NewP = realloc( V->Data, NewSz ) ) ) {
      WLOG;
      return -1;
   }
   V->Data = NewP;
   V->Size += ExpandBy;
   return 0;
}

int PVec_Add( PVector* V, void* PP )
{
   int ret;
   if( !V ) { WLOG; return -1; }
   if( V->Magic != PVECTOR_MAGIC ) { WLOG; return -1; }
   assert( V->Size >= 0 && V->Count >= 0 );
   assert( V->Count <= V->Size );
   if( V->Count == V->Size ) {
      int Delta = V->Delta;
      if( Delta < 4 )
         Delta = 4;
      if( Delta > 1000 )
         Delta = 1000;
      if( ( ret = PVec_Expand( V, Delta ) ) < 0 )
         return ret;
   }
   assert( V->Count < V->Size );
   assert( V->Data );
   V->Data[ V->Count++ ] = PP;
   return 0;
}

void PVec_Clear( PVector* V, void ( *clrfunc )( void* ) )
{
   if( !V ) { WLOG; return; }
   if( V->Magic != PVECTOR_MAGIC ) { WLOG; return; }
   if( clrfunc != NULL ) {
      while( V->Count > 0 ) {
         void* p;
         int i;
         i = V->Count - 1;
         p = V->Data[i];
         (*clrfunc)( p );
         V->Count--;
      }
   }
   free( V->Data );
   V->Data = NULL;
   V->Size = V->Count = 0;
}
