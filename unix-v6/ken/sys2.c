#
#include "../param.h"
#include "../systm.h"
#include "../user.h"
#include "../reg.h"
#include "../file.h"
#include "../inode.h"
//主要是 io 的系统调用，比如 creat,read,wirte,open etc.

/*
 * read system call
 */
read()
{
  //读写系统调用都是委托给 rdwr，传入不同mode
	rdwr(FREAD);
}

/*
 * write system call
 */
write()
{
	rdwr(FWRITE);
}

/*
 * common code for read and write calls:
 * check permissions, set base, count, and offset,
 * and switch out to readi, writei, or pipe code.
 * r0 文件描述符
 * u_arg[0] 读写数据的虚拟地址，字节为单位
 * u_arg[1] 数据的传输量，读写的长度，字节为单位
 */
rdwr(mode)
{
	register *fp, m;

	m = mode;
  //得到 file struct
	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
  //确认权限
	if((fp->f_flag&m) == 0) {
		u.u_error = EBADF;
		return;
	}
  //设定 io 参数，读取的地址、长度、标志等
  // user 结构体扮演着类似 thread local 的概念，隐式传参
	u.u_base = u.u_arg[0];
	u.u_count = u.u_arg[1];
	u.u_segflg = 0; //1 内核空间， 0 为用户空间
  //特别处理管道
	if(fp->f_flag&FPIPE) {
		if(m==FREAD)
			readp(fp); else
			writep(fp);
	} else {
    //将 fp 偏移量也设置到 user
		u.u_offset[1] = fp->f_offset[1];
		u.u_offset[0] = fp->f_offset[0];
    //执行 readi 或者 writei
		if(m==FREAD)
			readi(fp->f_inode); else
			writei(fp->f_inode);
    //增加 file struct 的 offset
		dpadd(fp->f_offset, u.u_arg[1]-u.u_count);
	}
  //返回实际的读写长度
	u.u_ar0[R0] = u.u_arg[1]-u.u_count;
}

/*
 * open system call
 */
open()
{
	register *ip;
	extern uchar;

  //转成 ip
	ip = namei(&uchar, 0);
  //不存在，返回
	if(ip == NULL)
		return;
  //因为 open mode 的 0 为读取，1 为写入，而 file 结构体则是1 为读取，2为写入，因此这里要递增下 mode
	u.u_arg[1]++;
	open1(ip, u.u_arg[1], 0);
}

/*
 * creat system call
 * u_arg[0] 路径
 * u_arg[1] 和 inode.i_mode 对应的打开模式
 */
creat()
{
	register *ip;
	extern uchar;

  //转成 inode
	ip = namei(&uchar, 1);
	if(ip == NULL) {
		if(u.u_error)
			return;
    //创建，清除高位 bit，无法创建目录和设置 sticky bit
		ip = maknode(u.u_arg[1]&07777&(~ISVTX));
		if (ip==NULL)
			return;
    //再打开
		open1(ip, FWRITE, 2);
	} else
    //已经存在，打开
		open1(ip, FWRITE, 1);
}

/*
 * common code for open and creat.
 * Check permissions, allocate an open file structure,
 * and call the device open routine if any.
 */
//trf: 0 打开已经存在的文件; 1 准备生成新的文件，切该文件已经存在 ;2 准备生成新的文件，且该文件不存在
open1(ip, mode, trf)
int *ip;
{
	register struct file *fp;
	register *rip, m;
	int i;

	rip = ip;
	m = mode;
	if(trf != 2) {
		if(m&FREAD)
      //确保有读权限
			access(rip, IREAD);
		if(m&FWRITE) {
      //确保有写权限
			access(rip, IWRITE);
      //不能写目录
			if((rip->i_mode&IFMT) == IFDIR)
				u.u_error = EISDIR;
		}
	}
	if(u.u_error)
		goto out;
  //1,2生成新文件，清空原来的 inode
	if(trf)
		itrunc(rip);
  //释放 inode 锁，因为 iget 或者 itrunc 都会加锁
	prele(rip);
  //分配 file struct
	if ((fp = falloc()) == NULL)
		goto out;
  //设置 flag
	fp->f_flag = m&(FREAD|FWRITE);
  //设置 inode
	fp->f_inode = rip;
  //暂存文件句柄 i，其实文件句柄就是 file struct 在 file[] 数组的下标
	i = u.u_ar0[R0];
  //特殊设备处理，比如字符设备
	openi(rip, m&FWRITE);
  //openi 成功，返回
	if(u.u_error == 0)
		return;
  //释放文件句柄
	u.u_ofile[i] = NULL;
  //递减文件参照计数
	fp->f_count--;

 out:
  //递减 inode 的参照计数
	iput(rip);
}

