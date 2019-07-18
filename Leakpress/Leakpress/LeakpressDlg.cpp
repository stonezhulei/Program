
// LeakpressDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Leakpress.h"
#include "LeakpressDlg.h"
#include "afxdialogex.h"
#include "FileManager.h"
#include "Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static pthread_mutex_t r_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static pthread_mutex_t plc_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static pthread_mutex_t ateq_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

UINT  WINAPI ThreadInit(LPVOID pParam);
UINT (WINAPI *pThread[NUM]) (LPVOID pParam) = {Thread1, Thread2, Thread3, Thread4, Thread5};
// CLeakpressDlg �Ի���

CLeakpressDlg::CLeakpressDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLeakpressDlg::IDD, pParent)
	, fins(new Fins(TransportType::Udp))
	, isWindowsLoaded(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

#ifdef _DEBUG
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
#endif
}

CLeakpressDlg::~CLeakpressDlg()
{
#ifdef _DEBUG
	fclose(stdin);
	fclose(stdout);
	fclose(stderr);
	FreeConsole();
#endif

	for (int i=0;i<NUM;i++)
	{
		if (mDlgChannleShow[i])
		{
			delete mDlgChannleShow[i];
			mDlgChannleShow[i] = NULL;
		}
	}

	delete fins;
}

void CLeakpressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLeakpressDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_MESSAGE(WM_USER_EVENT_MSG, &CLeakpressDlg::OnAteqEventMsg)
END_MESSAGE_MAP()


// CLeakpressDlg ��Ϣ�������

BOOL CLeakpressDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	LoadConfig();
	InitTabShow();

	isWindowsLoaded = true;
	MoveCtrl();

	Init();
	//AfxBeginThread((AFX_THREADPROC)ThreadInit, this, THREAD_PRIORITY_IDLE);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CLeakpressDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CLeakpressDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CLeakpressDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������
	MoveCtrl();
}

void CLeakpressDlg::LoadConfig()
{
	CString path = FileManager::GetAppPath();
	CString addrIni = path + "plcaddress.ini";
	CString configIni = path + "config.ini";

	memset(addr, 0, sizeof(addr));

	for (int i = 0; i < NUM; i++) {
		vector<CString> childs = FileManager::ReadChildsOnGroup(addrIni, "ATEQ" + Util::toString(i + 1));

		for (int k = 0; k < childs.size() && k < PLCADDSIZE; k++) {
			CString mKeyValueStr = childs[k];
			CString mkeyStr = mKeyValueStr.Left(mKeyValueStr.Find("="));
			CString mValueStr = mKeyValueStr.Mid(mKeyValueStr.Find("=") + 1);
			addr[i].address[k] = (BYTE)atoi(mValueStr);
		}
	}

	vector<CString> childs = FileManager::ReadChildsOnGroup(configIni, "PARA");
	for (int k = 0; k < childs.size(); k++) {
		CString mKeyValueStr = childs[k];
		CString mkeyStr = mKeyValueStr.Left(mKeyValueStr.Find("="));
		CString mValueStr = mKeyValueStr.Mid(mKeyValueStr.Find("=") + 1);
		if (mkeyStr == "IP")
		{
			para.ip = mValueStr;
		}
		else if (strstr(mkeyStr, "COM"))
		{
			int port = (atoi)(mkeyStr.Right(1)) - 1;
			para.coms[port] = mValueStr;
		}
		else if (strstr(mkeyStr, "PROG"))
		{
			int id = (atoi)(mkeyStr.Right(1)) - 1;
			para.progs[id] = (atoi)(mValueStr);
		}
		else if (strstr(mkeyStr, "FILE_PREFIX"))
		{
			RESULT r;
			int id = (atoi)(mkeyStr.Right(1)) - 1;
			para.prefix[id] = mValueStr;
		}
		else if (strstr(mkeyStr, "DIR"))
		{
			para.dir = mValueStr;
		}
	}
}

