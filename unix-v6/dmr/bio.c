#
/*
 */

#include "../param.h"
#include "../user.h"
#include "../buf.h"
#include "../conf.h"
#include "../systm.h"
#include "../proc.h"
#include "../seg.h"

/*
 * This is the set of buffers proper, whose heads
 * were declared in buf.h.  There can exist buffer
 * headers not pointing here that are used purely
 * as arguments to the I/O routines to describe
 * I/O to be done-- e.g. swbuf, just below, for
 * swapping.
 */
char	buffers[NBUF][514]; //块设备数据拷贝内存区域
struct	buf	swbuf; //专用于交换的缓冲区，没有　５１２　的限制

/*
 * Declarations of the tables for the magtape devices;
 * see bdwrite.
 */
int	tmtab;
int	httab;

/*
 * The following several routines allocate and free
 * buffers with various side effects.  In general the
 * arguments to an allocate routine are a device and
 * a block number, and the value is a pointer to
 * to the buffer header; the buffer is marked "busy"
 * so that no on else can touch it.  If the block was
 * already in core, no I/O need be done; if it is
 * already busy, the process waits until it becomes free.
 * The following routines allocate a buffer:
 *	getblk
 *	bread
 *	breada
 * Eventually the buffer must be released, possibly with the
 * side effect of writing it out, by using one of
 *	bwrite
 *	bdwrite
 *	bawrite
 *	brelse
 */

/*
 * Read in (if necessary) the block and return a buffer pointer.
 */
bread(dev, blkno)
{
	register struct buf *rbp;

	rbp = getblk(dev, blkno); //获得缓冲区
	if (rbp->b_flags&B_DONE) //如果已经完成，直接返回
		return(rbp);
	rbp->b_flags =| B_READ; //设置 B_READ 标记
	rbp->b_wcount = -256; //设置读取的长度, 512 字节
	(*bdevsw[dev.d_major].d_strategy)(rbp); //执行驱动里的设备访问函数，读取数据
	iowait(rbp); //等待 io 完成
	return(rbp); //返回读取的缓冲区，调用进程需要调用 belse 负责释放
}

/*
 * Read in the block, like bread, but also start I/O on the
 * read-ahead block (which is not allocated to the caller)
 */
//会预读
//rablekno 预读取的快编号
breada(adev, blkno, rablkno)
{
	register struct buf *rbp, *rabp;
	register int dev;

	dev = adev;
	rbp = 0;
	if (!incore(dev, blkno)) { //如果还没有分配缓冲区
		rbp = getblk(dev, blkno); //分配缓冲区
		if ((rbp->b_flags&B_DONE) == 0) { //未完成
			rbp->b_flags =| B_READ;
			rbp->b_wcount = -256;
			(*bdevsw[adev.d_major].d_strategy)(rbp);  //读取数据
		}
	}
	if (rablkno && !incore(dev, rablkno)) {
		rabp = getblk(dev, rablkno);
		if (rabp->b_flags & B_DONE) //已经读取，释放缓冲区，注意并没有清除 B_DONE，后面仍然可以读取的
			brelse(rabp);
		else {
			rabp->b_flags =| B_READ|B_ASYNC; //预读是异步读，不等待
			rabp->b_wcount = -256;
			(*bdevsw[adev.d_major].d_strategy)(rabp); //开始读取
		}
	}
	if (rbp==0) //准备读取的块的缓冲区已经存在，直接读取
		return(bread(dev, blkno));
	iowait(rbp);
	return(rbp);
}

/*
 * Write the buffer, waiting for completion.
 * Then release the buffer.
 */
bwrite(bp)
struct buf *bp;
{
	register struct buf *rbp;
	register flag;

	rbp = bp;
	flag = rbp->b_flags;
	rbp->b_flags =& ~(B_READ | B_DONE | B_ERROR | B_DELWRI); //这里清除了写、读、错误和完成等标记
	rbp->b_wcount = -256;
	(*bdevsw[rbp->b_dev.d_major].d_strategy)(rbp); //调用设备访问函数
	if ((flag&B_ASYNC) == 0) { //如果不是异步写入
		iowait(rbp); //等待完成
		brelse(rbp); //释放缓冲区
	} else if ((flag&B_DELWRI)==0) //如果不是延迟写入
		geterror(rbp); //检查有没有错误
}

