#
#include "../param.h"
#include "../systm.h"
#include "../user.h"
#include "../inode.h"
#include "../filsys.h"
#include "../conf.h"
#include "../buf.h"

/*
 * Look up an inode by device,inumber.
 * If it is in core (in the inode structure),
 * honor the locking protocol.
 * If it is not in core, read it in from the
 * specified device.
 * If the inode is mounted on, perform
 * the indicated indirection.
 * In all cases, a pointer to a locked
 * inode structure is returned.
 *
 * printf warning: no inodes -- if the inode
 *	structure is full
 * panic: no imt -- if the mounted file
 *	system is not in the mount table.
 *	"cannot happen"
 */
iget(dev, ino)
{
	register struct inode *p;
	register *ip2;
	int *ip1;
	register struct mount *ip;

loop:
	ip = NULL;
	for(p = &inode[0]; p < &inode[NINODE]; p++) {
    //在内存找到
		if(dev==p->i_dev && ino==p->i_number) {
      //如果正在被使用，等待睡眠
			if((p->i_flag&ILOCK) != 0) {
				p->i_flag =| IWANT;
				sleep(p, PINOD);
				goto loop;
			}
      //如果是 mount 挂载
			if((p->i_flag&IMOUNT) != 0) {
				for(ip = &mount[0]; ip < &mount[NMOUNT]; ip++)
				if(ip->m_inodp == p) {
          //获取挂载的设备号，继续循环，获取挂载点的 inode
					dev = ip->m_dev;
					ino = ROOTINO;
					goto loop;
				}
        //没有挂载，不应该出现
				panic("no imt");
			}
      //在内存里找到，并且没有被使用，不是挂载点，加锁返回
			p->i_count++;
			p->i_flag =| ILOCK;
			return(p);
		}
    //找到一个最近未使用的位置
		if(ip==NULL && p->i_count==0)
			ip = p;
	}
  // inode 表溢出了
	if((p=ip) == NULL) {
		printf("Inode table overflow\n");
		u.u_error = ENFILE;
		return(NULL);
	}
  //初始化 inode
	p->i_dev = dev;
	p->i_number = ino;
	p->i_flag = ILOCK;　//只保留 lock 标示
	p->i_count++;
	p->i_lastr = -1; //初始状态，没有预读的逻辑块
  //从设备中读取 inode
	ip = bread(dev, ldiv(ino+31,16));
	/*
	 * Check I/O errors
	 */
	if (ip->b_flags&B_ERROR) {
		brelse(ip);
		iput(p);
		return(NULL);
	}
  //从i_mode 到　i_addr 拷贝到 p
	ip1 = ip->b_addr + 32*lrem(ino+31, 16);
	ip2 = &p->i_mode;
	while(ip2 < &p->i_addr[8])
		*ip2++ = *ip1++;
  //释放缓冲区
	brelse(ip);
  //返回 p
	return(p);
}

/*
 * Decrement reference count of
 * an inode structure.
 * On the last reference,
 * write the inode out and if necessary,
 * truncate and deallocate the file.
 */
iput(p)
struct inode *p;
{
	register *rp;

	rp = p;
  //最后一个引用，特殊处理
	if(rp->i_count == 1) {
		rp->i_flag =| ILOCK;
    //来自目录的参照计数为０，文件可以删除了
		if(rp->i_nlink <= 0) {
      //清除文件
			itrunc(rp);
			rp->i_mode = 0;
      //返回空闲队列
			ifree(rp->i_dev, rp->i_number);
		}
    //写入块设备
		iupdat(rp, time);
		prele(rp);
		rp->i_flag = 0;
		rp->i_number = 0;
	}
  //不是最后一个，只是递减
	rp->i_count--;
  //解锁
	prele(rp);
}

/*
 * Check accessed and update flags on
 * an inode structure.
 * If either is on, update the inode
 * with the corresponding dates
 * set to the argument tm.
 */
