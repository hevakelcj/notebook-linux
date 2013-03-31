// NoteBook.cpp : 定义控制台应用程序的入口点。
//
#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include "Encrypt.h"
#include "NoteBook.h"
#include "VirtualArray.h"
#include "Interface.h"

using namespace std;

// 销毁文件
int DestoryFile( const char * pFileName )
{
	FILE *fp = NULL;
	fp = fopen( pFileName, "rb+" );
	if ( fp ){

		fseek( fp, 0, 2 );
		int len = ftell( fp );	// 读取文件大小
		rewind( fp );

		// 填充0
		while ( len-- ){
			fputc( 0, fp );
		}

		fclose( fp );
		fp = NULL;

		// 删除文件
		unlink( pFileName );
		cout << "-- INFO: File has been destory! --" << endl;

	} else {
		cout << "-- ERR: File doesn't exist! --" << endl;
	}
	
	return 0;
}

// 继续上次未完成的工作
int CloseTextFile( void )
{
	g_NoteMgr.LoadLog();

	if ( g_NoteMgr.GetLastStatus() == STATUS_CLOSE )
		return 0;

	CNoteFile _noteFile( g_NoteMgr.GetLastFile() );
	if ( _noteFile.Open() ){
		switch ( g_NoteMgr.GetLastStatus() ){
			case STATUS_APPEND:
				g_NoteMgr.EndAppend( _noteFile );
				break;
			case STATUS_EDIT:
				g_NoteMgr.EndEdit( _noteFile );
				break;
			case STATUS_READ:
				break;
		}
		g_NoteMgr.SaveLog( STATUS_CLOSE, NULL );
		char acTextFile[MAX_FILENAME_LEN];
		::ChangeFileSuffix( acTextFile, g_NoteMgr.GetLastFile(), "txt" );
		::DestoryFile( acTextFile );
	}else {
		cout << "-- ERR: Open file fail --" << endl;
	}
	return 0;
}

bool OpenNoteFile( CNoteFile &noteFile, char *pPwd )
{
	bool retVal = false;

	while ( !noteFile.Open() ){
		cout << "-- ERR: File not exist or format err! --" << endl;
		cout << "-- INFO: Will you want to create a new file? --" << endl;
		if ( !::GetUserChoice() ){
			return false;
		}
		g_NoteMgr.Create( noteFile.GetFileName() );
	}

	// NoteFile文件一定是打开成功了的
	if ( noteFile.IsLock() ){
		cout << "-- INFO: File has been lock, you should unlock it first! --" << endl;
		while (true){
			cout << "Input Password: ";
			if ( ::GetPassword( pPwd ) == 0 ){
				cout << "-- INFO: User cancel password vaild --" << endl;
				return false;
			}
			if ( noteFile.GetPasswordVaild( PWD_UNLOCK, pPwd ) ){
				noteFile.Unlock();
				noteFile.SaveHead();
				cout << "-- INFO: Lock success--" << endl;
				break;
			}
			usleep(2000);
		}
	}

	int	tries = 0;
	int _nLen = 0;
	
	cout << "-- INFO: Checking password. Please input password. --" << endl;
	// 密码验证
	while(true){

		if ( tries >= noteFile.GetTryTimes() ){
			break;
		}
		cout << "Please Input[" << noteFile.GetTryTimes() - tries <<"] : ";
		tries ++ ;

		_nLen = ::GetPassword( pPwd );
		if ( _nLen == 0 ){
			break;
		}
		if ( noteFile.GetPasswordVaild( PWD_NOTE, pPwd ) ){
			break;	
		}
	}
	if ( tries < noteFile.GetTryTimes() ){	// 尝试次数未超限

		if ( _nLen != 0 ){	// 用户输入了正确的密码
			
			cout << "-- INFO: Congratulation! Pass! --" << endl;
			retVal = true;

		}else {
			cout << "-- INFO: User cancel password vaild --" << endl;
		}
	} else {
		// 锁文件
		noteFile.Lock();
		noteFile.SaveHead();
		cout << "-- INFO: Try time out, file has been lock! --" << endl;
	}

	return retVal;
}

