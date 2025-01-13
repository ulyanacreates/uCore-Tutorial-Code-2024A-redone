## 问答作业
1. 实际情况是轮到 p1 执行吗？为什么？
执行时将stride加上pass值，那么下次切换时p1.stride = 255，p2.stride = 250 + 10 = 4 => 会选择调度进程p2，不是p1。
2. 为什么？尝试简单说明（传达思想即可，不要求严格证明）。
当优先级 >= 2，stride初始化位0，每次更新给stride值加上 <= BIG_STRIDE/2。那么按算法，每次选择最小的stride进程并加上pass得到的max值是最近更新的进程的stride或者之前更新的进程的stride => 保证了STRIDE_MAX – STRIDE_MIN <= BIG_STRIDE/2。
3. 

```
typedef unsigned long long Stride_t;
const Stride_t BIG_STRIDE = 0xffffffffffffffffULL;
int cmp(Stride_t a, Stride_t b) {
    // YOUR CODE HERE
    // return 1 if a > b
    // return -1 if a < b
    // return 0 if a == b
    if (a > b) {
        if (a - b > BIG_STRIDE/2) return -1;
        else return 1;
    }
    else if (a < b) {
        if (b - a > BIG_STRIDE/2) return 1;
		else return -1;
    }
	else return 0;
}
```