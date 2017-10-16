#
/*
 */

#include "../param.h"
#include "../systm.h"
#include "../user.h"
#include "../proc.h"
#include "../inode.h"
#include "../reg.h"

/*
 * Priority for tracing
 */
#define	IPCPRI	(-1)

/*
 * Structure to access an array of integers.
 */
struct
{
	int	inta[];
};

/*
 * Tracing variables.
 * Used to pass trace command from
 * parent to child being traced.
 * This data base cannot be
 * shared and is locked
 * per user.
 */
//保存父进程对于子进程的跟踪请求
struct
{
	int	ip_lock; //排它锁
	int	ip_req; //请求种类
	int	ip_addr; //处理对象的地址
	int	ip_data; //传递的数据
} ipc;

/*
 * Send the specified signal to
 * all processes with 'tp' as its
 * controlling teletype.
 * Called by tty.c for quits and
 * interrupts.
 */

//向和执行进程同一终端的所有进程发送信号
//通常用于退出或者中断终端
signal(tp, sig)
{
	register struct proc *p;

	for(p = &proc[0]; p < &proc[NPROC]; p++)
		if(p->p_ttyp == tp)
			psignal(p, sig);
}

/*
 * Send the specified signal to
 * the specified process.
 */
psignal(p, sig)
int *p;
{
	register *rp;

	if(sig >= NSIG) //无效信号，返回
		return;
	rp = p;
  //非 kill 信号，设置 p_sig
	if(rp->p_sig != SIGKIL)
		rp->p_sig = sig;
  //此处应该是 bug，本意应该是判断 p_pri ，也就是优先级
	if(rp->p_stat > PUSER)
		rp->p_stat = PUSER;
  //如果进程正在睡眠，唤醒调度它
	if(rp->p_stat == SWAIT)
		setrun(rp);
}

/*
 * Returns true if the current
 * process has a signal to process.
 * This is asked at least once
 * each time a process enters the
 * system.
 * A signal does not do anything
 * directly to a process; it sets
 * a flag that asks the process to
 * do something to itself.
 */
//确认进程是否收到信号
issig()
{
	register n;
	register struct proc *p;

	p = u.u_procp;
	if(n = p->p_sig) {
    //收到跟踪信号，暂停进程，等待父进程指令
		if (p->p_flag&STRC) {
			stop();
      //如果 stop 处理中清除了信号，返回 0
			if ((n = p->p_sig) == 0)
				return(0);
		}
    //偶数返回
		if((u.u_signal[n]&1) == 0)
			return(n);
	}
  //奇数忽略
	return(0);
}

/*
 * Enter the tracing STOP state.
 * In this state, the parent is
 * informed and the process is able to
 * receive commands from the parent.
 */
stop()
{
	register struct proc *pp, *cp;

loop:
	cp = u.u_procp;
	if(cp->p_ppid != 1)
	for (pp = &proc[0]; pp < &proc[NPROC]; pp++)
    //找到父进程
		if (pp->p_pid == cp->p_ppid) {
      //唤醒父进程
			wakeup(pp);
      //设置为 SSTOP 状态，等待父进程介入
			cp->p_stat = SSTOP;
      //希望能切换到父进程，执行调度
			swtch();
      //如果清除了 STRACE 信号，或者 procxmt 返回 true，返回，不再跟踪
			if ((cp->p_flag&STRC)==0 || procxmt())
				return;
      //持续跟踪
			goto loop;
		}
  //没有发现父进程，或者父进程为 init，认为是异常状态，终止进程
	exit();
}

/*
 * Perform the action specified by
 * the current signal.
 * The usual sequence is:
 *	if(issig())
 *		psig();
 */
//执行信号处理操作，仅在 issig 返回 true 的时候执行
psig()
{
	register n, p;
	register *rp;

	rp = u.u_procp;
	n = rp->p_sig;
	rp->p_sig = 0;
  //非退出信号
	if((p=u.u_signal[n]) != 0) {
		u.u_error = 0;
		if(n != SIGINS && n != SIGTRC)
			u.u_signal[n] = 0;
		n = u.u_ar0[R6] - 4;
		grow(n);
		suword(n+2, u.u_ar0[RPS]);
		suword(n, u.u_ar0[R7]);
		u.u_ar0[R6] = n; //设置 sp
		u.u_ar0[RPS] =& ~TBIT; //清除 PSW 的陷入位
		u.u_ar0[R7] = p; // pc 设定为处理函数地址
    //信号处理函数将在控制权返回用户进程后执行
		return;
	}
  //信号值为 0 的处理
	switch(n) {

	case SIGQIT:
	case SIGINS:
	case SIGTRC:
	case SIGIOT:
	case SIGEMT:
	case SIGFPT:
	case SIGBUS:
	case SIGSEG:
	case SIGSYS:
		u.u_arg[0] = n;
    //是否 coredump，如果是，将 n 的第七位设置为 1
		if(core())
			n =+ 0200;
	}
	u.u_arg[0] = (u.u_ar0[R0]<<8) | n;
  //退出进程
	exit();
}

/*
 * Create a core image on the file "core"
 * If you are looking for protection glitches,
 * there are probably a wealth of them here
 * when this occurs to a suid command.
 *
 * It writes USIZE block of the
 * user.h area followed by the entire
 * data+stack segments.
 */
