WINDOW 编译

1. 解压 wxWidgets-2.9.1.7z 到当前目录
2. 用vc 2008 打开  打开 .\wxWidgets-2.9.1\build\msw\wx_vc9.sln
3. 编译
4. 把 wxWidgets-2.9.1\include\*下的所有文件 拷贝到 当前文件所在的上级目录的 window\include
5. 把 wxWidgets-2.9.1\lib\*下的所有文件 拷贝到  当前文件所在的上级目录的 window\lib


LINUX 编译

./configure  --enable-monolithic --disable-debug_flag --disable-shared --disable-compat28 --enable-compat26=no 
