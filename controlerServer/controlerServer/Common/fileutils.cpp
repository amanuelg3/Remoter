#include "stdafx.h"
#include "fileutils.h"
#include "errorutils.h"

#include <QDir>
#include <string>
#include <QFileInfo>
#include <QFileInfoList>
#include <QTextStream>

#pragma comment (lib, "Version.lib")

FileUtils::FileUtils()
{

}


FileUtils::~FileUtils()
{

}


QString FileUtils::getAppInstancePath()
{
	QString exeFileName = QApplication::applicationFilePath();
	QFileInfo file(exeFileName);
	return file.canonicalPath();
}

QString FileUtils::getFullPath(const QString& fileName)
{
	if(QDir::isAbsolutePath(fileName))
	{
		return fileName;
	}

	return  getAppInstancePath() + "/" + fileName;
}

bool FileUtils::existsFullPath(const QString& fileName)
{
	QFileInfo file(getFullPath(fileName));
	return file.exists();
}

bool FileUtils::exists(const QString& fileName)
{
	QString newFileName = getFullPath(fileName);
	return existsFullPath(newFileName);
}

QString FileUtils::readAllText(const QString &fileName)
{
	QString fullPath = getFullPath(fileName);
	if(existsFullPath(fullPath)) return readAllTextFullPath(fullPath);
	return "";
}

bool FileUtils::writeText(const QString& fileName, QString& content)
{
	QFile file(FileUtils::getFullPath(fileName));
	if(!file.open(QIODevice::ReadWrite))
	{
		return false;
	}
	QTextStream out(&file);
	out<<content;
	file.close();
	return true;
}

QByteArray FileUtils::readLocalFile(const QString& path)
{
	QString fileName = getFullPath(path);
	QFile file(fileName);

	if(!file.open(QIODevice::ReadOnly))
	{
		Error::showError("���ļ�ʧ��!");
		return 0;
	}

	return file.readAll();
}

QString FileUtils::readAllTextFullPath(const QString& fileName)
{
	QFile file(fileName);

	if(!file.open(QIODevice::ReadOnly))
	{
		Error::showError("�ļ���ʧ��!");
		return "";
	}
	QTextStream os(&file);
	QString content = os.readAll();
	file.close();
	return content;
}

bool FileUtils::makeDir(const QString& strDir)
{
	if(strDir.isEmpty())  return false; 
	QString fullPath = getFullPath(strDir);
	QDir newDir;
	if(newDir.exists(fullPath)) return true;

	return newDir.mkdir(fullPath);
}

bool FileUtils::makePath(const QString& strPath)
{
	QString fullPath = getFullPath(strPath);
	QFileInfo pathInfo(fullPath);
	//if (!pathInfo.isDir()) return false;
	if(pathInfo.exists()) return true;

	QDir newPath;
	return newPath.mkpath(fullPath);
}

bool FileUtils::saveFile(const QByteArray* content, const QString& fileName, bool bCover)
{
	if(!content || fileName.isEmpty()) return false;
	//if(makePath(dir)) return false;
	int pos = fileName.lastIndexOf('/');
	if(pos>0) makePath(fileName.left(pos));

	QString fullPath  = getFullPath(fileName);
	QFile file(fullPath);
	
	int nByte = 0;
	if(!bCover)
	{
		if(file.exists()) return false;
	}

	if(!file.open(QIODevice::WriteOnly))
	{
		Error::showError("�����ļ�ʧ��");
		return false;
	}
	nByte = file.write(content->data(), content->length());
	file.close();
	return nByte<0 ? false: true;
}


QStringList FileUtils::getFileList(const QString& dir)
{
	QDir dirInfo(dir);
	return dirInfo.entryList(QDir::Files);
}

