// swhosts2.cpp : Defines the entry point for the application.
//

//#include "stdafx.h"
//#include "swhosts2.h"
#include <Windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ShellAPI.h>

struct hosts_s{
   char mac[ 128 ];
   char hosts_fn[ 512 ];
};
typedef struct hosts_s hosts_t, *hosts_p_t;

HANDLE SpawnAndRedirect(LPSTR commandLine, HANDLE *hStdOutputReadPipe, LPCTSTR lpCurrentDirectory)
{
	HANDLE hStdOutputWritePipe, hStdOutput, hStdError;
	CreatePipe(hStdOutputReadPipe, &hStdOutputWritePipe, NULL, 0);	// create a non-inheritable pipe
	DuplicateHandle(GetCurrentProcess(), hStdOutputWritePipe,
		GetCurrentProcess(), &hStdOutput,	// duplicate the "write" end as inheritable stdout
		0, TRUE, DUPLICATE_SAME_ACCESS);
	DuplicateHandle(GetCurrentProcess(), hStdOutput,
		GetCurrentProcess(), &hStdError,	// duplicate stdout as inheritable stderr
		0, TRUE, DUPLICATE_SAME_ACCESS);
	CloseHandle(hStdOutputWritePipe);								// no longer need the non-inheritable "write" end of the pipe

	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
	si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);	// (this is bad on a GUI app)
	si.hStdOutput = hStdOutput;
	si.hStdError  = hStdError;
	si.wShowWindow = SW_HIDE;						// IMPORTANT: hide subprocess console window
	//TCHAR commandLineCopy[1024];					// CreateProcess requires a modifiable buffer
	//strcpy(commandLineCopy, commandLine);
	if (!CreateProcess(	NULL, commandLine, NULL, NULL, TRUE,
		CREATE_NEW_CONSOLE, NULL, lpCurrentDirectory, &si, &pi))
	{
		CloseHandle(hStdOutput);
		CloseHandle(hStdError);
		CloseHandle(*hStdOutputReadPipe);
		*hStdOutputReadPipe = INVALID_HANDLE_VALUE;
		return NULL;
	}

	CloseHandle(pi.hThread);
	CloseHandle(hStdOutput);
	CloseHandle(hStdError);
	return pi.hProcess;
}

int getcmdexebuffer( char *cmdline, char *buffer, int bsz )
{
	DWORD read;
	int err, rt = FALSE;
	HANDLE hOutput, hProcess;

	hProcess = SpawnAndRedirect( cmdline, &hOutput, NULL );
	if (!hProcess ) return rt;
	err = ReadFile( hOutput, buffer, bsz, &read, NULL );
	if( err == TRUE && read != 0 ){
		buffer[read] = '\0';
		rt = TRUE;
	}
	CloseHandle(hOutput);
	CloseHandle(hProcess);

	return rt;
}


int getcmdpath(  char *cmdpath )
{
	char *cmdline;
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	char cmdline2[ 512 ];
	int len;

	cmdline = GetCommandLine( );
	
	if( cmdline[ 0 ] == '"' ) strcpy( cmdline2, &( cmdline[ 1 ] ));
	else strcpy( cmdline2, cmdline );
	len = strlen( cmdline2 );
	if( cmdline2[ len - 1 ] == '"' || cmdline2[ len - 1 ] == ' ' ) cmdline2[ len - 1 ] = 0;
	len = strlen( cmdline2 );
	if( cmdline2[ len - 1 ] == '"' || cmdline2[ len - 1 ] == ' ' ) cmdline2[ len - 1 ] = 0;

	_splitpath( cmdline2, drive, dir, fname, ext );
	_makepath( cmdpath, drive, dir, NULL, NULL );

	if(cmdpath[ strlen( cmdpath ) - 1 ] == '\\' ) cmdpath[ strlen( cmdpath ) - 1 ] = 0;

	return 0;
}


