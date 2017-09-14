#
/*
 */

#include "../param.h"
#include "../user.h"
#include "../proc.h"
#include "../text.h"
#include "../systm.h"
#include "../file.h"
#include "../inode.h"
#include "../buf.h"

/*
 * Give up the processor till a wakeup occurs
 * on chan, at which time the process
 * enters the scheduling queue at priority pri.
 * The most important effect of pri is that when
 * pri<0 a signal cannot disturb the sleep;
 * if pri>=0 signals will be processed.
 * Callers of this routine must be prepared for
 * premature return, and check that the reason for
 * sleeping has gone away.
 */
sleep(chan, pri)
{
	register *rp, s;

	s = PS->integ; //保存当前psw值，下面恢复
	rp = u.u_procp;
	if(pri >= 0) { //会处理信号
		if(issig())
			goto psig;
		spl6(); //提高优先级，关闭中断，防止被wakeup修改
		rp->p_wchan = chan;
		rp->p_stat = SWAIT; //注意状态是wait
		rp->p_pri = pri;
		spl0(); //恢复优先级
		if(runin != 0) { //不存在需要被换出到交换空间的对象？ system.h，调度标志位之一
			runin = 0;
			wakeup(&runin); //启动调度器
		}
		swtch(); //切换成执行进程
		if(issig())
			goto psig;
	} else {
		spl6();
		rp->p_wchan = chan;
		rp->p_stat = SSLEEP; //状态是sleep
		rp->p_pri = pri;
		spl0();
		swtch();
	}
	PS->integ = s; //恢复psw
	return;

	/*
	 * If priority was low (>=0) and
	 * there has been a signal,
	 * execute non-local goto to
	 * the qsav location.
	 * (see trap1/trap.c)
	 */
psig:
	aretu(u.u_qsav);
}

/*
 * Wake up all processes sleeping on chan.
 */
wakeup(chan)
{
	register struct proc *p;
	register c, i;

	c = chan;
	p = &proc[0];
	i = NPROC;
	do {
    //唤醒所有等在chan上的process
		if(p->p_wchan == c) {
			setrun(p);
		}
		p++;
	} while(--i);
}

/*
 * Set the process running;
 * arrange for it to be swapped in if necessary.
 */
setrun(p)
{
	register struct proc *rp;

	rp = p;
	rp->p_wchan = 0; //移除等待条件
	rp->p_stat = SRUN;　//设置为就绪
	if(rp->p_pri < curpri)
		runrun++; //优先级更高，h色织runrun
  //runout为0表示当前不存在可以从交换空间换入内存的进程，如果大于0，就有其他进程在交换空间，并且当前进程已经在交换空间
  //重置为0，启动调度器
	if(runout != 0 && (rp->p_flag&SLOAD) == 0) {
		runout = 0;
		wakeup(&runout);
	}
}

/*
 * Set user priority.
 * The rescheduling flag (runrun)
 * is set if the priority is higher
 * than the currently running process.
 */
setpri(up)
{
	register *pp, p;

	pp = up;
	p = (pp->p_cpu & 0377)/16; //占用cpu越多，优先级越低
	p =+ PUSER + pp->p_nice;
	if(p > 127)
		p = 127;
	if(p > curpri)　//这里是bug，应该小于号，runnrun大于０，表示存在有比当前进程更高优先级的进程，而p越大，反而优先级其实是越小的
		runrun++;
	pp->p_pri = p;
}

/*
 * The main loop of the scheduling (swapping)
 * process.
 * The basic idea is:
 *  see if anyone wants to be swapped in;
 *  swap out processes until there is room;
 *  swap him in;
 *  repeat.
 * Although it is not remarkably evident, the basic
 * synchronization here is on the runin flag, which is
 * slept on and is set once per second by the clock routine.
 * Core shuffling therefore takes place once per second.
 *
 * panic: swap error -- IO error while swapping.
 *	this is the one panic that should be
 *	handled in a less drastic way. Its
 *	very hard.
 */