iupdat(p, tm)
int *p;
int *tm;
{
	register *ip1, *ip2, *rp;
	int *bp, i;

	rp = p;
  //有更新或者访问的标示
	if((rp->i_flag&(IUPD|IACC)) != 0) {
    //读取超级块，判断不是只读挂载
		if(getfs(rp->i_dev)->s_ronly)
			return;
		i = rp->i_number+31;
		bp = bread(rp->i_dev, ldiv(i,16));
		ip1 = bp->b_addr + 32*lrem(i, 16);
		ip2 = &rp->i_mode;
    //更新 i_mode 到　i_addr 到块缓冲区，也就是写入
		while(ip2 < &rp->i_addr[8])
			*ip1++ = *ip2++;
    //如果有访问，更新时间戳 i_atime
		if(rp->i_flag&IACC) {
			*ip1++ = time[0];
			*ip1++ = time[1];
		} else
			ip1 =+ 2;
    //如果有更新，更新时间戳 i_mtime
		if(rp->i_flag&IUPD) {
			*ip1++ = *tm++;
			*ip1++ = *tm;
		}
    //写入设备
		bwrite(bp);
	}
}

/*
 * Free all the disk blocks associated
 * with the specified inode structure.
 * The blocks of the file are removed
 * in reverse order. This FILO
 * algorithm will tend to maintain
 * a contiguous free list much longer
 * than FIFO.
 */
//释放 inode 以及对应的缓冲区
itrunc(ip)
int *ip;
{
	register *rp, *bp, *cp;
	int *dp, *ep;

	rp = ip;
  //特殊文件，不做处理
	if((rp->i_mode&(IFCHR&IFBLK)) != 0)
		return;
	for(ip = &rp->i_addr[7]; ip >= &rp->i_addr[0]; ip--)
    //尝试释放存储空间
	if(*ip) {
    //间接引用
		if((rp->i_mode&ILARG) != 0) {
      //读取间接参照块
			bp = bread(rp->i_dev, *ip);
			for(cp = bp->b_addr+512; cp >= bp->b_addr; cp--)
			if(*cp) {
        //双重间接参照
				if(ip == &rp->i_addr[7]) {
					dp = bread(rp->i_dev, *cp);
          //释放双重间接参照的块
					for(ep = dp->b_addr+512; ep >= dp->b_addr; ep--)
					if(*ep)
						free(rp->i_dev, *ep);
					brelse(dp);
				}
        //返回给空闲队列
				free(rp->i_dev, *cp);
			}
      //释放块缓冲区
			brelse(bp);
		}
    //释放第一层
		free(rp->i_dev, *ip);
		*ip = 0;
	}
  //重置 inode 状态
	rp->i_mode =& ~ILARG;
	rp->i_size0 = 0;
	rp->i_size1 = 0;
	rp->i_flag =| IUPD;
}

/*
 * Make a new file.
 */
maknode(mode)
{
	register *ip;

  //分配 inode
	ip = ialloc(u.u_pdir->i_dev);
	if (ip==NULL)
		return(NULL);
  //设置更新和访问标志
	ip->i_flag =| IACC|IUPD;
  //创建模式
	ip->i_mode = mode|IALLOC;
  //引用为1
	ip->i_nlink = 1;
  //uid, gid 设置
	ip->i_uid = u.u_uid;
	ip->i_gid = u.u_gid;
  //写入目录项
	wdir(ip);
	return(ip);
}

/*
 * Write a directory entry with
 * parameters left as side effects
 * to a call to namei.
 * 往目录增加一个目录项
 */
wdir(ip)
int *ip;
{
	register char *cp1, *cp2;

  //设置 user 结构体
  // 当前目录编号设置为 inode 编号
	u.u_dent.u_ino = ip->i_number;
  // cp1 就是当前目录名
	cp1 = &u.u_dent.u_name[0];
  // 拷贝文件名到 cp1 后面
	for(cp2 = &u.u_dbuf[0]; cp2 < &u.u_dbuf[DIRSIZ];)
		*cp1++ = *cp2++;
  //设置 io 状态，地址、标志、数目等
	u.u_count = DIRSIZ+2;
	u.u_segflg = 1;
	u.u_base = &u.u_dent;
  //写入磁盘
	writei(u.u_pdir);
  //namei 会递增，因此这里要递减
	iput(u.u_pdir);
}
