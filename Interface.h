#pragma once
#include "typedef.h"

// nMaxLen 为获取密码的最大长度，不包含字串结束符。
// 实际传给 nMaxLen 的值必须为password实现空间减1
int		GetPassword( char *password , int nMaxLen=PWD_LEN );
char*	ChangeFileSuffix( char *pOutFile, const char *pSrcFile, const char *pSuffix );
bool	GetUserChoice( void );
bool	GetNewPassword( char *password );	// 获取新密码
