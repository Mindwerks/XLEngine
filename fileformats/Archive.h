#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <stdio.h>

class Archive
{
public:
	Archive()  { m_bOpen = false; };
	virtual ~Archive() { Close(); }

	virtual bool Open(const char *pszName) {return false;}
	virtual void Close() {};

	virtual bool OpenFile(const char *pszFile) {return false;}
	virtual bool OpenFile(const u32 uID) {return false;}
	virtual bool SearchForFile(const char *pszFileIn, char *pszFileOut) {return false;}
	virtual void CloseFile() {};
	virtual u32 GetFileLen() {return 0;}
	virtual bool ReadFile(void *pData, u32 uLength) {return false;}

	virtual s32 GetFileCount() {return 0;}
	virtual const char *GetFileName(s32 nFileIdx) {return 0;}
	virtual u32 GetFileID(s32 nFileIdx) {return 0;}

	virtual void *ReadFileInfo() {return 0;}

	bool IsOpen() { return m_bOpen; }

protected:
	bool m_bOpen;
};

#endif //ARCHIVE_H