void Wait()
{
	cout << "-- INFO: File has been open, Input \"close\" to close file. -- " << endl;
	string str;
	do{
		cout << "Please Input: " ;
		cin >> str;
	}while ( str != "close" );
}

void ChangePassword( CNoteFile &noteFile )
{
	// 三种密码：打开密码、权即修改密码、解锁密码
	while (true){
		char choice ;
		do{
			cout << "-- INFO: Please select your chioce --" << endl;
			cout << "<1> Notebook Password" << endl;
			cout << "<2> Function Password" << endl;
			cout << "<3> Unlock Password" << endl;
			cout << "<Q> Return" << endl;
			cout << "Your chioce: ";
			cin >> choice;
		}while ( choice<'1' && choice>'3' && choice!='q' && choice!='Q' );
		cout << endl;

		switch( choice ){
			case '1' :	// 日志进入密码
				cout << "-- INFO: Changing notebook password --" << endl;
				if ( g_NoteMgr.ChangePassword( PWD_NOTE, noteFile ) == 0 ){
					cout << "-- INFO: You should restart --" << endl;
					cin.get();
					exit(0);
				}
				break;
			case '2' :	// 权限设置密码
				cout << "-- INFO: Changing function password --" << endl;
				if ( g_NoteMgr.ChangePassword( PWD_PRIV, noteFile ) == 0 ){
					cout << "-- INFO: Success! --" << endl;
				}
				break;
			case '3' :	// 解锁密码
				cout << "-- INFO: Changing unlock password --" << endl;
				if ( g_NoteMgr.ChangePassword( PWD_UNLOCK, noteFile ) == 0 ){
					cout << "-- INFO: Success! --" << endl;
				}
				break;
			case 'q' :
			case 'Q' :
				return;
		}
	}
}

void SetPrivilege( CNoteFile &noteFile )
{
	// 密码验证
	char acPassword[PWD_LEN];
	cout << "-- INFO: Setting function, please input function password --" << endl;

	while ( true ){
		cout << "Please Input: ";	
		if ( ::GetPassword(acPassword) == 0 ){
			cout << "-- INFO: User cancel password vaild --" << endl;
			return;
		}
		cout << endl;	
		if ( noteFile.GetPasswordVaild( PWD_PRIV, acPassword ) ){
			cout << "-- INFO: Congratulations! Pass! --" << endl;
			break;
		}
		usleep(2000);
	}

	// 三种权限：追加、阅读、编辑，开启与关闭
	while (true){
		char choice ;
		do{
			cout << "-- INFO: Please Select your chioce --" << endl;
			cout << "<1> Append [" << (noteFile.GetAppendEnable()?"On":"Off") << "]" << endl;
			cout << "<2> Read   [" << (noteFile.GetReadEnable()?"On":"Off") << "]" << endl;
			cout << "<3> Edit   [" << (noteFile.GetEditEnable()?"On":"off") << "]" << endl;
			cout << "<Q> return" << endl;
			cout << "Your chioce: ";
			cin >> choice;
		}while ( choice<'1' && choice>'3' && choice!='q' && choice!='Q' );
		cout << endl;

		switch( choice ){
			case '1' :	// 追加
				noteFile.SetAppendEnable( !noteFile.GetAppendEnable() );
				break;
			case '2' :	// 阅读
				noteFile.SetReadEnable( !noteFile.GetReadEnable() );
				break;
			case '3' :	// 编辑
				noteFile.SetEditEnable( !noteFile.GetEditEnable() );
				break;
			case 'q' :
			case 'Q' :
				return;
		}
		noteFile.SaveHead();
	}
}

void SetTryTime( CNoteFile &noteFile )
{
	cout << "-- INFO: Please set try times. --" << endl;
	int times = 0;
	cout << "Your chioce: ";
	cin >> times;
	if ( times > 0 ){
		noteFile.SetTryTimes( times );
		noteFile.SaveHead();
		cout << "-- INFO: Set success! current value is [" << times << "] --" << endl;
	}else{
		cout << "-- INFO: Fail --" << endl;
	}
}

