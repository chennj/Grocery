spinlock 		自旋锁			：对读写线程一视同仁
rw spinlock 	读写自旋锁		：读优先，写靠后。
seqlock			顺序锁			：写优先，读随意。