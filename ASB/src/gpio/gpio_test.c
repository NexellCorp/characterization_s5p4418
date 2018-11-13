#include "gpio.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


void print_usage(void)
{
	    printf( "usage: options\n"
	            "-h ptint this message   \n"
		);
}

unsigned int atoh(char *a)
{
    char ch = *a;
    unsigned int cnt = 0, hex = 0;

    while(((ch != '\r') && (ch != '\n')) || (cnt == 0)) {
        ch  = *a;

        if (! ch)
            break;

        if (cnt != 8) {
            if (ch >= '0' && ch <= '9') {
                hex = (hex<<4) + (ch - '0');
                cnt++;
            } else if (ch >= 'a' && ch <= 'f') {
                hex = (hex<<4) + (ch-'a' + 0x0a);
                cnt++;
            } else if (ch >= 'A' && ch <= 'F') {
                hex = (hex<<4) + (ch-'A' + 0x0A);
                cnt++;
            }
        }
        a++;
    }
    return hex;
}

static int inout_test(int gpio0, int gpio1)
{
	int ret =0;
	if(gpio_export(gpio0)!=0)
	{
		printf("gpio open fail\n");
		ret = -1;
		goto err;
	}
	if(gpio_export(gpio1)!=0)
	{
		printf("gpio open fail\n");
		ret = -1;
		goto err;
	}
	
	/* gpio 0 out 1 in */
	gpio_dir_out(gpio0);
	gpio_dir_in(gpio1);
		
	gpio_set_value(gpio0, 0);
	//printf("get  %d",gpio_get_value(gpio1));
	if(gpio_get_value(gpio1) != 0)
	{
		printf("gpio value is not same! \n" );
		ret = -1;
		goto err;
	}
	
	gpio_set_value(gpio0, 1);	
//	printf(" %d",gpio_get_value(gpio1));
	if(gpio_get_value(gpio1) != 1)
	{
		printf("gpio value is not same! \n" );
		ret = -1;
		goto err;
	}
	
	
	gpio_dir_out(gpio1);
	gpio_dir_in(gpio0);
		
	gpio_set_value(gpio1, 0);
	
	if(gpio_get_value(gpio0) != 0)
	{
		printf("gpio value is not same! \n" );
		ret = -1;
		goto err;
	}
	
	gpio_set_value(gpio1, 1);	
	if(gpio_get_value(gpio0) != 1)
	{
		printf("gpio value is not same! \n" );
		ret = -1;
		goto err;
	}
	
err:	
	gpio_unexport(gpio0);
	gpio_unexport(gpio1);
	
	return ret;
	
}



int main(int argc, char **argv)
{
	int ret, opt;
	int gpio0 = 0, gpio1 =0 ;
		
	while (-1 != (opt = getopt(argc, argv, "a:b:"))) {
		switch(opt) {
		case 'a' : 		gpio0	= atoi(optarg); break;
		case 'b' : 		gpio1 = atoi(optarg); break;
		}
	}
	ret = inout_test(gpio0,gpio1);
	printf("ret = %d\n",ret);
	printf(" GPIO%d, GPIO %d Loop back Test resul %s\n" , gpio0,gpio1, ret ? "Fail" : "OK");
	return ret;
}
