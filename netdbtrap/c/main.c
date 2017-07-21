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
 * $Id: main.c,v 1.2 2001/06/15 15:03:50 netch Exp $
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <db.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "pvector.h"

#define BUF_SZ 1000
#define ADDR_NMAX 40

enum { FALSE, TRUE };
typedef unsigned char BOOL;

typedef struct {
   int HErrno, HAddrType;
   char* HostName;
   PVector Aliases;
   PVector Addresses;
} NetEnt_Response;

static const char Spaces[] = " \t\r\n\v\f";

static int Execute( const char* Line );
static int DoGetHostByName( const char* Command );
void strnlcpy( char* To, size_t S, const char* From );
void strnlcat( char* To, size_t S, const char* From );
BOOL strbuffull( const char* B, size_t S );
void strnlcpynz( char* To, size_t S, const char* From, size_t FS );
static BOOL Weq( const char* str1, size_t len1, const char* str2 );
static void GetToken( const char** BufP, const char** TokenP, size_t* LenP );
#ifndef HAVE_STRNDUP
static char* strndup( const char*, unsigned );
#endif
NetEnt_Response* TransformResponse( struct hostent* he );
static int SInsert( PVector*, const char* );
static int SIntersect( PVector* D, PVector* S1, PVector* S2 );
static int SCopy( PVector* D, PVector* S0 );
static int SDifference( PVector* D, PVector* S1, PVector* S2 );
static int SSymmetricDifference( PVector* D, PVector* S1, PVector* S2 );

typedef struct {
   const char* Name;
   short FunctionCode;
} RelationDef;

static RelationDef Relations[] = {
   { "if_common", 1 },
   { "if_any_common", 1 },
   { "if_none_common", 9 },
   { "if_new", 2 },
   { "if_any_extra", 2 },
   { "if_any_another", 2 },
   { "if_none_extra", 10 },
   { "if_none_another", 10 },
   { "if_exist", 3 },
   { "if_any", 3 },
   { "if_none", 11 },
   { "if_any_absent", 4 },
   { "if_none_absent", 12 },
   { "if_another", 6 },
   { "if_differs", 6 },
   { "if_not_differs", 14 },
   // terminator
   { NULL, 0 }
};

int
main( int argc, char *argv[] )
{
   int i;
   int Ret = 0;
   char LB[ BUF_SZ ];
   BOOL bWasFile = FALSE;
   for( i = 1; i < argc; i++ ) {
      FILE* fp;
      BOOL bStdin, bWaitCont;
      int nLine;
      char LB1[ 200 ];
      bStdin = !strcmp( argv[i], "-" );
      bWasFile = TRUE;
      fp = bStdin ? stdin : fopen( argv[i], "r" );
      if( !fp ) {
         fprintf( stderr, "netdbtrap: fopen(%s) failed: %s\n", argv[i],
               strerror( errno ) );
         continue;
      }
      nLine = 0;
      LB[ 0 ] = 0;
      bWaitCont = FALSE;
      while( !feof( fp ) && !ferror( fp ) ) {
         char* p;
         size_t n1;
         BOOL bNeedCont;
         if( !fgets( LB1, sizeof LB1, fp ) ) {
            if( feof( fp ) || ferror( fp ) )
               break;
            continue;
         }
         nLine++;
         p = LB1;
         if( bWaitCont ) {
            while( *p && strchr( " \t\r\n\v\f", *p ) && *p != '\n' )
               p++;
         }
         strnlcat( LB, sizeof LB, p );
         n1 = strlen( LB );
         if( n1 >= sizeof LB - 2 ) {
            fprintf( stderr, "netdbtrap: too long line %d at file %s\n",
                  nLine, argv[i] );
            break;
         }
         bNeedCont = ( n1 >= 2 &&
               LB[ n1-2 ] == '\\' && LB[ n1-1 ] == '\n' );
         if( bNeedCont )
            LB[ n1-2 ] = 0;
         bWaitCont = bNeedCont;
         if( !bNeedCont && n1 >= 1 && LB[ n1-1 ] == '\n' ) {
            int Ret2 = Execute( LB );
            if( Ret2 != 0 ) {
               fprintf( stderr,
                     "netdbtrap: retcode %d on file %s, line %d\n",
                     Ret2, argv[i], nLine );
               Ret |= Ret2;
            }
            sleep( 2 );
            LB[ 0 ] = 0;
         }
      }
      if( ferror( fp ) ) {
         fprintf( stderr, "netdbtrap: error reading file %s\n", argv[i] );
      }
      fclose( fp );
   }
   if( !bWasFile ) {
      fprintf( stderr, "netdbtrap: usage: no input files specified\n" );
      Ret |= 1;
   }
   fflush( NULL );
   return Ret;
}

