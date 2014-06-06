#ifndef BUFFEROBJ_DEF_H
#define BUFFEROBJ_DEF_H

#define BUFFER_DATA_VALID 1
#define BUFFER_DATA_INVALID 0

class CBufferObj
{
public:
	CBufferObj();
	virtual ~CBufferObj();

	INT32 BufferObjInit(INT32 iBufferSize);
	
	INT32 GetBufSize();
	INT32 GetDataSize();
	char* GetData();
	void* GetUserData();
	void SetDataSize(INT32 iDataSize);
	INT32 SetData(void* pUserData, char* pData, INT32 iDataLen);

	INT32 GetBufValid();
	void SetBufValid(INT32 iValid);

	INT32 CopyBufObj(CBufferObj* pObj);


protected:
	char* m_pObjBuf;
	INT32 m_iBufSize;
	INT32 m_iContentSize;
	void* m_pUserData;
	INT32 m_iDataValid;
};


#endif