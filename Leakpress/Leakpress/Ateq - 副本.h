#pragma once

#include "dataStruct.h"
#include "FmCom_NoActive.h"
#include "pthread.h"
#include <vector>
#define WM_USER_EVENT_MSG WM_USER+600

class CLeakpressDlg;
class Ateq : public cnComm
{
public:
	Ateq(void);
	virtual ~Ateq(void);

	void setWnd(int id, CLeakpressDlg *pMain)
	{
		this->id = id;
		mMainWnd = pMain;
	}

	void setPara(double *data, CString &name)
	{
		pdata = data;
		name = name;
	}

public:
	bool isConnect();
	bool connect(int port, CString mCommStr);
	void close();

	UINT getLeakPress() const;
	UINT getLeakValue() const;
	UINT getLeakTest1Press() const;
	UINT getLeakTest2Press() const;

	void parse(const unsigned char* hexarray, int length);
	
	std::vector<CString> getBuffer() const;
	void addBuffer(CString str);

	bool isReply() const;
	void setReply(bool reply);

	bool isStabledState() const;
	void setStabledState(bool stable);

	bool isResultReady() const;
	void setResultReady(bool ready);

protected:
	void OnReceive();

private:
	LEAK_PARAMETERS m_leak;		 // 充气稳压
	LEAK_PARAMETERS m_leakTest1; // 第一次放气
	LEAK_PARAMETERS m_leakTest2; // 第二次放气
	LEAK_PARAMETERS *leakParm;
	LEAK_PARAMETERS m_temp;

	pthread_t pid;
	std::vector<CString> buffer;

	bool reply;
	bool stable;
	bool ready;

	double *pdata;
	CString name;

	int id;
	CLeakpressDlg *mMainWnd;
};

