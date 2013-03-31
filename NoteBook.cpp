#include "NoteBook.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include "Interface.h"
#include "Encrypt.h"
#include "NoteFile.h"

using namespace std;

const char *_sNoteConfigFile = "notebook.log";

////////////////////////////////////////////////////////////////////////////

CNoteManager::CNoteManager()
{
	srand(static_cast <unsigned   int> (time(NULL)));
	LoadLog();
}

CNoteManager::~CNoteManager()
{
}

CNoteManager& CNoteManager::GetInstance()
{
	static CNoteManager instance;
	return instance;
}

void CNoteManager::LoadLog()
{
	FILE *fp = NULL;
	fp = fopen( _sNoteConfigFile, "rb" );
	if ( fp ){
		fread( &m_tLog, sizeof(m_tLog), 1, fp );
		fclose( fp );
	}else {
		m_tLog.eStatus = STATUS_CLOSE;
		m_tLog.acNoteFile[0] = '\0';
	}
}

void CNoteManager::SaveLog( EStatus eStatus , const char *pNoteFile )
{
	m_tLog.eStatus = eStatus;
	if ( pNoteFile ){
		strncpy( m_tLog.acNoteFile, pNoteFile, MAX_FILENAME_LEN );
	}

	FILE *fp = NULL;
	fp = fopen( _sNoteConfigFile, "wb" );
	if ( fp ){
		fwrite( &m_tLog, sizeof(m_tLog), 1, fp );
		fclose( fp );
	}else {
		cout << "-- INFO: Create notefile fail! --" << endl;
	}
}

EStatus CNoteManager::GetLastStatus()
{
	return m_tLog.eStatus;
}

const char *CNoteManager::GetLastFile()
{
	return m_tLog.acNoteFile;
}
int CNoteManager::Create( const char *pFileName )
{
	char	_acPassword[PWD_LEN+1];
	int		retVal = 0;

	cout << "-- INFO: Creatting new notebook... --" << endl;
	
	if ( pFileName == NULL || *pFileName == 0 ){
		cout << "-- ERR: File invaild --" << endl;
		return ERRNO_PARAM_INVAILD;
	}

	CNoteFile	_noteFile( pFileName );

	if ( _noteFile.Create() ){
		if ( GetNewPassword( _acPassword ) ){

			_noteFile.SetDefault();

			char _acTextBuff[50] = "";
			time_t _currTime = time(NULL);
			//ctime_s(_acTextBuff, 50, &_currTime);
		    strncpy( _acTextBuff, ctime(&_currTime), 50 );	
            _acTextBuff[ strlen(_acTextBuff)-1 ] = '\0'; // 去除ctimes()最后生成的 '\n'
			strncat( _acTextBuff, "\xd\xa创建日记本", 50 );
			int _nLen = (int)strlen(_acTextBuff);

			_noteFile.SaveBody( _acTextBuff, _nLen, _acPassword );
			_noteFile.SetPasswordVaild( PWD_NOTE, _acPassword );
			_noteFile.SetPasswordVaild( PWD_PRIV, _acPassword );
			_noteFile.SetPasswordVaild( PWD_UNLOCK, _acPassword );
			_noteFile.SetDataLen( _nLen );
			_noteFile.SetStatus( STATUS_CLOSE );
			_noteFile.SaveHead();
			
		}
		_noteFile.Close();
	} else {
		cout << "-- INFO: Creatting notebook fail! --" << endl;
		retVal = ERRNO_FILE_CREATE_FAIL;
	}

	return retVal;
}


int CNoteManager::Read( CNoteFile &noteFile, const char *pPwd )
{
	cout << "-- INFO: Opening read file --" << endl;

	if ( !noteFile.GetReadEnable() ){
		cout << "-- INFO: This function has been disabled! --" << endl;
		return ERRNO_FUNC_DISABLE;
	}

	return	_Open( noteFile, pPwd, false );
}

int CNoteManager::Edit( CNoteFile &noteFile, const char *pPwd )
{
	cout << "-- INFO: Opening edit file --" << endl;

	if ( !noteFile.GetEditEnable() ){
		cout << "-- INFO: This function has been disabled! --" << endl;
		return ERRNO_FUNC_DISABLE;
	}

	return	_Open( noteFile, pPwd, true );
}

int CNoteManager::EndEdit( CNoteFile &noteFile )
{
	cout << "-- INFO: Closing edit file --" << endl;
	return _Close( noteFile );
}

