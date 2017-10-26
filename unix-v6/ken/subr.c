#
/*
 */

#include "../param.h"
#include "../conf.h"
#include "../inode.h"
#include "../user.h"
#include "../buf.h"
#include "../systm.h"

/*
 * Bmap defines the structure of file system storage
 * by returning the physical block number on a device given the
 * inode and the logical block number in a file.
 * When convenient, it also leaves the physical
 * block number of the next block of the file in rablock
 * for use in read-ahead.
 */
//将逻辑块编号映射为物理块编号
bmap(ip, bn)
struct inode *ip;
int bn;
{
	register *bp, *bap, nb;
	int *nbp, d, i;

	d = ip->i_dev;
  //超过范围，报错
	if(bn & ~077777) {
		u.u_error = EFBIG;
		return(0);
	}

  //直接参照
	if((ip->i_mode&ILARG) == 0) {

		/*
		 * small file algorithm
		 */

		if((bn & ~7) != 0) {

			/*
			 * convert small to large
			 */
      //文件超过，转化为间接参照
			if ((bp = alloc(d)) == NULL)
				return(NULL);
			bap = bp->b_addr;
			for(i=0; i<8; i++) {
        //拷贝到新分配的缓冲区
				*bap++ = ip->i_addr[i];
				ip->i_addr[i] = 0;
			}
      //设置间接引用
			ip->i_addr[0] = bp->b_blkno;
      //写入块
			bdwrite(bp);
      //设置间接引用标志
			ip->i_mode =| ILARG;
      //进入间接引用算法
			goto large;
		}
		nb = ip->i_addr[bn];
    //第一次分配
		if(nb == 0 && (bp = alloc(d)) != NULL) {
			bdwrite(bp);
			nb = bp->b_blkno;
			ip->i_addr[bn] = nb;
			ip->i_flag =| IUPD;
		}
    //用于预读
		rablock = 0;
		if (bn<7)
			rablock = ip->i_addr[bn+1];
		return(nb);
	}

	/*
	 * large file algorithm
   * 间接引用算法
	 */

    large:
	i = bn>>8;
  //还是过大，使用双重间接
	if(bn & 0174000)
		i = 7;
	if((nb=ip->i_addr[i]) == 0) {
    //不存在，创建
		ip->i_flag =| IUPD;
		if ((bp = alloc(d)) == NULL)
			return(NULL);
		ip->i_addr[i] = bp->b_blkno;
	} else
    //否则读取
		bp = bread(d, nb);
  //获得了间接存储块地址，但是可能是双重间接引用块的地址，下面继续做处理
	bap = bp->b_addr;

	/*
	 * "huge" fetch of double indirect block
   *　双重间接引用处理
	 */

	if(i == 7) {
    //计算第一级
		i = ((bn>>8) & 0377) - 7;
    //不存在就创建
		if((nb=bap[i]) == 0) {
			if((nbp = alloc(d)) == NULL) {
				brelse(bp);
				return(NULL);
			}
			bap[i] = nbp->b_blkno;
			bdwrite(bp);
		} else {
      //否则读取
			brelse(bp);
			nbp = bread(d, nb);
		}
		bp = nbp;
    //获得地址
		bap = bp->b_addr;
	}

	/*
	 * normal indirect fetch
	 */
  //正常的间接引用处理流程
	i = bn & 0377;
	if((nb=bap[i]) == 0 && (nbp = alloc(d)) != NULL) {
		nb = nbp->b_blkno;
		bap[i] = nb;
		bdwrite(nbp);
		bdwrite(bp);
	} else
		brelse(bp);
  //预读处理
	rablock = 0;
	if(i < 255)
		rablock = bap[i+1];
	return(nb);
}

/*
 * Pass back  c  to the user at his location u_base;
 * update u_base, u_count, and u_offset.  Return -1
 * on the last character of the user's read.
 * u_base is in the user address space unless u_segflg is set.
 */
passc(c)
char c;
{

	if(u.u_segflg)
		*u.u_base = c; else
		if(subyte(u.u_base, c) < 0) {
			u.u_error = EFAULT;
			return(-1);
		}
	u.u_count--;
	if(++u.u_offset[1] == 0)
		u.u_offset[0]++;
	u.u_base++;
	return(u.u_count == 0? -1: 0);
}

/*
 * Pick up and return the next character from the user's
 * write call at location u_base;
 * update u_base, u_count, and u_offset.  Return -1
 * when u_count is exhausted.  u_base is in the user's
 * address space unless u_segflg is set.
 */
cpass()
{
	register c;

	if(u.u_count == 0)
		return(-1);
	if(u.u_segflg)
		c = *u.u_base; else
		if((c=fubyte(u.u_base)) < 0) {
			u.u_error = EFAULT;
			return(-1);
		}
	u.u_count--;
	if(++u.u_offset[1] == 0)
		u.u_offset[0]++;
	u.u_base++;
	return(c&0377);
}

/*
 * Routine which sets a user error; placed in
 * illegal entries in the bdevsw and cdevsw tables.
 */
nodev()
{

	u.u_error = ENODEV;
}

/*
 * Null routine; placed in insignificant entries
 * in the bdevsw and cdevsw tables.
 */
nulldev()
{
}

/*
 * copy count words from from to to.
 */
bcopy(from, to, count)
int *from, *to;
{
	register *a, *b, c;

	a = from;
	b = to;
	c = count;
	do
		*b++ = *a++;
	while(--c);
}
