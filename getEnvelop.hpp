#ifndef __GETENVELOP__H
#define __GETENVELOP__H
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
/**
 * 包络检波功能，模拟了硬件半波检波和全波检波功能
 */

class env_detect
{
private:
    float m_rct;
    int m_old;
public:
    env_detect(){m_rct = 100.0, m_old = 0.0;};
    void init(int rct);
    short env_half1(short in);
    void env_half(short in[], short out[], int N);
    int env_full(int in);
    void env_full(int in[], int out[], int N);
};


/** \brief 初始化
 *
 * \param rct 为RC低通滤波的时间常数
 * \return
 */
void env_detect::init(int rct)
{
    m_rct = rct;
    m_old = 0.0;
}

 /** \brief 半波包络检測
  *
  * \param in 输入波形。每次传入一个数据点
  * \return 输出波形
  */
short env_detect::env_half1(short in)
{
	float tmp = 0;
    if(in > m_old)
    {
        m_old = in;
    }
    else
    {
        tmp = (float) m_old * (m_rct / ( m_rct + 1 ));
		m_old = (int)tmp;
    }
	
    return m_old;
}
 /** \brief 半波包络检測
  *
  * \param in[] 输入波形
  * \param N 数组的点数
  * \param out[] 输出波形
  * \return
  */
void env_detect::env_half(short in[], short out[], int N)
{
	float tmp = 0;
    for(int i = 0; i < N; i++)
    {
        if( in[i] > m_old)
        {
            m_old = in[i];
            out[i] = m_old;
        }
        else
        {
            tmp = m_old *  m_rct / ( m_rct + 1 );  
			//tmp = 1.02;
            out[i] =(short) tmp ;
			m_old = (short) tmp;
        }
    }
}

/** \brief 全波包络检測
 *
 * \param in 输入波形，每次传入一个数据点
 * \return 输出波形
 */
int env_detect::env_full(int in)
{
    int abs_in = in>0?in:-in;
    if(abs_in > m_old)
    {
        m_old = abs_in;
    }
    else
    {
        m_old *= m_rct / ( m_rct + 1 );
    }
    return m_old;
}
 /** \brief 全波包络检測
  *
  * \param in[] 输入波形
  * \param N 数组的点数
  * \param out[] 输出波形
  * \return
  */
void env_detect::env_full(int in[], int out[], int N)
{
    int abs_in;
    for(int i = 0; i < N; i++)
    {
        abs_in = in[i]>0?in[i]:-in[i];
        if( abs_in > m_old)
        {
            m_old = abs_in;
            out[i] = m_old;
        }
        else
        {
            m_old *= m_rct / ( m_rct + 1 );
            out[i] = m_old;
        }
    }
}
#endif