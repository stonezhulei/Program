#include "StdAfx.h"
#include "FileManager.h"

static CCriticalSection  LockerSetConfig;
static CCriticalSection LockerGetConfig;

//����ļ����Ƿ����
BOOL FileManager::CheckFolderExist(CString FolderPath)   
{   
	DWORD attr;    
	attr = GetFileAttributes(FolderPath);    
	return (attr != (DWORD)(-1) ) && ( attr & FILE_ATTRIBUTE_DIRECTORY);    
}

//����ļ��Ƿ����
BOOL FileManager::CheckFileExist(CString FilePath)
{
	CFileFind finder; 
	BOOL bWorking = finder.FindFile(FilePath); 
	return bWorking;

	//BOOL CopyFile(LPCTSTR lpExistingFileName,LPCTSTR lpNewFileName, BOOL bFailIfExists);
	//������BOOL MoveFile( LPCTSTR   lpExistingFileName,   LPCTSTR   lpNewFileName);
	//system( "md d:\\aa\\zhao " ); // �����½��ļ���
	//system( "del d:\\aa\\zhao " ); // ɾ�����ļ����µ������ļ�
}

//�༶�ļ���Ŀ¼����
BOOL FileManager::CreateMuliteDirectory(CString FolderPath)   
{   
	int len=FolderPath.GetLength();   
	if ( len <2 ) return false;    
	if('\\'==FolderPath[len-1])  
	{  
		FolderPath=FolderPath.Left(len-1);  
		len=FolderPath.GetLength();  
	}  
	if ( len <=0 ) return false;  
	if (len <=3)   
	{  
		if (CheckFolderExist(FolderPath))return true;  
		else return false;   
	}  
	if (CheckFolderExist(FolderPath))return true;  
	CString Parent;  
	Parent=FolderPath.Left(FolderPath.ReverseFind('\\') );   
	if(Parent.GetLength()<=0)return false;    
	BOOL Ret=CreateMuliteDirectory(Parent);    
	if(Ret)    
	{   
		SECURITY_ATTRIBUTES sa;   
		sa.nLength=sizeof(SECURITY_ATTRIBUTES);   
		sa.lpSecurityDescriptor=NULL;   
		sa.bInheritHandle=0;   
		Ret=(CreateDirectory(FolderPath,&sa)==TRUE);   
		return Ret;   
	}   
	else  
		return FALSE;   
}

//�༶�ļ���Ŀ¼ɾ��
void FileManager::DeleteDirectory(CString FolderPath)   //ɾ��һ���ļ����µ���������  
{     
    CFileFind finder;  
    CString path;  
    path.Format("%s/*.*",FolderPath);  
    BOOL bWorking = finder.FindFile(path);  
    while(bWorking){  
        bWorking = finder.FindNextFile();  
        if(finder.IsDirectory() && !finder.IsDots()){//�����ļ���  
            DeleteDirectory(finder.GetFilePath()); //�ݹ�ɾ���ļ���  
            RemoveDirectory(finder.GetFilePath());  
        }  
        else{//�����ļ�  
            DeleteFile(finder.GetFilePath());  
        }  
    }  
	RemoveDirectory(FolderPath);  
}  

//��ȡĿ¼�������ļ�����Ϣ
std::vector<MyFileNameStruct> FileManager::GetAllFileList(CString   strParent) //·�� �ݹ���������ļ�
{   
	std::vector<MyFileNameStruct> mFileNameList;
	CString strText_i,str;  
	CFileFind finder;    
	bool bFind=finder.FindFile(strParent   +   "*.*"); 
	while (bFind) 
	{      
		bFind=finder.FindNextFile();  
		if (finder.IsDots())  
			continue;      
		if (finder.IsDirectory())//���ļ���
		{    
			CString mFileName=finder.GetFileName();//�ļ�������
			CTime refTime;
			finder.GetCreationTime( refTime);
			mFileNameList.push_back(MyFileNameStruct(mFileName,refTime));
			//FindAllFile(strParent+finder.GetFileName()+"\\");//�ݹ���ļ���  
		}     
	}
	return mFileNameList;
}


CString FileManager::GetAppPath()
{
	TCHAR	szEXEFolder[MAX_PATH];
	::GetModuleFileName(NULL,   szEXEFolder,   MAX_PATH);
	::PathRemoveFileSpec(szEXEFolder);
	return ( CString(szEXEFolder) + _T("\\") );
}

CString FileManager::GetFilePath(CString strPathName)
{
	int nPos = strPathName.ReverseFind(_T('\\'));
	return strPathName.Left(nPos+1);
}

