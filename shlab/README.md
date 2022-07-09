## 任务和测试
完成 tsh.c 文件，以实现一个shell程序。

测试：
比较 make testXX 和 make rtestXX （参考程序） 运行结果

## 实现细节
### 编写安全、正确、可移植的信号处理（捕获）程序：

本lab需要实现SIGCHLD, SIGINT, SIGTSTP三个信号的捕获函数：
```
void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);
```

安全编程规则：

1）printf,sprintf, malloc, exit都不是异步信号安全的函数（不能被信号处理程序安全地调用），
在信号处理（捕获）函数内应该使用以下函数：
```
/* 在头文件 csapp.h */
/* Sio wrappers */
ssize_t Sio_puts(char s[]); //输出字符串
ssize_t Sio_putl(long v);   //输出一个long类型的数
void Sio_error(char s[]);   //打印一条错误信息并终止
```
2）加入处理程序会修改errno，则在进入处理程序时把errno保存在一个局部变量里，在返回时恢复它；

3）在访问共享的全局数据结构 （如jobs结构体数组） 时阻塞所有信号；

4）用volatile声明全局变量，确保每次从内存中访问变量；

5）对于需要在处理程序和主函数间共享的全局标志，用以下格式声明：
```
volatile sig_atomic_t flag;
```
以保证对它的读写操作是原子的；


### 全局变量：
```
extern char **environ;      /* defined in libc */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
int nextjid = 1;            /* next job ID to allocate */
char sbuf[MAXLINE];         /* for composing sprintf messages */

struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list (max 16) */
```

### 使用信号屏蔽同步流以避免并发错误：

当shell父进程执行一个内置命令时，需要fork子进程再执行对应命令，之后通过addjob函数将该job添加到全局jobs列表。

当子进程终止时，用户自定义的SIGCHLD处理函数循环回收僵尸子进程，并调用deletejob函数将Job从jobs列表删除。

但是如果调用 addjob 函数之前子进程已终止并调用了 deletejob 函数，addjob将错误地添加一个已终止的job。

解决方法：在父进程fork之前，调用Sigprocmask 屏蔽SIGCHLD 信号，addjob 后再解除屏蔽，可以确保同步并发流。

代码：
```
if(!builtin_cmd(argv)){ // 不是内置命令
        /* 
        父进程fork前阻塞SIGCHLD信号，addjob完成后再解开，防止调用addjob前子进程先被父进程回收了
        */
        Sigemptyset(&mask_one);
        Sigaddset(&mask_one, SIGCHLD);
        Sigfillset(&mask_all);
        Sigprocmask(SIG_BLOCK, &mask_one, &oldmask); 

        pid = fork();
        if (pid < 0)
            unix_error("fork error");
        else if (pid == 0){ // 子进程执行该job
            Sigprocmask(SIG_SETMASK, &oldmask, NULL); // 子进程继承了blocked位向量，需要unblock
            ...
        }
        Sigprocmask(SIG_BLOCK, &mask_all, NULL);  // 在访问共享的全局数据结构时阻塞所有信号
        int state = (bg)? 2:1;
        addjob(jobs, pid, state, buf); // 添加子进程到jobs列表
        Sigprocmask(SIG_SETMASK, &oldmask, NULL);
        ... 
    }
```

### shell创建前台作业时，显式等待作业终止
设置一个全部pid变量：
```
volatile sig_atomic_t global_pid;
```
SIGCHLD信号捕获函数（注意不能用while循环，因为waitpid()会暂时停止目前进程的执行, 直到有信号来到或子进程结束，这是之前出错的原因）：
```
void sigchld_handler(int sig) 
{
    ...
    if((global_pid = waitpid(-1, &status, 0)) > 0){  // 回收僵尸子进程，不能用while循环，因为waitpid会暂停进程
        /* 从jobs列表删除作业 */
    }
    ...
}
```
eval 函数：
```
...
        if (!bg){ // 父进程循环等待foreground job终止
            global_pid = 0;
            while( !global_pid )  // 循环等待SIGCHLD信号
                ;
            
        }
```
while循环体内语句选择：
1）不能使用空语句，因为循环会浪费处理器资源；

2）不能使用pause()函数，因为加入在while和 pause 之间收到SIGCHLD信号，pause会永远休眠；

3）使用sleep()函数，间隔太大时会导致程序运行缓慢，间隔太小又会浪费处理器资源；

解决方法：使用sigsuspend函数：
```
        if (!bg){ // 父进程循环等待foreground job终止
            Sigprocmask(SIG_BLOCK, &mask_one, NULL); // 先阻塞SIGCHLD信号
            global_pid = 0;
            while( !global_pid )
                /* sigsuspend 暂时使用oldmask代替当前阻塞集，并挂起进程节省资源 */
                Sigsuspend(&oldmask);              
            Sigprocmask(SIG_SETMASK, &oldmask, NULL);
        }
```

### SIGCHLD 信号捕捉函数
当有子进程终止或暂停时，会向父进程发送SIGCHLD 信号。修改收到信号后的行为：
```
void sigchld_handler(int sig) 
{
    int old_errno = errno;
    sigset_t mask_all, prev_all;
    int status;

    Sigfillset(&mask_all);
    /* 回收僵尸子进程，不能用while循环，因为waitpid会暂停进程 */
    if((global_pid = waitpid(-1, &status, WNOHANG|WUNTRACED)) > 0){  

        if (WIFEXITED(status)){ // 正常结束, 删除job
            Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);  // 在访问共享的全局数据结构时阻塞所有信号

            deletejob(jobs, global_pid);

            Sigprocmask(SIG_SETMASK, &prev_all, NULL);
        }
        if (WIFSIGNALED(status)){   // 因信号结束，打印信息，删除job
            Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);  // 在访问共享的全局数据结构时阻塞所有信号

            printf("Job [%d] (%d) terminated by signal %d\n", pid2jid(global_pid), global_pid, WTERMSIG(status));
            deletejob(jobs, global_pid);

            Sigprocmask(SIG_SETMASK, &prev_all, NULL);
        }
        if (WIFSTOPPED(status)){   // 因信号停止，打印信息，修改状态
            Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);  // 在访问共享的全局数据结构时阻塞所有信号

            printf("Job [%d] (%d) stopped by signal %d\n", pid2jid(global_pid), global_pid, WSTOPSIG(status));
            struct job_t *temp_job = getjobpid(jobs, global_pid);
            temp_job->state = 3;   // 状态修改为ST
            Sigprocmask(SIG_SETMASK, &prev_all, NULL);
        }

    }
        
    // if (errno != ECHILD)
    //     Sio_error("waitpid error");
    errno = old_errno;
    return;
}
```

### SIGINT 和 SIGTSTP 信号捕获函数
整体逻辑：

收到信号后进入sig_handler函数 -> 函数内将信号发送给foreground job进程组所有进程 -> 

foreground job进程终止或暂停，发出SIGCHLD 信号 -> SIGCHLD 信号捕捉函数 ->

函数内分情况打印信息，从jobs列表删除job 或改变job状态。
