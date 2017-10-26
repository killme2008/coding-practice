#
/*
 */

#include "../param.h"
#include "../systm.h"
#include "../filsys.h"
#include "../conf.h"
#include "../buf.h"
#include "../inode.h"
#include "../user.h"

/*
 * iinit is called once (from main)
 * very early in initialization.
 * It reads the root's super block
 * and initializes the current date
 * from the last modified date.
 *
 * panic: iinit -- cannot read the super
 * block. Usually because of an IO error.
 */
iinit()
{
	register *cp, *bp;

	(*bdevsw[rootdev.d_major].d_open)(rootdev, 1);
	bp = bread(rootdev, 1);
	cp = getblk(NODEV);
	if(u.u_error)
		panic("iinit");
	bcopy(bp->b_addr, cp->b_addr, 256);
	brelse(bp);
	mount[0].m_bufp = cp;
	mount[0].m_dev = rootdev;
	cp = cp->b_addr;
	cp->s_flock = 0;
	cp->s_ilock = 0;
	cp->s_ronly = 0;
	time[0] = cp->s_time[0];
	time[1] = cp->s_time[1];
}

/*
 * alloc will obtain the next available
 * free disk block from the free list of
 * the specified device.
 * The super block has up to 100 remembered
 * free blocks; the last of these is read to
 * obtain 100 more . . .
 *
 * no space on dev x/y -- when
 * the free list is exhausted.
 */
alloc(dev)
{
	int bno;
	register *bp, *ip, *fp;

	fp = getfs(dev);
  //获取空闲队列锁，如果正在被使用，等待
	while(fp->s_flock)
		sleep(&fp->s_flock, PINOD);
	do {
    //没有空闲，跳转
		if(fp->s_nfree <= 0)
			goto nospace;
    //查找到一个空闲的
		bno = fp->s_free[--fp->s_nfree];
    //如果不指向任何存储位置，也跳转到 nospace
		if(bno == 0)
			goto nospace;
	} while (badblock(fp, bno, dev));
  //查找到一个，同时空闲队列没有了，补充下
	if(fp->s_nfree <= 0) {
    //加锁
		fp->s_flock++;
    //读取设备存储区
    //bno 指向了下一个未使用的块编号的队列的块，读取，见下面对 free 的注解
		bp = bread(dev, bno);
		ip = bp->b_addr;
    //读取的块的头部容纳了块编号的数量，拷贝到 s_nfree
		fp->s_nfree = *ip++;
    //拷贝队列到 s_free
		bcopy(ip, fp->s_free, 100);
    //释放缓冲区
		brelse(bp);
    //释放锁并唤醒
		fp->s_flock = 0;
		wakeup(&fp->s_flock);
	}
  //读取块对应的缓冲区
	bp = getblk(dev, bno);
  //清零
	clrbuf(bp);
	fp->s_fmod = 1;
  //返回
	return(bp);

nospace:
  //报错，没有空间了
	fp->s_nfree = 0;
	prdev("no space", dev);
	u.u_error = ENOSPC;
	return(NULL);
}

/*
 * place the specified disk block
 * back on the free list of the
 * specified device.
 */
free(dev, bno)
{
	register *fp, *bp, *ip;

	fp = getfs(dev);
  //设置 fs 更新标志，这个更适合放在 badblock 判断之后，减少不必要的写入
	fp->s_fmod = 1;
  //等待锁
	while(fp->s_flock)
		sleep(&fp->s_flock, PINOD);
  //非存储块，返回
	if (badblock(fp, bno, dev))
		return;
  //理论上不应该出现
	if(fp->s_nfree <= 0) {
		fp->s_nfree = 1;
		fp->s_free[0] = 0;
	}
  //空闲队列满了
	if(fp->s_nfree >= 100) {
    //加锁
		fp->s_flock++;
    //将空闲队列写回设备
		bp = getblk(dev, bno);
		ip = bp->b_addr;
		*ip++ = fp->s_nfree; //开始位置是总数
    //队列复制到缓冲区
		bcopy(fp->s_free, ip, 100);
    //清空 nfree
		fp->s_nfree = 0;
		bwrite(bp);
		fp->s_flock = 0;
		wakeup(&fp->s_flock);
	}
  //空闲队列追加块编号
  //如果上面空闲队列满的结果为真，那么 s_nfree 是 0，这里就将 s_free[0] 设置为下一个队列的块编号。
  //呼应 alloc 补充队列
	fp->s_free[fp->s_nfree++] = bno;
	fp->s_fmod = 1;
}

/*
 * Check that a block number is in the
 * range between the I list and the size
 * of the device.
 * This is used mainly to check that a
 * garbage file system has not been mounted.
 *
 * bad block on dev x/y -- not in range
 */
badblock(afp, abn, dev)
{
	register struct filsys *fp;
	register char *bn;

	fp = afp;
	bn = abn;
	if (bn < fp->s_isize+2 || bn >= fp->s_fsize) {
		prdev("bad block", dev);
		return(1);
	}
  //指向存储区域返回0
	return(0);
}

/*
 * Allocate an unused I node
 * on the specified device.
 * Used with file creation.
 * The algorithm keeps up to
 * 100 spare I nodes in the
 * super block. When this runs out,
 * a linear search through the
 * I list is instituted to pick
 * up 100 more.
 */
