#设置执行环境为当前脚本所在目录
cd `dirname $0`
##############################
#服务端IP地址
cmd="strIP=192.168.137.129"
#服务端端口
cmd="${cmd} nPort=4567"
#测试线程睡眠间隔
cmd="${cmd} nWorkSleep=1000"
#工作线程数量
cmd="${cmd} nThread=2"
#创建多少个客户端
cmd="${cmd} nClient=4"
###等待socket可写时才实际发送
#每个客户端在nSendSleep(毫秒)时间内
#最大可写入nMsg条Login消息
#每条消息100字节（Login）
cmd="${cmd} nMsg=1"
cmd="${cmd} nSendSleep=1000"
#客户端发送缓冲区大小（字节）
cmd="${cmd} nSendBuffSize=10240"
#客户端接收缓冲区大小（字节）
cmd="${cmd} nRecvBuffSize=10240"
#检查接收到的服务端消息ID是否连续
cmd="${cmd} -checkMsgID"
#检测-发送的消息已被服务器回应
#收到服务器回应后才发送下一条消息
cmd="${cmd} -chekSendBack"
##############################
##############################
#启动程序 传入参数
./gctest $cmd