#define MAX_BLOBS  16
#define MAX_COLORS 17  // reserve color #16 for internal use
#define MIN_BLOB_SIZE 5

#define index(xx, yy)  ((yy * imgWidth + xx) * 2) & 0xFFFFFFFC  // always a multiple of 4

//#define index(xx, yy)   (scaling_enabled > 0) ? ((imgWidth * (xx + ((63-yy) * imgWidth))) / 40) : ((yy * imgWidth + xx) * 2)

extern unsigned int vblob(unsigned char *, unsigned int);
extern unsigned int vblok(unsigned char *, unsigned int);
extern unsigned int vpix(unsigned char *, unsigned int, unsigned int);
extern void init_colors();
extern void vhist(unsigned char *frame_buf);
extern void vmean(unsigned char *frame_buf);
extern void color_segment(unsigned char *frame_buf);
extern void edge_detect(unsigned char *frame_buf, unsigned char *frame_buf2, int threshold);

extern unsigned int ymax[], ymin[], umax[], umin[], vmax[], vmin[];
extern unsigned int blobx1[], blobx2[], bloby1[], bloby2[], blobcnt[];
extern unsigned int hist0[], hist1[], hist2[], mean[];

void svs_segcode(unsigned char *spibuf, unsigned char *framebuf, int thresh);
void svs_segview(unsigned char *spibuf, unsigned char *framebuf);