int CNoteManager::_Open( CNoteFile &noteFile, const char *pPwd, bool isRemeberState )
{
	int		retVal = 0;

	if ( noteFile.GetStatus() == STATUS_CLOSE ){

		char *_pTextBuff = new char [noteFile.GetDataLen()];

		if ( _pTextBuff ){

			noteFile.LoadBody( _pTextBuff, noteFile.GetDataLen(), pPwd );
			
			char _acTextFileName[MAX_FILENAME_LEN] = "";

			ChangeFileSuffix( _acTextFileName, noteFile.GetFileName(), "txt" );
			
			// _pTextBuff 已是明文，可直接显示
			FILE *fp_text = NULL;
			fp_text = fopen( _acTextFileName, "wb" );
			if ( fp_text ){
				fwrite( _pTextBuff, noteFile.GetDataLen(), 1, fp_text );	// 将明文写到 _pTextFileName 文件去

				if ( isRemeberState ){
					noteFile.SetStatus( STATUS_READ );	// 保存日志状态
					noteFile.StorePassword( pPwd );
					noteFile.SaveHead();
				}

				fclose( fp_text );
				fp_text = NULL;
			}else {
				cout << "-- ERR: Create file [" << _acTextFileName << "] fail! --" << endl; 
				retVal = ERRNO_FILE_CREATE_FAIL;
			}
			memset( _pTextBuff, 0, noteFile.GetDataLen() );
			delete [] _pTextBuff;
			_pTextBuff = NULL;
		}else{
			cout << "-- ERR: Alloc memory fail! --" << endl;
			retVal = ERRNO_ALLOC_MEMORY_FAIL;
		}
	}else {
		cout << "-- ERR: File has been opened! --" << endl;
		retVal = ERRNO_FILE_STATUS_ERR;
	}
	
	return retVal;
}

int CNoteManager::_Close(  CNoteFile &noteFile, const char *pTextFile )
{

	FILE *fp_text = NULL;
	int	retVal = 0;

	if ( noteFile.GetStatus() == STATUS_READ ){

		char _acTextFileName[MAX_FILENAME_LEN];
		const char *_pTextFile = NULL;

		if ( pTextFile ){
			_pTextFile = pTextFile;
		}else {
			::ChangeFileSuffix( _acTextFileName, noteFile.GetFileName(), "txt" );
			_pTextFile = _acTextFileName;
		}

		FILE *fp_text ;
		fp_text = fopen( _pTextFile, "rb" );	// 打开pTextName文件，将数据提取出来。

		if ( fp_text ){	// 打开文件成功
			
			fseek( fp_text, 0, 2 );
			int text_len = ftell( fp_text );

			char *pBuff = new char [text_len];
			if ( pBuff ){
				
				rewind( fp_text );
				fread ( pBuff, text_len, 1, fp_text );
				
				char _acPassword[PWD_LEN];
				noteFile.LoadPassword( _acPassword );	// 导出暂存的密码
				
				noteFile.SaveBody( pBuff, text_len, _acPassword );	// 保存明文数据

				noteFile.SetStatus( STATUS_CLOSE );	// 保存日志文件头
				noteFile.SetDataLen( text_len );
				noteFile.SaveHead();

				delete [] pBuff;
				pBuff = NULL;
			}else {
				cout << "-- ERR: Alloc memory fail! --" << endl;
				retVal = ERRNO_ALLOC_MEMORY_FAIL;
			}
			fclose( fp_text );
		}else {
			cout << "-- ERR: Open [" << "] fail! --" << endl; 
			retVal = ERRNO_FILE_OPEN_FAIL;
		}
	}else {
		cout << "-- ERR: file has been closed! --" << endl;
		retVal = ERRNO_FILE_STATUS_ERR;
	}

	return retVal;
}

int CNoteManager::Append( CNoteFile &noteFile, const char *pPwd )
{
	FILE	*_pFile = NULL;
	int		retVal = 0;

	cout << "-- ERR: Openning append file! --" << endl;

	if ( !noteFile.GetAppendEnable() ){
		cout << "-- INFO: This function has been disabled! --" << endl;
		return ERRNO_FUNC_DISABLE;
	}

	if ( noteFile.GetStatus() == STATUS_CLOSE ){
		
		char _acTextFileName[MAX_FILENAME_LEN] = "";
		ChangeFileSuffix( _acTextFileName, noteFile.GetFileName(), "txt" );
		
		FILE *fp_text = NULL;
		fp_text = fopen( _acTextFileName, "wb" );
		if ( fp_text ){

			char _acTextBuff[50] = "";
			time_t _currTime = time(NULL);
			//ctime_s(_acTextBuff, 50, &_currTime);
		    strncpy( _acTextBuff, ctime(&_currTime), 50 );
            _acTextBuff[ strlen(_acTextBuff)-1 ] = '\0'; // 去除ctimes()最后生成的 '\n'
			strncat( _acTextBuff, "\xd\xa", 50 );
			int _nLen = (int)strlen(_acTextBuff);

			fwrite( _acTextBuff, _nLen, 1, fp_text );	// 添写日期

			noteFile.SetStatus( STATUS_APPEND );	// 保存日志状态
			noteFile.StorePassword( pPwd );
			noteFile.SaveHead();

			fclose( fp_text );
			fp_text = NULL;
		}else {
			cout << "-- ERR: Creating file [" << _acTextFileName << "] fail! --" << endl; 
			retVal = ERRNO_FILE_CREATE_FAIL;
		}
	}else {
		cout << "-- ERR: File has been opened! --" << endl;
		retVal = ERRNO_FILE_STATUS_ERR;
	}

	return retVal;
}

