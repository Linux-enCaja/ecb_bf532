#ifndef SRV_H
#define SRV_H

#define SSYNC asm("ssync;")
#define TRUE 1
#define FALSE 0

#define SCCB_ON  1   // enables SCCB mode for I2C (used by OV9655 camera)
#define SCCB_OFF 0

#define PWM_OFF 0
#define PWM_PWM 1
#define PWM_PPM 2
#define PWM_UART 3

#define FOUR_TWO_ZERO 1
#define FOUR_TWO_TWO  2

/* flash usage */
#define BOOT_SECTOR  0x00000000  // address in SPI flash of boot image 
#define FLASH_SECTOR 0x00040000  // address in SPI flash of user flash sector  

/* SDRAM allocation */
#define FLASH_BUFFER 0x00100000  // address in SDRAM for buffering flash and xmodem 
#define SPI_BUFFER1  0x00200000  // 512kB buffer for transfer of data via SPI bus
#define SPI_BUFFER2  0x00280000  // 512kB buffer for transfer of data via SPI bus
#define HEAPSTART    0x00300000  // put this above FLASH_BUFFER 
#define HEAPSIZE     0x00C00000  // 12MB for now - leave 1MB for JPEG buffer
#define DMA_BUF1     0x01000000  // address in SDRAM for DMA transfer of frames from camera
#define DMA_BUF2     0x01300000  //   second DMA buffer for double buffering
#define FRAME_BUF    0x01600000  // address in SDRAM for staging images for processing/jpeg
#define FRAME_BUF2   0x01900000  //   second buffer for storing reference frame
#define FRAME_BUF3   0x01C00000  //   another buffer for edge data
#define JPEG_BUF     0x00F00000  // address in SDRAM for JPEG compressed image

/* Stack info */
#define STACK_TOP    0xFFB00FFC
#define STACK_BOTTOM 0xFFB00000

/* Misc Init */
void init_io ();
void clear_sdram ();
void launch_editor ();
void init_heap ();
void show_stack_ptr ();
void show_heap_ptr ();
void reset_cpu ();
unsigned int stack_remaining();

/* Serial outputs */
void serial_out_version ();
void serial_out_time ();
void serial_out_flashbuffer ();

/* I2C */
void process_i2c();

/* Lasers */
void lasers_on ();
void lasers_off ();
void show_laser_range();
unsigned int laser_range(int);

/* Sonar */
void init_sonar();
void ping_sonar();

/* LED's */
void led0_on();
void led1_on();

/* Camera */
void grab_frame ();
void send_frame ();
void grab_reference_frame ();
void compute_frame_diff (unsigned char *, unsigned char *, int, int);
void enable_frame_diff ();
void enable_segmentation();
void enable_edge_detect();
void set_edge_thresh();
void grab_code_send();
void recv_grab_code();
void grab_dct_coeff();
void show_dct_coeff(), show_dct_coeff310();
void disable_frame_diff ();
void overlay_on ();
void overlay_off ();
void camera_setup ();
void camera_reset (unsigned int width);
void change_image_quality ();
void set_caption (unsigned char *str, unsigned int width);
void move_image (unsigned char *src1, unsigned char *src2, unsigned char *dst, unsigned int width, unsigned int height);
void invert_video(), restore_video();

/* Image Processing */
void process_colors(), init_colors();

/* Failsafe */
void enable_failsafe();
void disable_failsafe();
void check_failsafe();
void reset_failsafe_clock();
void check_battery();

/* Transfer */
void xmodem_receive ();

void start_cinterpreter ();
void start_lisp_from_buffer ();
void start_lisp_from_console ();

/* Flash */
void read_user_flash ();
void write_user_flash ();
void write_boot_flash ();
void clear_flash_buffer ();
void crc_flash_buffer ();

/* Motors */
void init_motors ();
void init_servos();
void update_servos();
void motor_command ();
void motor_increase_base_speed ();
void motor_decrease_base_speed ();
void motor_trim_left ();
void motor_trim_right ();
void motor_action (unsigned char ch);
void motor_set (unsigned char cc, int speed, int *ls, int *rs);
void ppm1_command ();
void ppm2_command ();
void initPWM ();
void initPPM1 ();
void initPPM2 ();
void setPWM (int mleft, int mright);
void setPPM1 (int mleft, int mright);
void setPPM2 (int mleft, int mright);
int check_bounds_0_100(int ix);

/* Misc */
int rand();
unsigned int isqrt(unsigned int);

/* Clock */
void initRTC ();
int readRTC ();
void clearRTC ();
void initTMR4 ();
void delayMS (int delay);  // delay up to 100000 millisecs (100 secs)


#endif

