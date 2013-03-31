#pragma once
#include <stdio.h>

// ĞéÄâÆ´½ÓÊı×é
class CVirtualArray
{
public:
	CVirtualArray( char *array1, int len1, char *array2, int len2)
		: m_pArray1(array1), m_pArray2(array2)
		, m_nLen1(len1), m_nLen2(len2)
	{
		if ( m_pArray1 == NULL || m_pArray2 == NULL 
			|| m_nLen1 < 0 || m_nLen2 < 0)
			throw "err param!";
	}

	char & operator [] ( int nIndex )
	{
		if (nIndex < 0){
			throw "over flow!";
		}else if (nIndex < m_nLen1){
			return m_pArray1[nIndex];
		}else if (nIndex < (m_nLen1 + m_nLen2)){
			return m_pArray2[nIndex - m_nLen1];
		}else{
			throw "over flow!";
		}
	}

	int Length(void) { return m_nLen1+m_nLen2; }

private:
	char *m_pArray1, *m_pArray2;
	int  m_nLen1, m_nLen2;
};