sched()
{
	struct proc *p1;
	register struct proc *rp;
	register a, n;

	/*
	 * find user to swap in
	 * of users ready, select one out longest
	 */

	goto loop;

sloop:
	runin++;
	sleep(&runin, PSWP);

loop:
	spl6();
	n = -1;
	for(rp = &proc[0]; rp < &proc[NPROC]; rp++)
	if(rp->p_stat==SRUN && (rp->p_flag&SLOAD)==0 &&
	    rp->p_time > n) {
		p1 = rp;
		n = rp->p_time;
	}
	if(n == -1) {
		runout++;
		sleep(&runout, PSWP);
		goto loop;
	}

	/*
	 * see if there is core for that process
	 */

	spl0();
	rp = p1;
	a = rp->p_size;
	if((rp=rp->p_textp) != NULL)
		if(rp->x_ccount == 0)
			a =+ rp->x_size;
	if((a=malloc(coremap, a)) != NULL)
		goto found2;

	/*
	 * none found,
	 * look around for easy core
	 */

	spl6();
	for(rp = &proc[0]; rp < &proc[NPROC]; rp++)
	if((rp->p_flag&(SSYS|SLOCK|SLOAD))==SLOAD &&
	    (rp->p_stat == SWAIT || rp->p_stat==SSTOP))
		goto found1;

	/*
	 * no easy core,
	 * if this process is deserving,
	 * look around for
	 * oldest process in core
	 */

	if(n < 3)
		goto sloop;
	n = -1;
	for(rp = &proc[0]; rp < &proc[NPROC]; rp++)
	if((rp->p_flag&(SSYS|SLOCK|SLOAD))==SLOAD &&
	   (rp->p_stat==SRUN || rp->p_stat==SSLEEP) &&
	    rp->p_time > n) {
		p1 = rp;
		n = rp->p_time;
	}
	if(n < 2)
		goto sloop;
	rp = p1;

	/*
	 * swap user out
	 */

found1:
	spl0();
	rp->p_flag =& ~SLOAD;
	xswap(rp, 1, 0);
	goto loop;

	/*
	 * swap user in
	 */

found2:
	if((rp=p1->p_textp) != NULL) {
		if(rp->x_ccount == 0) {
			if(swap(rp->x_daddr, a, rp->x_size, B_READ))
				goto swaper;
			rp->x_caddr = a;
			a =+ rp->x_size;
		}
		rp->x_ccount++;
	}
	rp = p1;
	if(swap(rp->p_addr, a, rp->p_size, B_READ))
		goto swaper;
	mfree(swapmap, (rp->p_size+7)/8, rp->p_addr);
	rp->p_addr = a;
	rp->p_flag =| SLOAD;
	rp->p_time = 0;
	goto loop;

swaper:
	panic("swap error");
}

/*
 * This routine is called to reschedule the CPU.
 * if the calling process is not in RUN state,
 * arrangements for it to restart must have
 * been made elsewhere, usually by calling via sleep.
 */
swtch()
{
	static struct proc *p;
	register i, n;
	register struct proc *rp;

	if(p == NULL)
		p = &proc[0]; //初始从起始位置
	/*
	 * Remember stack of caller
	 */
	savu(u.u_rsav);
	/*
	 * Switch to scheduler's stack
	 */
	retu(proc[0].p_addr); //切换到调度器进程

loop:
	runrun = 0; //为0表示有更高优先级的进程，调度标志之一，system.h
	rp = p;
	p = NULL;
	n = 128; //最低优先级127，越小优先级越高
	/*
	 * Search for highest-priority runnable process
	 */
	i = NPROC;
	do {
		rp++;
		if(rp >= &proc[NPROC])
			rp = &proc[0];
    //选择条件：1.就绪状态，2.并且加载在内存，3.并且优先级最高
		if(rp->p_stat==SRUN && (rp->p_flag&SLOAD)!=0) {
			if(rp->p_pri < n) {
				p = rp;
				n = rp->p_pri;
			}
		}
	} while(--i);
	/*
	 * If no process is runnable, idle.
	 */
	if(p == NULL) {
		p = rp;
		idle();
		goto loop;
	}
	rp = p;
	curpri = n;
	/*
	 * Switch to stack of the new process and set up
	 * his segmentation registers.
	 */
	retu(rp->p_addr); //切换到选中的进程stack，同时会变更内核ARP的值使全局变量u指向rp的user结构体
  //将保存于被选择进程的user结构体的APR值恢复到硬件的用户ARP。
	sureg();
	/*
	 * If the new process paused because it was
	 * swapped out, set the stack level to the last call
	 * to savu(u_ssav).  This means that the return
	 * which is executed immediately after the call to aretu
	 * actually returns from the last routine which did
	 * the savu.
	 *
	 * You are not expected to understand this.（哥，要不要这么傲娇）
	 */
	if(rp->p_flag&SSWAP) {
		rp->p_flag =& ~SSWAP;
		aretu(u.u_ssav);
	}
	/*
	 * The value returned here has many subtle implications.
	 * See the newproc comments.
	 */
  //返回savu的调用者的调用者
	return(1);
}

/*
 * Create a new process-- the internal version of
 * sys fork.
 * It returns 1 in the new process.
 * How this happens is rather hard to understand.
 * The essential fact is that the new process is created
 * in such a way that appears to have started executing
 * in the same call to newproc as the parent;
 * but in fact the code that runs is that of swtch.
 * The subtle implication of the returned value of swtch
 * (see above) is that this is the value that newproc's
 * caller in the new process sees.
 */