void CLeakpressDlg::InitTabShow()
{
	for (int i=0;i<NUM;i++)
	{
		mDlgChannleShow[i]=new DlgChannleShow(this);
		mDlgChannleShow[i]->Create(IDD_DlgChannleShow, this);
		mDlgChannleShow[i]->ShowWindow(SW_SHOW);
		mDlgChannleShow[i]->SetChannle(i+1,"ATEQ - "+Util::toString(i+1));
	}
}

void CLeakpressDlg::MoveCtrl()
{
	if (!isWindowsLoaded){
		return;
	}

	int nTop=0;
	CRect rcClient;
	GetClientRect(rcClient);
	int nWindowWidth=rcClient.Width();
	int nWindowHeight=rcClient.Height();
	int nLeft=2;
	int nWidth=nWindowWidth-nLeft;
	CRect mRect;
	if (mDlgChannleShow[0]->GetSafeHwnd())
	{
		CRect rcMove;
		rcMove.left =nLeft;
		rcMove.top = 0;
		rcMove.right =rcMove.left+nWidth/3;
		rcMove.bottom =rcMove.top+ nWindowHeight/2;
		mDlgChannleShow[0]->MoveWindow(rcMove,TRUE);
		mRect=rcMove;
	}

	if (mDlgChannleShow[1]->GetSafeHwnd())
	{
		CRect rcMove;
		rcMove.left =mRect.right;
		rcMove.top = 0;
		rcMove.right =rcMove.left+nWidth/3;
		rcMove.bottom =rcMove.top+ nWindowHeight/2;
		mDlgChannleShow[1]->MoveWindow(rcMove,TRUE);
	}

	if (mDlgChannleShow[2]->GetSafeHwnd())
	{
		CRect rcMove;
		rcMove.left =mRect.right + nWidth/3;
		rcMove.top = 0;
		rcMove.right =rcMove.left+nWidth/3;
		rcMove.bottom = rcMove.top+nWindowHeight/2;
		mDlgChannleShow[2]->MoveWindow(rcMove,TRUE);
	}

	if (mDlgChannleShow[3]->GetSafeHwnd())
	{
		CRect rcMove;
		rcMove.left =mRect.left;
		rcMove.top = mRect.bottom;
		rcMove.right =rcMove.left+nWidth/3;
		rcMove.bottom =rcMove.top+ nWindowHeight/2;
		mDlgChannleShow[3]->MoveWindow(rcMove,TRUE);
	}

	if (mDlgChannleShow[4]->GetSafeHwnd())
	{
		CRect rcMove;
		rcMove.left =mRect.right;
		rcMove.top = mRect.bottom;
		rcMove.right =rcMove.left+nWidth/3;
		rcMove.bottom = rcMove.top+nWindowHeight/2;
		mDlgChannleShow[4]->MoveWindow(rcMove,TRUE);
	}
}

void CLeakpressDlg::Init()
{
	bool initSuccess = PLCConnect() && AteqConnect();
}

bool CLeakpressDlg::PLCConnect()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		printf("WSAStartup failed with error: %d\n", err);
		return false;
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		printf("Could not find a usable version of Winsock.dll\n");
		WSACleanup();
		return false;
	}
	else
		printf("The Winsock 2.2 dll was found okay\n");


	fins->SetRemote(para.ip.GetBuffer(0));
	if (!fins->Connect())
	{
		WSACleanup();
		MessageBox("PLC���Ӵ���", "PLC����", MB_OK);
		return false;
	}

	return true;
}

bool CLeakpressDlg::AteqConnect()
{
	for (int i = 0; i < NUM; i++)
	{
		vector<CString> commPara = Util::SpiltString(para.coms[i], ' ');

		if (commPara.size() >= 2) {
			int port = (atoi)(commPara[0]);
			CString mCommStr = commPara[1];

			ateqs[i].connect(port, mCommStr.GetBuffer(0));
			ateqs[i].setPara(i, this);
		}	
	}

	CString info;
	for (int i = 0; i < NUM; i++) {
		vector<CString> commPara = Util::SpiltString(para.coms[i], ' ');

		if (ateqs[i].isConnect()) {
			ateqFlag[i] = ATEQ_REST;
			AfxBeginThread((AFX_THREADPROC)pThread[i], this, THREAD_PRIORITY_IDLE);
		} else {
			info += "ATEQ" + commPara[0] + " ";
		}
	}

	if (info.GetLength() > 0) 
	{
		info += "����ʧ��";
		MessageBox(info, "ATEQ����", MB_OK);
		return false;
	}

	return true;
}