CString FileManager::GetFileName(CString strPathName)
{
	int nPos = strPathName.ReverseFind(_T('\\'));
	return strPathName.Right(strPathName.GetLength() - nPos - 1);
}

CString FileManager::GetExtName(CString strPathName)
{
	int nPos = strPathName.ReverseFind(_T('.'));
	return strPathName.Right(strPathName.GetLength() - nPos - 1);
}

CString FileManager::GetFileTitle(CString strFileName)
{
	int nPos = strFileName.ReverseFind(_T('.'));
	return strFileName.Left(nPos);
}


double FileManager::ReadIniToDouble(CString strFilePath ,CString strSection,CString strSectionKey)
{
	TCHAR mDataStr[100];
	DWORD  stringOfIni =GetPrivateProfileString(strSection,strSectionKey,NULL,mDataStr,sizeof(mDataStr)-1,strFilePath);
	CString mstr=mDataStr;

	double cfgOfDouble = _ttof(mstr);
	return cfgOfDouble;
}

int FileManager::ReadIniToInt(CString strFilePath ,CString strSection,CString strSectionKey)
{
	TCHAR mDataStr[100];
	DWORD  stringOfIni =GetPrivateProfileString(strSection,strSectionKey,NULL,mDataStr,sizeof(mDataStr)-1,strFilePath);
	CString mstr=mDataStr;

	int cfgOfInt = _ttoi(mstr);
	return cfgOfInt;
}

CString FileManager::ReadIniToString(CString strFilePath,CString strSection,CString strSectionKey)
{
	TCHAR mDataStr[MAX_STR_LEN + 1] = { _T('\0') };
	DWORD  stringOfIni =GetPrivateProfileString(strSection,strSectionKey,NULL,mDataStr,sizeof(mDataStr)-1,strFilePath);
	CString mstr=mDataStr;
	return mstr;
}

void FileManager::WriteIntToIni(CString strFilePath,CString strSection,CString strSectionKey,int cfg)
{
	CString str;
	str.Format(_T("%d"),cfg);
	WritePrivateProfileString(strSection,strSectionKey,str,strFilePath);
}

void FileManager::WriteDoubleToIni(CString strFilePath, CString strSection, CString strSectionKey, double cfg)
{
	CString str;
	str.Format(_T("%g"),cfg);
	WritePrivateProfileString(strSection,strSectionKey,str,strFilePath);
}

void FileManager::WriteStringToIni(CString m_szFileName, CString strSection, CString strSectionKey, CString cfg)
{
	WritePrivateProfileString(strSection,strSectionKey,cfg,m_szFileName);
}

//��ȡSeletion�µ����м�ֵ�� "��-ֵ"
std::vector<CString> FileManager::ReadChildsOnGroup(CString mPath,CString mGroupName )
{
	std::vector<CString>  mResultVec;
	int i;  
	int iPos=0;
	CString strKeyValue;
	int iMaxCount;
	TCHAR chKeyNames[5000]={0}; //�ܵ���������ַ���
	TCHAR chKey[300]={0}; //�������һ������

	GetPrivateProfileSection(mGroupName,chKeyNames,5000,mPath);

	for(i=0;i<5000;i++)
	{
		if (chKeyNames[i]==0)
			if (chKeyNames[i]==chKeyNames[i+1])
				break;
	}

	iMaxCount=i+1; //Ҫ��һ��0��Ԫ�ء����ҳ�ȫ���ַ����Ľ������֡�

	for(i=0;i<iMaxCount;i++)
	{
		chKey[iPos++]=chKeyNames[i];
		if(chKeyNames[i]==0)
		{
			strKeyValue=chKey;
			mResultVec.push_back(strKeyValue);
			//CString mkeyStr=strKeyValue.Left(strKeyValue.Find("="));
			//CString mValueStr=strKeyValue.Mid(strKeyValue.Find("=")+1);
			iPos=0;
		}
	}
	return mResultVec;
}



BOOL FileManager::SaveFile(vector<vector<CString>> strVecVec, CString strFilePath, CString SpitChar, CString SpitChar2, BOOL app)
{
	CString str;
	if(strVecVec.size()>0){
		int row = strVecVec[0].size();
		int col = strVecVec.size();

		if(row>0){
			for(int i =0;i<row;i++){

				for(int j=0;j<col;j++){
					if(strVecVec[j].size()-1>=i){
						str.Append(strVecVec[j][i]);
					}else{
						str.Append("-999");
					}
					if (j < col-1){
						str.Append(SpitChar); // �������ݷָ���
					}
				}
				str.Append(SpitChar2); // �м�(��)�ָ���
			}
		}
	}
	return SaveFile(str, strFilePath, app);
}

