#
/*
 */

#include "../param.h"
#include "../systm.h"
#include "../user.h"
#include "../proc.h"
#include "../buf.h"
#include "../reg.h"
#include "../inode.h"

/*
 * exec system call.
 * Because of the fact that an I/O buffer is used
 * to store the caller's arguments during exec,
 * and more buffers are needed to read in the text file,
 * deadly embraces waiting for free buffers are possible.
 * Therefore the number of processes simultaneously
 * running in exec has to be limited to NEXEC.
 */
#define EXPRI	-1

exec()
{
	int ap, na, nc, *bp;
	int ts, ds, sep;
	register c, *ip;
	register char *cp;
	extern uchar;

	/*
	 * pick up file names
	 * and check various modes
	 * for execute permission
	 */

	ip = namei(&uchar, 0);
	if(ip == NULL)
		return;
	while(execnt >= NEXEC) //如果超过最大执行进程限制，等待
		sleep(&execnt, EXPRI);
	execnt++;
	bp = getblk(NODEV); //分配块缓冲区，用于参数处理
	if(access(ip, IEXEC) || (ip->i_mode&IFMT)!=0)
		goto bad;

	/*
	 * pack up arguments into
	 * allocated disk buffer
	 */

	cp = bp->b_addr;
	na = 0;
	nc = 0;
	while(ap = fuword(u.u_arg[1])) {
		na++;
		if(ap == -1)
			goto bad;
		u.u_arg[1] =+ 2; //16位
		for(;;) {
			c = fubyte(ap++);
			if(c == -1)
				goto bad;
			*cp++ = c;
			nc++;
			if(nc > 510) { //总参数字节超过510
				u.u_error = E2BIG;
				goto bad;
			}
			if(c == 0)
				break;
		}
	}
	if((nc&1) != 0) { //如果为奇数，加一个字节变成偶数，系统以字为单位处理
		*cp++ = 0;
		nc++;
	}

	/*
	 * read in first 8 bytes
	 * of file for segment
	 * sizes:
	 * w0 = 407/410/411 (410 implies RO text) (411 implies sep ID)
	 * w1 = text size
	 * w2 = data size
	 * w3 = bss size
	 */

	u.u_base = &u.u_arg[0];
	u.u_count = 8;
	u.u_offset[1] = 0;
	u.u_offset[0] = 0;
	u.u_segflg = 1;
	readi(ip);
	u.u_segflg = 0;
	if(u.u_error)
		goto bad;
	sep = 0;
	if(u.u_arg[0] == 0407) {
		u.u_arg[2] =+ u.u_arg[1]; //代码段和数据段在一起
		u.u_arg[1] = 0;
	} else
	if(u.u_arg[0] == 0411)
		sep++; else
	if(u.u_arg[0] != 0410) {
		u.u_error = ENOEXEC;
		goto bad;
	}
	if(u.u_arg[1]!=0 && (ip->i_flag&ITEXT)==0 && ip->i_count!=1) {
		u.u_error = ETXTBSY;
		goto bad;
	}

	/*
	 * find text and data sizes
	 * try them out for possible
	 * exceed of max sizes
	 */

	ts = ((u.u_arg[1]+63)>>6) & 01777;
	ds = ((u.u_arg[2]+u.u_arg[3]+63)>>6) & 01777;
	if(estabur(ts, ds, SSIZE, sep)) //这里并没有真正分配，而是做约束校验
		goto bad;

	/*
	 * allocate and clear core
	 * at this point, committed
	 * to the new image
	 */

	u.u_prof[3] = 0; //统计用
	xfree(); //释放当前使用的代码段，跟父进程告别
	expand(USIZE);
	xalloc(ip);
	c = USIZE+ds+SSIZE;
	expand(c); //缩小到真正的user结构体相同的长度
	while(--c >= USIZE) //清除之后的数据，防止脏数据
		clearseg(u.u_procp->p_addr+c);

	/*
	 * read in data segment
	 */

	estabur(0, ds, 0, 0); //将进程数据段的虚拟地址变更为0
	u.u_base = 0; //读取数据段
	u.u_offset[1] = 020+u.u_arg[1];
	u.u_count = u.u_arg[2];
	readi(ip);

	/*
	 * initialize stack segment
	 */

	u.u_tsize = ts;
	u.u_dsize = ds;
	u.u_ssize = SSIZE;
	u.u_sep = sep;
  //最终确定用户空间
	estabur(u.u_tsize, u.u_dsize, u.u_ssize, u.u_sep);
  //缓冲区的参数转到栈区域
	cp = bp->b_addr;
	ap = -nc - na*2 - 4;
	u.u_ar0[R6] = ap; //设置sp
	suword(ap, na);
	c = -nc;
	while(na--) {
		suword(ap=+2, c);
		do
			subyte(c++, *cp);
		while(*cp++);
	}
	suword(ap+2, -1);

	/*
	 * set SUID/SGID protections, if no tracing
	 */

	if ((u.u_procp->p_flag&STRC)==0) {
		if(ip->i_mode&ISUID)
			if(u.u_uid != 0) {
				u.u_uid = ip->i_uid;
				u.u_procp->p_uid = ip->i_uid;
			}
		if(ip->i_mode&ISGID)
			u.u_gid = ip->i_gid;
	}

	/*
	 * clear sigs, regs and return
	 */

	c = ip;
	for(ip = &u.u_signal[0]; ip < &u.u_signal[NSIG]; ip++)
		if((*ip & 1) == 0)
			*ip = 0;
	for(cp = &regloc[0]; cp < &regloc[6];)
		u.u_ar0[*cp++] = 0;
	u.u_ar0[R7] = 0;
	for(ip = &u.u_fsav[0]; ip < &u.u_fsav[25];)
		*ip++ = 0;
	ip = c;

bad:
	iput(ip); //递减程序文件inode计数
	brelse(bp); //释放缓冲区
	if(execnt >= NEXEC) //如果有其他继承在等待，唤醒他们
		wakeup(&execnt);
	execnt--; //递减计数
}

