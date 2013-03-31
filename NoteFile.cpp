
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "Encrypt.h"
#include "NoteFile.h"
#include <iostream>

using namespace std;

CNoteFile::CNoteFile( const char *pNoteFile )
	: m_pFile(NULL)
{
	if ( sizeof( m_head ) > START_POS ){
		cout << "-- SERIOUS ERR: Data may be cover! --" << endl;
	}
	memset(&m_head, 0, sizeof(m_head));
	SetFileName( pNoteFile );
}

CNoteFile::~CNoteFile()
{
	Close();
}

void CNoteFile::SetFileName( const char *pNoteFile )
{
	if ( pNoteFile ){
		strncpy( m_acNoteFile, pNoteFile, MAX_FILENAME_LEN );
	}
}

bool CNoteFile::Create()
{
	bool retVal = false;

	if ( m_acNoteFile == NULL || *m_acNoteFile == 0 ){
		cout << "-- ERR: Filename invaild! --" << endl;
		return false;
	}
	
	Close();	// 如原来的文件指针未关闭，则关闭它

	m_pFile = fopen( m_acNoteFile, "wb+" );

	if ( m_pFile ){
		SetDefault();
		SaveHead();
		return true;
	}
	return false;
}

bool CNoteFile::Open()
{
	bool retVal = false;

	if ( m_acNoteFile == NULL || *m_acNoteFile == 0 ){
		cout << "-- ERR: Filename invaild! --" << endl;
		return false;
	}
	
	Close();	// 如原来的文件指针未关闭，则关闭它

	m_pFile = fopen( m_acNoteFile, "rb+" );

	if ( m_pFile ){

		LoadHead();

		if ( IsVaild() ){
			return true;
		}else{
			cout << "-- ERR: Notefile format invaild! --" << endl;
		}
	}else {
		cout << "-- ERR: Open notebook fail! --" << endl;
	}
	return false;
}

void CNoteFile::Close()
{
	if ( m_pFile ){
		SaveHead();
		fclose ( m_pFile );
		m_pFile = NULL;
	}
}

bool CNoteFile::SaveHead()
{
	if ( m_pFile ){

		rewind( m_pFile );
		size_t len = fwrite( &m_head, sizeof(m_head), 1, m_pFile );
		fflush( m_pFile );

		return (len == sizeof(m_head));
	}
	return false;
}

bool CNoteFile::LoadHead()
{
	if ( m_pFile ){

		rewind( m_pFile );
		size_t len = fread( &m_head, sizeof(m_head), 1, m_pFile );

		return ( len == sizeof(m_head) );
	}
	return false;
}

void CNoteFile::SetDefault()
{
	m_head.magic = MAGIC;
	strncpy(m_head.tag, TAG, 10);
	m_head.startPos = START_POS;
	m_head.dataLen = 0;
	m_head.lastLock = 0;
	m_head.m_acNotePwd[0] = 0;
	m_head.m_acNotePwd[1] = 0;
	m_head.m_acNotePwd[2] = 0;

	m_head.status = STATUS_READ;
	m_head.tryTimes = TRYTIMES;

	m_head.append_en = true;
	m_head.read_en = true;
	m_head.edit_en = true;
}

bool CNoteFile::IsVaild()
{
	if ( m_head.magic != MAGIC || 
		 strcmp(m_head.tag, TAG) ||
		 m_head.startPos != START_POS )
		return false;
	else
		return true;
}

void CNoteFile::SetTryTimes(int times)
{
	m_head.tryTimes = times;
}

// 根据lock与上次锁定时间综合判定
bool CNoteFile::IsLock()
{
	time_t currTime = time(NULL);
	if ( currTime < m_head.lastLock || m_head.lastLock == -1 )
		return true;
	else
		return false;
}

void CNoteFile::Lock( bool isForever )
{
	if (isForever){
		m_head.lastLock = -1;
	}else {
		m_head.lastLock = time(NULL) + (60*60*24);	// 锁24小时
	}
}
void CNoteFile::Unlock(void)
{
	m_head.lastLock = 0;
}

void CNoteFile::SetPasswordVaild( EPWDType eType, const char *pwd )
{
	int *pTmp = NULL;
	
	switch ( eType ){
		case PWD_NOTE:
			pTmp = m_head.m_acNotePwd;
			break;
		case PWD_PRIV:
			pTmp = m_head.m_acPrivPwd;
			break;
		case PWD_UNLOCK:
			pTmp = m_head.m_acUnlockPwd;
	}

	pTmp[0] = rand();
	pTmp[1] = rand();
	pTmp[2] = ( pTmp[0] * 3 + pTmp[1] * 2 ) / 5; 
	
	CEncrypt::Left( (char*)pTmp, 3*sizeof(int), pwd, (int)strlen(pwd) );
}

bool CNoteFile::GetPasswordVaild( EPWDType eType, const char *pwd )
{
	int buff[3] = {0};
	int *pTmp = NULL;
	
	switch ( eType ){
		case PWD_NOTE:
			pTmp = m_head.m_acNotePwd;
			break;
		case PWD_PRIV:
			pTmp = m_head.m_acPrivPwd;
			break;
		case PWD_UNLOCK:
			pTmp = m_head.m_acUnlockPwd;
	}

	buff[0] = pTmp[0];
	buff[1] = pTmp[1];
	buff[2] = pTmp[2];

	CEncrypt::Right( (char*)buff, 3*sizeof(int), pwd, (int)strlen(pwd) );

	int tmp = ( buff[0] * 3 + buff[1] * 2 ) / 5;
	if ( tmp == buff[2] ){
		return true;
	}else {
		return false;
	}
}

void CNoteFile::StorePassword( const char * pwd )
{
	m_head.key = rand() & 0xff;
	strncpy( m_head.pwd, pwd, PWD_LEN );
	CEncrypt::Left( m_head.pwd, PWD_LEN, &m_head.key, 1 );
}

void CNoteFile::LoadPassword( char * pwd )
{
	CEncrypt::Right( m_head.pwd, PWD_LEN, &m_head.key, 1 );	// 解密
	strncpy( pwd, m_head.pwd, PWD_LEN );
	memset( m_head.pwd, 0, PWD_LEN );
	m_head.key = 0;
}

// 读取文件数据，并使用pPwd密码进行解密
int	CNoteFile::LoadBody( char *pBuff , int nLen, const char *pPwd )
{
	if ( m_pFile!=NULL && pBuff!=NULL && nLen>0 && pPwd!=NULL ){

		fseek( m_pFile, START_POS, 0 );
		int r_count = (int)fread( pBuff, nLen, 1, m_pFile );
		CEncrypt::Right( pBuff, nLen, pPwd, (int)strlen(pPwd) );
		
		return (r_count*nLen);
	}
	return 0;
}

// 加密文件，并保存到文件
int	CNoteFile::SaveBody( char *pBuff , int nLen, const char *pPwd )
{
	if ( m_pFile!=NULL && pBuff!=NULL && nLen>0 && pPwd!=NULL ){

		CEncrypt::Left( pBuff, nLen, pPwd, (int)strlen(pPwd) );
		fseek( m_pFile, START_POS, 0 );
		int r_count = (int)fwrite( pBuff, nLen, 1, m_pFile );
		fflush( m_pFile );

		return (r_count*nLen);
	}
	return 0;
}