int fileisexist( char *fn )
{
	FILE *fp;

	fp = fopen( fn, "rb" );
	if( fp == NULL ) return FALSE;

	fclose( fp );
	return TRUE;

}

void TrimString( char *line )
//过滤注释行（假定以"//"开头），去除回车换行符，以及头尾空格
{
     int i, len;

     if( line == NULL ) return;

     //清除回车换行符号
     if( line[ strlen( line ) - 1 ] == 0x0A || line[ strlen( line ) - 1 ] == 0x0D )
          line[ strlen( line ) - 1 ] = '\0';            
     if( line[ strlen( line ) - 1 ] == 0x0A || line[ strlen( line ) - 1 ] == 0x0D )
          line[ strlen( line ) - 1 ] = '\0';        


     //裁剪尾部字空格
     len = strlen( line );
     for( i = len - 1; i >= 0; i-- ){
          if( line[ i ] == ' ' ) line[ i ] = '\0';
          else break;
     }

     //裁剪头部空格
     strrev( line );
     len = strlen( line );
     for( i = len - 1; i >= 0; i-- ){
          if( line[ i ] == ' ' ) line[ i ] = '\0';
          else break;
     }
     strrev( line );

     if( line[ 0 ] == '/' && line[ 1 ] == '/' )
          line[ 0 ] = 0; //过滤注释
     if( line[ 0 ] == '#' )
          line[ 0 ] = 0; //过滤注释

     return;
}

int gethostinfo( char *cmdpath, hosts_t *myhosts, int *num )
{
	FILE *fp;
	int i = 0;
	char inifile[ 512 ], line[ 512 ], *strp;

	*num = 0;
	sprintf( inifile, "%s\\%s", cmdpath, "swhosts.ini" );
	fp = fopen( inifile, "rb" );
	if( fp == NULL ) return FALSE;

	while( TRUE ){

		if( fgets( line, 511, fp ) == NULL ) break;
		TrimString( line );
		if( line[ 0 ] == 0 ) continue;
		
		strp = strtok( line, " " ); //mac string
		if( strp != NULL ) strcpy( myhosts[ i ].mac, strp );
		else continue;
		
		strp = strtok( NULL, " " ); //hosts file string
		if( strp != NULL ) sprintf( myhosts[ i ].hosts_fn, "%s\\%s", cmdpath, strp );
		else continue;

		i++;
	}
	fclose( fp );

	*num = i;

	return TRUE;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	char buffer[ 2048 ];
	char cmdpath[ 512 ], winpath[ 512 ], cmdfile[ 512 ];
	hosts_t myhosts[ 32 ];
	int num, err, i, find;
	FILE *fp;

	getcmdpath( cmdpath );
	GetWindowsDirectory( winpath, 511 );
	strcat( winpath, "\\system32" ); // c:\windows\system32
	
	num = 0;
	err = gethostinfo( cmdpath, myhosts, &num );
	if( err == FALSE ) return FALSE;
	
	err = getcmdexebuffer( "arp -a", buffer, 2048 );
	if( err == FALSE ) return FALSE;

	sprintf( cmdfile, "%s%s", cmdpath, "\\swh.bat" );
	fp = fopen( cmdfile, "wb" );
	if( fp == NULL ) return FALSE;
	find = 0;
	for( i = 0; i < num; i++ ){
		if( strstr( buffer, myhosts[ i ].mac ) == NULL ) continue;
		if( fileisexist( myhosts[ i ].hosts_fn ) ){ //switch router hosts 
			fprintf( fp, "copy %s %s\\drivers\\etc\\hosts\n", myhosts[ i ].hosts_fn, winpath );
			find = 1;
			break;
		}
	}
	if( find == 0 ){
	    //switch default hosts
		fprintf( fp, "copy %s\\hosts.default %s\\drivers\\etc\\hosts\n", cmdpath, winpath );
	}

	fclose( fp );

	ShellExecute( NULL, "open", cmdfile, NULL, NULL, SW_HIDE );
	//DeleteFile( cmdfile );

	return TRUE;
}