/*
 * exit system call:
 * pass back caller's r0
 */
rexit()
{

  //高8位为完结状态，低8位错误信息
	u.u_arg[0] = u.u_ar0[R0] << 8;
	exit();
}

/*
 * Release resources.
 * Save u. area for parent to look at.
 * Enter zombie state.
 * Wake up parent and init processes,
 * and dispose of children.
 */
//1.关闭打开的文件
//2.将inode计数减1
//3.释放代码段
//4.将user结构体复制到交换空间
//5.释放数据段
//6.进入僵尸状态
//7.唤醒父进程和init进程
//8.如果当前存在子进程，将它们设置为init进程的子进程，防止成为孤儿进程
exit()
{
	register int *q, a;
	register struct proc *p;

	u.u_procp->p_flag =& ~STRC; //去掉跟踪标记
	for(q = &u.u_signal[0]; q < &u.u_signal[NSIG];)
		*q++ = 1; //忽略所有信号
	for(q = &u.u_ofile[0]; q < &u.u_ofile[NOFILE]; q++)
		if(a = *q) {
			*q = NULL;
			closef(a); //关闭打开文件
		}
	iput(u.u_cdir); //释放代码段
	xfree();
	a = malloc(swapmap, 1);
	if(a == NULL)
		panic("out of swap");
	p = getblk(swapdev, a);
  //将数据段拷贝到交换空间
	bcopy(&u, p->b_addr, 256);
	bwrite(p);
	q = u.u_procp;
  //释放内存
	mfree(coremap, q->p_size, q->p_addr);
	q->p_addr = a;
	q->p_stat = SZOMB;　//设置为僵尸状态

loop:
	for(p = &proc[0]; p < &proc[NPROC]; p++)
    //找到父进程
	if(q->p_ppid == p->p_pid) {
		wakeup(&proc[1]);　//唤醒init进程
    wakeup(p); //唤醒父进程
		for(p = &proc[0]; p < &proc[NPROC]; p++)
		if(q->p_pid == p->p_ppid) {
			p->p_ppid  = 1;　//找到子进程，托管给 init　进程
      if (p->p_stat == SSTOP) //设置为就绪状态
				setrun(p);
		}
		swtch();　//调度
		/* no return */
	}
	q->p_ppid = 1;
	goto loop;
}

/*
 * Wait system call.
 * Search for a terminated (zombie) child,
 * finally lay it to rest, and collect its status.
 * Look also for stopped (traced) children,
 * and pass back status from them.
 */