void CLeakpressDlg::StartTest(int id)
{
	pthread_mutex_lock(&plc_mutex);
	fins->WriteDM((uint16_t)addr[id].address[1], (uint16_t)5);
	pthread_mutex_unlock(&plc_mutex);
}

bool CLeakpressDlg::IsStartState(int id)
{
	pthread_mutex_lock(&plc_mutex);
	WORD urData = 0xff;
	fins->ReadDM((uint16_t)addr[id].address[0], urData);
	pthread_mutex_unlock(&plc_mutex);

	return urData == 0;
}

bool CLeakpressDlg::IsGetResult(int id)
{
	pthread_mutex_lock(&plc_mutex);
	WORD urData = 0xff;
	fins->ReadDM((uint16_t)addr[id].address[0], urData);
	pthread_mutex_unlock(&plc_mutex);

	return urData == 9;
}

bool CLeakpressDlg::IsEndState(int id)
{
	pthread_mutex_lock(&plc_mutex);
	WORD urData = 0xff;
	fins->ReadDM((uint16_t)addr[id].address[0], urData);
	pthread_mutex_unlock(&plc_mutex);

	return urData == 8;
}

RESULT CLeakpressDlg::getResult(int id)
{
	pthread_mutex_lock(&r_mutex);
	RESULT r = mDlgChannleShow[id]->getResult();
	pthread_mutex_unlock(&r_mutex);
	return r;
}

void CLeakpressDlg::setResult(int id, RESULT *r)
{
	pthread_mutex_lock(&r_mutex);
	mDlgChannleShow[id]->setResult(*r);
	pthread_mutex_unlock(&r_mutex);
}


void CLeakpressDlg::SendResult(int id)
{
	RESULT r = getResult(id);

	CTime curTime = CTime::GetCurrentTime();
	CString csCurTime = curTime.Format("%Y-%m-%d %H:%M:%S");
	CString dateString = curTime.Format("%m%d");
	CString fileName = para.prefix[id] + dateString;
	memcpy(r.fileName, fileName, 6);

	//// �ȴ����ݻ�ȡ�ɹ�
	//while (!r.ready) {
	//	Sleep(100);
	//	r = getResult(id);
	//}

	//r.ready = FALSE;

	if (id == 0 || id == 1) {
		pthread_mutex_lock(&plc_mutex);
		fins->WriteDM((uint16_t)addr[id].address[0], (uint16_t)(10));
		fins->WriteDM((uint16_t)addr[id].address[1], (uint16_t)(r.wLeakPress));
		fins->WriteDM((uint16_t)addr[id].address[2], (uint16_t)(r.wLeakValue));
		fins->WriteDM((uint16_t)addr[id].address[3], r.fileName, 6);
		
		// ��ȡ Workpress
		WORD urData = 0;
		fins->ReadDM((uint16_t)addr[id].address[4], urData); r.wWorkPress = urData * 1000;
		pthread_mutex_unlock(&plc_mutex);

		setResult(id, &r);
		WriteResultToFile(para.dir, fileName, csCurTime, r, false);
	}
	else {
		pthread_mutex_lock(&plc_mutex);
		fins->WriteDM((uint16_t)addr[id].address[0], (uint16_t)(10));
		fins->WriteDM((uint16_t)addr[id].address[2], (uint16_t)(r.wLeakPress / 10));
		fins->WriteDM((uint16_t)addr[id].address[3], (uint16_t)(r.wLeakValue / 100));
		fins->WriteDM((uint16_t)addr[id].address[6], r.fileName, 6);

		// ��ȡ P1 P2
		WORD urData = 0xff;
		fins->ReadDM((uint16_t)addr[id].address[7], urData); r.wTestPress1 = urData * 10;
		fins->ReadDM((uint16_t)addr[id].address[8], urData); r.wTestPress2 = urData * 10;
		pthread_mutex_unlock(&plc_mutex);

		setResult(id, &r);
		WriteResultToFile(para.dir, fileName, csCurTime, r, true);
	}

}

