
// LeakpressDlg.h : ͷ�ļ�
//

#pragma once

#include "Ateq.h"
#include "fins.h"
#include <vector>
#include "pthread.h"
#include "DlgChannleShow.h"
#include <direct.h>

using namespace std;
using namespace OmronPlc;

// CLeakpressDlg �Ի���
class CLeakpressDlg : public CDialogEx
{
// ����
public:
	CLeakpressDlg(CWnd* pParent = NULL);	// ��׼���캯��
	virtual ~CLeakpressDlg();

	// ֻ�����߳�ʹ��
	friend UINT WINAPI Thread1(LPVOID pParam);
	friend UINT WINAPI Thread2(LPVOID pParam);
	friend UINT WINAPI Thread3(LPVOID pParam);
	friend UINT WINAPI Thread4(LPVOID pParam);
	friend UINT WINAPI Thread5(LPVOID pParam);

// �Ի�������
	enum { IDD = IDD_LEAKPRESS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	void LoadConfig();
	void Init();
	bool PLCConnect();
	bool AteqConnect();

	void StartTest(int id);
	bool IsStartState(int id);
	bool IsGetResult(int id);
	bool IsEndState(int id);
	void SendResult(int id);
	void SendTestPress1(int id);
	void SendTestPress2(int id);
	void SendTestPress(int id);

	bool QueryAteqTest(int id); // ��ѯʵʱֵ
	bool QueryAteqResult(int id); // �����Խ��

	bool IsAteqStateMatch(int id, ATEQ_STATE state, bool reply = true);
	ATEQ_STATE QueryAteqState(int id);
	void ResetAteqState(int id);
	void SetAteqState(ATEQ_EVENT *e);

	RESULT getResult(int id);
	void setResult(int id, RESULT *r);

	void WriteResultToFile(CString dir, CString fileName, CString dt, RESULT r);

private:
	Fins *fins;
	Ateq ateqs[NUM];
	PLC_ADDR addr[NUM];
	CONFIG_PARA para;
	pthread_t pid;

	ATEQ_STATE ateqFlag[NUM];

	bool isWindowsLoaded;
	DlgChannleShow* mDlgChannleShow[NUM];

private:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnAteqEventMsg(WPARAM wParam, LPARAM lParam);
	void OnTest(int id, bool bstart, bool isAteqLow = true);
	void OnHighTest(int id, bool bstart, bool isAteqHigh = true);
	void InitTabShow();
	void MoveCtrl();
};
