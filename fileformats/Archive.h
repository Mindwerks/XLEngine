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
	virtual bool OpenFile(const uint32_t uID) {return false;}
	virtual bool SearchForFile(const char *pszFileIn, char *pszFileOut) {return false;}
	virtual void CloseFile() {};
	virtual uint32_t GetFileLen() {return 0;}
	virtual bool ReadFile(void *pData, uint32_t uLength) {return false;}

	virtual int32_t GetFileCount() {return 0;}
	virtual const char *GetFileName(int32_t nFileIdx) {return 0;}
	virtual uint32_t GetFileID(int32_t nFileIdx) {return 0;}

	virtual void *ReadFileInfo() {return 0;}

	bool IsOpen() { return m_bOpen; }

protected:
	bool m_bOpen;
};

#endif //ARCHIVE_H