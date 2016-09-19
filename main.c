#include <string.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include "fat.h"
#include "fat_config.h"
#include "partition.h"
#include "sd_raw.h"
#include "sd_raw_config.h"
#include "uart.h"
#include "ADC.h"
#include <avr/delay.h>
#define DEBUG 1

static uint8_t read_line(char* buffer, uint8_t buffer_length);
static uint32_t strtolong(const char* str);
static uint8_t find_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name, struct fat_dir_entry_struct* dir_entry);
static struct fat_file_struct* open_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name); 
static uint8_t print_disk_info(const struct fat_fs_struct* fs);

 struct fat_fs_struct* fs;
 struct fat_dir_struct* dd;
 struct fat_dir_entry_struct directory;
 struct partition_struct* partition;
 struct fat_file_struct* fd ;
 
 void sd_init();
 void create_file(char* fileName);
 void do_ls();
 void write_to_file(char* fileName,char* text);
 
int main()
	{
	  ADC_init();
	  sd_init();
	  /* print directory listing in serial */
	  do_ls();
	  create_file("adc_value.txt");
	  uart_puts("created\n\n");
	  do_ls();
		  fd = open_file_in_dir(fs,dd,"adc_value.txt");
	  
	  do_ls();
	  uint16 ADC_val;
	  char output[30];
	  
	while(1)
		{
		strcat(output,"ADC value is ");
		ADC_val = ADC_read(0); //0 is channel num 0 in adc pins
		//ADC_val =  (ADC_val*150*5)/(1023*1.5); /* calculate the temp from the ADC value Just for this test*/
		char ADC_VALUE[5];
		itoa(ADC_val,ADC_VALUE,10);
		strcat(output,ADC_VALUE);
		strcat(output,"\n");
		uart_puts(output);
		fat_write_file(fd,(uint8_t*)output,strlen(output));
		output[0]='\0';
		_delay_ms(5000);
		}
	
	/*close file*/
	 fat_close_file(fd);
	 /* close directory */
	 fat_close_dir(dd);

	 /* close file system */
	 fat_close(fs);

	 /* close partition */
	 partition_close(partition);
	 
	 
    return 0;
	}
 
 
	void write_to_file(char* fileName,char* text)
		{
			fd = open_file_in_dir(fs,dd,fileName);
	  		uint8_t x = strlen(text);
			fat_write_file(fd,(uint8_t*)text,x);
		}
		
	void create_file(char* fileName)
		{
			struct fat_dir_entry_struct file_entry;
			if(!fat_create_file(dd, fileName, &file_entry))
			{
				uart_puts_p(PSTR("error creating file: "));
				uart_puts(fileName);
				uart_putc('\n');
			}
		}

	void do_ls()
		{
			struct fat_dir_entry_struct dir_entry;
			while(fat_read_dir(dd, &dir_entry))
			{
				uint8_t spaces = sizeof(dir_entry.long_name) - strlen(dir_entry.long_name) + 4;
				uart_puts(dir_entry.long_name);
				uart_putc(dir_entry.attributes & FAT_ATTRIB_DIR ? '/' : ' ');
				while(spaces--)
				uart_putc(' ');
				uart_putdw_dec(dir_entry.file_size);
				uart_putc('\n');
			}
		}		
	
	void sd_init()
		{
			/* use ordinary idle mode */
			set_sleep_mode(SLEEP_MODE_IDLE);
			/* setup uart */
			uart_init();
           /* setup sd card slot */
			if(!sd_raw_init())
				{
					#if DEBUG
					uart_puts_p(PSTR("MMC/SD initialization failed\n"));
					#endif
					return;
				}

				/* open first partition */
					partition = partition_open(sd_raw_read,
                    sd_raw_read_interval,	
					#if SD_RAW_WRITE_SUPPORT
                    sd_raw_write,
					sd_raw_write_interval,
					#else
									0,
									0,
					#endif
									0
											);

					if(!partition)
					{
						/* If the partition did not open, assume the storage device
						 * is a "superfloppy", i.e. has no MBR.
						 */
						partition = partition_open(sd_raw_read,
												   sd_raw_read_interval,
			#if SD_RAW_WRITE_SUPPORT
												   sd_raw_write,
												   sd_raw_write_interval,
			#else
												   0,
												   0,
			#endif
												   -1
												  );
							if(!partition)
							{
				#if DEBUG
								uart_puts_p(PSTR("opening partition failed\n"));
				#endif
								return;
							}
						}

						/* open file system */
						fs = fat_open(partition);
						if(!fs)
						{
				#if DEBUG
							uart_puts_p(PSTR("opening filesystem failed\n"));
				#endif
							return;
						}

					/* open root directory */
					
					fat_get_dir_entry_of_path(fs, "/", &directory);

					dd = fat_open_dir(fs, &directory);
					if(!dd)
					{
			#if DEBUG
						uart_puts_p(PSTR("opening root directory failed\n"));
			#endif
						return;
					}
					
					/* print some card information as a boot message */
					uart_puts("hello");

		}


	uint8_t read_line(char* buffer, uint8_t buffer_length)
		{
			memset(buffer, 0, buffer_length);
			uint8_t read_length = 0;
			while(read_length < buffer_length - 1)
		{
        uint8_t c = uart_getc();
        if(c == 0x08 || c == 0x7f)
        {
            if(read_length < 1)
                continue;
            --read_length;
            buffer[read_length] = '\0';

            uart_putc(0x08);
            uart_putc(' ');
            uart_putc(0x08);

            continue;
        }

        uart_putc(c);

        if(c == '\n')
        {
            buffer[read_length] = '\0';
            break;
        }
			else
			{
				buffer[read_length] = c;
				++read_length;
			}
		}

		return read_length;
		}

	uint32_t strtolong(const char* str)
	{
		uint32_t l = 0;
		while(*str >= '0' && *str <= '9')
			l = l * 10 + (*str++ - '0');

		return l;
	}

	uint8_t find_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name, struct fat_dir_entry_struct* dir_entry)
	{
		while(fat_read_dir(dd, dir_entry))
		{
			if(strcmp(dir_entry->long_name, name) == 0)
			{
				fat_reset_dir(dd);
				return 1;
			}
		}

		return 0;
	}

	struct fat_file_struct* open_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name)
	{
		struct fat_dir_entry_struct file_entry;
		if(!find_file_in_dir(fs, dd, name, &file_entry))
			return 0;

		return fat_open_file(fs, &file_entry);
	}



	uint8_t print_disk_info(const struct fat_fs_struct* fs)
	{
		if(!fs)
		return 0;

		struct sd_raw_info disk_info;
		if(!sd_raw_get_info(&disk_info))
		return 0;

		uart_puts_p(PSTR("manuf:  0x")); uart_putc_hex(disk_info.manufacturer); uart_putc('\n');
		uart_puts_p(PSTR("oem:    ")); uart_puts((char*) disk_info.oem); uart_putc('\n');
		uart_puts_p(PSTR("prod:   ")); uart_puts((char*) disk_info.product); uart_putc('\n');
		uart_puts_p(PSTR("rev:    ")); uart_putc_hex(disk_info.revision); uart_putc('\n');
		uart_puts_p(PSTR("serial: 0x")); uart_putdw_hex(disk_info.serial); uart_putc('\n');
		uart_puts_p(PSTR("date:   ")); uart_putw_dec(disk_info.manufacturing_month); uart_putc('/');
		uart_putw_dec(disk_info.manufacturing_year); uart_putc('\n');
		uart_puts_p(PSTR("size:   ")); uart_putdw_dec(disk_info.capacity / 1024 / 1024); uart_puts_p(PSTR("MB\n"));
		uart_puts_p(PSTR("copy:   ")); uart_putw_dec(disk_info.flag_copy); uart_putc('\n');
		uart_puts_p(PSTR("wr.pr.: ")); uart_putw_dec(disk_info.flag_write_protect_temp); uart_putc('/');
		uart_putw_dec(disk_info.flag_write_protect); uart_putc('\n');
		uart_puts_p(PSTR("format: ")); uart_putw_dec(disk_info.format); uart_putc('\n');
		uart_puts_p(PSTR("free:   ")); uart_putdw_dec(fat_get_fs_free(fs)); uart_putc('/');
		uart_putdw_dec(fat_get_fs_size(fs)); uart_putc('\n');

		return 1;
	}