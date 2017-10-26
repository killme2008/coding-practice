/*
 * Definition of the unix super block.
 * The root super block is allocated and
 * read in iinit/alloc.c. Subsequently
 * a super block is allocated and read
 * with each mount (smount/sys3.c) and
 * released with unmount (sumount/sys3.c).
 * A disk block is ripped off for storage.
 * See alloc.c for general alloc/free
 * routines for free list and I list.
 */
//块设备分为四个部分：启动区域、超级块、inode区域、存储区域
//filesys 就是超级块定义，包含了块设备的元信息
struct	filsys
{
	int	s_isize;	/* size in blocks of I list -- inode 区域块数*/
	int	s_fsize;	/* size in blocks of entire volume -- 存储区域块数*/
	int	s_nfree;	/* number of in core free blocks (0-100) -- 存储空闲的有效元素个数*/
	int	s_free[100];	/* in core free blocks  -- 存储空闲队列*/
	int	s_ninode;	/* number of in core I nodes (0-100) -- inode 区域空闲的有效元素个数*/
	int	s_inode[100];	/* in core free I nodes -- inode 空闲队列 */
	char	s_flock;	/* lock during free list manipulation -- 存储空闲队列的锁*/
	char	s_ilock;	/* lock during I list manipulation -- inode空闲队列锁*/
	char	s_fmod;		/* super block modified flag --更新标志*/
	char	s_ronly;	/* mounted read-only flag --是否为只读 */
	int	s_time[2];	/* current date of last update -- 当前时刻或者最后更新时刻 */
	int	pad[50] ; /*填充对齐*/
};
