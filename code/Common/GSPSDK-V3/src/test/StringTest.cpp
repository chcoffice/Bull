#include "GSPString.h"
#include "cmdline.h"

using namespace GSP;

static int _OnTestFormat(const char *czCmd,const char *args)
{
CGSPString strTest;
char *pBuffer;
char *p;
    pBuffer = (char*)malloc( 1L<<20);
    p = pBuffer;
    *p = '\0';
    for(int i=0; i< 128; i++ )
    {
        
        sprintf_s(p,128, "%08d-", i );
        p+=9;
    }
    strTest.Format("%s (%d) %s (%d) %s (%d) %s (%d)\n", 
        pBuffer, 0, 
        pBuffer+9, 1, 
        pBuffer+18, 2,
        pBuffer+27, 3);
    GSP_PRINTF("%s", strTest.c_str() );
    free(pBuffer);
    return 0;
}


static StruOptionLine _Options[]=
{

        {
            "-f",
                "-format",
                "Test string format function.",
                _OnTestFormat
            },

            {
                NULL,
                    NULL,
                    NULL,
                    NULL,
            }
};

int TestGSPString(const char *czCmd,const char *args)
{
    GSP_PRINTF("Test CGSPString enter..\n");

    OptionsEntery(_Options);

    GSP_PRINTF("Test CGSPString leave..\n");
    return 0;


}