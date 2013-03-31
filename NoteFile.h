#pragma once

#include <stdio.h>
#include <time.h>
#include "typedef.h"

#define	TAG				"NoteBook"
#define	MAGIC			0x3fa83ec9
#define	START_POS		200
#define	TRYTIMES		5

typedef enum _Status{
	STATUS_CLOSE,	// 关闭
	STATUS_READ,	// 打开只读
	STATUS_EDIT,	// 打开编辑
	STATUS_APPEND	// 打开追加
}EStatus;

typedef enum _PWDType{
	PWD_NOTE,
	PWD_PRIV,
	PWD_UNLOCK
}EPWDType;

class CNoteFile
{
public:
	CNoteFile( const char *pNoteFile );
	~CNoteFile();

	void	SetFileName( const char *pNoteFile );
	const char * GetFileName(void) { return m_acNoteFile; }
	bool	Create(void);
	bool	Open(void);	// 打开并导入文件头，对格式进行验证
	void	Close(void);	// 保存文件头，并关闭文件

	bool	LoadHead(void);		// 从文件里导入文件头
	bool	SaveHead(void);		// 将m_head保存到文件中
	
	void	SetDefault(void);
	bool	IsVaild(void);
	
	void	SetTryTimes(int times);
	int		GetTryTimes(void){ return m_head.tryTimes; };

	bool	IsLock(void);
	void	Lock( bool isForever = false );
	void	Unlock(void);

	EStatus	GetStatus(void) { return m_head.status; }
	void	SetStatus(EStatus newStatus) { m_head.status = newStatus; }

	int		GetDataLen(void) { return m_head.dataLen; }
	void	SetDataLen( int len ) { m_head.dataLen = len; }

	bool	GetPasswordVaild( EPWDType eType, const char *pwd );	// 获取密码验证结果
	void	SetPasswordVaild( EPWDType eType, const char *pwd );	// 设置密码验证因子
	
	void	StorePassword( const char * pwd );	// 将密码加密后暂存在文件头中
	void	LoadPassword( char * pwd );	// 从文件头读出密码并解压出来

	int		LoadBody( char *pBuff , int nLen, const char *pPwd );	// 读取文件数据，并使用pPwd密码进行解密
	int		SaveBody( char *pBuff , int nLen, const char *pPwd );	// 加密文件，并保存到文件

	void	SetAppendEnable( bool newState ) { m_head.append_en = newState; }
	void	SetReadEnable( bool newState ) { m_head.read_en = newState; }
	void	SetEditEnable( bool newState ) { m_head.edit_en = newState; }
	bool	GetAppendEnable(void) { return m_head.append_en; }
	bool	GetReadEnable(void) { return m_head.read_en; }
	bool	GetEditEnable(void) { return m_head.edit_en; }

private:

	void	FillRandCode(void);	// 按一定规则填充RandCode区域
	bool	CheckRandCode( const int * pData );// 检查RandCode数据是否符合规则

private:
	
	struct{
		char	tag[10];	// "NoteBook\0"
		int		magic;		// 魔幻数

		EStatus	status;		// 状态

		int		tryTimes;	// 密码尝试失败次数
		time_t	lastLock;	// 上次被锁时间

		int		startPos;	// 密文数据起始地址
		int		dataLen;	// 密文数据长度

		int		m_acNotePwd[3];	// 密码校验串
		char	key;
		char	pwd[PWD_LEN];	// 密码暂存

		bool	append_en;	// 追加使能
		bool	read_en;	// 阅读使能
		bool	edit_en;	// 编辑使能

		int		m_acPrivPwd[3];
		int 	m_acUnlockPwd[3];

	}			m_head;

	FILE		*m_pFile;
	char		m_acNoteFile[MAX_FILENAME_LEN];	// 日志文件名
} ;
