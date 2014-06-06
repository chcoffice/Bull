//#define OBJECT_DEBUG

#ifdef OBJECT_DEBUG

#include <set>

#define DEFINE_OBJ_DEBUG( classname ) \
	static CGSMutex _s_TestMutex_##classname; \
	static std::set< const GSP::CGSPObject *> _s_TestSet_##classname;

#define OBJ_DEBUG_NEW( classname )  \
    do{ \
	   CGSAutoMutex locker( &_s_TestMutex_##classname ); _s_TestSet_##classname.insert( (GSP::CGSPObject *)this); \
	   if( g_pLog ) { \
	   g_pLog->Message(CLog::LV_INFO,"OBJ",  #classname " @New %d ,%p\n", _s_TestSet_##classname.size(), (void*)this  ); \
       }else{ \
	   MY_DEBUG_PRINTF("OBJ   " #classname " @New %d ,%p\n", _s_TestSet_##classname.size(), (void*)this  ); \
	   } \
    }while(0);

#define OBJ_DEBUG_DEL( classname )  do{ CGSAutoMutex locker( &_s_TestMutex_##classname ); \
    if( _s_TestSet_##classname.find(this) ==_s_TestSet_##classname.end() ) { GS_ASSERT(0); } \
    _s_TestSet_##classname.erase(this); \
	if( g_pLog ) {\
	g_pLog->Message(CLog::LV_INFO,"OBJ",   #classname " @Del %d ,%p\n", _s_TestSet_##classname.size(), (void*)this  ); \
	}else{ \
	MY_DEBUG_PRINTF( "OBJ   " #classname " @Del %d ,%p\n", _s_TestSet_##classname.size(), (void*)this  ); \
	} \
    }while(0);


	class MyTestSingleEnter
	{
		GSAtomicInter *m_pTicks;
		MyTestSingleEnter(GSAtomicInter *pTicks )
		{
			m_pTicks = pTicks;
			if( AtomicInterInc(*m_pTicks) != 1)
			{
				GS_ASSERT(0);
			}
		}
		~MyTestSingleEnter(void)
		{
			if( AtomicInterDec(*m_pTicks) != 0)
			{
				GS_ASSERT(0);
			}
		}
	};

#define DEFINE_SINGLE_ENTER_TEST(AtomicPonter)  MyTestSingleEnter csSingleEnter(AtomicPonter);
#else

#define OBJ_DEBUG_NEW(...)
#define OBJ_DEBUG_DEL(...)
#define DEFINE_OBJ_DEBUG( ... )
#define DEFINE_SINGLE_ENTER_TEST(...)

#endif