void CLeakpressDlg::WriteResultToFile(CString dir, CString fileName, CString dt, RESULT r, bool isLow)
{
	long total_lines = 1;
	CString lineString;

	if (isLow) {
		dir += "\\LowLeakpress";
	}
	else {
		dir += "\\HighLeakpress";
	}

	if (0 != access(dir, 0))   //if the floder not exits,create a new one
	{
		mkdir(dir);   // ���� 0 ��ʾ�����ɹ���-1 ��ʾʧ��  
	}

	CString path = dir + "\\" + fileName + ".csv";
	if (!FileManager::CheckFileExist(path)) {
		lineString.Append("NO, DATETIME, TYPE, PRESS, LEAK");
		if (isLow) {
			lineString.Append(", P1, P2, P1 - P2\n");
		}
		else {
			lineString.Append(", Workpress\n");
		}
	} else {
		total_lines = FileManager::FileTotalLines(path);
	}

	CString temp;	
	temp.Format("%d, ", total_lines);
	lineString.Append(temp);
	lineString.Append(dt);
	lineString.Append(", ");
	lineString.Append(fileName);
	if (isLow) {
		temp.Format(", %ld, %ld, %ld, %ld, %ld\n", r.wLeakPress, r.wLeakValue, r.wTestPress1, r.wTestPress2, r.wTestPress1 - r.wTestPress2);
	}
	else {
		temp.Format(", %ld, %ld, %ld\n", r.wLeakPress, r.wLeakValue, r.wWorkPress);
	}
	
	lineString.Append(temp);

	FileManager::SaveFile(lineString, path);
}

void CLeakpressDlg::SendTestPress1(int id)
{
	RESULT r = getResult(id);

	pthread_mutex_lock(&plc_mutex);
	fins->WriteDM((uint16_t)addr[id].address[4], (uint16_t)(r.wTestPress1 / 10));
	pthread_mutex_unlock(&plc_mutex);
}

void CLeakpressDlg::SendTestPress2(int id)
{
	RESULT r = getResult(id);

	pthread_mutex_lock(&plc_mutex);
	fins->WriteDM((uint16_t)addr[id].address[5], (uint16_t)(r.wTestPress2 / 10));
	pthread_mutex_unlock(&plc_mutex);
}

void CLeakpressDlg::SendTestPress(int id)
{
	RESULT r = getResult(id);

	pthread_mutex_lock(&plc_mutex);
	fins->WriteDM((uint16_t)addr[id].address[1], (uint16_t)(5));
	fins->WriteDM((uint16_t)addr[id].address[4], (uint16_t)(r.wTestPress1 / 10));
	pthread_mutex_unlock(&plc_mutex);
}

bool CLeakpressDlg::QueryAteqTest(int id)
{
	BYTE cmd[] = { 0xFF, 0x03, 0, 0x30, 0, 0x0D, 0x91, 0xDE };
	return ateqs[id].Write(cmd, 8) == 8;
}

bool CLeakpressDlg::QueryAteqResult(int id)
{
	BYTE cmd[] = { 0xFF, 0x03, 0, 0x11, 0, 0x0C, 0x00, 0x14 };
	return ateqs[id].Write(cmd, 8) == 8;
}

bool CLeakpressDlg::IsAteqStateMatch(int id, ATEQ_STATE state, bool reply)
{
	pthread_mutex_lock(&ateq_mutex);
	ATEQ_STATE state1 = ateqFlag[id];
	pthread_mutex_unlock(&ateq_mutex);

	if (state1 == ATEQ_REPLY) {
		return reply;
	}

	return state1 == state;
}