/*
 * close system call
 */
close()
{
	register *fp;

	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
	u.u_ofile[u.u_ar0[R0]] = NULL;
	closef(fp);
}

/*
 * seek system call
 */
seek()
{
	int n[2];
	register *fp, t;

	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
	if(fp->f_flag&FPIPE) {
		u.u_error = ESPIPE;
		return;
	}
	t = u.u_arg[1];
	if(t > 2) {
		n[1] = u.u_arg[0]<<9;
		n[0] = u.u_arg[0]>>7;
		if(t == 3)
			n[0] =& 0777;
	} else {
		n[1] = u.u_arg[0];
		n[0] = 0;
		if(t!=0 && n[1]<0)
			n[0] = -1;
	}
	switch(t) {

	case 1:
	case 4:
		n[0] =+ fp->f_offset[0];
		dpadd(n, fp->f_offset[1]);
		break;

	default:
		n[0] =+ fp->f_inode->i_size0&0377;
		dpadd(n, fp->f_inode->i_size1);

	case 0:
	case 3:
		;
	}
	fp->f_offset[1] = n[1];
	fp->f_offset[0] = n[0];
}

/*
 * link system call
 */
link()
{
	register *ip, *xp;
	extern uchar;

	ip = namei(&uchar, 0);
	if(ip == NULL)
		return;
	if(ip->i_nlink >= 127) {
		u.u_error = EMLINK;
		goto out;
	}
	if((ip->i_mode&IFMT)==IFDIR && !suser())
		goto out;
	/*
	 * unlock to avoid possibly hanging the namei
	 */
	ip->i_flag =& ~ILOCK;
	u.u_dirp = u.u_arg[1];
	xp = namei(&uchar, 1);
	if(xp != NULL) {
		u.u_error = EEXIST;
		iput(xp);
	}
	if(u.u_error)
		goto out;
	if(u.u_pdir->i_dev != ip->i_dev) {
		iput(u.u_pdir);
		u.u_error = EXDEV;
		goto out;
	}
	wdir(ip);
	ip->i_nlink++;
	ip->i_flag =| IUPD;

out:
	iput(ip);
}

/*
 * mknod system call
 */
mknod()
{
	register *ip;
	extern uchar;

	if(suser()) {
		ip = namei(&uchar, 1);
		if(ip != NULL) {
			u.u_error = EEXIST;
			goto out;
		}
	}
	if(u.u_error)
		return;
	ip = maknode(u.u_arg[1]);
	if (ip==NULL)
		return;
	ip->i_addr[0] = u.u_arg[2];

out:
	iput(ip);
}

/*
 * sleep system call
 * not to be confused with the sleep internal routine.
 */
sslep()
{
	char *d[2];

	spl7();
	d[0] = time[0];
	d[1] = time[1];
	dpadd(d, u.u_ar0[R0]);

  //while 检测时间是否到，没有到就继续睡眠
	while(dpcmp(d[0], d[1], time[0], time[1]) > 0) {
		if(dpcmp(tout[0], tout[1], time[0], time[1]) <= 0 ||
		   dpcmp(tout[0], tout[1], d[0], d[1]) > 0) {
			tout[0] = d[0];
			tout[1] = d[1];
		}
		sleep(tout, PSLEP);
	}
	spl0();
}
