/*
 * Used to dissect integer device code
 * into major (driver designation) and
 * minor (driver parameter) parts.
 */
struct
{
	char	d_minor;
	char	d_major;
};

/*
 * Declaration of block device
 * switch. Each entry (row) is
 * the only link between the
 * main unix code and the driver.
 * The initialization of the
 * device switches is in the
 * file conf.c.
 */
struct	bdevsw
{
	int	(*d_open)(); //打开
	int	(*d_close)(); //关闭
	int	(*d_strategy)(); //执行
	int	*d_tab; //指向 buf.h 里的 devtab 结构体
} bdevsw[]; //块设备列表，关联代码和驱动

/*
 * Nblkdev is the number of entries
 * (rows) in the block switch. It is
 * set in binit/bio.c by making
 * a pass over the switch.
 * Used in bounds checking on major
 * device numbers.
 */
int	nblkdev; //统计块设备数量，在 bio.c 里的 binit 里设置。

/*
 * Character device switch.
 */
struct	cdevsw
{
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_read)();
	int	(*d_write)();
	int	(*d_sgtty)();
} cdevsw[]; //字符设备列表

/*
 * Number of character switch entries.
 * Set by cinit/tty.c
 */
int	nchrdev;
