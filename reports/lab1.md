## 实现
本次由于是重做，因此不讲在报告里面提供解释，而是在代码里面提出实现步骤，可以搜索 STEP 关键词来查看连续的操作。

## 问答作业
1. 正确进入 U 态后，程序的特征还应有：使用 S 态特权指令，访问 S 态寄存器后会报错。请同学们可以自行测试这些内容（参考 前三个测例 ，描述程序出错行为，同时注意注明你使用的 sbi 及其版本。

sbi版本是0.3.0-alpha.2
所参考的例子分别是ch2的三个bad case，其中有__ch2_bad_address.c、__ch2_bad_instruction.c和__ch2_bad_register.c测例。若单独运行这些测例，可以得到如下输出：
```
$ make test CHAPTER=2_bad
[ERROR 0]unknown trap: 0x0000000000000007, stval = 0x0000000000000000 sepc = 0x0000000080400002
[ERROR 0]IllegalInstruction in application, epc = 0x0000000080400002, core dumped.
[ERROR 0]IllegalInstruction in application, epc = 0x0000000080400002, core dumped.
```
第一个为bad address的测例，查看代码可以看到所输出的错误原因在于用户态下尝试访问内存为0的地址，而其不能访问到。
第二个 bad instruction测例中又尝试了在用户模式下使用S态指令sret，这个肯定是违法的。
最后，bad register测例也尝试使用了一个违法的指令，具体是在用户态下使用了S态指令cssr，用来访问sstatus。

2. 请结合用例理解 trampoline.S 中两个函数 userret 和 uservec 的作用，并回答如下几个问题:


    1. L79: 刚进入 userret 时，a0、a1 分别代表了什么值。
    答案：a0是中断发生时的U态trapframe地址，a1是用户页表的地址。

    2. L87-L88: sfence 指令有何作用？为什么要执行该指令，当前章节中，删掉该指令会导致错误吗？
    ```
    csrw satp, a1
    sfence.vma zero, zero
    ```
    sfence用于刷新TLB页表项，使用与进程切换导致TLB数据失去的刷新。由于当前没有使用虚拟内存抽象，因此现在删除这个功能也不会导致严重错误。
    3. L96-L125: 为何注释中说要除去 a0？哪一个地址代表 a0？现在 a0 的值存在何处？
    ```
    # restore all but a0 from TRAPFRAME
    ld ra, 40(a0)
    ld sp, 48(a0)
    ld t5, 272(a0)
    ld t6, 280(a0)
    ``` 
    uservec还需要用a0来存trapframe地址，因此说 all but。其中a0由112（a0）代表，而a0的值在sscratch处。

    4. userret：中发生状态切换在哪一条指令？为何执行之后会进入用户态？
    userret中发生状态切换在sret指令处。因为S态到U态的切换当中，会把pc设置到sepc并切换到trap地址或下一条执行指令，而这个就是要进入到用户态，回复执行。

    5. L29： 执行之后，a0 和 sscratch 中各是什么值，为什么？
    
    ```
    csrrw a0, sscratch, a0
    ```

    a0 == trapframe地址
    sscratch == a0值
    调用userret之前（进入U态），sscratch存的是trapframe地址，使用scrrw切换。

    6. L32-L61: 从 trapframe 第几项开始保存？为什么？是否从该项开始保存了所有的值，如果不是，为什么？
    ```
    sd ra, 40(a0)
    sd sp, 48(a0)
    ...
    sd t5, 272(a0)
    sd t6, 280(a0)
    ```
    由trapframe定义，且ra为第六页保存的，因此从trapframe的第六页开始保存。由于a0的值是sscratch负责保存，因此trapframe仅没有保存a0值。
    7. 进入 S 态是哪一条指令发生的？
    调用ecall会发生异常并切换到S态，处理CSR寄存器且跳转至uservec函数。

    8. L75-L76: ld t0, 16(a0) 执行之后，`t0`中的值是什么，解释该值的由来？
    ```
    ld t0, 16(a0)
    jr t0
    ```
    参考代码在于trap.c中：
    ```
    struct trapframe *trapframe = curr_proc()->trapframe;
    trapframe->kernel_satp = r_satp(); // kernel page table
    trapframe->kernel_sp =
    curr_proc()->kstack + PGSIZE; // process's kernel stack
    trapframe->kernel_trap = (uint64)usertrap;
    trapframe->kernel_hartid = r_tp(); // hartid for cpuid()
    ```
    由于16(a0)为kernel_trap的值，而这段代码争吵把kernel_trap赋值user_trap值，跳转到了trapframe->kernel_trap的地址，也就是t0，返回到U态了。