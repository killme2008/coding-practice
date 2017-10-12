#
#include "../param.h"
#include "../systm.h"
#include "../user.h"
#include "../proc.h"
#include "../reg.h"
#include "../seg.h"

#define	EBIT	1		/* user error bit in PS: C-bit */
#define	UMODE	0170000		/* user-mode bits in PS word */
#define	SETD	0170011		/* SETD instruction */
#define	SYS	0104400		/* sys (trap) instruction */
#define	USER	020		/* user-mode flag added to dev */

/*
 * structure of the system entry table (sysent.c)
 */
struct sysent	{
	int	count;		/* argument count */ //参数数量
	int	(*call)();	/* name of handler 系统调用处理函数的地址*/
} sysent[64];

/*
 * Offsets of the user's registers relative to
 * the saved r0. See reg.h
 */
char	regloc[9]
{
	R0, R1, R2, R3, R4, R5, R6, R7, RPS
};

/*
 * Called from l40.s or l45.s when a processor trap occurs.
 * The arguments are the words saved on the system stack
 * by the hardware and software during the trap processing.
 * Their order is dictated by the hardware and the details
 * of C's calling sequence. They are peculiar in that
 * this call is not 'by value' and changed user registers
 * get copied back on return.
 * dev is the kind of trap that occurred.
 */
trap(dev, sp, r1, nps, r0, pc, ps)
{
	register i, a;
	register struct sysent *callp;

	savfp();
	if ((ps&UMODE) == UMODE)
		dev =| USER; //用户模式，设置 dev user 标志
	u.u_ar0 = &r0; // 将 r0 地址存入 u.u_ar0，通过 u.u_ar0[Rn] 可以访问陷入进程的 r{n} 寄存器
  //根据陷入种类做派发处理
	switch(dev) {

	/*
	 * Trap not expected.
	 * Usually a kernel mode bus error.
	 * The numbers printed are used to
	 * find the hardware PS/PC as follows.
	 * (all numbers in octal 18 bits)
	 *	address_of_saved_ps =
	 *		(ka6*0100) + aps - 0140000;
	 *	address_of_saved_pc =
	 *		address_of_saved_ps - 2;
	 */
	default:
		printf("ka6 = %o\n", *ka6);
		printf("aps = %o\n", &ps);
		printf("trap type %o\n", dev);
		panic("trap");

	case 0+USER: /* bus error */
		i = SIGBUS;
		break;

	/*
	 * If illegal instructions are not
	 * being caught and the offending instruction
	 * is a SETD, the trap is ignored.
	 * This is because C produces a SETD at
	 * the beginning of every program which
	 * will trap on CPUs without 11/45 FPU.
	 */
	case 1+USER: /* illegal instruction */
		if(fuiword(pc-2) == SETD && u.u_signal[SIGINS] == 0)
			goto out;
		i = SIGINS;
		break;

	case 2+USER: /* bpt or trace */
		i = SIGTRC;
		break;

	case 3+USER: /* iot */
		i = SIGIOT;
		break;

	case 5+USER: /* emt */
		i = SIGEMT;
		break;
    //系统调用
	case 6+USER: /* sys call */
    //重置 u_error
		u.u_error = 0;
    //清除错误位
		ps =& ~EBIT;
    //获取系统调用种类
		callp = &sysent[fuiword(pc-2)&077];
    //如果是间接系统调用，也就是 sysent[0]
		if (callp == sysent) { /* indirect */
			a = fuiword(pc);
			pc =+ 2;
			i = fuword(a);
			if ((i & ~077) != SYS) //如果不是系统调用
				i = 077;	/* illegal */ //设置为 nosys
			callp = &sysent[i&077]; //实际的系统调用
      //设置系统调用参数
			for(i=0; i<callp->count; i++)
				u.u_arg[i] = fuword(a =+ 2);
		} else {
      //处理直接系统调用
			for(i=0; i<callp->count; i++) {
				u.u_arg[i] = fuiword(pc);
				pc =+ 2;
			}
		}
    //设置文件路径名
		u.u_dirp = u.u_arg[0];
    //执行系统调用
		trap1(callp->call);
    //如果系统调用被信号中断，返回 EINTR 错误
		if(u.u_intflg)
			u.u_error = EINTR;
    //发生错误
		if(u.u_error < 100) {
			if(u.u_error) {
        //设置 PSW[0]
				ps =| EBIT;
				r0 = u.u_error;
			}
			goto out;
		}
		i = SIGSYS;
		break;

	/*
	 * Since the floating exception is an
	 * imprecise trap, a user generated
	 * trap may actually come from kernel
	 * mode. In this case, a signal is sent
	 * to the current process to be picked
	 * up later.
	 */
	case 8: /* floating exception */

    //通知给用户进程
		psignal(u.u_procp, SIGFPT);
		return;

	case 8+USER:
    //用户模式，直接跟总线错误处理一样
		i = SIGFPT;
		break;

	/*
	 * If the user SP is below the stack segment,
	 * grow the stack automatically.
	 * This relies on the ability of the hardware
	 * to restart a half executed instruction.
	 * On the 11/40 this is not the case and
	 * the routine backup/l40.s may fail.
	 * The classic example is on the instruction
	 *	cmp	-(sp),-(sp)
	 */
	case 9+USER: /* segmentation exception */
    //扩展栈区域
		a = sp;
    //先保存
		if(backup(u.u_ar0) == 0)
		if(grow(a))
			goto out;
		i = SIGSEG;
		break;
	}
	psignal(u.u_procp, i);

out:
  //信号处理
	if(issig())
		psig();
  //重新计算进程优先级
	setpri(u.u_procp);
}

/*
 * Call the system-entry routine f (out of the
 * sysent table). This is a subroutine for trap, and
 * not in-line, because if a signal occurs
 * during processing, an (abnormal) return is simulated from
 * the last caller to savu(qsav); if this took place
 * inside of trap, it wouldn't have a chance to clean up.
 *
 * If this occurs, the return takes place without
 * clearing u_intflg; if it's still set, trap
 * marks an error which means that a system
 * call (like read on a typewriter) got interrupted
 * by a signal.
 */
trap1(f)
int (*f)();
{

	u.u_intflg = 1; //设置中断标志
	savu(u.u_qsav); //保存寄存器，如果被信号中断，会从 trap1 返回，并且 u_intflg 仍然为1，表示系统调用被中断
	(*f)(); //执行系统调用
	u.u_intflg = 0; //清除中断标志
}

/*
 * nonexistent system call-- set fatal error code.
 */
nosys()
{
	u.u_error = 100;
}

/*
 * Ignored system call
 */
nullsys()
{
}
