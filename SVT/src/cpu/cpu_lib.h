
/*
 *	sys APIs
 */
int sys_read(const char *file, char *buffer, int buflen);
int sys_write(const char *file, const char *buffer, int buflen);

/*
 *	Check ASV IDS/RO
 */
int get_sys_ecid(unsigned int ecid[4]);
unsigned int get_ecid_ids(unsigned int ecid[4]);
unsigned int get_ecid_ro(unsigned int ecid[4]);

/*
 *	Frequency APIs (Unit is KHZ)
 */
int set_cpu_speed(long khz);
unsigned long get_cpu_speed(void);
unsigned long get_max_speed(void);
int cpu_avail_freqnr(void);
int cpu_avail_freqs(long *khz, int size);

/*
 *	Voltage APIs (Unit is uV)
 */
int set_cpu_voltage(int id, long uV);
unsigned long get_cpu_voltage(int id);
int cpu_avail_volts(long *uV, int size);

/*
 *	CPU cores for SMP
 */
int get_cpu_nr(void);
pid_t gettid(void);
