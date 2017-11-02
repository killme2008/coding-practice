#
/*
 */

#include "../param.h"
#include "../user.h"
#include "../filsys.h"
#include "../file.h"
#include "../conf.h"
#include "../inode.h"
#include "../reg.h"

//文件相关系统调用的底层函数
/*
 * Convert a user supplied
 * file descriptor into a pointer
 * to a file structure.
 * Only task is to check range
 * of the descriptor.
 */
//文件句柄转成实际的file struct
getf(f)
{
	register *fp, rf;

	rf = f;
  //范围检查
	if(rf<0 || rf>=NOFILE)
		goto bad;
  //直接取数组元素了
	fp = u.u_ofile[rf];
	if(fp != NULL)
		return(fp);
bad:
	u.u_error = EBADF;
	return(NULL);
}

/*
 * Internal form of close.
 * Decrement reference count on
 * file structure and call closei
 * on last closef.
 * Also make sure the pipe protocol
 * does not constipate.
 */
closef(fp)
int *fp;
{
	register *rfp, *ip;

	rfp = fp;
  //管道特别处理
	if(rfp->f_flag&FPIPE) {
		ip = rfp->f_inode;
		ip->i_mode =& ~(IREAD|IWRITE);
		wakeup(ip+1);
		wakeup(ip+2);
	}
  //递减引用，如果最后一个，关闭 inode
	if(rfp->f_count <= 1)
		closei(rfp->f_inode, rfp->f_flag&FWRITE);
	rfp->f_count--;
}

/*
 * Decrement reference count on an
 * inode due to the removal of a
 * referencing file structure.
 * On the last closei, switchout
 * to the close entry point of special
 * device handler.
 * Note that the handler gets called
 * on every open and only on the last
 * close.
 */
closei(ip, rw)
int *ip;
{
	register *rip;
	register dev, maj;

	rip = ip;
	dev = rip->i_addr[0];
	maj = rip->i_addr[0].d_major;
  //最后一个引用
	if(rip->i_count <= 1)
    //特殊文件处理，调用设备驱动关闭函数
	switch(rip->i_mode&IFMT) {

	case IFCHR:
		(*cdevsw[maj].d_close)(dev, rw);
		break;

	case IFBLK:
		(*bdevsw[maj].d_close)(dev, rw);
	}
  //递减 inode 计数
	iput(rip);
}

/*
 * openi called to allow handler
 * of special files to initialize and
 * validate before actual IO.
 * Called on all sorts of opens
 * and also on mount.
 */
openi(ip, rw)
int *ip;
{
	register *rip;
	register dev, maj;

	rip = ip;
  //设备编号
	dev = rip->i_addr[0];
  //获取设备的大编号
	maj = rip->i_addr[0].d_major;
  //特殊文件处理，打开设备
	switch(rip->i_mode&IFMT) {
    //字符设备
	case IFCHR:
		if(maj >= nchrdev)
			goto bad;
    //执行打开
		(*cdevsw[maj].d_open)(dev, rw);
		break;

	case IFBLK:
    //块设备
		if(maj >= nblkdev)
			goto bad;
    //执行打开
		(*bdevsw[maj].d_open)(dev, rw);
	}
	return;

bad:
	u.u_error = ENXIO;
}

/*
 * Check mode permission on inode pointer.
 * Mode is READ, WRITE or EXEC.
 * In the case of WRITE, the
 * read-only status of the file
 * system is checked.
 * Also in WRITE, prototype text
 * segments cannot be written.
 * The mode is shifted to select
 * the owner/group/other fields.
 * The super user is granted all
 * permissions except for EXEC where
 * at least one of the EXEC bits must
 * be on.
 */
//注释很清楚了，不啰嗦
access(aip, mode)
int *aip;
{
	register *ip, m;

	ip = aip;
	m = mode;
	if(m == IWRITE) {
    //写权限，检查是否挂载为 readonly
		if(getfs(ip->i_dev)->s_ronly != 0) {
			u.u_error = EROFS;
			return(1);
		}
    //代码段，也不能写
		if(ip->i_flag & ITEXT) {
			u.u_error = ETXTBSY;
			return(1);
		}
	}
  //超级用户 root
	if(u.u_uid == 0) {
    //读和写无条件赋予，但是执行权限要检查下
    //至少有一个执行权限位是设置为1
		if(m == IEXEC && (ip->i_mode &
			(IEXEC | (IEXEC>>3) | (IEXEC>>6))) == 0)
				goto bad;
		return(0);
	}
  //检查用户权限
	if(u.u_uid != ip->i_uid) {
		m =>> 3;
    //检查用户组权限
		if(u.u_gid != ip->i_gid)
			m =>> 3;
	}
  //有权限，返回0
	if((ip->i_mode&m) != 0)
		return(0);

bad:
	u.u_error = EACCES;
  //无权限，返回1，并设置 error
	return(1);
}

/*
 * Look up a pathname and test if
 * the resultant inode is owned by the
 * current user.
 * If not, try for super-user.
 * If permission is granted,
 * return inode pointer.
 */
owner()
{
	register struct inode *ip;
	extern uchar();

	if ((ip = namei(uchar, 0)) == NULL)
		return(NULL);
	if(u.u_uid == ip->i_uid)
		return(ip);
	if (suser())
		return(ip);
	iput(ip);
	return(NULL);
}

/*
 * Test if the current user is the
 * super user.
 */
suser()
{

  //０　是　超级用户
	if(u.u_uid == 0)
    //返回1
		return(1);
	u.u_error = EPERM;
  //其他返回 0
	return(0);
}

/*
 * Allocate a user file descriptor.
 */
ufalloc()
{
	register i;

	for (i=0; i<NOFILE; i++)
    //查找 u_ofile 的一个空闲位置
		if (u.u_ofile[i] == NULL) {
			u.u_ar0[R0] = i;
			return(i);
		}
  //打开文件太多了
	u.u_error = EMFILE;
	return(-1);
}

/*
 * Allocate a user file descriptor
 * and a file structure.
 * Initialize the descriptor
 * to point at the file structure.
 *
 * no file -- if there are no available
 * 	file structures.
 */
falloc()
{
	register struct file *fp;
	register i;

  //分配句柄，也就是 o_file 数组中的位置，失败就直接返回了
	if ((i = ufalloc()) < 0)
		return(NULL);
  //遍历 file 数组，查找一个空闲位置
	for (fp = &file[0]; fp < &file[NFILE]; fp++)
		if (fp->f_count==0) {
			u.u_ofile[i] = fp;
      //递增计数
			fp->f_count++;
      //设定偏移量
			fp->f_offset[0] = 0;
			fp->f_offset[1] = 0;
			return(fp);
		}
  //没有空闲空间了，报错 no file
	printf("no file\n");
	u.u_error = ENFILE;
	return(NULL);
}
