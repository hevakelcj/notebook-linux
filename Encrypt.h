#pragma once
#include "typedef.h"

// º”√‹À„∑®
class CEncrypt
{
public:
	static char* Left  ( char *data , int d_len , const char *key , int p_len );
	static char* Right ( char *data , int d_len , const char *key , int p_len );
private:
	static char* GenKey ( char *key , int p_len );
};
