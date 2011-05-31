#include "colors.h"

extern unsigned int imgWidth, imgHeight;

unsigned int ymax[MAX_COLORS], ymin[MAX_COLORS], umax[MAX_COLORS], umin[MAX_COLORS], vmax[MAX_COLORS], vmin[MAX_COLORS];
unsigned int blobx1[MAX_BLOBS], blobx2[MAX_BLOBS], bloby1[MAX_BLOBS], bloby2[MAX_BLOBS], blobcnt[MAX_BLOBS];
unsigned int hist0[256], hist1[256], mean[3];

void init_colors() {
    unsigned int ii;
    
    for(ii = 0; ii<MAX_COLORS; ii++) {
        ymax[ii] = 0;
        ymin[ii] = 0;
        umax[ii] = 0;
        umin[ii] = 0;
        vmax[ii] = 0;
        vmin[ii] = 0;
    }
}

unsigned int vpix(unsigned char *frame_buf, unsigned int xx, unsigned int yy) {
        unsigned int ix;
        ix = index(xx,yy); 
        return    ((unsigned int)frame_buf[ix] << 24) +    // returns UYVY packed into 32-bit word
                        ((unsigned int)frame_buf[ix+1] << 16) +
                        ((unsigned int)frame_buf[ix+2] << 8) +
                        (unsigned int)frame_buf[ix+3];
}

//  compute boundaries x1,y1 x2,y2 boundaries for a color bin 'ii'
//  save values in blobcnt[0], blobx1[0], bloby1[0] ...
//  return pixel count for color bin
unsigned int vblok(unsigned char *frame_buf, unsigned int ii) {
    unsigned int ix, xx, yy, y, u, v;
    unsigned int maxx, maxy, x1, x2, y1, y2, count;

    maxx = imgWidth;
    maxy = imgHeight;

    count = 0;
    x1 = maxx;
    x2 = 0;
    y1 = maxy;
    y2 = 0;
    
    for (xx=0; xx<maxx; xx+=2) {
        for (yy=0; yy<maxy; yy++) {
            ix = index(xx,yy);
            y = (((unsigned int)frame_buf[ix+1] + (unsigned int)frame_buf[ix+3])) >> 1;
            u = (unsigned int)frame_buf[ix];
            v = (unsigned int)frame_buf[ix+2];
            if ((y >= ymin[ii])
             && (y <= ymax[ii]) 
             && (u >= umin[ii]) 
             && (u <= umax[ii]) 
             && (v >= vmin[ii]) 
             && (v <= vmax[ii])) {
                count++;
                if (y2 < yy)
                    y2 = yy;
                if (y1 > yy)
                    y1 = yy;
                if (x2 < xx)
                    x2 = xx;
                if (x1 > xx)
                    x1 = xx;
            }
        }
    }
    blobx1[0] = x1;
    blobx2[0] = x2;
    bloby1[0] = y1;
    bloby2[0] = y2;
    blobcnt[0] = count;
    return count;
}

// return number of blobs found that match the search color
unsigned int vblob(unsigned char *frame_buf, unsigned int ii) {
    unsigned int jj, ix, xx, yy, y, u, v, count, bottom, top, tmp;
    unsigned int maxx, maxy;

    maxx = imgWidth;
    maxy = imgHeight;

    for (jj=0; jj<MAX_BLOBS; jj++) {
        blobcnt[jj] = 0;
        blobx1[jj] = maxx;
        blobx2[jj] = 0;
        bloby1[jj] = maxy;
        bloby2[jj] = 0;
    }
        
    jj = 0;    // jj indicates the current blob being processed
    for (xx=0; xx<maxx; xx+=2) {
        count = 0;
        bottom = maxy;
        top = 0;
        for (yy=0; yy<maxy; yy++) {
            ix = index(xx,yy);
            y = (((unsigned int)frame_buf[ix+1] + (unsigned int)frame_buf[ix+3])) >> 1;
            u = (unsigned int)frame_buf[ix];
            v = (unsigned int)frame_buf[ix+2];
            if ((y >= ymin[ii])
             && (y <= ymax[ii]) 
             && (u >= umin[ii]) 
             && (u <= umax[ii]) 
             && (v >= vmin[ii]) 
             && (v <= vmax[ii])) {
                count++;
                if (bottom > yy)
                    bottom = yy;
                if (top < yy)
                    top = yy;
            }
        }
        if (count) {
            if (bloby1[jj] > bottom)
                bloby1[jj] = bottom;
            if (bloby2[jj] < top)
                bloby2[jj] = top;
            if (blobx1[jj] > xx)
                blobx1[jj] = xx;
            if (blobx2[jj] < xx)
                blobx2[jj] = xx;
            blobcnt[jj] += count;
        } else {
            if (blobcnt[jj])    // move to next blob if a gap is found
                jj++;
            if (jj > (MAX_BLOBS-2))
                goto blobbreak;
        }
    }
blobbreak:     // now sort blobs by size, largest to smallest pixel count
    for (xx=0; xx<=jj; xx++) {
        if (blobcnt[xx] == 0)    // no more blobs, so exit
            return xx;
        for (yy=xx; yy<=jj; yy++) {
            if (blobcnt[yy] == 0)
                break;
            if (blobcnt[xx] < blobcnt[yy]) {
                tmp = blobcnt[xx];
                blobcnt[xx] = blobcnt[yy];
                blobcnt[yy] = tmp;
                tmp = blobx1[xx];
                blobx1[xx] = blobx1[yy];
                blobx1[yy] = tmp;
                tmp = blobx2[xx];
                blobx2[xx] = blobx2[yy];
                blobx2[yy] = tmp;
                tmp = bloby1[xx];
                bloby1[xx] = bloby1[yy];
                bloby1[yy] = tmp;
                tmp = bloby2[xx];
                bloby2[xx] = bloby2[yy];
                bloby2[yy] = tmp;
            }
        }
    }
    return xx;
}

