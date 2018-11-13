#include "gpio.h" 
int gpio_export(unsigned gpio)
{
	int fd, len;
	char buf[11];
 
	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}
 
	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
 
	return 0;
}
 
int gpio_unexport(unsigned gpio)
{
	int fd, len;
	char buf[11];
 
	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}
 
	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
	return 0;
}
 
int gpio_dir(unsigned gpio, unsigned dir)
{
	int fd;
	char buf[60];
 
	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/direction", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}
 
	if (dir == GPIO_DIR_OUT)
		write(fd, "out", 4);
	else
		write(fd, "in", 3);
 
	close(fd);
	return 0;
}
 
int gpio_dir_out(unsigned gpio)
{
	return gpio_dir(gpio, GPIO_DIR_OUT);
}
 
int gpio_dir_in(unsigned gpio)
{
	return gpio_dir(gpio, GPIO_DIR_IN);
}
 
int gpio_set_value(unsigned gpio, unsigned value)
{
	int fd ;
	char buf[60];
 
	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/value");
		return fd;
	}
 
	if (value)
		write(fd, "1", 2);
	else
		write(fd, "0", 2);
 
	close(fd);
	return 0;
}

int gpio_get_value(unsigned gpio)
{
	int fd;
	char buf[60];
	int value = 0; 

	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", gpio);
 
	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		perror("gpio/value");
		return fd;
	}
 
	
	value = read(fd,buf ,1);
//	printf("Value %d %s  \n",value, buf);
	close(fd);
	if( buf[0] == '0')
		value = 0;
	else 
		value = 1;
	return value;
}


