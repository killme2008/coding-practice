/*
 * One file structure is allocated
 * for each open/creat/pipe call.
 * Main use is to hold the read/write
 * pointer associated with each open
 * file.
 */
struct	file
{
	char	f_flag;
	char	f_count;	/* reference count ，引用计数*/
	int	f_inode;	/* pointer to inode structure 指向 inode*/
	char	*f_offset[2];	/* read/write character pointer，读和写的文件偏移量 */
} file[NFILE];

/* flags */
#define	FREAD	01
#define	FWRITE	02
#define	FPIPE	04