// histogram function - 
//  hist0[] holds frequency of u|v combination  
//  hist1[] holds average luminance corresponding each u|v combination

void vhist(unsigned char *frame_buf) {
    unsigned int ix, iy, xx, yy, y1, u1, v1;
    unsigned int maxx, maxy;

    maxx = imgWidth;
    maxy = imgHeight;

    for (ix=0; ix<256; ix++) {          hist0[ix] = 0;  // accumulator 
        hist1[ix] = 0;  
    }
    for (xx=0; xx<maxx; xx+=2) {   
        for (yy=0; yy<maxy; yy++) {
            ix = index(xx,yy);  
            y1 = (((unsigned int)frame_buf[ix+1] + (unsigned int)frame_buf[ix+3])) >> 1;
            u1 = ((unsigned int)frame_buf[ix]);
            v1 = ((unsigned int)frame_buf[ix+2]);
            iy = (u1 & 0xF0) + (v1 >> 4);
            hist0[iy]++;
            hist1[iy] += y1;
        }
    }
    for (ix=0; ix<256; ix++)
        if (hist1[ix])
            hist1[ix] /= hist0[ix];  // normalize by number of hits
}

/* mean color function - computes mean value for Y, U and V
   mean[0] = Y mean, mean[1] = U mean, mean[2] = V mean */
void vmean(unsigned char *frame_buf) {
    unsigned int ix, xx, yy, y1, u1, v1;
    unsigned int my, mu, mv;
    unsigned int maxx, maxy;

    maxx = imgWidth;
    maxy = imgHeight;
    my = mu = mv = 0;

    for (xx=0; xx<maxx; xx+=2) {   
        for (yy=0; yy<maxy; yy++) {
            ix = index(xx,yy);  // yx, uv, vx range from 0-63 (yuv value divided by 4)
            y1 = (((unsigned int)frame_buf[ix+1] + (unsigned int)frame_buf[ix+3])) >> 1;
            u1 = ((unsigned int)frame_buf[ix]);
            v1 = ((unsigned int)frame_buf[ix+2]);
            my += y1;
            mu += u1;
            mv += v1;
        }
    }
    mean[0] = ((my*2) / maxx) / maxy;
    mean[1] = ((mu*2) / maxx) / maxy;
    mean[2] = ((mv*2) / maxx) / maxy;
}

void color_segment(unsigned char *frame_buf) {
    unsigned int ix, xx, yy, y, u, v, clr;
    unsigned int ymid[MAX_COLORS], umid[MAX_COLORS], vmid[MAX_COLORS];
    
    for (ix=0; ix<MAX_COLORS; ix++) {
        ymid[ix] = (ymax[ix] + ymin[ix]) >> 1;
        umid[ix] = (umax[ix] + umin[ix]) >> 1;
        vmid[ix] = (vmax[ix] + vmin[ix]) >> 1;
    }
    for (xx=0; xx<imgWidth; xx+=2) {   
        for (yy=0; yy<imgHeight; yy++) {
            ix = index(xx,yy);
            y = (((unsigned int)frame_buf[ix+1] + (unsigned int)frame_buf[ix+3])) >> 1;
            //y = (unsigned int)frame_buf[ix+1];
            u = (unsigned int)frame_buf[ix];
            v = (unsigned int)frame_buf[ix+2];
            for (clr=0; clr<MAX_COLORS; clr++) {
                if (ymax[clr] == 0)    // skip this color if not defined
                    continue;
                if ((y >= ymin[clr])
                  && (y <= ymax[clr]) 
                  && (u >= umin[clr]) 
                  && (u <= umax[clr]) 
                  && (v >= vmin[clr]) 
                  && (v <= vmax[clr])) {
                    frame_buf[ix+1] = frame_buf[ix+3] = ymid[clr];
                    frame_buf[ix] = umid[clr];
                    frame_buf[ix+2] = vmid[clr];
                    break;
                }
            }
            if (clr == MAX_COLORS) {  // if no match, black out the pixel
                frame_buf[ix+1] = frame_buf[ix+3] = 0;
                frame_buf[ix] = frame_buf[ix+2] = 128;
            }
        }
    }
}

