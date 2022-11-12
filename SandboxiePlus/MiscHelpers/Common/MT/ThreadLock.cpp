#include "stdafx.h"
#include "ThreadLock.h"


/**********************************************************************************************
* CThreadLock
*/

CThreadLock::CThreadLock()
{
	m_Released = false;
}

void CThreadLock::Reset()
{
	m_Mutex.lock();
	m_Released = false;
	m_Mutex.unlock();
}

bool CThreadLock::Lock(unsigned long time)
{
	bool bRet = true;
	m_Mutex.lock();
	if(m_Released == false)
		bRet = m_Wait.wait(&m_Mutex, time);
	m_Mutex.unlock();
	return bRet;
}

void CThreadLock::Release()
{
	m_Mutex.lock();
	m_Released = true;
	m_Wait.wakeAll();
	m_Mutex.unlock();
}