static int
Execute( const char* Line )
{
   const char *p1, *p2, *p3;
   int n1, n2;
   p1 = Line;
   p1 += strspn( p1, Spaces );
   if( !*p1 || !isalpha( *p1 ) )
      return 0; /* comment line */
   p2 = p1 + strcspn( p1, Spaces );
   n1 = p2 - p1;
   p2 += strspn( p2, Spaces );
   p3 = p2 + strcspn( p2, Spaces );
   n2 = p3 - p2;
   p3 += strspn( p3, Spaces );
   if( Weq( p1, n1, "r" ) && Weq( p2, n2, "gethostbyname" ) )
      return DoGetHostByName( p3 );
   fprintf( stderr, "netdbtrap: Execute(): unknown command\n" );
   return 2;
}

static int
DoGetHostByName( const char* Command )
{
   /* Format: hostname key addresses */
   char Host[ 300 ], TestTypeName[ 30 ];
   int nTestType;
   const char *pLast, *pNext;
   size_t LastLen;
   int i, se, rc = -1;
   PVector EtalonSet;
   PVector OpResultSet;
   struct hostent* phe;
   NetEnt_Response* R;

   pNext = Command + strspn( Command, Spaces );

   GetToken( &pNext, &pLast, &LastLen );
   strnlcpynz( Host, sizeof Host, pLast, LastLen );
   if( strbuffull( Host, sizeof Host ) ) {
      fprintf( stderr, "netdbtrap++: GetHostByName(): Host too long\n" );
      return 1;
   }
   for( i = 0; Host[ i ] != 0; i++ ) {
      if( !strchr( "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz-.", Host[ i ] ) )
      {
         fprintf( stderr, "netdbtrap++: invalid characters in host name\n" );
         return 1;
      }
   }

   GetToken( &pNext, &pLast, &LastLen );
   strnlcpynz( TestTypeName, sizeof TestTypeName, pLast, LastLen );
   if( strbuffull( TestTypeName, sizeof TestTypeName ) ) {
      fprintf( stderr, "netdbtrap++: GetHostByName(): TestType too long\n" );
      return 1;
   }
   nTestType = -1;
   for( i = 0; ; i++ ) {
      if( !Relations[i].Name )
         break;
      if( !strcmp( Relations[i].Name, TestTypeName ) ) {
         nTestType = Relations[i].FunctionCode;
         break;
      }
   }
   if( nTestType < 0 ) {
      fprintf( stderr, "netdbtrap: unknown TestType: %s\n", TestTypeName );
      return 1;
   }

   /* Scan addresses */
   PVec_Init( &EtalonSet, 0, 0 );
   while( *pNext != 0 ) {
      char* Addr;
      GetToken( &pNext, &pLast, &LastLen );
      Addr = strndup( pLast, LastLen );
      if( !Addr ) {
         fprintf( stderr, "netdbtrap: GetHostByName(): no memory\n" );
         return 1;
      }
      if( SInsert( &EtalonSet, Addr ) < 0 ) {
         fprintf( stderr, "netdbtrap: GetHostByName(): no memory\n" );
         free( Addr );
         return 1;
      }
   }

   // Сделать резолвинг, получить реальные адреса
   phe = gethostbyname( Host );
   se = h_errno;
   if( se != 0 && se != HOST_NOT_FOUND && se != NO_DATA )
   {
      fprintf( stderr, "netdbtrap++: resolving error: host %s: ", Host );
      if( se == TRY_AGAIN )
         fprintf( stderr, "TRY_AGAIN" );
      else if( se == NO_RECOVERY )
         fprintf( stderr, "NO_RECOVERY" );
      else
         fprintf( stderr, "%d", se );
      fputc( '\n', stderr );
      return 1;
   }
   if( phe == NULL ) {
      if( se != HOST_NOT_FOUND && se != NO_DATA ) {
         fprintf( stderr,
               "netdbtrap: netdb failure with unreported reason\n" );
         return 1;
      }
   }
   R = TransformResponse( phe );
   if( !R ) {
      fprintf( stderr, "netdbtrap: no memory\n" );
      return 1;
   }
   R->HErrno = se;

   // Определить требуемую ситуацию,
   // произвести резолвинг, вывести результат

   PVec_Init( &OpResultSet, 0, 0 );
   switch( nTestType & 7 )
   {
      case 0:
      case 5:
      case 7:
         fprintf( stderr, "netdbtrap(%d): software error\n", __LINE__ );
         return 1;
      break;
      case 1: // if_any_common, if_none_common
         rc = SIntersect( &OpResultSet, &EtalonSet, &R->Addresses );
      break;
      case 2: // if_any_extra, if_none_extra
         rc = SDifference( &OpResultSet, &R->Addresses, &EtalonSet );
      break;
      case 3: // if_any, if_none
         rc = SCopy( &OpResultSet, &R->Addresses );
      break;
      case 4: // if_any_absent, if_none_absent
         rc = SDifference( &OpResultSet, &EtalonSet, &R->Addresses );
      break;
      case 6: // if_differs, if_not_differs
         rc = SSymmetricDifference( &OpResultSet, &EtalonSet, &R->Addresses );
      break;
   }
   if( rc != 0 ) {
      fprintf( stderr, "netdbtrap: set operation failed\n" );
      return 1;
   }

   if( nTestType < 8 && OpResultSet.Count > 0 ) {
      for( i = 0; i < OpResultSet.Count; i++ ) {
         printf( "netdbtrap: host %s: %s: %s\n",
               Host, TestTypeName, 
               ( const char* ) PVec_GetAt( &OpResultSet, i ) );
      }
   }
   if( nTestType >= 8 && OpResultSet.Count == 0 ) {
      printf( "netdbtrap: host %s: %s: TRAP\n", Host, TestTypeName );
   }
   return 0;
}

