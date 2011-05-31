#define RETURN_ERROR 0x01
#define RETURN_OK 0x00

short SRV1X_setup();
short SRV1X_unlock(); //don't use this unless you have an ICD2
void SRV1X_get_bootloader_version();
void SRV1X_reboot();
void SRV1X_write_flash();
void SRV1X_boot_application();
void SRV1X_console();

void SRV1X_get_version();
void SRV1X_read_analogs();
void SRV1X_stream_GPS();
