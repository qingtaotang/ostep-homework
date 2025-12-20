# 编译程序
gcc -O2 -o tlb tlb.c -lrt

# 运行示例：测量访问16页，重复100000次
./tlb 16 100000

# 扫描不同页面数的情况（推荐脚本）
for pages in 1 2 4 8 16 32 64 128 256 512 1024 2048 4096; do
    echo -n "Pages: $pages -> "
    taskset -c 0 ./tlb $pages 100000 2>/dev/null | grep "Average time per page access"
done