BOOL FileManager::SaveFile(vector<CString> strVec, CString strFilePath, CString SpitChar, BOOL app)
{
	CString str;
	for (UINT i=0; i<strVec.size(); i++)
	{
		str.Append(strVec[i]);
		if (i < strVec.size()-1) // ȥ����ĩβ�ļ����
			str.Append(SpitChar);
	}
	str.Append(_T("\n")); // �л�����һ��
	return SaveFile(str, strFilePath, app);
}

BOOL FileManager::SaveFile(CString str, CString strFilePath, BOOL app)
{
	ofstream of;
	ios_base::openmode mode = app ? ios::app : ios::trunc;
	of.open(strFilePath, mode);
	if (!of.is_open())
	{
		return FALSE;
	}

	// USES_CONVERSION;
	TCHAR *datas = NULL;
	datas = str.GetBuffer();
	of.write((const TCHAR *)datas, strlen(datas));
	of.close();
	str.ReleaseBuffer();
	return TRUE;
}

BOOL FileManager::ReadFileByLine(vector<CString> &strVec, CString strFilePath)
{
	ifstream of;
	TCHAR datas[256];
	of.open(strFilePath, ios::out);
	if (!of.is_open())
	{
		//AfxMessageBox(_T("�ļ���ʧ��"));
		return FALSE;
	}
	while(of)
	{
		of.getline((TCHAR *)datas, 256);
		strVec.push_back(datas);
	}

	of.close();
	return TRUE;
}

long FileManager::FileTotalLines(CString strFilePath, bool removeEmptyLine)
{
	long total_line = 0;
	vector<CString> linesVec;
	
	if (ReadFileByLine(linesVec, strFilePath)) {
		total_line = linesVec.size();
		for (int i = 0; removeEmptyLine && i < linesVec.size(); i++) {
			if (linesVec[i].Trim().IsEmpty()) {
				total_line--;
			}
		}
	}

	return total_line;
}

void FileManager::InitConfigMap(CString mPath)
{
	mIniPath=mPath;

	std::vector<CString> mVec=ReadChildsOnGroup(mIniPath,"Map");
	if(mVec.size()>0)
	{
		for (int i=0;i<mVec.size();i++)
		{
			CString mKeyValueStr=mVec[i];

			CString mkeyStr=mKeyValueStr.Left(mKeyValueStr.Find("="));
			CString mValueStr=mKeyValueStr.Mid(mKeyValueStr.Find("=")+1);
			SetConfig(mkeyStr,mValueStr,false);
		}
	}
}
CString FileManager::GetConfig(CString mName)
{
	if(mConfigMap.size()>0)
	{
		LockerGetConfig.Lock();
		map<CString,CString>::iterator it=mConfigMap.find(mName);
		LockerGetConfig.Unlock();
		if (it == mConfigMap.end()) //ֵ������
		{
			return "";
		}
		else //ֵ����
		{
			return it->second;
		}
	}
	return "";
}
void FileManager::SetConfig(CString mName,CString mValue,bool flagAutoSave)
{
	if(mConfigMap.size()>0)
	{
		LockerSetConfig.Lock();
		map<CString,CString>::iterator it=mConfigMap.find(mName);
		if (it == mConfigMap.end()) //ֵ������
		{
			mConfigMap.insert (pair<CString,CString>(mName,mValue));  
		}
		else //ֵ����
		{
			mConfigMap[mName]=mValue;
		}
		LockerSetConfig.Unlock();
	}
	else
	{
		mConfigMap.insert (pair<CString,CString>(mName,mValue));  
	}

	if(flagAutoSave)
	{
		WriteStringToIni(mIniPath,("Map"),mName,mValue);
	}
}
void FileManager::SetConfig(CString mName,int mValue,bool flagAutoSave)
{
	CString mStr;
	mStr.Format("%d",mValue);
	SetConfig(mName,mStr,flagAutoSave);
}
void FileManager::SetConfig(CString mName,float mValue,bool flagAutoSave)
{
	CString mStr;
	mStr.Format("%.8f",mValue);
	SetConfig(mName,mStr,flagAutoSave);
}
void FileManager::SetConfig(CString mName,double mValue,bool flagAutoSave)
{
	CString mStr;
	mStr.Format("%f",mValue);
	SetConfig(mName,mStr,flagAutoSave);
}
