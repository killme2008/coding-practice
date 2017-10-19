#
/*
 */

/*
 * RK disk driver
 */

#include "../param.h"
#include "../buf.h"
#include "../conf.h"
#include "../user.h"

#define	RKADDR	0177400
#define	NRK	4
#define	NRKBLK	4872

#define	RESET	0
#define	GO	01
#define	DRESET	014
#define	IENABLE	0100
#define	DRY	0200
#define	ARDY	0100
#define	WLO	020000
#define	CTLRDY	0200

// rk 磁盘寄存器
struct {
	int rkds;
	int rker;
	int rkcs;
	int rkwc;
	int rkba;
	int rkda;
};

struct	devtab	rktab;
// rk 磁盘的读写缓冲区
struct	buf	rrkbuf;

//rk磁盘读写的函数，定义在 conf/mkconf.c 里
rkstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;
	register *qc, *ql;
	int d;

	bp = abp;
	if(bp->b_flags&B_PHYS)
		mapalloc(bp);
	d = bp->b_dev.d_minor-7;
	if(d <= 0)
		d = 1;
	if (bp->b_blkno >= NRKBLK*d) {
		bp->b_flags =| B_ERROR;
		iodone(bp);
		return;
	}
	bp->av_forw = 0; //将缓冲区末尾设置为０
	spl5();　//禁止 rk 中断
  //如果队列为空
	if (rktab.d_actf==0)
		rktab.d_actf = bp;
	else
		rktab.d_actl->av_forw = bp;　//不为空，追加到末尾
	rktab.d_actl = bp;
  //不处于处理状态，启动处理
	if (rktab.d_active==0)
		rkstart();
  //重置优先级
	spl0();
}

rkaddr(bp)
struct buf *bp;
{
	register struct buf *p;
	register int b;
	int d, m;

	p = bp;
	b = p->b_blkno;
	m = p->b_dev.d_minor - 7;
	if(m <= 0)
		d = p->b_dev.d_minor;
	else {
		d = lrem(b, m);
		b = ldiv(b, m);
	}
	return(d<<13 | (b/12)<<4 | b%12);
}

rkstart()
{
	register struct buf *bp;
  //缓冲区为空，返回
	if ((bp = rktab.d_actf) == 0)
		return;
	rktab.d_active++;
  //启动磁盘处理
	devstart(bp, &RKADDR->rkda, rkaddr(bp), 0);
}

//设备处理结束，引发中断，删除头部的缓冲区
rkintr()
{
	register struct buf *bp;

  //未启动，忽略
	if (rktab.d_active == 0)
		return;
	bp = rktab.d_actf;
	rktab.d_active = 0;
	if (RKADDR->rkcs < 0) {		/* error bit */
		deverror(bp, RKADDR->rker, RKADDR->rkds);
		RKADDR->rkcs = RESET|GO;
		while((RKADDR->rkcs&CTLRDY) == 0) ;
    //有错误，重试１０次
		if (++rktab.d_errcnt <= 10) {
			rkstart();
			return;
		}
    //超过重试上限，设置　error
		bp->b_flags =| B_ERROR;
	}
	rktab.d_errcnt = 0;
  //删除头部缓冲区
	rktab.d_actf = bp->av_forw;
  //iodone 通知
	iodone(bp);
  //继续处理下一个缓冲区
	rkstart();
}

//使用　physio　读写设备
rkread(dev)
{

	physio(rkstrategy, &rrkbuf, dev, B_READ);
}

rkwrite(dev)
{

	physio(rkstrategy, &rrkbuf, dev, B_WRITE);
}
