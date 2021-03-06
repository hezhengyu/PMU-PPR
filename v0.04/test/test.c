//#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include "matrix_multiply.h"

#include "perf_event.h"

#define PMU_PERF_TYPE PERF_TYPE_HARDWARE

__u64 PMU_COUNETERS_LIST[2] = {
    PERF_COUNT_HW_CPU_CYCLES,
    PERF_COUNT_HW_INSTRUCTIONS
};

#ifndef __NR_perf_event_open
#if defined(__i386__)
#define __NR_perf_event_open    336
#elif defined(__x86_64__)
#define __NR_perf_event_open    298
#elif defined __powerpc__
#define __NR_perf_event_open    319
#elif defined __arm__
#define __NR_perf_event_open    364
#endif
#endif

int perf_event_open(struct perf_event_attr *hw_event_uptr,
		    pid_t pid, int cpu, int group_fd, unsigned long flags)
{

    return syscall(__NR_perf_event_open, hw_event_uptr, pid, cpu,
		   group_fd, flags);
}

#define __NR_pmu_ppr_open	     	351

#define pmu_ppr_open(x, y, z) syscall(__NR_pmu_ppr_open, x, y, z)


int main(int argc, char **argv)
{


    struct perf_event_attr pe;

    int i, group_fd = -1, fd[2];

    for (i = 0; i < 2; i++) {

	memset(&pe, 0, sizeof(struct perf_event_attr));
	pe.type = PMU_PERF_TYPE;
	pe.size = sizeof(struct perf_event_attr);
	pe.config = PMU_COUNETERS_LIST[i];
	pe.disabled = 1;
	pe.exclude_kernel = 1;
	pe.exclude_hv = 1;


	fd[i] = perf_event_open(&pe, 0, -1, group_fd, 0);
	if (fd < 0) {
	    fprintf(stderr, "Error opening %llu\n",
		    (long long unsigned) pe.config);
	    exit(1);
	}

	ioctl(fd[i], PERF_EVENT_IOC_ENABLE, 0);

	if (group_fd == -1)
	    group_fd = fd[i];

    }


    //pmu_ppr_open(fd[0], fd[1], 2000);
    printf("syscall %d\n", __NR_pmu_ppr_open);

    naive_matrix_multiply(1);
    sleep(1);


    __u64 counts[2];
    read(fd[0], &counts[0], sizeof(__u64));
    read(fd[1], &counts[1], sizeof(__u64));
    printf("%ld %ld\n", counts[0], counts[1]);

/*
	printf("char %ld\n", sizeof(char) );
	printf("int %ld\n", sizeof(int) );
	printf("long %ld\n", sizeof(long) );
	printf("long long %ld\n", sizeof(long long) );
	printf("long int %ld\n", sizeof(long int) );
	printf("char * %ld\n", sizeof(char *) );
	printf("int * %ld\n", sizeof(int *) );
	printf("long * %ld\n", sizeof(long *) );
	printf("long long * %ld\n", sizeof(long long*) );
	printf("long int * %ld\n", sizeof(long int*) );
	printf("void * %ld\n", sizeof(void *) );
*/
    return 0;
}
