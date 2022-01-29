#include "dwell.h"
void app_main(void)
{
	while(1)
	{
	int brr[] = {0,100,200,300,480,550};
	int c = (sizeof(brr)/sizeof(brr[0]));
	wifi_sniffer_init();
	//printf("main time : %ld \n",now.tv_sec);
	int *n = dwellTime(brr,c);
	printf("after time : %ld \n",now.tv_sec);
	for(int m=0;m<c-1 ;m++)
	{
		printf("Difference between %d and %d is : %d  \n", brr[m],brr[m+1],n[m]);
	}
	}
}

