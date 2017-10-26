#
/*
 */

#include "../param.h"
#include "../systm.h"
#include "../reg.h"
#include "../buf.h"
#include "../filsys.h"
#include "../user.h"
#include "../inode.h"
#include "../file.h"
#include "../conf.h"

/*
 * the fstat system call.
 */
fstat()
{
	register *fp;

	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
	stat1(fp->f_inode, u.u_arg[0]);
}

/*
 * the stat system call.
 */
stat()
{
	register ip;
	extern uchar;

	ip = namei(&uchar, 0);
	if(ip == NULL)
		return;
	stat1(ip, u.u_arg[1]);
	iput(ip);
}

/*
 * The basic routine for fstat and stat:
 * get the inode and pass appropriate parts back.
 */
stat1(ip, ub)
int *ip;
{
	register i, *bp, *cp;

	iupdat(ip, time);
	bp = bread(ip->i_dev, ldiv(ip->i_number+31, 16));
	cp = bp->b_addr + 32*lrem(ip->i_number+31, 16) + 24;
	ip = &(ip->i_dev);
	for(i=0; i<14; i++) {
		suword(ub, *ip++);
		ub =+ 2;
	}
	for(i=0; i<4; i++) {
		suword(ub, *cp++);
		ub =+ 2;
	}
	brelse(bp);
}

/*
 * the dup system call.
 */
dup()
{
	register i, *fp;

	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
	if ((i = ufalloc()) < 0)
		return;
	u.u_ofile[i] = fp;
	fp->f_count++;
}

/*
 * the mount system call.
 * mount 系统调用，三个参数：
 * u_arg[0] 与挂载设备对应的特殊文件的路径
 * u_arg[1] 挂载点的路径
 * u_arg[2] 读写标志，１表示只读
 */
smount()
{
	int d;
	register *ip;
	register struct mount *mp, *smp;
	extern uchar;

  //获取设备的大编号
	d = getmdev();
	if(u.u_error)
		return;
	u.u_dirp = u.u_arg[1];
  //　获取代表挂载点路径的 inode[] 元素
	ip = namei(&uchar, 0);
	if(ip == NULL)
		return;
  //确认无人访问，并且确认不是字符设备或者块设备的特殊文件
	if(ip->i_count!=1 || (ip->i_mode&(IFBLK&IFCHR))!=0)
		goto out;
  //查找空闲的 mount 位置
	smp = NULL;
	for(mp = &mount[0]; mp < &mount[NMOUNT]; mp++) {
		if(mp->m_bufp != NULL) {
      //已经挂载了，报错
			if(d == mp->m_dev)
				goto out;
		} else
		if(smp == NULL)
			smp = mp;
	}
  //找不到空闲，报错
	if(smp == NULL)
		goto out;
  //打开挂载设备
	(*bdevsw[d.d_major].d_open)(d, !u.u_arg[2]);
	if(u.u_error)
		goto out;
  //将超级快读入缓冲区
	mp = bread(d, 1);
	if(u.u_error) {
		brelse(mp);
		goto out1;
	}
	smp->m_inodp = ip;
	smp->m_dev = d;
	smp->m_bufp = getblk(NODEV);
  //复制超级块数据到缓冲区
	bcopy(mp->b_addr, smp->m_bufp->b_addr, 256);
	smp = smp->m_bufp->b_addr;
	smp->s_ilock = 0;
	smp->s_flock = 0;
	smp->s_ronly = u.u_arg[2] & 1;
	brelse(mp);//释放读取缓冲区
  //设置 inode IMOUNT 标志
	ip->i_flag =| IMOUNT;
  //解除 inode 锁
	prele(ip);
	return;

out:
	u.u_error = EBUSY;
out1:
	iput(ip);
}

/*
 * the umount system call.
 */
sumount()
{
	int d;
	register struct inode *ip;
	register struct mount *mp;

	update();
	d = getmdev();
	if(u.u_error)
		return;
	for(mp = &mount[0]; mp < &mount[NMOUNT]; mp++)
    //找到设备在 mount 数组中的位置
		if(mp->m_bufp!=NULL && d==mp->m_dev)
			goto found;
	u.u_error = EINVAL;
	return;

found:
  //遍历 inode，如果还在使用，返回 busy 错误
	for(ip = &inode[0]; ip < &inode[NINODE]; ip++)
		if(ip->i_number!=0 && d==ip->i_dev) {
			u.u_error = EBUSY;
			return;
		}
  //关闭设备
	(*bdevsw[d.d_major].d_close)(d, 0);
	ip = mp->m_inodp;
  //移除　IMOUNT 标示
	ip->i_flag =& ~IMOUNT;
  //递减计数
	iput(ip);
	ip = mp->m_bufp;
  //释放超级块缓冲区
	mp->m_bufp = NULL;
	brelse(ip);
}

/*
 * Common code for mount and umount.
 * Check that the user's argument is a reasonable
 * thing on which to mount, and return the device number if so.
 */
getmdev()
{
	register d, *ip;
	extern uchar;

  //路径转为 inode[] 元素
	ip = namei(&uchar, 0);
	if(ip == NULL)
		return;
  //非块设备的特殊文件，报错
	if((ip->i_mode&IFMT) != IFBLK)
		u.u_error = ENOTBLK;
	d = ip->i_addr[0];
  //编号过大，报错
	if(ip->i_addr[0].d_major >= nblkdev)
		u.u_error = ENXIO;
	iput(ip);
	return(d);
}
