#设置执行环境为当前脚本所在目录
cd `dirname $0`
#服务端IP地址
strIP="any"
#服务端端口
nPort=4567
#消息线程数量 CellServer
nThread=1
#限制客户端数量 未启用
nClient=3
#启动服务端
./EasyTcpServer $strIP $nPort $nThread $nClient
#
read -p "..按任意键退出.." var
#read -p "Press any key to exit." var