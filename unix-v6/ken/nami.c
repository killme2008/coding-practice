#
#include "../param.h"
#include "../inode.h"
#include "../user.h"
#include "../systm.h"
#include "../buf.h"

/*
 * Convert a pathname into a pointer to
 * an inode. Note that the inode is locked.
 *
 * func = function called to get next char of name
 *	&uchar if name is in user space
 *	&schar if name is in system space
 * flag = 0 if name is sought
 *	1 if name is to be created
 *	2 if name is to be deleted
 */
// func 用于获取下一个字符，如果是内核模式，是 schar 地址，否则是 uchar 地址
// flag : 0 用于查找， 1 创建, 2 删除
namei(func, flag)
int (*func)();
{
	register struct inode *dp;
	register c;
	register char *cp;
	int eo, *bp;

	/*
	 * If name starts with '/' start from
	 * root; otherwise start from current dir.
	 */
  //dp指向进程当前目录
	dp = u.u_cdir;
  // 从根目录开始，因为路径以 '/' 开始
	if((c=(*func)()) == '/')
		dp = rootdir;
  //获取目录的 inode，本质是执行加锁，确认文件系统挂载
	iget(dp->i_dev, dp->i_number);
  //忽略连续的 '/'
	while(c == '/')
		c = (*func)();
  //到底末尾，根目录不允许创建和删除
	if(c == '\0' && flag != 0) {
		u.u_error = ENOENT;
		goto out;
	}

cloop:
  //这里循环处理每个子目录 dp
	/*
	 * Here dp contains pointer
	 * to last component matched.
	 */

	if(u.u_error)
		goto out;
  //到达末尾， dp 指向了最终的 inode，返回
	if(c == '\0')
		return(dp);

	/*
	 * If there is another component,
	 * dp must be a directory and
	 * must have x permission.
	 */

  //确保 dp 是目录
	if((dp->i_mode&IFMT) != IFDIR) {
		u.u_error = ENOTDIR;
		goto out;
	}
  //确保有执行权限
	if(access(dp, IEXEC))
		goto out;

	/*
	 * Gather up name into
	 * users' dir buffer.
	 */

  //拷贝目录名到 u_dbuf
	cp = &u.u_dbuf[0];
	while(c!='/' && c!='\0' && u.u_error==0) {
		if(cp < &u.u_dbuf[DIRSIZ])
			*cp++ = c;
		c = (*func)();
	}
  //填空缓冲区末尾
	while(cp < &u.u_dbuf[DIRSIZ])
		*cp++ = '\0';
  //同样，忽略后续重复的 ‘/’
	while(c == '/')
		c = (*func)();
  //错误处理
	if(u.u_error)
		goto out;

	/*
	 * Set up to search a directory.
	 */
  //准备查找目录中匹配 u.u_dbuf 的项
	u.u_offset[1] = 0;
	u.u_offset[0] = 0;
	u.u_segflg = 1;
	eo = 0;
	u.u_count = ldiv(dp->i_size1, DIRSIZ+2);
	bp = NULL;

eloop:
  //进入查找目录中的子项循环
	/*
	 * If at the end of the directory,
	 * the search failed. Report what
	 * is appropriate as per flag.
	 */

  //搜索到目录项列表的末尾，没有找到
	if(u.u_count == 0) {
		if(bp != NULL)
			brelse(bp);
    //创建文件或者目录
		if(flag==1 && c=='\0') {
      //没有写权限，报错
			if(access(dp, IWRITE))
				goto out;
      //设定父目录
			u.u_pdir = dp;
      //eo表示是不是指向末尾
			if(eo)
				u.u_offset[1] = eo-DIRSIZ-2; //设定子项所在位置
      else
				dp->i_flag =| IUPD; //第一个子项
			return(NULL);
		}
    //查找或者删除，不存在都要报错
		u.u_error = ENOENT;
		goto out;
	}

	/*
	 * If offset is on a block boundary,
	 * read the next directory block.
	 * Release previous if it exists.
	 */

  //目录项指向了块边界，读下一个块。第一次查找一定会走这个逻辑，因为要读第一个块
	if((u.u_offset[1]&0777) == 0) {
		if(bp != NULL)
			brelse(bp);
		bp = bread(dp->i_dev,
			bmap(dp, ldiv(u.u_offset[1], 512)));
	}

	/*
	 * Note first empty directory slot
	 * in eo for possible creat.
	 * String compare the directory entry
	 * and the current component.
	 * If they do not match, go back to eloop.
	 */

  //复制一个目录项
	bcopy(bp->b_addr+(u.u_offset[1]&0777), &u.u_dent, (DIRSIZ+2)/2);
	u.u_offset[1] =+ DIRSIZ+2;
  //递减查找的总数
	u.u_count--;
  //没有记录（可能已经被删除，参见 unlink(sys4.c)），特别处理下
	if(u.u_dent.u_ino == 0) {
		if(eo == 0)
			eo = u.u_offset[1]; //eo指向空记录后的记录
		goto eloop;
	}
  //比较记录中的 u_name 和 u_dbuf
	for(cp = &u.u_dbuf[0]; cp < &u.u_dbuf[DIRSIZ]; cp++)
		if(*cp != cp[u.u_dent.u_name - u.u_dbuf])
      //如果名字不匹配，继续循环查找
			goto eloop;

	/*
	 * Here a component matched in a directory.
	 * If there is more pathname, go back to
	 * cloop, otherwise return.
	 */

	if(bp != NULL)
		brelse(bp);
  //查找到目录子项了
  //如果是删除
	if(flag==2 && c=='\0') {
    //没有写权限，错误
		if(access(dp, IWRITE))
			goto out;
    //返回的是要删除文件的父目录
		return(dp);
	}
	bp = dp->i_dev;
  //释放 inode 锁和引用
	iput(dp);
  //指向了下一个子路径
	dp = iget(bp, u.u_dent.u_ino);
	if(dp == NULL)
		return(NULL);
  //继续下一个（也许是子目录）的查找过程，或者找到终止
	goto cloop;

out:
	iput(dp); //确保释放
	return(NULL);
}

/*
 * Return the next character from the
 * kernel string pointed at by dirp.
 */
schar()
{
  //内核模式下，路径字符串直接在 u_dirp 里
	return(*u.u_dirp++ & 0377);
}

/*
 * Return the next character from the
 * user string pointed at by dirp.
 */
uchar()
{
	register c;
  //在用户空间处理
	c = fubyte(u.u_dirp++);
	if(c == -1)
		u.u_error = EFAULT;
	return(c);
}