ATEQ_STATE CLeakpressDlg::QueryAteqState(int id)
{
	pthread_mutex_lock(&ateq_mutex);
	ATEQ_STATE state = ateqFlag[id];
	pthread_mutex_unlock(&ateq_mutex);

	return state;
}

void CLeakpressDlg::ResetAteqState(int id)
{
	pthread_mutex_lock(&ateq_mutex);
	ateqFlag[id] = ATEQ_REST;
	pthread_mutex_unlock(&ateq_mutex);
}

void CLeakpressDlg::SetAteqState(ATEQ_EVENT *e)
{
	pthread_mutex_lock(&ateq_mutex);
	ateqFlag[e->id] = e->state;
	pthread_mutex_unlock(&ateq_mutex);
}

afx_msg LRESULT CLeakpressDlg::OnAteqEventMsg(WPARAM wParam, LPARAM lParam)
{
	ATEQ_EVENT *e = (ATEQ_EVENT *)lParam;
	RESULT r = getResult(e->id);

	switch (e->state)
	{
	case ATEQ_REPLY:
		break;
	case ATEQ_TEST_1:
		//r.wLeakPress = e->press;
		//r.wLeakValue = e->leak;
		break;
	case ATEQ_STABLE_1:
		break;
	case ATEQ_RESULT_1:
		r.wLeakPress = e->press;
		r.wLeakValue = e->leak;
		r.ready = TRUE;
		break;
	case ATEQ_ERROR_1:
		break;

	case ATEQ_TEST_2:
		break;
	case ATEQ_STABLE_2:
		r.wTestPress1 = e->press;
		r.wTestPress2 = e->press; // ����1��2��ͬһ��ͨ��
		break;
	case ATEQ_RESULT_2:
		//r.wLeakPress = e->press;
		//r.wLeakValue = e->leak;	
		break;
	case ATEQ_ERROR_2:
		break;
	default:
		break;
	}

	this->setResult(e->id, &r);
	this->SetAteqState(e);
	delete e;
	return 0;
}

void CLeakpressDlg::OnHighTest(int id, bool bstart, bool isAteqLow )
{
	// 1.��ѯ PLC ��λ�ź�
	if (!bstart) {
		bstart = IsStartState(id);
		Sleep(500);
		if (!bstart) {
			return;
		}
	}

	printf("====start %d====\n", id + 1);
	ResetAteqState(id);

	// 2.�ȴ� PLC ��ȡ���
	puts("wait plc get result");
	while (!IsGetResult(id)) {
		Sleep(200);
	}

	puts("send result");
	SendResult(id); // �� PLC ���ͽ��

	puts("waiting end");


}

void CLeakpressDlg::OnTest(int id, bool bstart, bool isAteqLow)
{
	// 1.��ѯ PLC ��λ�ź�
	if (!bstart) {
		bstart = IsStartState(id);
		Sleep(500);
		if (!bstart) {
			return;
		}
	}

	printf("====start %d====\n", id + 1);
	ResetAteqState(id);

	if (isAteqLow) {
		// 2.�ȴ� 2 ����ѹ
		do {
			QueryAteqTest(id);
			Sleep(100);
		} while (IsAteqStateMatch(id, ATEQ_REST));

		// 3.�ȴ� 2 ����ѹ����
		bool error = true;
		while (IsAteqStateMatch(id, ATEQ_STABLE_1)) {
			QueryAteqTest(id);
			error = false;
			Sleep(100);
		}

		if (error) {
			puts("2 ��©��");
			return;
		}
		else {
			puts("====2 ����ѹ����====");
			//Sleep(1000);
		}

		while (!IsAteqStateMatch(id, ATEQ_TEST_1, false)) {
			QueryAteqTest(id);
			Sleep(100);
		}

		puts("====��ѯ2 �ν��====");

		// 4.��ѯ 2 �ν��
		do {
			QueryAteqResult(id);
			Sleep(100);
		} while (!IsAteqStateMatch(id, ATEQ_RESULT_1, false));

		puts("�ȴ� 3 ����ѹ");

		// 5. �ȴ� 3 ����ѹ
		do {
			QueryAteqTest(id);
			Sleep(100);
		} while (!IsAteqStateMatch(id, ATEQ_STABLE_2, false));

		if (QueryAteqState(id) == ATEQ_STABLE_2) {
			puts("sending test press");
		}

		// 6. 3 ����ѹ��һֱ���ͽ��
		while (IsAteqStateMatch(id, ATEQ_STABLE_2, false)) {
			QueryAteqTest(id);
			Sleep(100);
			SendTestPress(id);
		}

		puts("====3 ����ѹ�������ȴ� PLC ��ȡ 2 �ν��====");
	}
	else {
		puts("====�ȴ� PLC ��ȡ 2 �ν��====");
	}

	// 7.�ȴ� PLC ��ȡ���
	puts("wait plc get result");
	while (!IsGetResult(id)) {
		Sleep(200);
	}

	puts("send result");
	SendResult(id); // �� PLC ���ͽ��

	puts("waiting end");

	// 8.�ȴ� PLC ����
	while (!IsEndState(id)) {
		Sleep(100);
	}

	bstart = false;

	puts("====end====");
}