/*
 * Release the buffer, marking it so that if it is grabbed
 * for another purpose it will be written out before being
 * given up (e.g. when writing a partial block where it is
 * assumed that another write for the same block will soon follow).
 * This can't be done for magtape, since writes must be done
 * in the same order as requested.
 */
bdwrite(bp)
struct buf *bp;
{
	register struct buf *rbp;
	register struct devtab *dp;

	rbp = bp;
	dp = bdevsw[rbp->b_dev.d_major].d_tab;
	if (dp == &tmtab || dp == &httab) //磁带设备，异步写入
		bawrite(rbp);
	else {
		rbp->b_flags =| B_DELWRI | B_DONE; //设置延迟写入标记和 b_done 标记，预读发现 b_done，认为数据已经“读取”，也就不用重新读取了。
		brelse(rbp); //直接释放缓冲区，等缓冲区下次被使用的时候(getblk)，发现有延迟写入标志，才写入或者 bflush 定期写入
	}
}

/*
 * Release the buffer, start I/O on it, but don't wait for completion.
 */
bawrite(bp)
struct buf *bp;
{
	register struct buf *rbp;

	rbp = bp;
	rbp->b_flags =| B_ASYNC; //设置异步写入标志
	bwrite(rbp);
}

/*
 * release the buffer, with no I/O implied.
 */
brelse(bp)
struct buf *bp;
{
	register struct buf *rbp, **backp;
	register int sps;

	rbp = bp;
	if (rbp->b_flags&B_WANTED) //唤醒等待这个缓冲区进程
		wakeup(rbp);
	if (bfreelist.b_flags&B_WANTED) { //唤醒等待释放缓冲区到 av-list 的进程
		bfreelist.b_flags =& ~B_WANTED;
		wakeup(&bfreelist);
	}
	if (rbp->b_flags&B_ERROR) //设备操作错误，将小编号设置为-1，防止错误数据被使用
		rbp->b_dev.d_minor = -1;  /* no assoc. on error */
	backp = &bfreelist.av_back;
	sps = PS->integ;
	spl6();
	rbp->b_flags =& ~(B_WANTED|B_BUSY|B_ASYNC); //清除标记，注意并没有清除 B_DONE
	(*backp)->av_forw = rbp; //加到 av-list 末尾
	rbp->av_back = *backp;
	*backp = rbp;
	rbp->av_forw = &bfreelist;
	PS->integ = sps;
}

/*
 * See if the block is associated with some buffer
 * (mainly to avoid getting hung up on a wait in breada)
 */
//检查分配给设备的某个缓冲区是否存在，不存在返回0，否则返回缓冲区
incore(adev, blkno)
{
	register int dev;
	register struct buf *bp;
	register struct devtab *dp;

	dev = adev;
	dp = bdevsw[adev.d_major].d_tab;
  //遍历设备的缓冲区列表
	for (bp=dp->b_forw; bp != dp; bp = bp->b_forw)
		if (bp->b_blkno==blkno && bp->b_dev==dev)
			return(bp);
	return(0);
}

/*
 * Assign a buffer for the given block.  If the appropriate
 * block is already associated, return it; otherwise search
 * for the oldest non-busy buffer and reassign it.
 * When a 512-byte area is wanted for some random reason
 * (e.g. during exec, for the user arglist) getblk can be called
 * with device NODEV to avoid unwanted associativity.
 */
