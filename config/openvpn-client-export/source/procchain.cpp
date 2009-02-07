
/*
 * Copyright (c) 2008
 *      Shrew Soft Inc.  All rights reserved.
 *
 * AUTHOR : Matthew Grooms
 *          mgrooms@shrew.net
 *
 */

#include <windows.h>
#include <stdio.h>

bool runproc( char * path )
{
	STARTUPINFO si;
	memset( &si, 0, sizeof( si ) );
	si.cb = sizeof( si );

	PROCESS_INFORMATION pi;
	memset( &pi, 0, sizeof( pi ) );

	// Start the child process.
	if( !CreateProcess(
			NULL,		// No module name (use command line).
			path,		// Command line. 
			NULL,		// Process handle not inheritable. 
			NULL,		// Thread handle not inheritable. 
			FALSE,		// Set handle inheritance to FALSE. 
			0,			// No creation flags. 
			NULL,		// Use parent's environment block. 
			NULL,		// Use parent's starting directory. 
			&si,		// Pointer to STARTUPINFO structure.
			&pi ) )		// Pointer to PROCESS_INFORMATION structure.
	{
		return false;
	}

	// Wait until child process exits.
	WaitForSingleObject( pi.hProcess, INFINITE );

	// Get the exit code
	DWORD ExitCode;
	GetExitCodeProcess( pi.hProcess, &ExitCode );

	// Close process and thread handles. 
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );

	return ( ExitCode == 0 );
}

int APIENTRY WinMain(
	HINSTANCE hinstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow )
{
	FILE * fp;
	if( fopen_s( &fp, lpCmdLine, "r" ) )
		return -1;

	while( true )
	{
		char cmd[ MAX_PATH ];
		memset( cmd, 0, MAX_PATH );
		if( fgets( cmd, MAX_PATH, fp ) == NULL )
			break;

		char * term = strchr( cmd, '\n' );
		if( term != NULL )
			*term = 0;

		if( !runproc( cmd ) )
			return -2;
	}

	return 0;
}