//执行 coredump
core()
{
	register s, *ip;
	extern schar;

	u.u_error = 0;
	u.u_dirp = "core";
	ip = namei(&schar, 1);
	if(ip == NULL) {
		if(u.u_error)
			return(0);
		ip = maknode(0666);
		if(ip == NULL)
			return(0);
	}
	if(!access(ip, IWRITE) &&
	   (ip->i_mode&IFMT) == 0 &&
	   u.u_uid == u.u_ruid) {
		itrunc(ip);

    //写入 user 结构体
		u.u_offset[0] = 0;
		u.u_offset[1] = 0;
		u.u_base = &u;
		u.u_count = USIZE*64;
		u.u_segflg = 1;
		writei(ip);
    //写入数据区和栈
		s = u.u_procp->p_size - USIZE;
		estabur(0, s, 0, 0);
		u.u_base = 0;
		u.u_count = s*64;
		u.u_segflg = 0;
		writei(ip);
	}
	iput(ip);
	return(u.u_error==0);
}

/*
 * grow the stack to include the SP
 * true return if successful.
 */
//扩展栈o空间
grow(sp)
char *sp;
{
	register a, si, i;

	if(sp >= -u.u_ssize*64) //栈空间足够大，不做处理
		return(0);
  //计算新地址，扩展 20*64
	si = ldiv(-sp, 64) - u.u_ssize + SINCR;
	if(si <= 0)
		return(0);
  //更新用户 APR
	if(estabur(u.u_tsize, u.u_dsize, u.u_ssize+si, u.u_sep))
		return(0);
  //扩展
	expand(u.u_procp->p_size+si);
	a = u.u_procp->p_addr + u.u_procp->p_size;
  //移动栈区域
	for(i=u.u_ssize; i; i--) {
		a--;
		copyseg(a-si, a);
	}
  //清零新扩展的区域
	for(i=si; i; i--)
		clearseg(--a);
  //增加长度
	u.u_ssize =+ si;
	return(1);
}

/*
 * sys-trace system call.
 */
// r0: 设定 ipc.ipc_data 的值
// u_arg[0]: 跟踪对象进程的 pid
// u_arg[1]: 设定 ipc.ipc_addr 的值
// u_arg[2]: 指令种类，如果为 0， 表示当前进程正在被跟踪，用于子进程自身的设定
ptrace()
{
	register struct proc *p;

  //子进程正在被跟踪，设定 STRC 标志
	if (u.u_arg[2] <= 0) {
		u.u_procp->p_flag =| STRC;
		return;
	}
	for (p=proc; p < &proc[NPROC]; p++)
		if (p->p_stat==SSTOP
		 && p->p_pid==u.u_arg[0]
		 && p->p_ppid==u.u_procp->p_pid)
      //查找到指定的进程 pid
			goto found;
	u.u_error = ESRCH;
	return;

    found:
  //如果正在处理，等待锁释放唤醒
	while (ipc.ip_lock)
		sleep(&ipc, IPCPRI);
  //设定 ipc 数据，注意锁为 pid
	ipc.ip_lock = p->p_pid;
	ipc.ip_data = u.u_ar0[R0];
	ipc.ip_addr = u.u_arg[1] & ~01;
	ipc.ip_req = u.u_arg[2];
  // 移除 SWTED 标志
	p->p_flag =& ~SWTED;
  //设置为就绪状态
	setrun(p);
  //睡眠直到 ip_req 小于等于0
	while (ipc.ip_req > 0)
		sleep(&ipc, IPCPRI);
	u.u_ar0[R0] = ipc.ip_data;
  //小于0认为是错误
	if (ipc.ip_req < 0)
		u.u_error = EIO;
  //释放锁，并唤醒其他等待的进程
	ipc.ip_lock = 0;
	wakeup(&ipc);
}

/*
 * Code that the child process
 * executes to implement the command
 * of the parent process in tracing.
 */
//子进程执行父进程的指令
procxmt()
{
	register int i;
	register int *p;
  //锁不正确，返回
	if (ipc.ip_lock != u.u_procp->p_pid)
		return(0);
	i = ipc.ip_req;
  //设置 ip_req 为 0
	ipc.ip_req = 0;
  // 唤醒等待在 ptrace 上的进程
	wakeup(&ipc);
  //根据父进程请求种类执行派发
	switch (i) {

	/* read user I */
	case 1:
		if (fuibyte(ipc.ip_addr) == -1)
			goto error;
		ipc.ip_data = fuiword(ipc.ip_addr);
		break;

	/* read user D */
	case 2:
		if (fubyte(ipc.ip_addr) == -1)
			goto error;
		ipc.ip_data = fuword(ipc.ip_addr);
		break;

	/* read u */
	case 3:
		i = ipc.ip_addr;
		if (i<0 || i >= (USIZE<<6))
			goto error;
		ipc.ip_data = u.inta[i>>1];
		break;

	/* write user I (for now, always an error) */
	case 4:
		if (suiword(ipc.ip_addr, 0) < 0)
			goto error;
		suiword(ipc.ip_addr, ipc.ip_data);
		break;

	/* write user D */
	case 5:
		if (suword(ipc.ip_addr, 0) < 0)
			goto error;
		suword(ipc.ip_addr, ipc.ip_data);
		break;

	/* write u */
	case 6:
		p = &u.inta[ipc.ip_addr>>1];
		if (p >= u.u_fsav && p < &u.u_fsav[25])
			goto ok;
		for (i=0; i<9; i++)
			if (p == &u.u_ar0[regloc[i]])
				goto ok;
		goto error;
	ok:
		if (p == &u.u_ar0[RPS]) {
			ipc.ip_data =| 0170000;	/* assure user space */
			ipc.ip_data =& ~0340;	/* priority 0 */
		}
		*p = ipc.ip_data;
		break;

	/* set signal and continue */
	case 7:
		u.u_procp->p_sig = ipc.ip_data;
		return(1);

	/* force exit */
	case 8:
		exit();

	default:
	error:
		ipc.ip_req = -1;
	}
	return(0);
}