wait()
{
	register f, *bp;
	register struct proc *p;

	f = 0;　//计数子进程

loop:
	for(p = &proc[0]; p < &proc[NPROC]; p++)
   //找到当前执行进程的子进程
	if(p->p_ppid == u.u_procp->p_pid) {
		f++;
		if(p->p_stat == SZOMB) {　//清理僵尸状态进程
      u.u_ar0[R0] = p->p_pid;　//返回值子进程pid
      bp = bread(swapdev, f=p->p_addr);//读取交换空间的数据段并释放交换空间
			mfree(swapmap, 1, f);　
      p->p_stat = NULL;　//清零一些计数和状态
			p->p_pid = 0;
			p->p_ppid = 0;
			p->p_sig = 0;
			p->p_ttyp = 0;
			p->p_flag = 0;
			p = bp->b_addr;
			u.u_cstime[0] =+ p->u_cstime[0];　//子进程的cpu时间加到当前进程上
			dpadd(u.u_cstime, p->u_cstime[1]);
			dpadd(u.u_cstime, p->u_stime);
			u.u_cutime[0] =+ p->u_cutime[0];
			dpadd(u.u_cutime, p->u_cutime[1]);
			dpadd(u.u_cutime, p->u_utime);
			u.u_ar0[R1] = p->u_arg[0];　//状态值放到高８位？
			brelse(bp);
			return;
		}
		if(p->p_stat == SSTOP) {　//非运行状态，设置wait状态
			if((p->p_flag&SWTED) == 0) {
				p->p_flag =| SWTED;
				u.u_ar0[R0] = p->p_pid;
				u.u_ar0[R1] = (p->p_sig<<8) | 0177;
				return;
			}
			p->p_flag =& ~(STRC|SWTED); //移除wait并设置为就绪
			setrun(p);
		}
	}
	if(f) {　//找到子进程，等在wait上
   sleep(u.u_procp, PWAIT); //这里可以跟exit里的wakeup(p)对照起来，那里会唤醒父进程，这里等在当前进程的proc结构体上
		goto loop;
	}
	u.u_error = ECHILD;　//没有子进程
}

/*
 * fork system call.
 */
fork()
{
	register struct proc *p1, *p2;

	p1 = u.u_procp;
	for(p2 = &proc[0]; p2 < &proc[NPROC]; p2++)
		if(p2->p_stat == NULL)
			goto found;　/*找到空位*/
  u.u_error = EAGAIN;　
	goto out;

found:
	if(newproc()) { //子进程返回1，父进程返回0
		u.u_ar0[R0] = p1->p_pid; /*将父进程的pid保存到子进程的r0，在 fork.s 里保存到 _par_uid，然后清零返回给子进程的fork 调用*/
		u.u_cstime[0] = 0;
		u.u_cstime[1] = 0;
		u.u_stime = 0;
		u.u_cutime[0] = 0;
		u.u_cutime[1] = 0;
		u.u_utime = 0;
		return;
	}
	u.u_ar0[R0] = p2->p_pid;　/*将子进程的pid拷贝到r0寄存器返回给父进程，父进程 fork 得到子进程pid×/

out:
	u.u_ar0[R7] =+ 2;　/*父进程的pc指向下一条指令*/
}

/*
 * break system call.
 *  -- bad planning: "break" is a dirty word in C.
 */
sbreak()
{
	register a, n, d;
	int i;

	/*
	 * set n to new data size
	 * set d to new-old
	 * set n to new total size
	 */

	n = (((u.u_arg[0]+63)>>6) & 01777);
	if(!u.u_sep)
		n =- nseg(u.u_tsize) * 128;
	if(n < 0)
		n = 0;
	d = n - u.u_dsize;
	n =+ USIZE+u.u_ssize;
	if(estabur(u.u_tsize, u.u_dsize+d, u.u_ssize, u.u_sep))
		return;
	u.u_dsize =+ d;
	if(d > 0)
		goto bigger;
	a = u.u_procp->p_addr + n - u.u_ssize;
	i = n;
	n = u.u_ssize;
	while(n--) {
		copyseg(a-d, a);
		a++;
	}
	expand(i);
	return;

bigger:
	expand(n);
	a = u.u_procp->p_addr + n;
	n = u.u_ssize;
	while(n--) {
		a--;
		copyseg(a-d, a);
	}
	while(d--)
		clearseg(--a);
}
