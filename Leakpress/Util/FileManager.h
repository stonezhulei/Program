#pragma once
#include<vector>
#include <cstring>
#include <map>
#include <cstring>
#include <iostream>
#include <fstream>
using namespace std;
#define MAX_STR_LEN  1000  // �������ļ�������ַ�����󳤶�

struct	MyFileNameStruct
{
	CString mName;
	CTime mTime;

	MyFileNameStruct(CString name,CTime time)
	{
		mName=name;
		mTime=time;
	}

	CString GetTimeString()
	{
		CString date = mTime.Format("%Y-%m-%d %H:%M:%S");
		return date;
	}
};

namespace FileManager
{
	// �ļ���
	BOOL CheckFolderExist(CString FolderPath);
	BOOL CheckFileExist(CString FilePath);

	BOOL CreateMuliteDirectory(CString FolderPath);
	void DeleteDirectory(CString FolderPath);

	std::vector<MyFileNameStruct> GetAllFileList(CString   strParent);
	// �ļ�·��
	CString GetAppPath();  // exeȫ·��
	CString GetFilePath(CString strPathName);  // ��ȡ·��
	CString GetFileName(CString strPathName);  // �ļ���+��׺��
	CString GetExtName(CString strPathName);   // ��׺��
	CString GetFileTitle(CString strFileName); // �ļ���


	///INI ��д����
	int	    ReadIniToInt    (CString strFilePath, CString strSection, CString strSectionKey);
	double  ReadIniToDouble (CString strFilePath, CString strSection, CString strSectionKey);
	CString ReadIniToString (CString strFilePath, CString strSection, CString strSectionKey);	
	void	WriteIntToIni	(CString strFilePath, CString strSection, CString strSectionKey, int cfg);
	void	WriteDoubleToIni(CString strFilePath, CString strSection, CString strSectionKey, double cfg);
	void	WriteStringToIni(CString strFilePath, CString strSection, CString strSectionKey, CString cfg);

	vector<CString> ReadChildsOnGroup(CString mPath,CString mGroupName);


	//�ı����� TXT  CSV
	BOOL SaveFile(CString str, CString strFilePath, BOOL app=TRUE); // �����ַ���
	BOOL SaveFile(vector<CString> strVec, CString strFilePath, CString SpitChar=_T(","), BOOL app=TRUE); // ����һ������
	BOOL SaveFile(vector<vector<CString>> strVecVec, CString strFilePath, CString SpitChar=_T(","), CString strSencodSign=_T("\n"), BOOL app=TRUE); // ������������
	BOOL ReadFileByLine(vector<CString> &strVec, CString strFilePath); // ���ж�ȡ

	long FileTotalLines(CString strFilePath, bool removeEmptyLine=true);


	//ϵͳ����
	static std::map<CString, CString> mConfigMap;
	static CString mIniPath;
	void InitConfigMap(CString mPath);
	CString GetConfig(CString mName);
	void SetConfig(CString mName,CString mValue,bool flagAutoSave=true);
	void SetConfig(CString mName,int mValue,bool flagAutoSave=true);
	void SetConfig(CString mName,float mValue,bool flagAutoSave=true);
	void SetConfig(CString mName,double mValue,bool flagAutoSave=true);

};