bool FileUtils::copyFile(const QString& srcFile, const QString& destFile)
{
	QString srcFullPath = getFullPath(srcFile);
	QString dstFullPath = getFullPath(destFile);
	if(!existsFullPath(srcFullPath)) return false;
	if(!srcFullPath.compare(dstFullPath, Qt::CaseInsensitive)) return true;
	QFileInfo fileInfo(dstFullPath);
	if(!fileInfo.exists())	
	{
		makePath(fileInfo.path());
	}
	else if(existsFullPath(dstFullPath)) 
	{
		QFile::remove(dstFullPath);
	}
	return QFile::copy(srcFullPath, dstFullPath);
}

/*
	ɾ���ǿ��ļ���
*/
void FileUtils::rmPath(const QString& strDir)
{
	QString dstDir = getFullPath(strDir);
	QFileInfo file(dstDir);
	delDir(file);
}

void FileUtils::rmPath(const QString& strDir, const QString& exceptDir)
{
	QString dstDir = getFullPath(strDir);
	QFileInfo file(dstDir);
	delDir(file, getFullPath(exceptDir));
}

void FileUtils::delDir(QFileInfo rmFile, const QString& exceptDir)
{
	if(rmFile.isDir() && rmFile.path() == exceptDir) return;
	return delDir(rmFile);
}

void FileUtils::delDir(QFileInfo rmFile)
{
	if(rmFile.isDir())
	{
		int childCount =0;
		QString dir = rmFile.filePath();
		QDir thisDir(dir);
		childCount = thisDir.entryInfoList().count();
		QFileInfoList newFileList = thisDir.entryInfoList();
		if(childCount>2){
			for(int i=0;i<childCount;i++){
				if(newFileList.at(i).fileName() == "." || newFileList.at(i).fileName() == ".."){
					continue;
				}
				delDir(newFileList.at(i));
			}
		}
		rmFile.absoluteDir().rmpath(rmFile.fileName());
	}
	else if(rmFile.isFile())
	{
		rmFile.absoluteDir().remove(rmFile.fileName());
	}
}


int FileUtils::fileCount(const QString& filePath)
{
	int fCount = 0;
	fileCounter(filePath, &fCount);
	return fCount;
}

void FileUtils::fileCounter(const QString& filePath, int* pCounter)
{
	QFileInfo curFile(filePath);
	if(!curFile.exists()) return;

	if (curFile.isDir())
	{
		QString dir = curFile.filePath();
		QDir curDir(dir);
		int childCount = curDir.entryInfoList().count();
		QFileInfoList newFileList = curDir.entryInfoList();
		if(childCount > 2)
		{
			for(int i = 0;i< childCount; i++)
			{
				if(newFileList.at(i).fileName() == "." || newFileList.at(i).fileName() == "..")
					continue;
				fileCounter(newFileList.at(i).filePath(), pCounter);
			}
		}
	}
	else
	{
		if(pCounter) *pCounter += 1;
	}
}

bool FileUtils::isAbsolute(const QString& filePath) 
{
	QFileInfo fInfo(filePath);
	return fInfo.isAbsolute();
}

bool FileUtils::rmFile(const QString& fileName)
{
	if(!FileUtils::exists(fileName)) return true;
	return QFile::remove(FileUtils::getFullPath(fileName));
}

//************************************************************************
// Method:    renameFile	�������ļ�
// Access:    public static 
// Returns:   bool
// Parameter: fileName
// Parameter: newName
//************************************************************************
bool FileUtils::renameFile( const QString& fileName, const QString& newName )
{
	QString srcName = getFullPath(fileName);
	QString destName = getFullPath(newName);
	if(!FileUtils::exists(fileName)) return false;
	return QFile::rename(srcName, destName);
}

//***********************************************************************
// Method:    createFile	�����ļ�
// Access:    public static 
// Returns:   bool
// Parameter: fileName		�������ļ�·��
// Parameter: initSize		�ļ��ĳ�ʼ����С
// Parameter: bForce		�Ƿ�ǿ�ƴ��������ڵ�·��,��ֵΪtrueʱ,
//							���·��������,����·��;
//***********************************************************************
bool FileUtils::createFile( const QString& fileName, int initSize /*= 0*/, bool bForce /*= false*/ )
{
	QString fileFullPath = getFullPath(fileName);
	QFileInfo fileInfo(fileFullPath);
	if(fileFullPath.isEmpty() || fileInfo.isDir())
		return false;

	QFile file(fileFullPath);
	if(bForce && !QFileInfo(fileInfo.path()).exists())
		makePath(fileInfo.path());

	bool bSuc = true;
	if(!file.open(QIODevice::ReadWrite)) return false;
	if(initSize > 0) bSuc = file.resize(initSize);
	file.close();

	return bSuc;
}