static void GetToken( const char** BufP, const char** TokenP,
      size_t* LenP )
{
   const char *pp1, *pp2;
   pp1 = *BufP;
   pp1 += strspn( pp1, Spaces );
   pp2 = pp1 + strcspn( pp1, Spaces );
   *LenP = pp2 - pp1;
   pp2 += strspn( pp2, Spaces );
   *BufP = pp2;
   *TokenP = pp1;
}

NetEnt_Response* TransformResponse( struct hostent* he )
{
   NetEnt_Response* R;
   char** pp;
   R = ( NetEnt_Response*) calloc( 1, sizeof( NetEnt_Response ) );
   if( !R )
      return NULL;
   PVec_Init( &R->Aliases, 0, 0 );
   PVec_Init( &R->Addresses, 0, 0 );
   /* Return empty object if !he */ 
   if( !he )
      return R; 
   R->HostName = strdup( he->h_name );
   if( !R->HostName )
      return NULL;
   for( pp = he->h_aliases; *pp != NULL; pp++ ) {
      char* ss;
      ss = strdup( *pp );
      if( !ss ) {
         fprintf( stderr, "netdbtrap: no memory\n" );
         return NULL;
      }
      if( ( PVec_Add( &R->Aliases, ss ) ) < 0 ) {
         fprintf( stderr, "netdbtrap: no memory\n" );
         return NULL;
      }
   }
   R->HAddrType = he->h_addrtype;
   for( pp = he->h_addr_list; *pp != NULL; pp++ ) {
      struct in_addr* iap;
      char* ss;
      iap = ( struct in_addr* ) *pp;
      ss = strdup( inet_ntoa( *iap ) );
      if( !ss ) {
         fprintf( stderr, "netdbtrap: no memory\n" );
         return NULL;
      }
      if( ( PVec_Add( &R->Addresses, ss ) ) < 0 ) {
         fprintf( stderr, "netdbtrap: no memory\n" );
         return NULL;
      }
   }
   return R;
}

