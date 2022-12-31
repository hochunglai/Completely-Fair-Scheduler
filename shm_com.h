struct task_struct{
    int time_slice;
};

struct process_info {
    char process_type[128];
    int expected_execution_time;
    int pid;
    int sp;
    int dp;
    int remain_time;
    struct task_struct time_slice;
    int accu_time_slice;
    int last_cpu;
    int first_pri;
};

struct queue{
    int count;
    struct process_info processes[5];
};


