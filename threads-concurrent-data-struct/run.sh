#!/bin/bash

# 配置参数
TOTAL_LOOPS=100000000  # 总任务量：一亿次累加
MAX_THREADS=12        # 最大线程数
THRESHOLD=1024        # Sloppy Counter 的同步阈值

# 1. 编译所有目标
echo "正在编译程序..."
make clean > /dev/null
make all > /dev/null

if [ $? -ne 0 ]; then
    echo "编译失败，请检查 Makefile 和源代码。"
    exit 1
fi

echo "开始对比压测..."
echo "总任务量: $TOTAL_LOOPS | Sloppy 阈值: $THRESHOLD"
echo "-----------------------------------------------------------------------"
# 调整表头格式，增加 Sloppy 的对比列
printf "%-8s | %-12s | %-15s | %-15s\n" "线程数" "每程循环数" "Simple 耗时" "Sloppy 耗时"
echo "-----------------------------------------------------------------------"

# 2. 迭代测试
for (( t=1; t<=$MAX_THREADS; t++ ))
do
    LOOPS_PER_THREAD=$(( TOTAL_LOOPS / t ))
    
    # 测试 Simple Counter
    TIME_SIMPLE=$(./simple_counter $t $LOOPS_PER_THREAD | grep "Time (seconds):" | awk -F ":" '{print $2}')
    
    # 测试 Sloppy Counter (增加第三个参数 threshold)
    TIME_SLOPPY=$(./sloppy_counter $t $LOOPS_PER_THREAD $THRESHOLD | grep "Time (seconds):" | awk -F ":" '{print $2}')
    
    # 格式化输出
    printf "%-8d | %-12d | %-15s | %-15s\n" "$t" "$LOOPS_PER_THREAD" "$TIME_SIMPLE" "$TIME_SLOPPY"
done

echo "-----------------------------------------------------------------------"
make clean
echo "压测完成。"