ialloc(dev)
{
	register *fp, *bp, *ip;
	int i, j, k, ino;

	fp = getfs(dev);
  //等待 inode 锁
	while(fp->s_ilock)
		sleep(&fp->s_ilock, PINOD);
loop:
  //有空闲的
	if(fp->s_ninode > 0) {
		ino = fp->s_inode[--fp->s_ninode];
		ip = iget(dev, ino);
		if (ip==NULL)
			return(NULL);
    //是 itrunc 释放的
		if(ip->i_mode == 0) {
      //清空　i_mode 到　i_addr 的数据
			for(bp = &ip->i_mode; bp < &ip->i_addr[8];)
				*bp++ = 0;
      //设置 filsys 更新标志
			fp->s_fmod = 1;
      //返回
			return(ip);
		}
		/*
		 * Inode was allocated after all.
		 * Look some more.
		 */
		iput(ip);
    //找到的 i_mode 不为０，继续尝试
		goto loop;
	}
  //加锁
	fp->s_ilock++;
	ino = 0;
  //遍历超级块所有的 inode
	for(i=0; i<fp->s_isize; i++) {
		bp = bread(dev, i+2);
		ip = bp->b_addr;
		for(j=0; j<256; j=+16) {
			ino++;
      // i_mode 不为 0，忽略
			if(ip[j] != 0)
				continue;
			for(k=0; k<NINODE; k++)
        //已分配的，忽略
			if(dev==inode[k].i_dev && ino==inode[k].i_number)
				goto cont;
      //找到未分配的，加入空闲队列
			fp->s_inode[fp->s_ninode++] = ino;
      //超过１００个空闲，跳出循环
			if(fp->s_ninode >= 100)
				break;
		cont:;
		}
		brelse(bp);
		if(fp->s_ninode >= 100)
			break;
	}
  //释放锁并唤醒
	fp->s_ilock = 0;
	wakeup(&fp->s_ilock);
  //有空闲的，进入　loop 重新分配
	if (fp->s_ninode > 0)
		goto loop;
  //报错， inode 溢出
	prdev("Out of inodes", dev);
	u.u_error = ENOSPC;
	return(NULL);
}

/*
 * Free the specified I node
 * on the specified device.
 * The algorithm stores up
 * to 100 I nodes in the super
 * block and throws away any more.
 */
ifree(dev, ino)
{
	register *fp;

	fp = getfs(dev);
  //已被锁定使用中，不处理了，后续等 ialloc 会回收
	if(fp->s_ilock)
		return;
  //空闲超过 100　个，忽略
	if(fp->s_ninode >= 100)
		return;
  //返回空闲队列
	fp->s_inode[fp->s_ninode++] = ino;
  //设置更新标志
	fp->s_fmod = 1;
}

/*
 * getfs maps a device number into
 * a pointer to the incore super
 * block.
 * The algorithm is a linear
 * search through the mount table.
 * A consistency check of the
 * in core free-block and i-node
 * counts.
 *
 * bad count on dev x/y -- the count
 *	check failed. At this point, all
 *	the counts are zeroed which will
 *	almost certainly lead to "no space"
 *	diagnostic
 * panic: no fs -- the device is not mounted.
 *	this "cannot happen"
 */
//根据设备号获取超级块
getfs(dev)
{
	register struct mount *p;
	register char *n1, *n2;

  //遍历 mount 数组，找到对应设备号的 mount 结构体
	for(p = &mount[0]; p < &mount[NMOUNT]; p++)
	if(p->m_bufp != NULL && p->m_dev == dev) {
    //p就是超级块
		p = p->m_bufp->b_addr;
		n1 = p->s_nfree;
		n2 = p->s_ninode;
    //异常值处理
		if(n1 > 100 || n2 > 100) {
			prdev("bad count", dev);
			p->s_nfree = 0;
			p->s_ninode = 0;
		}
		return(p);
	}
  //没有找到设备，异常
	panic("no fs");
}

/*
 * update is the internal name of
 * 'sync'. It goes through the disk
 * queues to initiate sandbagged IO;
 * goes through the I nodes to write
 * modified nodes; and it goes through
 * the mount table to initiate modified
 * super blocks.
 */
update()
{
	register struct inode *ip;
	register struct mount *mp;
	register *bp;

	if(updlock)
		return;
	updlock++;
	for(mp = &mount[0]; mp < &mount[NMOUNT]; mp++)
		if(mp->m_bufp != NULL) {
			ip = mp->m_bufp->b_addr;
			if(ip->s_fmod==0 || ip->s_ilock!=0 ||
			   ip->s_flock!=0 || ip->s_ronly!=0)
				continue;
			bp = getblk(mp->m_dev, 1);
			ip->s_fmod = 0;
			ip->s_time[0] = time[0];
			ip->s_time[1] = time[1];
			bcopy(ip, bp->b_addr, 256);
			bwrite(bp);
		}
	for(ip = &inode[0]; ip < &inode[NINODE]; ip++)
		if((ip->i_flag&ILOCK) == 0) {
			ip->i_flag =| ILOCK;
			iupdat(ip, time);
			prele(ip);
		}
	updlock = 0;
	bflush(NODEV);
}
