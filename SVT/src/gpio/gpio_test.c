#include "gpio.h"
#define INPUT 0
#define OUTPUT 1

void print_usage(void)
{
	printf( "usage: options\n"
			"-p gpio num\n"
			"-d driection 0: in 1: out\n"
			"-v input validation value\n"
				
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



int main(int argc, char **argv)
{

	int opt;
    int gpio = 0,dir = 0, value = 0;
	int ret = -1, read=0;
	int count=1,i=0;
    while (-1 != (opt = getopt(argc, argv, "p:d:v:C:T:"))) {
		switch(opt) {
		case 'p' :      gpio   = atoi(optarg); break;
		case 'd' :      dir 	= atoi(optarg); break;													
		case 'v' :      value 	= atoi(optarg); break;													
		case 'C' :      count 	= atoi(optarg); break;													
		}
   }
	if(count==0)
		count = 1;
	for(i=0;i<count;i++) {
    	if(gpio_export(gpio)!=0)
    	{	
			printf("gpio open fail\n");
   	    	ret = -1;
	        goto err;
	    }
		if(dir == INPUT)
		{
			printf("INPUT TEST\n");
			gpio_dir_in(gpio);
			read = gpio_get_value(gpio);
			//printf(" read : %d %d \n",read,value);
		    if(read == value)
			ret=0;	
		}
		else 
		{
			printf("OUTPUT TEST\n");
			gpio_dir_out(gpio);
			value=0;
			gpio_set_value(gpio,value);
			read = gpio_get_value(gpio);
			printf("Set Low , Read : %d \n",read);
		    if(read != value)
				goto err;
			value=1;
			gpio_set_value(gpio,value);
			read = gpio_get_value(gpio);
			printf("Set High , Read : %d \n",read);
		    if(read != value)
				goto err;
			ret=0;
		}
	}			


err:
	gpio_unexport(gpio);

	printf("Return value : %d \n",ret);
	return ret;

}