/*------------------------------------------------------------*/

static BOOL
SHas( PVector* V, const char* s )
{
   int i;
   for( i = 0; i < V->Count; i++ ) {
      char* E = ( char*) PVec_GetAt( V, i );
      if( !E )
         continue;
      if( !strcmp( E, s ) )
         return TRUE;
   }
   return FALSE;
}

static int
SInsert( PVector* V, const char* s )
{
   int rc;
   if( SHas( V, s ) )
      return 0;
   rc = PVec_Add( V, s );
   if( rc < 0 )
      return rc;
   return 1;
}

static int
SIntersect( PVector* D, PVector* S1, PVector* S2 )
{
   int i, rc;
   char* DE;
   for( i = 0; i < S1->Count; i++ ) {
      char* E = ( char*) PVec_GetAt( S1, i );
      if( !E )
         continue;
      if( !SHas( S2, E ) )
         continue;
      DE = strdup( E );
      if( !DE )
         return -1;
      rc = SInsert( D, DE );
      if( rc < 0 )
         return rc;
      if( rc == 0 )
         free( DE );
   }
   return 0;
}

static int
SCopy( PVector* D, PVector* S0 )
{
   int i, rc;
   char* DE;
   for( i = 0; i < S0->Count; i++ ) {
      char* E = ( char*) PVec_GetAt( S0, i );
      if( !E )
         continue;
      DE = strdup( E );
      if( !DE )
         return -1;
      rc = SInsert( D, DE );
      if( rc < 0 )
         return rc;
      if( rc == 0 )
         free( DE );
   }
   return 0;
}

static int
SDifference( PVector* D, PVector* S1, PVector* S2 )
{
   int i, rc;
   char* DE;
   for( i = 0; i < S1->Count; i++ ) {
      char* E = ( char*) PVec_GetAt( S1, i );
      if( !E )
         continue;
      if( SHas( S2, E ) )
         continue;
      DE = strdup( E );
      if( !DE )
         return -1;
      rc = SInsert( D, DE );
      if( rc < 0 )
         return rc;
      if( rc == 0 )
         free( DE );
   }
   return 0;
}

static int
SSymmetricDifference( PVector* D, PVector* S1, PVector* S2 )
{
   int rc;
   /* I use here that SDifference() does not clear target vector */
   rc = SDifference( D, S1, S2 );
   if( rc < 0 )
      return rc;
   rc = SDifference( D, S2, S1 );
   if( rc < 0 )
      return rc;
   return 0;
}

/*------------------------------------------------------------*/


static BOOL Weq( const char* str1, size_t len1, const char* str2 )
{
   return len1 == strlen( str2 ) && !strncmp( str1, str2, len1 );
}

void strnlcpy( char* To, size_t S, const char* From )
{
   strncpy( To, From, S - 1 );
   To[ S - 1 ] = 0;
}

size_t strnlen( const char* B, size_t S )
{
   register size_t C = 0;
   while( C < S && B[ C ] != 0 )
      C++;
   return C;
}

void strnlcat( char* To, size_t S, const char* From )
{
   size_t L;
   S--;
   To[ S ] = 0;
   L = strlen( To );
   if( L < S )
      strncpy( To + L, From, S - L );
   To[ S ] = 0;
}

BOOL strbuffull( const char* B, size_t S )
{
   return strnlen( B, S - 1 ) >= S - 1;
}

void strnlcpynz( char* To, size_t S, const char* From, size_t FS )
{
   size_t RS = ( S > FS+1 ) ? FS+1 : S;
   strnlcpy( To, RS, From );
}

#ifndef HAVE_STRNDUP
static char*
strndup( const char* ss, unsigned maxlen )
{
   char* res;
   unsigned realsize = strnlen( ss, maxlen );
   res = (char*) malloc( realsize + 1 );
   if( !res )
      return NULL;
   strnlcpy( res, realsize+1, ss );
   return res;
}
#endif