// �����������,����Ӳ��
UINT WINAPI ThreadInit(LPVOID pParam)
{
	CLeakpressDlg *pMain=(CLeakpressDlg*)pParam;

	if (NULL != pMain)
	{
		pMain->Init();
	}

	return 0;
}

UINT WINAPI Thread1(LPVOID pParam)
{
	int id = 0;

	CLeakpressDlg *pMain = (CLeakpressDlg*)pParam;
	DlgChannleShow *pDlg = pMain->mDlgChannleShow[id];

	pDlg->m_flagThreadStart = true;

	bool bstart = false;
	while (!pDlg->m_flagThreadExit)
	{
		pMain->OnTest(id, bstart, false);
	}

	pDlg->m_flagThreadStart = false;
	return 0;
}


UINT WINAPI Thread2(LPVOID pParam)
{
	int id = 1;

	CLeakpressDlg *pMain = (CLeakpressDlg*)pParam;
	DlgChannleShow *pDlg = pMain->mDlgChannleShow[id];

	pDlg->m_flagThreadStart = true;

	bool bstart = false;
	while (!pDlg->m_flagThreadExit)
	{
		pMain->OnTest(id, bstart, false);
	}

	pDlg->m_flagThreadStart = false;
	return 0;
}


UINT WINAPI Thread3(LPVOID pParam)
{
	int id = 2;

	CLeakpressDlg *pMain = (CLeakpressDlg*)pParam;
	DlgChannleShow *pDlg = pMain->mDlgChannleShow[id];

	pDlg->m_flagThreadStart = true;

	bool bstart = false;
	while (!pDlg->m_flagThreadExit)
	{
		pMain->OnTest(id, bstart);
	}

	pDlg->m_flagThreadStart = false;
	return 0;
}

UINT WINAPI Thread4(LPVOID pParam)
{
	int id = 3;

	CLeakpressDlg *pMain = (CLeakpressDlg*)pParam;
	DlgChannleShow *pDlg = pMain->mDlgChannleShow[id];

	pDlg->m_flagThreadStart = true;

	bool bstart = false;
	while (!pDlg->m_flagThreadExit)
	{
		pMain->OnTest(id, bstart);
	}

	pDlg->m_flagThreadStart = false;
	return 0;
}

UINT WINAPI Thread5(LPVOID pParam)
{
	int id = 4;

	CLeakpressDlg *pMain = (CLeakpressDlg*)pParam;
	DlgChannleShow *pDlg = pMain->mDlgChannleShow[id];

	pDlg->m_flagThreadStart = true;

	bool bstart = false;
	while (!pDlg->m_flagThreadExit)
	{
		pMain->OnTest(id, bstart);
	}

	pDlg->m_flagThreadStart = false;
	return 0;
}

