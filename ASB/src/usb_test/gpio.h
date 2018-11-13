#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define	DBG_INFO    0	
#define	DBG_DBG		0
#define	DBG_VBS		0

#define	DBG_TAG		"[GPIO] "

#define GPIO_DIR_IN		0

#define GPIO_DIR_OUT	1


//  Declare gpio tool functions
int gpio_export(unsigned gpio);
int gpio_unexport(unsigned gpio);
int gpio_dir(unsigned gpio, unsigned dir);
int gpio_dir_out(unsigned gpio);
int gpio_dir_in(unsigned gpio);
int gpio_set_value(unsigned gpio, unsigned value);
int gpio_get_value(unsigned gpio);

