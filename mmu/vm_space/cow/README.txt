

https://zhuanlan.zhihu.com/p/27604276

This article will trace the dirty cow issue.


1) how to reproduce?


2) what is the role of '/proc/self/mem'?

I think /proc/self points  to the task.


    int f=open("/proc/self/mem",O_RDWR);


The corresponding handling will be done by mem_open().

Through this file, the thread can access the address space mapped by the master
thread/task :

        lseek(f,(uintptr_t) map,SEEK_SET);
        c+=write(f,str,strlen(str));

The 'map' is the virtual address mapped by the master thread :

	map=mmap(NULL,st.st_size,PROT_READ,MAP_PRIVATE,f,0);

Another thread will hint the kernel to free all the pages for the mapping:

	c+=madvise(map,100,MADV_DONTNEED);

This will lead to page fault when 'write(f, str, strlen(str))'.




