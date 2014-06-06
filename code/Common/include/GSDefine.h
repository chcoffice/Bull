#ifndef _GS_DEFINE_H
#define _GS_DEFINE_H



/******************************************************************************/
/********************************错误码宏定义********************************* ***/
/******************************************************************************/

#define ERROR_BASE_START   0

#define ERROR_BASE_SUCCESS	(ERROR_BASE_START+0)
#define ERROR_BASE_UNKNOWN	(ERROR_BASE_START+1)
#define ERROR_BASE_PARAM	(ERROR_BASE_START+2)

#define ERROR_NET_START		2000

#define ERROR_NET_REFUSE	(ERROR_NET_START+0)


/******************************************************************************/
/********************************字节序转换宏定义********************************* ***/
/******************************************************************************/


#define	INT16_TO_LE(val)			((INT16) (val))
#define	UINT16_TO_LE(val)			((UINT16) (val))
#define	INT16_TO_BE(val)			((INT16) UINT16_SWAP_LE_BE (val))
#define	UINT16_TO_BE(val)			(UINT16_SWAP_LE_BE (val))
#define	INT16_FROM_LE(val)			((INT16) (val))
#define	UINT16_FROM_LE(val)			((UINT16) (val))
#define	INT16_FROM_BE(val)			((INT16) UINT16_SWAP_LE_BE (val))
#define	UINT16_FROM_BE(val)			(UINT16_SWAP_LE_BE (val))

#define	INT32_TO_LE(val)			((INT32) (val))
#define	UINT32_TO_LE(val)			((UINT32) (val))
#define	INT32_TO_BE(val)			((INT32) UINT32_SWAP_LE_BE (val))
#define	UINT32_TO_BE(val)			(UINT32_SWAP_LE_BE (val))
#define	INT32_FROM_LE(val)			((INT32) (val))
#define	UINT32_FROM_LE(val)			((UINT32) (val))
#define	INT32_FROM_BE(val)			((INT32) UINT32_SWAP_LE_BE (val))
#define	UINT32_FROM_BE(val)			(UINT32_SWAP_LE_BE (val))

#ifdef HAVE_INT64
#define	INT64_TO_LE(val)			((INT64) (val))
#define	UINT64_TO_LE(val)			((UINT64) (val))
#define	INT64_TO_BE(val)			((INT64) UINT64_SWAP_LE_BE (val))
#define	UINT64_TO_BE(val)			(UINT64_SWAP_LE_BE (val))
#define	INT64_FROM_LE(val)			((INT64) (val))
#define	UINT64_FROM_LE(val)			((UINT64) (val))
#define	INT64_FROM_BE(val)			((INT64) UINT64_SWAP_LE_BE (val))
#define	UINT64_FROM_BE(val)			(UINT64_SWAP_LE_BE (val))
#endif

#define UINT16_SWAP_LE_BE(val)		((UINT16) ( \
	(((UINT16) (val) & (UINT16) 0x00ffU) << 8) | \
	(((UINT16) (val) & (UINT16) 0xff00U) >> 8)))
#define UINT32_SWAP_LE_BE(val)		((UINT32) ( \
	(((UINT32) (val) & (UINT32) 0x0x000000ffU) << 24) | \
	(((UINT32) (val) & (UINT32) 0x0x0000ff00U) <<  8) | \
	(((UINT32) (val) & (UINT32) 0x0x00ff0000U) >>  8) | \
	(((UINT32) (val) & (UINT32) 0x0xff000000U) >> 24)))

#ifdef HAVE_INT64
#define UINT64_SWAP_LE_BE(val)         ((UINT64) ( \
	(((UINT64) (val) & (UINT64) 0x00000000000000ffU) << 56) | \
	(((UINT64) (val) & (UINT64) 0x000000000000ff00U) << 40) | \
	(((UINT64) (val) & (UINT64) 0x0000000000ff0000U) << 24) | \
	(((UINT64) (val) & (UINT64) 0x00000000ff000000U) <<  8) | \
	(((UINT64) (val) & (UINT64) 0x000000ff00000000U) >>  8) | \
	(((UINT64) (val) & (UINT64) 0x0000ff0000000000U) >> 24) | \
	(((UINT64) (val) & (UINT64) 0x00ff000000000000U) >> 40) | \
	(((UINT64) (val) & (UINT64) 0xff00000000000000U) >> 56)))
#endif

#endif //_GS_DEFINE_H
