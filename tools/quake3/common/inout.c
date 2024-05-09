/*
   Copyright (C) 1999-2006 Id Software, Inc. and contributors.
   For a list of contributors, see the accompanying CONTRIBUTORS file.

   This file is part of GtkRadiant.

   GtkRadiant is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   GtkRadiant is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GtkRadiant; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

//-----------------------------------------------------------------------------
//
//
// DESCRIPTION:
// deal with in/out tasks, for either stdin/stdout or network/XML stream
//

#include "cmdlib.h"
#include "mathlib.h"
#include "polylib.h"
#include "inout.h"
#include <sys/types.h>
#include <sys/stat.h>

#include <setjmp.h>

#ifdef WIN32
#include <direct.h>
#include <windows.h>
#endif

// network broadcasting
#include "l_net/l_net.h"

#ifdef WIN32
HWND hwndOut = NULL;
qboolean lookedForServer = qfalse;
UINT wm_BroadcastCommand = -1;
#endif

typedef void (*OutputFunc)(const char* text);
extern OutputFunc g_OutputFunc;


socket_t *brdcst_socket;
netmessage_t msg;

qboolean verbose = qfalse;


// in include
#include "stream_version.h"

void Broadcast_Setup( const char *dest ){
	address_t address;
	char sMsg[1024];

	Net_Setup();
	Net_StringToAddress( dest, &address );
	brdcst_socket = Net_Connect( &address, 0 );
	if ( brdcst_socket ) {
		// send in a header
		sprintf( sMsg, "<?xml version=\"1.0\"?><q3map_feedback version=\"" Q3MAP_STREAM_VERSION "\">" );
		NMSG_Clear( &msg );
		NMSG_WriteString( &msg, sMsg );
		Net_Send( brdcst_socket, &msg );
	}
}

void Broadcast_Shutdown(){
	if ( brdcst_socket ) {
		Sys_Printf( "Disconnecting\n" );
		Net_Disconnect( brdcst_socket );
		brdcst_socket = NULL;
	}
}

// all output ends up through here
void FPrintf( int flag, char *buf ){
	char level[2];

	fprintf(stderr, "%s", buf );

	if (g_OutputFunc)
	{
		buf[strlen(buf)-1] = '\0'; // remove last \n
		g_OutputFunc(buf);
	}
}

void Sys_FPrintf( int flag, const char *format, ... ){
	char out_buffer[4096];
	va_list argptr;

	if ( ( flag == SYS_VRB ) && ( verbose == qfalse ) ) {
		return;
	}

	va_start( argptr, format );
	vsprintf( out_buffer, format, argptr );
	va_end( argptr );

	FPrintf( flag, out_buffer );
}

void Sys_Printf( const char *format, ... ){
	char out_buffer[4096];
	va_list argptr;

	va_start( argptr, format );
	vsprintf( out_buffer, format, argptr );
	va_end( argptr );

	FPrintf( SYS_STD, out_buffer );
}

/*
   =================
   Error

   For abnormal program terminations
   =================
 */
extern jmp_buf g_ErrorHandler;

void Error( const char *error, ... ){
	char out_buffer[4096];
	char tmp[4096];
	va_list argptr;

	va_start( argptr,error );
	vsprintf( tmp, error, argptr );
	va_end( argptr );

	sprintf( out_buffer, "************ ERROR ************\n%s\n", tmp );

	FPrintf( SYS_ERR, out_buffer );

	Broadcast_Shutdown();
	
	longjmp(g_ErrorHandler, 1);
}
