#
/*
 */

#include "../param.h"
#include "../systm.h"
#include "../user.h"
#include "../proc.h"
#include "../text.h"
#include "../inode.h"

/*
 * Swap out process p.
 * The ff flag causes its core to be freed--
 * it may be off when called to create an image for a
 * child process in newproc.
 * Os is the old size of the data area of the process,
 * and is supplied during core expansion swaps.
 *
 * panic: out of swap space
 * panic: swap error -- IO error
 */
xswap(p, ff, os)
int *p;
{
	register *rp, a;

	rp = p;
	if(os == 0)
		os = rp->p_size;
	a = malloc(swapmap, (rp->p_size+7)/8); //分配交换空间
	if(a == NULL)
		panic("out of swap space");
	xccdec(rp->p_textp); //递减文本段参照计数
	rp->p_flag =| SLOCK; //锁定，说明处于交换中
	if(swap(a, rp->p_addr, os, 0))
		panic("swap error");
	if(ff)
		mfree(coremap, os, rp->p_addr); //设置ff标示，释放内存
	rp->p_addr = a;
	rp->p_flag =& ~(SLOAD|SLOCK); //移除加载和锁定状态
	rp->p_time = 0;
	if(runout) { //如果当前状态是不存在作为换入进程
		runout = 0; //因为p换出了，所以有了可以换入的进程，清零标识并唤醒调度器
		wakeup(&runout);
	}
}

/*
 * relinquish use of the shared text segment
 * of a process.
 */
xfree()
{
	register *xp, *ip;

	if((xp=u.u_procp->p_textp) != NULL) {
		u.u_procp->p_textp = NULL; //清空user结构体的代码段指针
		xccdec(xp); //递减内存内的参照计数
		if(--xp->x_count == 0) { //如果没有任何进程指向该文本段
			ip = xp->x_iptr;
			if((ip->i_mode&ISVTX) == 0) { //非 stricky bit 模式
				xp->x_iptr = NULL;
				mfree(swapmap, (xp->x_size+7)/8, xp->x_daddr); //释放文本段
				ip->i_flag =& ~ITEXT; //移除 inode 文本段标示
				iput(ip);
			}
		}
	}
}

/*
 * Attach to a shared text segment.
 * If there is no shared text, just return.
 * If there is, hook up to it:
 * if it is not currently being used, it has to be read
 * in from the inode (ip) and established in the swap space.
 * If it is being used, but is not currently in core,
 * a swap has to be done to get it back.
 * The full coroutine glory has to be invoked--
 * see slp.c-- because if the calling process
 * is misplaced in core the text image might not fit.
 * Quite possibly the code after "out:" could check to
 * see if the text does fit and simply swap it in.
 *
 * panic: out of swap space
 */
xalloc(ip)
int *ip;
{
	register struct text *xp;
	register *rp, ts;

	if(u.u_arg[1] == 0) //exec设置了代码段长度，如果为０，不处理
		return;
	rp = NULL;
	for(xp = &text[0]; xp < &text[NTEXT]; xp++)
		if(xp->x_iptr == NULL) {
			if(rp == NULL) //查找一个空位，注意，这里并没有break
				rp = xp;
		} else
			if(xp->x_iptr == ip) {　//已经存在，增加计数，指向已存在的。
				xp->x_count++;
				u.u_procp->p_textp = xp;
				goto out;
			}
	if((xp=rp) == NULL) //没有已经存在的，并且没有空位，panic报错
		panic("out of text");
	xp->x_count = 1;
	xp->x_ccount = 0;
	xp->x_iptr = ip;
	ts = ((u.u_arg[1]+63)>>6) & 01777; //计算代码段长度，64个字节为单位
	xp->x_size = ts;
	if((xp->x_daddr = malloc(swapmap, (ts+7)/8)) == NULL) //从 swap 空间分配
		panic("out of swap space");
	expand(USIZE+ts); //扩展数据段
	estabur(0, ts, 0, 0);
	u.u_count = u.u_arg[1];
	u.u_offset[1] = 020;
	u.u_base = 0;
	readi(ip); //读取程序文件
	rp = u.u_procp;
	rp->p_flag =| SLOCK;
	swap(xp->x_daddr, rp->p_addr+USIZE, ts, 0); //交换出去，等待下次运行从交换空间换入
	rp->p_flag =& ~SLOCK;
	rp->p_textp = xp;
	rp = ip;
	rp->i_flag =| ITEXT; //inode增加文本段标示
	rp->i_count++; //增加inode技术
	expand(USIZE); //压缩数据段到最小长度

out:
	if(xp->x_ccount == 0) { //没有内存参照计数，将该程序暂时换出内存，执行调度，下次执行将从推出xalloc的地方运行(exec)
		savu(u.u_rsav);
		savu(u.u_ssav);
		xswap(u.u_procp, 1, 0);
		u.u_procp->p_flag =| SSWAP;
		swtch();
		/* no return */
	}
	xp->x_ccount++; //递增内存进程参照计数
}

/*
 * Decrement the in-core usage count of a shared text segment.
 * When it drops to zero, free the core space.
 */
xccdec(xp)
int *xp;
{
	register *rp;

	if((rp=xp)!=NULL && rp->x_ccount!=0) //递减内存参照计数，如果为0
		if(--rp->x_ccount == 0)
			mfree(coremap, rp->x_size, rp->x_caddr); //释放内存
}
