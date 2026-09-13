////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: drevil $
// $LastChangedDate: 2008-01-16 09:41:57 -0800 (Wed, 16 Jan 2008) $
// $LastChangedRevision: 2334 $
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __MESSAGEHELPER_H__
#define __MESSAGEHELPER_H__

#include <assert.h>

//////////////////////////////////////////////////////////////////////////

struct SubscriberHandle
{
	union 
	{
		struct
		{
			short m_MessageId;
			short m_SerialNum;
		} split;		
		int	m_Int;
	} u;
};

//////////////////////////////////////////////////////////////////////////

class MessageHelper
{
public:
	friend class MessageRouter;

	template<class Type>
	Type *Get() const
	{
		assert(sizeof(Type) == m_BlockSize && "Memory Block Doesn't match!");
		return static_cast<Type*>(m_pVoid);
	}

	template<class Type>
	void Get2(Type *&_p) const
	{
		if (!m_pVoid || m_BlockSize != sizeof(Type)) {
			_p = 0;
			return;
		}
		_p = static_cast<Type*>(m_pVoid);
	}

	int GetMessageId() const { return m_MessageId; }
	obuint32 GetBlockSize() const { return m_BlockSize; }
	void *GetVoid() const { return m_pVoid; }

	operator bool() const
	{
		return (m_MessageId != 0);
	}
	
	MessageHelper(int _msgId, void *_void = 0, obuint32 _size = 0) :
		m_MessageId	(_msgId),
		m_pVoid		(_void),
		m_BlockSize	(_size)
	{
	}
	~MessageHelper() {};
private:
	mutable int m_MessageId;
	void		*m_pVoid;
	obuint32	m_BlockSize;

	MessageHelper();
};

#define OB_GETMSG(msgtype) msgtype *pMsg = 0; _data.Get2(pMsg);

//////////////////////////////////////////////////////////////////////////

typedef void (*pfnMessageFunction)(const MessageHelper &_helper);

//////////////////////////////////////////////////////////////////////////

#endif
