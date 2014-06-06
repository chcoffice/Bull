#include "GSPProDebug.h"
using namespace GSP;


namespace GSP
{
	typedef  struct _StruGSPCmdDescri
	{
		EnumGSPCommandID eID;
		const char *czName;
	}StruGSPCmdDescri;

	static StruGSPCmdDescri s_aGSPCmdDescri[]  = 
	{
		{ GSP_CMD_ID_NONE, "None"  },
		{ GSP_CMD_ID_REQUEST, "Request Source"  },   
		{ GSP_CMD_ID_RET_REQUEST, "Response Request"  },
		{ GSP_CMD_ID_CTRL, "Control"  },
		{ GSP_CMD_ID_RET_CTRL, "Response Control"  },
		{ GSP_CMD_ID_KEEPAVLIE, "Keepalive"  },
		{ GSP_CMD_ID_CLOSE, "Close"  },
		{ GSP_CMD_ID_RET_CLOSE, "Response Close"  },   

		{ GSP_CMD_ID_COMPLETE, "Complete"  },
		{ GSP_CMD_ID_RET_COMPLETE, "Response Complete"  },
		{ GSP_CMD_ID_RESEND, "Resend"  },
		{ GSP_CMD_ID_MEDIAATTRI, "Meddia Attri"  },
		{ GSP_CMD_ID_REQUEST_STATUS, "Request Sataus"  },
		{ GSP_CMD_ID_RET_STATUS, "Response Status"  },
		{ GSP_CMD_ID_RET_UNKNOWN_COMMAND, "Unknow Command"  },       
		{ GSP_CMD_ID_STREAM_ASSERT, "Stream Assert"  },     
		{ GSP_CMD_ID_ASSERT_AND_CLOSE, "Assert And Close"  }, 
		{ (EnumGSPCommandID)-1, "Invalid"  },
	};

	const char *GSPCommandName(EnumGSPCommandID eCmdID)
	{
		int i = 0;
		for( i = 0; s_aGSPCmdDescri[i].eID !=(EnumGSPCommandID)-1; i++  )
		{
			if( s_aGSPCmdDescri[i].eID == eCmdID )
			{
				return  s_aGSPCmdDescri[i].czName;
			}
		}
		return  s_aGSPCmdDescri[i].czName;
	}



	typedef  struct _StruGSPErrorDescri
	{
		INT  iErrno;
		const char *czName;
	}StruGSPErrorDescri;

	static StruGSPErrorDescri s_aGSPErrorDescri[]  = 
	{
		{ GSP_PRO_RET_SUCCES, "Success" },
		{ GSP_PRO_RET_EUNKNOWN, "Unknown" },
		{ GSP_PRO_RET_EPERMIT, "Invalid permit" },
		{ GSP_PRO_RET_ENCHN, "Not exist channel" },
		{ GSP_PRO_RET_EVER, "Version invalid" },
		{ GSP_PRO_RET_ECODE, "Not support code" },
		{ GSP_PRO_RET_ECLOSE, "Connect is closed" },
		{ GSP_PRO_RET_EINVALID, "Invalid params" },
		{ GSP_PRO_RET_EEND, "Stream play end" },
		{ GSP_PRO_RET_ENEXIST, "Source not exist" },
		{ GSP_PRO_RET_EPRO, "Invalid protocol" },
		{ GSP_PRO_RET_ECMD, "Invalid command" },
		{ GSP_PRO_RET_EILLEGAL, "Illegal process" },
		{ GSP_PRO_RET_EEXIST, "Source is opened" },
		{ GSP_PRO_RET_EASSERT, "Operate assert" },
		{ GSP_PRO_RET_EIO, "Read i/o fail" },
		{  GSP_PRO_RET_EBUSY, "Server is busy" },  //服务器资源缺乏
		{ GSP_PRO_RET_EPACKET, "Invalid Packet" },  //收到非法数据包
		{ GSP_PRO_RET_EKP, "Keepavlie timeout" },    // Keepalive 超时
		{GSP_PRO_RET_ESTREAM_ASSERT, "Stream source is assert" }, //流异常
		{ -1, "Invalid errno"  },
	};


	const char *GSPError(INT iGSPErrno)
	{
		int i = 0;
		for( i = 0; s_aGSPErrorDescri[i].iErrno !=-1; i++  )
		{
			if(s_aGSPErrorDescri[i].iErrno == iGSPErrno )
			{
				return  s_aGSPErrorDescri[i].czName;
			}
		}
		return  s_aGSPErrorDescri[i].czName;
	}

	static StruGSPErrorDescri s_aGSPCtrlDescri[]  = 
	{
		{ GSP_CTRL_PLAY, "Play"},
		{ GSP_CTRL_STOP, "Stop"},
		{ GSP_CTRL_PAUSE, "Pause"},
		{ GSP_CTRL_FAST, "Faster"},
		{ GSP_CTRL_BFAST, "Backward Faster"},
		{ GSP_CTRL_STEP, "Step"},
		{ GSP_CTRL_BSTEP, "Backward Step"}, 
		{ GSP_CTRL_SLOW, "Slower"}, 
		{ GSP_CTRL_BSLOW, "Backward Slower"}, 
		{ GSP_CTRL_SETPOINT, "Set Point"},
		{ GSP_CTRL_SECTION, "Section"}, 
		{ -1, "Invalid"  },
	};

	const char *GSPCtrlName( INT iCtrlID)
	{
		int i = 0;
		for( i = 0; s_aGSPCtrlDescri[i].iErrno !=-1; i++  )
		{
			if(s_aGSPCtrlDescri[i].iErrno == iCtrlID )
			{
				return  s_aGSPCtrlDescri[i].czName;
			}
		}
		return  s_aGSPCtrlDescri[i].czName;
	}

}
