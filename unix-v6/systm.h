/*
 * Random set of variables
 * used by more than one
 * routine.
 */
char	canonb[CANBSIZ];	/* buffer for erase and kill (#@) */
int	coremap[CMAPSIZ];	/* space for core allocation 内存*/
int	swapmap[SMAPSIZ];	/* space for swap allocation 交换空间*/
int	*rootdir;		/* pointer to inode of root directory */
int	cputype;		/* type of cpu =40, 45, or 70 */
int	execnt;			/* number of processes in exec */
int	lbolt;			/* time of day in 60th not in time */
//启动的时候，从根磁盘超级块中读取，见 ken/alloc.c
//在时钟中断处理中更新
int	time[2];		/* time in sec from 1970 */
int	tout[2];		/* time of day of next sleep */
/*
 * The callout structure is for
 * a routine arranging
 * to be called by the clock interrupt
 * (clock.c) with a specified argument,
 * in a specified amount of time.
 * Used, for example, to time tab
 * delays on teletypes.
 */
//时钟定期执行函数结构体
struct	callo
{
	int	c_time;		/* incaremental time */ //距离前一个元素的相对时间
	int	c_arg;		/* argument to routine */ //函数的参数
	int	(*c_func)();	/* routine */ //函数指针
} callout[NCALL]; //最多20个，param.h
/*
 * Mount structure.
 * One allocated on every mount.
 * Used to find the super block.
 */
struct	mount
{
	int	m_dev;		/* device mounted 挂载的块设备编号*/
	int	*m_bufp;	/* pointer to superblock 指向块设备的超级块，see filsys.h */
	int	*m_inodp;	/* pointer to mounted on inode，指向代表挂载点的 inode []　元素 */
} mount[NMOUNT];
int	mpid;			/* generic for unique process id's */
char	runin;			/* scheduling flag */
char	runout;			/* scheduling flag */
char	runrun;			/* scheduling flag */
char	curpri;			/* more scheduling */
int	maxmem;			/* actual max memory per process */
int	*lks;			/* pointer to clock device */
int	rootdev;		/* dev of root see conf.c */
int	swapdev;		/* dev of swap see conf.c */
int	swplo;			/* block number of swap space */
int	nswap;			/* size of swap space */
int	updlock;		/* lock for sync */
int	rablock;		/* block to be read ahead */
char	regloc[];		/* locs. of saved user registers (trap.c) */
