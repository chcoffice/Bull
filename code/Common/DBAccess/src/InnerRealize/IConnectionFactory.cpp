#include "IConnectionFactory.h"

using	namespace DBAccessModule;

// 自增长连接ID
INT		IConnectionFactory::m_iAutoConnectID = coniDefaultConnectionAutoId;

IConnectionFactory::IConnectionFactory(void)
{
	m_iAutoConnectID = coniDefaultConnectionAutoId;
}

IConnectionFactory::~IConnectionFactory(void)
{
}


