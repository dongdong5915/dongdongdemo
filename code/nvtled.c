/*
 *  nvt_led.c
 *
 *  Author:	Alvin lin
 *  Created:	May 4, 2014
 *  Copyright:	Novatek Inc.
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

#define SYS_DEV_MEM_PATH 	"/dev/mem"
#define NVT_LED_PHY_BASE 	0xFC040000
#define NVT_LED_PHY_SIZE 	0x1000
#define NVT_LED_MUX_OFS 	0x438
#define NVT_LED_DIR_OFS 	0x42C
#define NVT_LED_CTL_OFS 	0x420
#define NVT_LED_OPS_SFT 	(1 << 6)

#define __REG(x) (*((volatile unsigned int *) (x)))

int main(int argc, char* argv[])
{
	int ret = 0;
	int fd_mem = -1;
	void *pvaddr = NULL;
	int i,toggle = 0;

	fd_mem = open(SYS_DEV_MEM_PATH, O_RDWR);

	if(fd_mem < 0) {
		printf("%s open mem fail (%s)\n",argv[0], strerror(errno));
		goto out;
	}

	pvaddr = mmap(NULL, NVT_LED_PHY_SIZE, (PROT_READ | PROT_WRITE), MAP_SHARED, fd_mem, NVT_LED_PHY_BASE);

	if (pvaddr < 0) {
		printf("mmap fail ! (%s)\n",strerror(errno));
		goto out;
	}

	//set up gpf 6 as gpio output
	__REG((pvaddr + NVT_LED_MUX_OFS)) |= NVT_LED_OPS_SFT;
	__REG((pvaddr + NVT_LED_DIR_OFS)) |= NVT_LED_OPS_SFT;

	for(i=0;i < 10; i++) {
		if(toggle) {
			toggle = 0;
			__REG((pvaddr + NVT_LED_CTL_OFS)) &= ~NVT_LED_OPS_SFT;
		} else {
			toggle = 1;
			__REG((pvaddr + NVT_LED_CTL_OFS)) |= NVT_LED_OPS_SFT;
		}
		sleep(1);
	}


	munmap(pvaddr, NVT_LED_PHY_SIZE);
out:
	if(fd_mem >= 0)
		close(fd_mem);
	return ret;

}
