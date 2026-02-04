CC = gcc
CFLAGS = -Wall -O2 -pthread

# 只需要在这里增加新程序的名字
TARGETS = simple_counter timer_test sloppy_counter

all: $(TARGETS)

# 模式规则：% 匹配任何字符串
# 这条规则表示：任何名为 X 的目标都可以由 X.c 编译生成
%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean