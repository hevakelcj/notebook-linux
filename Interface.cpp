
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "Interface.h"

using namespace std;

#define ASCII_ENTER      13
#define ASCII_TAB        '\t' 
#define ASCII_BACKSPACE  0x08
#define ASCII_SPACE      ' '
#define ASCII_ESC        27 

// 返回密码实际长度
int	GetPassword( char *password , int nMaxLen )
{
    strncpy( password, getpass(""), nMaxLen );
    return strlen( password );
}

char* ChangeFileSuffix( char *pOutFile, const char *pSrcFile, const char *pSuffix )
{
	if ( pOutFile && pSrcFile && pSuffix ){
		
		strncpy( pOutFile, pSrcFile, MAX_FILENAME_LEN );
		size_t len = strlen( pOutFile );
		
		char ch;
		do{
			ch = pOutFile[--len];
		}while ( ch != '.' && ch != '\\' && ch != '/' && len );
		
		if ( ch == '.' ){
			pOutFile[len+1] = '\0';
			strncat( pOutFile, pSuffix, MAX_FILENAME_LEN );
			pOutFile[len+4] = '\0';

			return pOutFile;
		}
	}

	return NULL;
}

bool	GetUserChoice( void )
{
	while (1){
		cout << "Yes or No(Y/N) : " ;
		char ch = getchar();
		switch ( ch ){
		case 'Y':
		case 'y':
			cout << "Yes" << endl;
			return true;
		case 'N':
		case 'n':
			cout << "No" << endl;
			return false;
		default:
			cout << "Please input again" << endl;
		}
	}
}

bool	GetNewPassword( char *password )
{
	int _nLen1, _nLen2;
	char _acComfirm[PWD_LEN];

	while(true){
		cout << "Please input new password: ";
		_nLen1 = ::GetPassword( password );
		if (_nLen1==0){
			break;
		}
		cout << "Input new password again: ";
		_nLen2 = ::GetPassword( _acComfirm );
		if (_nLen2==0){
			break;
		}
		if ( (_nLen1==_nLen2) && (strcmp( password , _acComfirm) == 0) ){
			break;
		}
		cout << "-- INFO: Password invaild, Please input again --" << endl;
	}

	if ( _nLen1 == 0 || _nLen2 == 0 ){
		cout << "-- INFO: User cancel --" << endl;
		return false;
	}else{
		return true;
	}
}