void edge_detect(unsigned char *inbuf, unsigned char *outbuf, int thresh) {
    unsigned int ix, xx, yy, y2, u2, v2, skip;
    unsigned char *ip, *op;
    unsigned int *ip1, *op1;
    unsigned int gx, gy;
    
    skip = imgWidth*2;
    for (xx=2; xx<imgWidth-2; xx+=2) {   
        for (yy=1; yy<imgHeight-1; yy++) {
            gx = gy = 0;
            ix = index(xx, yy);
            ip = inbuf + ix;
            op = outbuf + ix;

            y2 = *(ip+5) + *(ip+7) - *(ip-3) - *(ip-1);
            u2 = *(ip+4) - *(ip-4);
            v2 = *(ip+6) - *(ip-2);
            gy = ((y2*y2)>>2) + u2*u2 + v2*v2;
            
            y2 = *(ip+1+skip) + *(ip+3+skip) - *(ip+1-skip) - *(ip+3-skip);
            u2 = *(ip+skip) - *(ip-skip); 
            v2 = *(ip+2+skip) - *(ip+2-skip); 
            gx = ((y2*y2)>>2) + u2*u2 + v2*v2;
 
            if ((gx > thresh) || (gy > thresh)) {
                *op = 128;
                *(op+2) = 128;
                *(op+1) = 255;
                *(op+3) = 255;
            } else {
                *op = (*(ip) & 0x80) | 0x40;
                *(op+2) = (*(ip+2) & 0x80) | 0x40;
                *(op+1) = (*(ip+1) & 0xC0) | 0x20;
                *(op+3) = (*(ip+3) & 0xC0) | 0x20;
            }
        }
    }

    op1 = (unsigned int *)outbuf;
    ip1 = (unsigned int *)inbuf;
    for (ix=0; ix<(imgWidth*imgHeight>>1); ix++)
        ip1[ix] = op1[ix];
}

void svs_segcode(unsigned char *spibuf, unsigned char *framebuf, int thresh) {
    unsigned int ix, xx, yy, y2, u2, v2, skip;
    unsigned char *ip, *op, cc;
    unsigned int gx, gy;
    
    if (imgWidth > 640)   // buffer size limits this function to 640x480 resolution
        return;
    
    skip = imgWidth*2;
    op = spibuf;
    for (yy=0; yy<imgHeight; yy++) {
        for (xx=0; xx<imgWidth; xx+=2) {   
            if ((xx < 2) || (xx >= imgWidth-2) || (yy < 1) || (yy >= imgHeight-1)) {
                *op++ = 0;
                continue;
            }
            gx = gy = 0;
            ix = index(xx, yy);
            ip = framebuf + ix;

            y2 = *(ip+5) + *(ip+7) - *(ip-3) - *(ip-1);
            u2 = *(ip+4) - *(ip-4);
            v2 = *(ip+6) - *(ip-2);
            gy = ((y2*y2)>>2) + u2*u2 + v2*v2;
            
            y2 = *(ip+1+skip) + *(ip+3+skip) - *(ip+1-skip) - *(ip+3-skip);
            u2 = *(ip+skip) - *(ip-skip); 
            v2 = *(ip+2+skip) - *(ip+2-skip); 
            gx = ((y2*y2)>>2) + u2*u2 + v2*v2;
 
            cc = ((*ip >> 2) & 0x38)       // threshold U to 0x0C position
               + ((*(ip+2) >> 5) & 0x07);  // threshold V to 0x03 position
            if (gx > thresh)  
                cc |= 0x80;               // add 0x80 flag if this is edge pixel
            if (gy > thresh)  
                cc |= 0x40;               // add 0x40 flag if this is edge pixel
            *op++ = cc;
        }
    }
}

void svs_segview(unsigned char *spibuf, unsigned char *framebuf) {
    unsigned int ix;
    unsigned char *ip, *op;

    if (imgWidth > 640)   // buffer size limits this function to 640x480 resolution
        return;
    
    ip = spibuf;
    op = framebuf;
    for (ix=0; ix<imgWidth*imgHeight; ix+=2) {   
        if (*ip & 0xC0) {      // is this an edge pixel ?
            *(op+1) = *(op+3) = 0xFF;
            *op = *(op+2) = 0x80;
        } else {
            *(op+1) = *(op+3) = 0xA0;
            *op = ((*ip & 0x38) << 2) + 0x10;
            *(op+2) = ((*ip & 0x07) << 5) + 0x10;
        }
        op += 4;
        ip++;
    }
}