// 设置
void Setting(  CNoteFile &noteFile )
{
	while (true){
		char choice ;
		do{
			cout << "-- INFO: Please select your chioce --" << endl;
			cout << "<1> Set Password" << endl;
			cout << "<2> Set Function" << endl;
			cout << "<3> Lock NoteBook" << endl;
			cout << "<4> Set Try times" << endl;
			cout << "<Q> return" << endl;
			cout << "Your chioce: ";
			cin >> choice;
		}while ( choice<'1' && choice>'4' && choice!='q' && choice!='Q' );
		cout << endl;

		switch( choice ){
			case '1' :
				ChangePassword( noteFile );
				break;
			case '2' :
				SetPrivilege( noteFile );
				break;
			case '3' :	// 加锁
				cout << "-- INFO: Do you really want to lock notebook? --" << endl;
				if ( ::GetUserChoice() ){
					noteFile.Lock( true );
					noteFile.SaveHead();
					cout << "-- INFO: NoteFile has been locked! --" << endl;
				}
				break;
			case '4' :
				SetTryTime( noteFile );
				break;
			case 'q' :
			case 'Q' :
				return;
		}
	}
}

void NoteBook( const char *pNoteFile )
{
	char	_acPassword[PWD_LEN+1];
	CNoteFile _noteFile( pNoteFile );

	if ( OpenNoteFile( _noteFile, _acPassword ) ){
		while (true){
			char choice ;
			do{
				cout << "-- INFO: Please select your chioce --" << endl;
				cout << "<1> Append" << endl;
				cout << "<2> Read" << endl;
				cout << "<3> Edit" << endl;
				cout << "<4> Setting" << endl;
				cout << "<Q> Exit" << endl;
				cout << "Please Input: ";
				cin >> choice;
			}while ( choice<'1' && choice>'4' && choice!='q' && choice!='Q' );
			cout << endl;
			
			char acTextFile[MAX_FILENAME_LEN];
			::ChangeFileSuffix( acTextFile, pNoteFile, "txt" );

			switch( choice ){
				case '1' :
                case 'a' :
					if ( g_NoteMgr.Append( _noteFile, _acPassword ) == 0 ){
						g_NoteMgr.SaveLog( STATUS_APPEND, pNoteFile );
						::Wait();
						g_NoteMgr.EndAppend( _noteFile );
						::DestoryFile( acTextFile );
					}
					break;

				case '2' :
                case 'r' :
					if ( g_NoteMgr.Read( _noteFile, _acPassword ) == 0 ){
						g_NoteMgr.SaveLog( STATUS_READ, pNoteFile );
						::Wait();
						g_NoteMgr.EndRead( _noteFile );
						::DestoryFile( acTextFile );
					}
					break;

				case '3' :
                case 'e' :
					if ( g_NoteMgr.Edit( _noteFile, _acPassword ) == 0 ){
						g_NoteMgr.SaveLog( STATUS_EDIT, pNoteFile );
						::Wait();
						g_NoteMgr.EndEdit( _noteFile );
						::DestoryFile( acTextFile );
					}
					break;

				case '4' :
                case 's' :
					Setting( _noteFile );
					break;

				case 'q' :
				case 'Q' :
					return;
			}
			g_NoteMgr.SaveLog( STATUS_CLOSE, NULL );
		}
	}else {
		cout << "-- INFO: Cann't open notefile. --" << endl;
		cin.get();
	}
}
void Wellcom()
{
	cout << "+-------------------------------------------+" << endl;
	cout << "|   Wellcome back! I am your loyal friend   |" << endl;
	cout << "+-------------------------------------------+" << endl;
	cout << "Press any to continue..." << endl;
	char ch = getchar();
	if ( ch == '?' ){
		cout << "I am <<NoteBook V1.0>>, Please use." << endl;
	}
	cout << endl << endl;
}

int main(int argc, char* argv[])
{
	char acNoteFile[MAX_FILENAME_LEN];
	char *pNoteFile = NULL;

	CloseTextFile();
	
	Wellcom();

	if ( argc < 2 ){
		cout << "-- INFO: Please input notefile name --" << endl;
		cout << "FileName : ";
		cin >> acNoteFile;
		pNoteFile = acNoteFile;
	}else {
		pNoteFile = argv[1];
	}
	NoteBook( pNoteFile );

	return 0;
}
