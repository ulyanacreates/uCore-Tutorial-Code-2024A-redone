#include "syscall.h"
#include "proc.h"
#include "defs.h"
#include "loader.h"
#include "syscall_ids.h"
#include "timer.h"
#include "trap.h"
#include "proc.h"

uint64 sys_write(int fd, uint64 va, uint len)
{
	debugf("sys_write fd = %d va = %x, len = %d", fd, va, len);
	if (fd != STDOUT)
		return -1;
	struct proc *p = curr_proc();
	char str[MAX_STR_LEN];
	int size = copyinstr(p->pagetable, str, va, MIN(len, MAX_STR_LEN));
	debugf("size = %d", size);
	for (int i = 0; i < size; ++i) {
		console_putchar(str[i]);
	}
	return size;
}

__attribute__((noreturn)) void sys_exit(int code)
{
	exit(code);
	__builtin_unreachable();
}

uint64 sys_sched_yield()
{
	yield();
	return 0;
}

uint64 sys_gettimeofday(TimeVal *val, int _tz) // TODO: implement sys_gettimeofday in pagetable. (VA to PA)
{
	// YOUR CODE
	// val->sec = 0;
	// val->usec = 0;
	
	/* The code in `ch3` will leads to memory bugs*/
	// STEP1: update the implementation of sys_gettimeofday
	struct proc *p = curr_proc();
	TimeVal curr_val;
	uint64 cycle = get_cycle();
	curr_val.sec = cycle / CPU_FREQ;
	curr_val.usec = (cycle % CPU_FREQ) * 1000000 / CPU_FREQ;
	copyout(p->pagetable, (uint64)val, (char *)&curr_val, (uint64)sizeof(*val));
	// uint64 cycle = get_cycle();
	// val->sec = cycle / CPU_FREQ;
	// val->usec = (cycle % CPU_FREQ) * 1000000 / CPU_FREQ;
	return 0;
}

uint64 sys_sbrk(int n)
{
	uint64 addr;
        struct proc *p = curr_proc();
        addr = p->program_brk;
        if(growproc(n) < 0)
                return -1;
        return addr;	
}



// TODO: add support for mmap and munmap syscall.
// hint: read through docstrings in vm.c. Watching CH4 video may also help.
// Note the return value and PTE flags (especially U,X,W,R)
// STEP4: implement mmap syscall
int mmap(void* start, unsigned long long len, int port, int flag, int fd) {
	if (((uint64)start & (PAGE_SIZE- 1)) != 0) return -1;
	if (len > (1 << 30)) return -1;
	if (((port & ~0x7) != 0) || ((port & 0x7) == 0)) return -1;
	len = PGROUNDUP(len); 
	for (uint64 assign_va = (uint64)start; assign_va != (uint64)start + len; assign_va += PAGE_SIZE) {
		void *assigned_pa;
		if ((assigned_pa = kalloc()) == 0) return -1;
		int perm = (port << 1) | PTE_U;
		if (mappages(curr_proc()->pagetable, assign_va, PAGE_SIZE, (uint64)assigned_pa, perm) != 0) return -1;
	}
	return 0;
}

// STEP5: implement munmap syscall
int munmap(void* start, unsigned long long len) {
	if (((uint64)start & (PAGE_SIZE- 1)) != 0) return -1;
	if (len > (1 << 30)) return -1;
	len = PGROUNDUP(len);
	pagetable_t pagetable = curr_proc()->pagetable;
	for (uint64 assign_va = (uint64)start; assign_va != (uint64)start + len; assign_va += PAGE_SIZE) {
		if (walkaddr(pagetable, assign_va) == 0) return -1;
		uvmunmap(pagetable, assign_va, 1, 1);
	}
	return 0;
}


/*
* LAB1: you may need to define sys_task_info here
*/
// STEP2: update the implementation of sys_task_info
int sys_task_info(TaskInfo *ti) {
	struct proc *p = curr_proc();
	TaskInfo curr_task;
	curr_task.status = Running;
	memmove(curr_task.syscall_times, p->syscall_times, sizeof(curr_task.syscall_times));
	curr_task.time = (int)((get_cycle() - p->start_time)/(CPU_FREQ/1000));
	copyout(p->pagetable, (uint64)ti, (char *)&curr_task, (uint64)sizeof(*ti));
	return 0;
}

extern char trap_page[];

void syscall()
{
	struct trapframe *trapframe = curr_proc()->trapframe;
	int id = trapframe->a7, ret;
	uint64 args[6] = { trapframe->a0, trapframe->a1, trapframe->a2,
			   trapframe->a3, trapframe->a4, trapframe->a5 };
	tracef("syscall %d args = [%x, %x, %x, %x, %x, %x]", id, args[0],
	       args[1], args[2], args[3], args[4], args[5]);
	/*
	* LAB1: you may need to update syscall counter for task info here
	*/
	curr_proc()->syscall_times[id] += 1;
	switch (id) {
	case SYS_write:
		ret = sys_write(args[0], args[1], args[2]);
		break;
	case SYS_exit:
		sys_exit(args[0]);
		// __builtin_unreachable();
	case SYS_sched_yield:
		ret = sys_sched_yield();
		break;
	case SYS_gettimeofday:
		ret = sys_gettimeofday((TimeVal *)args[0], args[1]);
		break;
	case SYS_sbrk:
		ret = sys_sbrk(args[0]);
		break;
	/*
	* LAB1: you may need to add SYS_taskinfo case here
	*/
	case SYS_task_info:
		ret = sys_task_info((TaskInfo *)args[0]);
		break;
	// STEP3: add the case for mmap and munmap
	case SYS_mmap:
		ret = mmap((void *)args[0], (uint64)args[1], (int)args[2], (int)args[3], (int)args[4]);
		break;
	case SYS_munmap:
		ret = munmap((void *)args[0], (uint64)args[1]);
		break;
	default:
		ret = -1;
		errorf("unknown syscall %d", id);
	}
	trapframe->a0 = ret;
	tracef("syscall ret %d", ret);
}