//dev 设备编号
//blkno 块编号
getblk(dev, blkno)
{
	register struct buf *bp;
	register struct devtab *dp;
	extern lbolt;

	if(dev.d_major >= nblkdev)
		panic("blkdev");

    loop:
	if (dev < 0) //NODEV(-1)处理,设置为 b-list 头
		dp = &bfreelist;
	else {
		dp = bdevsw[dev.d_major].d_tab;
		if(dp == NULL)
			panic("devtab");
    //遍历设备的 b-list ,检查是否存在所需要的缓冲区 [设备编号,块编号]
		for (bp=dp->b_forw; bp != dp; bp = bp->b_forw) {
			if (bp->b_blkno!=blkno || bp->b_dev!=dev)
				continue;
			spl6();
			if (bp->b_flags&B_BUSY) {
				bp->b_flags =| B_WANTED;
				sleep(bp, PRIBIO); //正在被使用,睡眠等待释放
				spl0();
				goto loop;
			}
			spl0();
			notavail(bp); //设置正在使用标记,从av-list删除并返回
			return(bp);
		}
	}
	spl6(); //防止中断,因为块设备在处理结束因触发的中断处理会操作缓冲区,防止中断来避免发生冲突
	if (bfreelist.av_forw == &bfreelist) { //av-list 为空
		bfreelist.b_flags =| B_WANTED;
		sleep(&bfreelist, PRIBIO); //等待
		spl0();
		goto loop;
	}
	spl0();
	notavail(bp = bfreelist.av_forw); //获得开始元素,从av-list删除，同时设置了 busy 标记
	if (bp->b_flags & B_DELWRI) { //如果设置了延迟写入标记，立刻做同步写入。
		bp->b_flags =| B_ASYNC; //移除 b_async 标志
		bwrite(bp); //同步写入
		goto loop;
	}
  //从 av-list 分配的，下面是直接设置标记，原有的什么 B_DONE 都忽略掉，全新
	bp->b_flags = B_BUSY | B_RELOC; //设置 busy 标记
	bp->b_back->b_forw = bp->b_forw;  //从当前 b-list 删除
	bp->b_forw->b_back = bp->b_back;
	bp->b_forw = dp->b_forw; //加入新的b-list开始位置
	bp->b_back = dp;
	dp->b_forw->b_back = bp;
	dp->b_forw = bp;
	bp->b_dev = dev; //命名缓冲区，设备编号
	bp->b_blkno = blkno; //块编号
	return(bp);
}

/*
 * Wait for I/O completion on the buffer; return errors
 * to the user.
 */
iowait(bp)
struct buf *bp;
{
	register struct buf *rbp;

	rbp = bp;
	spl6(); //屏蔽中断
	while ((rbp->b_flags&B_DONE)==0)
		sleep(rbp, PRIBIO); //spin wait，等待完成
	spl0();
	geterror(rbp);
}

/*
 * Unlink a buffer from the available list and mark it busy.
 * (internal interface)
 */
notavail(bp)
struct buf *bp;
{
	register struct buf *rbp;
	register int sps;

	rbp = bp;
	sps = PS->integ;
	spl6(); //禁止中断
	rbp->av_back->av_forw = rbp->av_forw; //从 av-list 删除
	rbp->av_forw->av_back = rbp->av_back;
	rbp->b_flags =| B_BUSY; //设置 busy 标记
	PS->integ = sps; //恢复处理器优先级
}

/*
 * Mark I/O complete on a buffer, release it if I/O is asynchronous,
 * and wake up anyone waiting for it.
 */
iodone(bp)
struct buf *bp;
{
	register struct buf *rbp;

	rbp = bp;
	if(rbp->b_flags&B_MAP)
		mapfree(rbp);
	rbp->b_flags =| B_DONE; //标记 done
	if (rbp->b_flags&B_ASYNC) //异步读写，释放缓冲区
		brelse(rbp);
	else {
		rbp->b_flags =& ~B_WANTED;
		wakeup(rbp); //唤醒等待完成的进程
	}
}

/*
 * Zero the core associated with a buffer.
 */
clrbuf(bp)
int *bp;
{
	register *p;
	register c;

	p = bp->b_addr;
	c = 256;
	do
		*p++ = 0; //清空 buffer
	while (--c);
}

/*
 * Initialize the buffer I/O system by freeing
 * all buffers and setting all device buffer lists to empty.
 */
