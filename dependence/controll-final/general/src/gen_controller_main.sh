#设置执行环境为当前脚本所在目录
cd `dirname $0`
#服务端IP地址
cmd="strIP=any"
#服务端端口
cmd="${cmd} nPort=4567"
#工作线程数量
cmd="${cmd} nThread=4"
#限制客户端数量
cmd="${cmd} nMaxClient=10000"
#cmd="${cmd} sendback=1"
#客户端发送缓冲区大小（字节）
cmd="${cmd} nSendBuffSize=131072"
#客户端接收缓冲区大小（字节）
cmd="${cmd} nRecvBuffSize=131072"
#检查接收到的服务端消息ID是否连续
#cmd="${cmd} -checkMsgID"
#收到回应后才发送下一条消息
#cmd="${cmd} -sendback"
#启动服务端
./controller $cmd
