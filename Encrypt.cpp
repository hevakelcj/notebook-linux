#include <string.h>
#include "Encrypt.h"
#include "VirtualArray.h"

/*
*********************************************************************************************
	描述：加密函数，可以对任意数据进行加密。
	输入：	
		pData	要加密数据的首地址。
		d_len	数据的长度
		pPwd	密码
		p_len	密码长度
	输出：
		字符类型	加密后的数据首地址，等于data
	注：
*********************************************************************************************
*/
char* CEncrypt::Left ( char *pData , int d_len , const char *pPwd , int p_len )
{
	if ( pData && pPwd && d_len > 0 && p_len > 0 ){

		char *pKey = new char [p_len];

		if (pKey){

			memcpy( pKey , pPwd , p_len );

			CVirtualArray vArray(pData, d_len, pKey, p_len);	// 将pData与pKey用vArray连接成一个虚拟数组

			for ( int j = 1 ; j <= p_len ; j ++ ){
				for ( int i = 0 ; ( i + j ) != ( d_len + p_len ) ; i ++ ){
					vArray [i] ^= vArray [i+j] ;
				}
			}

			memset( pKey , 0 , p_len );
			delete [] pKey;
		}
	}
	return 	pData ;
}

/*
*********************************************************************************************
	描述：解密码转换成转换码
	输入：pKey		密码
		  p_len		密码长度
	输出：
		字符类型	解密后的数据首地址，等于data
*********************************************************************************************
*/
char* CEncrypt::GenKey ( char *pKey , int p_len )
{
	if ( pKey && p_len>0 ){
		for ( int j = 1; j <= p_len; j ++ ){
			for ( int i = 0 ; (i+j) < p_len; i++ ){
				pKey[i] ^= pKey[i+j];
			}
		}
	}
	return pKey;
}

/*
*********************************************************************************************
	描述：解密函数，可以对密文进行解密。
	输入：	
		pData	要密文数据的首地址。
		d_len	数据的长度
		pPwd	密码
		p_len	密码长度
	输出：
		字符类型	解密后的数据首地址，等于data
*********************************************************************************************
*/
char* CEncrypt::Right ( char *pData , int d_len , const char *pPwd , int p_len )
{
	if ( pData && pPwd && d_len > 0 && p_len > 0 ){
		
		char *pKey = new char[p_len];

		if ( pKey ){

			memcpy( pKey, pPwd, p_len );	// 因为pPwd是不可写的，所以要申请一个pKey代替pPwd进行计算

			GenKey( pKey, p_len );	// 将密码转换成钥匙
			
			CVirtualArray vArray( pData, d_len, pKey, p_len );	// 将pData与pKey用vArray连接成一个虚拟数组

			for ( int j = p_len ; j > 0 ; j -- ){	// 解密算法...
				for ( int i = d_len + p_len - 1 ; ( i - j ) >= 0 ; i -- ){
					vArray [i-j] ^= vArray [i] ;
				}
			}

			memset( pKey , 0 , p_len );	// 销毁钥匙
			delete [] pKey;
		}
	}
	return	pData ;
}
