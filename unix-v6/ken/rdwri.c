#
/*
 */

#include "../param.h"
#include "../inode.h"
#include "../user.h"
#include "../buf.h"
#include "../conf.h"
#include "../systm.h"

/*
 * Read the file corresponding to
 * the inode pointed at by the argument.
 * The actual read arguments are found
 * in the variables:
 *	u_base		core address for destination
 *	u_offset	byte offset in file
 *	u_count		number of bytes to read
 *	u_segflg	read to kernel/user
 */
readi(aip)
struct inode *aip;
{
	int *bp;
	int lbn, bn, on;
	register dn, n;
	register struct inode *ip;

	ip = aip;
  //读取是0，直接返回
	if(u.u_count == 0)
		return;
  //更新访问的标志位
	ip->i_flag =| IACC;
  //字符设备，需要调用设备驱动
	if((ip->i_mode&IFMT) == IFCHR) {
		(*cdevsw[ip->i_addr[0].d_major].d_read)(ip->i_addr[0]);
		return;
	}
  //块设备有 bio 层
	do {
    //块的逻辑编号， 2**9 = 512
		lbn = bn = lshift(u.u_offset, -9);
    //偏移量
		on = u.u_offset[1] & 0777;
    //计算读取长度，避免超过一个块大小512
		n = min(512-on, u.u_count);
		if((ip->i_mode&IFMT) != IFBLK) {
      //非特殊文件，一般文件处理
      //不要超过文件长度
			dn = dpcmp(ip->i_size0&0377, ip->i_size1,
				u.u_offset[0], u.u_offset[1]);
			if(dn <= 0)
				return;
			n = min(n, dn);
      //转换为物理块编号
			if ((bn = bmap(ip, lbn)) == 0)
				return;
			dn = ip->i_dev;
		} else {
      //dn设置为块设备编号
			dn = ip->i_addr[0];
      //预读块
			rablock = bn+1;
		}
    //上一次读取的编号加上1是这次读取的，表示是连续读取，执行预读
		if (ip->i_lastr+1 == lbn)
			bp = breada(dn, bn, rablock);
		else
			bp = bread(dn, bn); //不预读
    //记住本次读取的编号
		ip->i_lastr = lbn;
    //从块缓冲区，复制到进程的虚拟地址
		iomove(bp, on, n, B_READ);
    //释放读取的缓冲区
		brelse(bp);
	} while(u.u_error==0 && u.u_count!=0); //循环处理下一个块
}

/*
 * Write the file corresponding to
 * the inode pointed at by the argument.
 * The actual write arguments are found
 * in the variables:
 *	u_base		core address for source
 *	u_offset	byte offset in file
 *	u_count		number of bytes to write
 *	u_segflg	write to kernel/user
 */
writei(aip)
struct inode *aip;
{
	int *bp;
	int n, on;
	register dn, bn;
	register struct inode *ip;

	ip = aip;
  //更新标志
	ip->i_flag =| IACC|IUPD;
  //字符设备，执行设备驱动函数，并返回
	if((ip->i_mode&IFMT) == IFCHR) {
		(*cdevsw[ip->i_addr[0].d_major].d_write)(ip->i_addr[0]);
		return;
	}
  //无需写入，返回
	if (u.u_count == 0)
		return;

	do {
    //计算逻辑块
		bn = lshift(u.u_offset, -9);
    //块内偏移量
		on = u.u_offset[1] & 0777;
    //写入长度
		n = min(512-on, u.u_count);
    //如果是一般文件
		if((ip->i_mode&IFMT) != IFBLK) {
      //换算成物理块编号
			if ((bn = bmap(ip, bn)) == 0)
				return;
      //dn 设定为设备编号
			dn = ip->i_dev;
		} else
      //特殊文件，直接设置为设备编号
			dn = ip->i_addr[0];
    //如果是 512 整块写入，直接获取缓冲区
		if(n == 512)
			bp = getblk(dn, bn); else
      //否则要先读取（对齐写入的重要性）
			bp = bread(dn, bn);
    //从虚拟地址拷贝到缓冲区
		iomove(bp, on, n, B_WRITE);
    //拷贝处理处理
		if(u.u_error != 0)
			brelse(bp); else
      //遇到块边界，异步方式立即写入
		if ((u.u_offset[1]&0777)==0)
			bawrite(bp); else
      //否则，延迟写入
			bdwrite(bp);
		if(dpcmp(ip->i_size0&0377, ip->i_size1,
		  u.u_offset[0], u.u_offset[1]) < 0 &&
		  (ip->i_mode&(IFBLK&IFCHR)) == 0) {
      //文件增大
			ip->i_size0 = u.u_offset[0];
			ip->i_size1 = u.u_offset[1];
		}
    //更新标志
		ip->i_flag =| IUPD;
	} while(u.u_error==0 && u.u_count!=0); //循环多次
}

/*
 * Return the logical maximum
 * of the 2 arguments.
 */
max(a, b)
char *a, *b;
{

	if(a > b)
		return(a);
	return(b);
}

/*
 * Return the logical minimum
 * of the 2 arguments.
 */
min(a, b)
char *a, *b;
{

	if(a < b)
		return(a);
	return(b);
}

/*
 * Move 'an' bytes at byte location
 * &bp->b_addr[o] to/from (flag) the
 * user/kernel (u.segflg) area starting at u.base.
 * Update all the arguments by the number
 * of bytes moved.
 *
 * There are 2 algorithms,
 * if source address, dest address and count
 * are all even in a user copy,
 * then the machine language copyin/copyout
 * is called.
 * If not, its done byte-by-byte with
 * cpass and passc.
 */
//用于虚拟空间和设备缓冲区之间拷贝数据，如果是在用户空间内拷贝，使用汇编编写的 copyin/copyout 函数拷贝以字(word)为单位
//否则，使用 cpass/passc 以字节为单位传输
//也是使用 user 传参:
/*
 *u.u_base 读写的虚拟地址，处理结束将递增实际的传输长度
 *u.u_offset 读写的偏移量，处理借宿也将递增实际的传输长度
 *u.u_count 读写的数据量（都是以字节为单位）
 *u.u_segflg  1 内核空间读写 0 用户空间读写
 */
iomove(bp, o, an, flag)
/*
 * bp 块设备缓冲区
 * o 缓冲区内偏移量
 * an 数据传输的长度（字节）
 * flag 读还是写的标志
 */
struct buf *bp;
{
	register char *cp;
	register int n, t;

	n = an;
	cp = bp->b_addr + o;
  //用户空间，并且以字为单位
	if(u.u_segflg==0 && ((n | cp | u.u_base)&01)==0) {
    //使用 copyin/copyout
		if (flag==B_WRITE)
			cp = copyin(u.u_base, cp, n);
		else
			cp = copyout(cp, u.u_base, n);
		if (cp) {
			u.u_error = EFAULT;
			return;
		}
    //递增
		u.u_base =+ n;
		dpadd(u.u_offset, n);
		u.u_count =- n;
		return;
	}
  //在内核空间，使用 cpass/passc
	if (flag==B_WRITE) {
		while(n--) {
			if ((t = cpass()) < 0)
				return;
			*cp++ = t;
		}
	} else
		while (n--)
			if(passc(*cp++) < 0)
				return;
}