binit()
{
	register struct buf *bp;
	register struct devtab *dp;
	register int i;
	struct bdevsw *bdp;

	bfreelist.b_forw = bfreelist.b_back =
	    bfreelist.av_forw = bfreelist.av_back = &bfreelist;
  //初始化缓冲区
	for (i=0; i<NBUF; i++) {
		bp = &buf[i];
		bp->b_dev = -1; // NODEV 状态
		bp->b_addr = buffers[i];
		bp->b_back = &bfreelist;
		bp->b_forw = bfreelist.b_forw;
		bfreelist.b_forw->b_back = bp;
		bfreelist.b_forw = bp;
		bp->b_flags = B_BUSY;
		brelse(bp);
	}
	i = 0;
  //初始化 devtab 结构
	for (bdp = bdevsw; bdp->d_open; bdp++) {
		dp = bdp->d_tab;
		if(dp) {
			dp->b_forw = dp; //各设备的 b-list 头部的 forw/back指向自身
			dp->b_back = dp;
		}
		i++;
	}
	nblkdev = i;
}

/*
 * Device start routine for disks
 * and other devices that have the register
 * layout of the older DEC controllers (RF, RK, RP, TM)
 */
#define	IENABLE	0100
#define	WCOM	02
#define	RCOM	04
#define	GO	01
devstart(bp, devloc, devblk, hbcom)
struct buf *bp;
int *devloc;
{
	register int *dp;
	register struct buf *rbp;
	register int com;

	dp = devloc;
	rbp = bp;
	*dp = devblk;			/* block address */
	*--dp = rbp->b_addr;		/* buffer address */
	*--dp = rbp->b_wcount;		/* word count */
	com = (hbcom<<8) | IENABLE | GO |
		((rbp->b_xmem & 03) << 4);
	if (rbp->b_flags&B_READ)	/* command + x-mem */
		com =| RCOM;
	else
		com =| WCOM;
	*--dp = com;
}

/*
 * startup routine for RH controllers.
 */
#define	RHWCOM	060
#define	RHRCOM	070

rhstart(bp, devloc, devblk, abae)
struct buf *bp;
int *devloc, *abae;
{
	register int *dp;
	register struct buf *rbp;
	register int com;

	dp = devloc;
	rbp = bp;
	if(cputype == 70)
		*abae = rbp->b_xmem;
	*dp = devblk;			/* block address */
	*--dp = rbp->b_addr;		/* buffer address */
	*--dp = rbp->b_wcount;		/* word count */
	com = IENABLE | GO |
		((rbp->b_xmem & 03) << 8);
	if (rbp->b_flags&B_READ)	/* command + x-mem */
		com =| RHRCOM; else
		com =| RHWCOM;
	*--dp = com;
}

/*
 * 11/70 routine to allocate the
 * UNIBUS map and initialize for
 * a unibus device.
 * The code here and in
 * rhstart assumes that an rh on an 11/70
 * is an rh70 and contains 22 bit addressing.
 */
int	maplock;
mapalloc(abp)
struct buf *abp;
{
	register i, a;
	register struct buf *bp;

	if(cputype != 70)
		return;
	spl6();
	while(maplock&B_BUSY) {
		maplock =| B_WANTED;
		sleep(&maplock, PSWP);
	}
	maplock =| B_BUSY;
	spl0();
	bp = abp;
	bp->b_flags =| B_MAP;
	a = bp->b_xmem;
	for(i=16; i<32; i=+2)
		UBMAP->r[i+1] = a;
	for(a++; i<48; i=+2)
		UBMAP->r[i+1] = a;
	bp->b_xmem = 1;
}

mapfree(bp)
struct buf *bp;
{

	bp->b_flags =& ~B_MAP;
	if(maplock&B_WANTED)
		wakeup(&maplock);
	maplock = 0;
}

/*
 * swap I/O
 */
swap(blkno, coreaddr, count, rdflg)
{
	register int *fp;

	fp = &swbuf.b_flags;
	spl6();
	while (*fp&B_BUSY) {
		*fp =| B_WANTED;
		sleep(fp, PSWP);
	}
	*fp = B_BUSY | B_PHYS | rdflg;
	swbuf.b_dev = swapdev;
	swbuf.b_wcount = - (count<<5);	/* 32 w/block */
	swbuf.b_blkno = blkno;
	swbuf.b_addr = coreaddr<<6;	/* 64 b/block */
	swbuf.b_xmem = (coreaddr>>10) & 077;
	(*bdevsw[swapdev>>8].d_strategy)(&swbuf);
	spl6();
	while((*fp&B_DONE)==0)
		sleep(fp, PSWP);
	if (*fp&B_WANTED)
		wakeup(fp);
	spl0();
	*fp =& ~(B_BUSY|B_WANTED);
	return(*fp&B_ERROR);
}

