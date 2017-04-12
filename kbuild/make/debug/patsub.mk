obj-y	:= /home/yzc/linux-world/ /home/yzc/t1.o 
new-y	:= $(patsubst %/, %/built-in.o, $(obj-y))

all: FORCE
	@echo 'obj-y = $(obj-y)'
	@echo 'new-y = $(new-y)'

FORCE:

