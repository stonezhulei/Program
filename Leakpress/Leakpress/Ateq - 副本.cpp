#include "StdAfx.h"
#include "Ateq.h"
#include "resource.h"
#include "LeakpressDlg.h"

static pthread_mutex_t cs_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

Ateq::Ateq(void)// : cnComm(false, 0) // 启动监视线程  同步读取
{
	memset(&m_leak, 0, sizeof(LEAK_PARAMETERS));
	memset(&m_leakTest1, 0, sizeof(LEAK_PARAMETERS));
	memset(&m_leakTest2, 0, sizeof(LEAK_PARAMETERS));
}


Ateq::~Ateq(void)
{
}

UINT Ateq::getLeakPress() const
{
	return m_leak.wLeakPress;
}

UINT Ateq::getLeakValue() const
{
	return m_leak.wLeakValue;
}

UINT Ateq::getLeakTest1Press() const
{
	return m_leakTest1.wLeakPress;
}

UINT Ateq::getLeakTest2Press() const
{
	return m_leakTest2.wLeakPress;
}

void Ateq::OnReceive()
{
	CString comrecestr;

	char str[100]; // 最大接收 100 字节
	ReadString((char *)str, 100);
	comrecestr += str;

	if (strlen(str) > 0)
	{
		parse((unsigned char *)str, strlen(str));
	}
}

void* OnParse(void* pv)
{
	//static pthread_mutex_t cs_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

	pthread_mutex_lock(&cs_mutex);

	Ateq *ateq = (Ateq *)pv;

	//while (ateq->buffer.size() > 0)
	//{

	//	Sleep(100);
	//}

	pthread_mutex_unlock(&cs_mutex);

	return NULL;
}

bool Ateq::isConnect()
{
	return IsOpen();
}

bool Ateq::connect(int port, CString mCommStr)
{
	bool bconnect = Open(port, mCommStr.GetBuffer(0));

	////if (bconnect)
	////{
	////	int ret = pthread_create(&pid, NULL, OnParse, this);
	////	if (ret == -1)
	////	{
	////		printf("ATEQ%1 parse thread create fail.", port);
	////	}
	////}

	return bconnect;
}

void Ateq::close()
{
	Close();
}

std::vector<CString> Ateq::getBuffer() const
{
	return buffer;
}

void Ateq::addBuffer(CString str)
{
}

bool Ateq::isReply() const
{
	pthread_mutex_lock(&cs_mutex);
	bool reply = this->reply;
	pthread_mutex_unlock(&cs_mutex);
	return reply;
}

void Ateq::setReply(bool reply)
{
	pthread_mutex_lock(&cs_mutex);
	this->reply = reply;
	pthread_mutex_unlock(&cs_mutex);
}

bool Ateq::isStabledState() const
{
	pthread_mutex_lock(&cs_mutex);
	bool stable = this->stable;
	pthread_mutex_unlock(&cs_mutex);
	return stable;
}

void Ateq::setStabledState(bool stable)
{
	pthread_mutex_lock(&cs_mutex);
	this->stable = stable;
	pthread_mutex_unlock(&cs_mutex);
}

bool Ateq::isResultReady() const
{
	pthread_mutex_lock(&cs_mutex);
	bool ready = this->ready;
	pthread_mutex_unlock(&cs_mutex);
	return ready;
}

void Ateq::setResultReady(bool ready)
{
	pthread_mutex_lock(&cs_mutex);
	this->ready = ready;
	pthread_mutex_unlock(&cs_mutex);
}

void Ateq::parse(const unsigned char* hexarray, int length)
{
	leakParm = &m_temp;
	if (hexarray[0] == 0xFF && hexarray[1] == 0x03)
	{
		leakParm->wTestPrg = 0;//测试程序
		leakParm->wTestStep = 0;
		leakParm->wTestState = 0;
		leakParm->wLeakPress = 0;
		leakParm->wLeakValue = 0;
	}

	// 3段程序，步骤5
	if (leakParm->wTestPrg == 2 && leakParm->wTestStep == 5)
	{
		// 数据
		pdata[0] = 1;
		pdata[1] = 1;
		setStabledState(true);
		::PostMessage(mMainWnd->GetSafeHwnd(), WM_USER_EVENT_MSG, 0, (LPARAM)new ATEQEvent(id, ATEQ_RESULT));
		//::PostMessage
	}
	else if (leakParm->wTestPrg == 1 && leakParm->wTestStep == 5)
	{
		// 数据
		pdata[2] = 1;
		pdata[3] = 1;
		name = "ddd";
		setResultReady(true); // 结果值状态
	}
}