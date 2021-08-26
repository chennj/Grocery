#设置执行环境为当前脚本所在目录
cd `dirname $0`
#服务端IP地址
strIP="any"
#服务端端口
nPort=12345
#工作线程数量 WorkServer
nThread=4
#限制客户端数量
nMaxClient=100
#发送缓冲区
nSendBuffSize=102400
#接收缓冲区
nRecvBuffSize=102400
#启动服务端
./server strIP=$strIP nPort=$nPort nThread=$nThread nMaxClient=$nMaxClient nSendBuffSize=$nSendBuffSize nRecvBuffSize=$nRecvBuffSize
#
read -p "..按任意键退出.." var
#read -p "Press any key to exit." var