/*
 * make sure all write-behind blocks
 * on dev (or NODEV for all)
 * are flushed out.
 * (from umount and update)
 */
bflush(dev)
{
	register struct buf *bp;

loop:
	spl6(); //防止中断
	for (bp = bfreelist.av_forw; bp != &bfreelist; bp = bp->av_forw) {
		if (bp->b_flags&B_DELWRI && (dev == NODEV||dev==bp->b_dev)) {
			bp->b_flags =| B_ASYNC; //加入延迟写入标志
			notavail(bp); // 从 av-list 移除，设置 busy 标记
			bwrite(bp); //写入设备
			goto loop;
		}
	}
	spl0();
}

/*
 * Raw I/O. The arguments are
 *	The strategy routine for the device
 *	A buffer, which will always be a special buffer
 *	  header owned exclusively by the device for this purpose
 *	The device number
 *	Read/write flag
 * Essentially all the work is computing physical addresses and
 * validating them.
 */
physio(strat, abp, dev, rw)
struct buf *abp;
int (*strat)();
{
	register struct buf *bp;
	register char *base;
	register int nb;
	int ts;

	bp = abp;
	base = u.u_base;
	/*
	 * Check odd base, odd count, and address wraparound
	 */
	if (base&01 || u.u_count&01 || base>=base+u.u_count)
		goto bad;
	ts = (u.u_tsize+127) & ~0177;
	if (u.u_sep)
		ts = 0;
	nb = (base>>6) & 01777;
	/*
	 * Check overlap with text. (ts and nb now
	 * in 64-byte clicks)
	 */
	if (nb < ts)
		goto bad;
	/*
	 * Check that transfer is either entirely in the
	 * data or in the stack: that is, either
	 * the end is in the data or the start is in the stack
	 * (remember wraparound was already checked).
	 */
	if ((((base+u.u_count)>>6)&01777) >= ts+u.u_dsize
	    && nb < 1024-u.u_ssize)
		goto bad;
	spl6();
	while (bp->b_flags&B_BUSY) {
		bp->b_flags =| B_WANTED;
		sleep(bp, PRIBIO);
	}
	bp->b_flags = B_BUSY | B_PHYS | rw;
	bp->b_dev = dev;
	/*
	 * Compute physical address by simulating
	 * the segmentation hardware.
	 */
	bp->b_addr = base&077;
	base = (u.u_sep? UDSA: UISA)->r[nb>>7] + (nb&0177);
	bp->b_addr =+ base<<6;
	bp->b_xmem = (base>>10) & 077;
	bp->b_blkno = lshift(u.u_offset, -9);
	bp->b_wcount = -((u.u_count>>1) & 077777);
	bp->b_error = 0;
	u.u_procp->p_flag =| SLOCK;　//锁定，防止被换出
　(*strat)(bp); //执行设备函数
	spl6();
	while ((bp->b_flags&B_DONE) == 0)
		sleep(bp, PRIBIO);
	u.u_procp->p_flag =& ~SLOCK;
	if (bp->b_flags&B_WANTED)
		wakeup(bp);
	spl0();
	bp->b_flags =& ~(B_BUSY|B_WANTED);
	u.u_count = (-bp->b_resid)<<1;
	geterror(bp);
	return;
    bad:
	u.u_error = EFAULT;
}

/*
 * Pick up the device's error number and pass it to the user;
 * if there is an error but the number is 0 set a generalized
 * code.  Actually the latter is always true because devices
 * don't yet return specific errors.
 */
geterror(abp)
struct buf *abp;
{
	register struct buf *bp;

	bp = abp;
	if (bp->b_flags&B_ERROR)
		if ((u.u_error = bp->b_error)==0)
			u.u_error = EIO;
}
