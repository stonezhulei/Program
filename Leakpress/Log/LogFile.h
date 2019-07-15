#pragma once

// CLogFile ����Ŀ��

class CLogFile : public CObject
{
public:
	CLogFile();
	virtual ~CLogFile();
	BOOL InitLogFile(void);
	BOOL WriteLog(DWORD dwError, CString strExtraInfo);
	BOOL WriteLog(CString strMessage);
	CString GetLastErrorMessage(void);

private:
	HANDLE m_hMutex;
	CString m_strFilePath;
	CString m_strErrorMessage;
};


