/*
 * Inode structure as it appears on
 * the disk. Not used by the system,
 * but by things like check, df, dump.
 */
//块设备里的 inode 结构,32个字节，一个块　５１２　可以包含　１６　个 inode
struct	inode
{
	int	i_mode; //状态，控制信息，低１１位表示权限
	char	i_nlink;　//来自目录的参照爽
	char	i_uid;
	char	i_gid;
	char	i_size0; //文件长度的高８位
	char	*i_size1; //文件长度低２４位
	int	i_addr[8];  //使用的存储区域的块编号，指向存储区域
	int	i_atime[2]; //参照时间
	int	i_mtime[2]; //更新时间
};

/* modes */
#define	IALLOC	0100000 //已分配
#define	IFMT	060000　//调查格式的时候使用
#define		IFDIR	040000　//目录
#define		IFCHR	020000　//字符特殊文件
#define		IFBLK	060000 //块特殊文件
#define	ILARG	010000 //文件长度较大，使用间接参照
#define	ISUID	04000　//suid位
#define	ISGID	02000 //sgid位
#define ISVTX	01000 //sticky位
#define	IREAD	0400 //读取权限
#define	IWRITE	0200　//写入权限
#define	IEXEC	0100　//执行权限