int CNoteManager::EndAppend( CNoteFile &noteFile )
{
	int		retVal = 0;

	cout << "-- INFO: Closing append file --" << endl;

	if ( noteFile.GetStatus() == STATUS_APPEND ){
		
		char _acTextFileName[MAX_FILENAME_LEN] = "";
		ChangeFileSuffix( _acTextFileName, noteFile.GetFileName(), "txt" );
		
		FILE *fp_text = NULL;
		fp_text = fopen( _acTextFileName, "rb" );
		if ( fp_text ){

			fseek( fp_text, 0, 2 );
			int nTextLen = ftell( fp_text );	// 获取txt文件的长度
			rewind( fp_text );
			int nTotalLen = nTextLen + noteFile.GetDataLen() + 4;	// 总长度 = 文本长度 + 密文长度 + 4B的“\xd\xa\xd\xa”

			char *pBuff = new char [nTotalLen];
			if ( pBuff ){

				fread ( pBuff, nTextLen, 1, fp_text );	// 将文本文件中数据读到pBuff中来
				strncpy( pBuff+nTextLen, "\xd\xa\xd\xa", 5 );	// 追加“\xd\xa\xd\xa”
				char _acPassword[PWD_LEN];
				noteFile.LoadPassword( _acPassword );	// 获得密码
				noteFile.LoadBody( pBuff+nTextLen+4, noteFile.GetDataLen(), _acPassword) ;	// 将密文解密到后面
				noteFile.SaveBody( pBuff, nTotalLen, _acPassword) ;	// 将pBuff数据用密码进行加密并存入日志文件
				// 更改noteFile的数据长度与状态
				noteFile.SetDataLen( nTotalLen );
				noteFile.SetStatus( STATUS_CLOSE );
				noteFile.SaveHead();
				
				delete [] pBuff;
				pBuff = NULL;
			}else {
				cout << "-- ERR: Alloc memory fail! --" << endl;
				retVal = ERRNO_ALLOC_MEMORY_FAIL;
			}
			fclose( fp_text );
			fp_text = NULL;
		}else {
			cout << "-- ERR: Open file [" << _acTextFileName << "] fail! --" << endl; 
			retVal = ERRNO_FILE_CREATE_FAIL;
		}
	}else {
		cout << "-- ERR: File isn't open in append status! --" << endl;
		retVal = ERRNO_FILE_STATUS_ERR;
	}

	return retVal;
}

int CNoteManager::EndRead( CNoteFile &noteFile )
{
	noteFile.SetStatus( STATUS_CLOSE );
	return 0;
}

int CNoteManager::ChangePassword( EPWDType eType, CNoteFile &noteFile )
{
	if ( noteFile.GetStatus() == STATUS_CLOSE ){
		cout << "-- INFO: Please input password --" << endl;
		
		char acOldPassword[PWD_LEN];
		char acNewPassword[PWD_LEN];
		
		cout << "Please input: ";
		::GetPassword( acOldPassword );
		if ( noteFile.GetPasswordVaild( eType, acOldPassword ) ){
			if ( ::GetNewPassword( acNewPassword ) ){
				if ( eType == PWD_NOTE ){
					int len = noteFile.GetDataLen();
					char *pBuff = new char [len];
					if (pBuff){
						noteFile.LoadBody( pBuff, len, acOldPassword );
						noteFile.SaveBody( pBuff, len, acNewPassword );
						noteFile.SetPasswordVaild( PWD_NOTE, acNewPassword );	// 更新密码验证
						noteFile.SaveHead();
						delete [] pBuff;
						pBuff = NULL;
					}else {
						cout << "-- ERR: Alloc memory fail! --" << endl;
						return ERRNO_ALLOC_MEMORY_FAIL;
					}
				}else{
					noteFile.SetPasswordVaild( eType, acNewPassword );
				}
			}else{
				cout << "-- INFO: User cancel --" << endl;  
				return ERRNO_USER_CANCEL;
			}
		}else{
			cout << "-- INFO: Password invaild! --" << endl;
			usleep(2000);
			return ERRNO_PWD_INVAILD;
		}
	}else{
		cout << "-- ERR: NoteBook isn't close, pleasse close first! --" << endl;
		return ERRNO_FILE_STATUS_ERR;
	}
	return 0;
}

int CNoteManager::Lock( const char *pFileName )
{
	return 0;
}

int CNoteManager::Unlock( const char *pFileName )
{
	return 0;
}