QString FileUtils::getFileInfo( const QString& fileName, const QString& whatInfo )
{
#ifdef Q_OS_WIN
	//��QT5�б���ʹ�� std::wstring ������ʱ���� fileName.toStdWString().c_str();������ֱ��ʧ��
	std::wstring stdStr = getFullPath(fileName).toStdWString();
	LPCTSTR fName = stdStr.c_str();
	DWORD infoSize = ::GetFileVersionInfoSize(fName, NULL);

	if(infoSize <= 0)  return "";
	QScopedArrayPointer<TCHAR> pBuf(new TCHAR[infoSize]);  

	if(::GetFileVersionInfo(fName, NULL, infoSize, pBuf.data()))     
	{    
		LPCTSTR pInfo(NULL);
		UINT nSize = 0;
		//		QString translation, strTemp;
		//		��֪��Ϊʲô,��ȡ����translation,���ǲ��ɹ�,����ֱ��д080404b0���й���
		// 		if(!::VerQueryValue(pBuf.data(), LPCTSTR("\\VarFileInfo\\Translation"),
		// 			(LPVOID *)&lpTranslate , &nSize))  return "";
		//		QString subBlock = strTemp.sprintf("\\StringFileInfo\\%04x%04x", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
		//080404b0Ϊ���ģ�040904E4ΪӢ��
		QString strTmp = "\\StringFileInfo\\080404b0\\" + whatInfo;
		stdStr = strTmp.toStdWString();
		LPCTSTR pBlock = stdStr.c_str();
		if(::VerQueryValue(pBuf.data(), pBlock, (LPVOID *)&pInfo, &nSize))     
		{    
			return QString::fromWCharArray(pInfo);
		}     
	}     
	return  "";

#endif
}

//�ڻ�ȡ�ļ��汾��ʱ��ǿ�ҽ�����ʹ�øú���,������ʹ��getFileInfo(str, "FileVerion");
QString FileUtils::getFileVersion( const QString& fileName )
{
#ifdef Q_OS_WIN
	//��QT5�б���ʹ�� std::wstring ������ʱ���� fileName.toStdWString().c_str();������ֱ��ʧ��
	std::wstring stdStr = getFullPath(fileName).toStdWString();
	if(stdStr.size() == 0) return "";
	LPCWSTR fName = stdStr.c_str();
	DWORD infoSize = ::GetFileVersionInfoSize(fName, NULL);

	if(infoSize > 0)  
	{   
		QScopedArrayPointer<char> pBuf(new char[infoSize]);  
		if(::GetFileVersionInfo(fName, NULL, infoSize, pBuf.data()))     
		{    
			VS_FIXEDFILEINFO  *pInfo; 
			unsigned int iFileInfoSize = sizeof(VS_FIXEDFILEINFO); 
			if(::VerQueryValue(pBuf.data(), (LPCWSTR)"\\", (void **)&pInfo, &iFileInfoSize))     
			{    
				QString ver = QString("%0.%1.%2.%3")
					.arg(HIWORD(pInfo->dwFileVersionMS))
					.arg(LOWORD(pInfo->dwFileVersionMS))
					.arg(HIWORD(pInfo->dwFileVersionLS))
					.arg(LOWORD(pInfo->dwFileVersionLS));
				return ver;
			}     
		}     
	}
	return  "";
#endif
}

QString FileUtils::getProductName(const QString& fileName)
{
#ifdef Q_OS_WIN
	return getFileInfo(fileName, "ProductName");
#endif
}

QString FileUtils::getFileDescription( const QString& fileName )
{
	return getFileInfo(fileName, "FileDescription");
}