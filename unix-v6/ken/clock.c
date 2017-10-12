#
#include "../param.h"
#include "../systm.h"
#include "../user.h"
#include "../proc.h"

#define	UMODE	0170000
#define	SCHMAG	10

/*
 * clock is called straight from
 * the real time clock interrupt.
 * 时间中断处理函数
 * Functions:
 *	reprime clock
 *	copy *switches to display
 *	implement callouts
 *	maintain user/system times
 *	maintain date
 *	profile
 *	tout wakeup (sys sleep)
 *	lightning bolt wakeup (every 4 sec)
 *	alarm clock signals
 *	jab the scheduler
 */
clock(dev, sp, r1, nps, r0, pc, ps)
{
	register struct callo *p1, *p2;
	register struct proc *pp;

	/*
	 * restart clock
   * 重设时钟设备寄存器
	 */

	*lks = 0115;

	/*
	 * display register
	 */

	display();

	/*
	 * callouts
	 * if none, just return
	 * else update first non-zero time
   * 执行定时任务
	 */

  //不存在定时任务，直接跳出
	if(callout[0].c_func == 0)
		goto out;
  //递减没有到时间的定时任务的时间间隔
	p2 = &callout[0];
	while(p2->c_time<=0 && p2->c_func!=0)
		p2++;
	p2->c_time--;

	/*
	 * if ps is high, just return
	 */

	if((ps&0340) != 0)
		goto out; //有更高优先级的任务，先不调度 callout

	/*
	 * callout
   * 定时任务处理
	 */

	spl5();
	if(callout[0].c_time <= 0) {
		p1 = &callout[0];
		while(p1->c_func != 0 && p1->c_time <= 0) {
			(*p1->c_func)(p1->c_arg); //调用定时函数
			p1++;
		}
		p2 = &callout[0];
    //c_time大于0的向前移动
		while(p2->c_func = p1->c_func) {
			p2->c_time = p1->c_time;
			p2->c_arg = p1->c_arg;
			p1++;
			p2++;
		}
	}

	/*
	 * lightning bolt time-out
	 * and time of day
	 */

out:
	if((ps&UMODE) == UMODE) { //用户模式，增加 ucpu time
		u.u_utime++;
		if(u.u_prof[3])
			incupc(pc, u.u_prof);
	} else
		u.u_stime++; //否则，增加 scpu time
	pp = u.u_procp;
	if(++pp->p_cpu == 0) //递增占用cpu累计时间， cpu tick，溢出递减
		pp->p_cpu--;
  //进入 1 秒处理周期
	if(++lbolt >= HZ) {
		if((ps&0340) != 0) //有更高优先级的，先不处理
			return;
		lbolt =- HZ; //递减一秒
    //递增时间
		if(++time[1] == 0)
			++time[0];
    //因为后面操作较为费时，因此减低处理器优先级
		spl1();
    //唤醒通过 sleep 系统调用休眠的进程
		if(time[1]==tout[1] && time[0]==tout[0])
			wakeup(tout);
    //4秒为周期，唤醒通过 lbolt 进入睡眠状态的进程，这个处理叫 Lightning bolt
    //对于不存在使得自身再次运行的事件，并且待机时间也不明确的进程，可以利用 Lightning bolt 进行短暂休眠
    //休眠进程的 proc.w_chan 为 lbolt
		if((time[1]&03) == 0) {
			runrun++; //有更高优先级的进程需要调度
			wakeup(&lbolt); //唤醒休眠在 lbolt 上的进程
		}
    //遍历进程数组，重新计算各个进程的优先级
		for(pp = &proc[0]; pp < &proc[NPROC]; pp++)
		if (pp->p_stat) {
      //递增 p_time: 在内存或者交换空间存在的时间，最大 127
			if(pp->p_time != 127)
				pp->p_time++;
      //确保 p_cpu 小于 SCHMAG=10
			if((pp->p_cpu & 0377) > SCHMAG)
				pp->p_cpu =- SCHMAG; else
				pp->p_cpu = 0;
      //如果优先级小于等于 PUSER，重新计算优先级
			if(pp->p_pri > PUSER)
				setpri(pp);
		}
    //不存在可以换入的进程，尝试重新调度
		if(runin!=0) {
			runin = 0;
			wakeup(&runin);
		}
    //信号处理
		if((ps&UMODE) == UMODE) {
			u.u_ar0 = &r0; //让信号处理函数可以访问被中断进程的寄存器，将 u.u_ar0 设置为保存在 r0 中的地址
			if(issig())
				psig();
			setpri(u.u_procp);
		}
	}
}

/*
 * timeout is called to arrange that
 * fun(arg) is called in tim/HZ seconds.
 * An entry is sorted into the callout
 * structure. The time in each structure
 * entry is the number of HZ's more
 * than the previous entry.
 * In this way, decrementing the
 * first entry has the effect of
 * updating all entries.
 */
timeout(fun, arg, tim)
{
	register struct callo *p1, *p2;
	register t;
	int s;

	t = tim;
	s = PS->integ;
	p1 = &callout[0];
	spl7();
	while(p1->c_func != 0 && p1->c_time <= t) {
		t =- p1->c_time;
		p1++;
	}
	p1->c_time =- t; //找到的位置的后一个，调整时间
	p2 = p1;
	while(p2->c_func != 0)
		p2++;
	while(p2 >= p1) { //位置之后的向后移动一个位置
		(p2+1)->c_time = p2->c_time;
		(p2+1)->c_func = p2->c_func;
		(p2+1)->c_arg = p2->c_arg;
		p2--;
	}
  //加入元素
	p1->c_time = t;
	p1->c_func = fun;
	p1->c_arg = arg;
	PS->integ = s;
}