newproc()
{
	int a1, a2;
	struct proc *p, *up;
	register struct proc *rpp;
	register *rip, n;

	p = NULL;
	/*
	 * First, just locate a slot for a process
	 * and copy the useful info from this process into it.
	 * The panic "cannot happen" because fork has already
	 * checked for the existence of a slot.
	 */
retry:
	mpid++;
	if(mpid < 0) {
		mpid = 0;
		goto retry;
	}
	for(rpp = &proc[0]; rpp < &proc[NPROC]; rpp++) {
		if(rpp->p_stat == NULL && p==NULL)
			p = rpp;
		if (rpp->p_pid==mpid)
			goto retry;
	}
	if ((rpp = p)==NULL)
		panic("no procs");

	/*
	 * make proc entry for new proc
	 */

	rip = u.u_procp;
	up = rip;
	rpp->p_stat = SRUN;
	rpp->p_flag = SLOAD;
	rpp->p_uid = rip->p_uid;
	rpp->p_ttyp = rip->p_ttyp;
	rpp->p_nice = rip->p_nice;
	rpp->p_textp = rip->p_textp;
	rpp->p_pid = mpid;
	rpp->p_ppid = rip->p_pid;
	rpp->p_time = 0;

	/*
	 * make duplicate entries
	 * where needed
	 */

	for(rip = &u.u_ofile[0]; rip < &u.u_ofile[NOFILE];)
		if((rpp = *rip++) != NULL)
			rpp->f_count++;
	if((rpp=up->p_textp) != NULL) {
		rpp->x_count++;
		rpp->x_ccount++;
	}
	u.u_cdir->i_count++;
	/*
	 * Partially simulate the environment
	 * of the new process so that when it is actually
	 * created (by copying) it will look right.
	 */
	savu(u.u_rsav);
	rpp = p;
	u.u_procp = rpp; //父节点的proc元素暂时指向了子节点的，下面恢复
	rip = up;　//暂存父节点proc元素，用于下面的恢复
	n = rip->p_size;
	a1 = rip->p_addr;
	rpp->p_size = n;
	a2 = malloc(coremap, n);
	/*
	 * If there is not enough core for the
	 * new process, swap out the current process to generate the
	 * copy.
	 */
	if(a2 == NULL) {
    //内存不足，复制数据段到交换空间
    //为了防止复制过程中数据变化，将父进程设置为idle，无法运行，也无法交换出去
		rip->p_stat = SIDL;
    //子进程数据段地址设置为父进程的，下面的xswap，就会复制父进程的数据段到交换空间，作为未来子进程的数据段。
		rpp->p_addr = a1;
    //注意，跟swtch对应
		savu(u.u_ssav);
		xswap(rpp, 0, 0);
		rpp->p_flag =| SSWAP;
		rip->p_stat = SRUN; //赋值完成，恢复父进程状态
	} else {
	/*
	 * There is core, so just copy.
	 */
    //拷贝数据段
		rpp->p_addr = a2;
		while(n--)
			copyseg(a1++, a2++);
	}
	u.u_procp = rip;　//恢复父节点的proc元素
  return(0); //对父进程调用返回0
}

/*
 * Change the size of the data+stack regions of the process.
 * If the size is shrinking, it's easy-- just release the extra core.
 * If it's growing, and there is core, just allocate it
 * and copy the image, taking care to reset registers to account
 * for the fact that the system's stack has moved.
 * If there is no core, arrange for the process to be swapped
 * out after adjusting the size requirement-- when it comes
 * in, enough core will be allocated.
 * Because of the ssave and SSWAP flags, control will
 * resume after the swap in swtch, which executes the return
 * from this stack level.
 *
 * After the expansion, the caller will take care of copying
 * the user's stack towards or away from the data area.
 */
expand(newsize)
{
	int i, n;
	register *p, a1, a2;

	p = u.u_procp;
	n = p->p_size;
	p->p_size = newsize;
	a1 = p->p_addr;
	if(n >= newsize) {
		mfree(coremap, n-newsize, a1+newsize); //释放不需要的内存
		return;
	}
	savu(u.u_rsav); //后面有调用retu，这里需要先保存
	a2 = malloc(coremap, newsize);
	if(a2 == NULL) {
		savu(u.u_ssav); //swap出去
		xswap(p, 1, n);
		p->p_flag =| SSWAP;
		swtch(); //执行调度，换入的时候将从退出expand的地方开始执行
		/* no return */
	}
	p->p_addr = a2;
	for(i=0; i<n; i++)
		copyseg(a1+i, a2++); //拷贝到新内存地址
	mfree(coremap, n, a1); //释放老的
	retu(p->p_addr); //更新用户空间
	sureg();
}
