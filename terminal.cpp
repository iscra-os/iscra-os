#include "terminal.h"
#include "string.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <climits>

#define PSF1_FONT_MAGIC 0x0436
 
typedef struct {
    uint16_t magic; // Magic bytes for idnetiifcation.
    uint8_t fontMode; // PSF font mode
    uint8_t characterSize; // PSF character size.
} PSF1_Header;
 
 
#define PSF_FONT_MAGIC 0x864ab572
 
typedef struct {
    uint32_t magic;         /* magic bytes to identify PSF */
    uint32_t version;       /* zero */
    uint32_t headersize;    /* offset of bitmaps in file, 32 */
    uint32_t flags;         /* 0 if there's no unicode table */
    uint32_t numglyph;      /* number of glyphs */
    uint32_t bytesperglyph; /* size of each glyph */
    uint32_t height;        /* height in pixels */
    uint32_t width;         /* width in pixels */
} PSF_font;


/* import our font that's in the object file we've created above */
#define _binary_font_psf_start _binary_zap_ext_light24_psf_start
#define _binary_font_psf_end _binary_zap_ext_light24_psf_end

extern char _binary_font_psf_start;
extern char _binary_font_psf_end;

uint16_t *unicode;
 
void psf_init()
{
    uint16_t glyph = 0;
    /* cast the address to PSF header struct */
    PSF_font *font = (PSF_font*)&_binary_font_psf_start;
    /* is there a unicode table? */
    if (font->flags) {
        unicode = NULL;
        return; 
    }
 
    /* get the offset of the table */
    char *s = (char *)(
    (unsigned char*)&_binary_font_psf_start +
      font->headersize +
      font->numglyph * font->bytesperglyph
    );
    /* allocate memory for translation table */
    unicode = static_cast<uint16_t*>(calloc(USHRT_MAX, 2));
    while(s > &_binary_font_psf_end) {
		
        uint16_t uc = (uint16_t)(((unsigned char *)s)[0]);
        if(uc == 0xFF) {
            glyph++;
            s++;
            continue;
        } else if(uc & 128) {
            /* UTF-8 to unicode */
            if((uc & 32) == 0 ) {
                uc = ((s[0] & 0x1F)<<6)+(s[1] & 0x3F);
                s++;
            } else
            if((uc & 16) == 0 ) {
                uc = ((((s[0] & 0xF)<<6)+(s[1] & 0x3F))<<6)+(s[2] & 0x3F);
                s+=2;
            } else
            if((uc & 8) == 0 ) {
                uc = ((((((s[0] & 0x7)<<6)+(s[1] & 0x3F))<<6)+(s[2] & 0x3F))<<6)+(s[3] & 0x3F);
                s+=3;
            } else
                uc = 0;
        }
        /* save translation */
        unicode[uc] = glyph;
        s++;
    }
}

/* the linear framebuffer */
extern char *fb;
/* number of bytes in each line, it's possible it's not screen width * bytesperpixel! */
extern int scanline;
 
#define PIXEL uint32_t   /* pixel pointer */

void terminal_putentryat(
    /* note that this is int, not char as it's a unicode character */
    unsigned short int c,
    /* cursor position on screen, in characters not in pixels */
    int cx, int cy,
    /* foreground and background colors, say 0xFFFFFF and 0x000000 */
    uint32_t fg, uint32_t bg)
{
    /* cast the address to PSF header struct */
    PSF_font *font = (PSF_font*)&_binary_font_psf_start;
    /* we need to know how many bytes encode one row */
    int bytesperline=(font->width+7)/8;
    /* unicode translation */
    if(unicode != NULL) {
        c = unicode[c];
    }
    /* get the glyph for the character. If there's no
       glyph for a given character, we'll display the first glyph. */
    unsigned char *glyph =
     (unsigned char*)&_binary_font_psf_start +
     font->headersize +
     (c>0&&c<font->numglyph?c:0)*font->bytesperglyph;
    /* calculate the upper left corner on screen where we want to display.
       we only do this once, and adjust the offset later. This is faster. */
    int offs =
        (cy * font->height * scanline) +
        (cx * (font->width + 1) * sizeof(PIXEL));
    /* finally display pixels according to the bitmap */
    int x,y, line,mask;
    for(y=0;y<=font->height;y++){
        /* save the starting position of the line */
        line=offs;
        mask=1<<(font->width-1);
        /* display a row */
        for(x=0;x<=font->width;x++){
            *((PIXEL*)(fb + line)) = *((unsigned int*)glyph) & mask ? fg : bg;
            /* adjust to the next pixel */
            mask >>= 1;
            line += sizeof(PIXEL);
        }
        /* adjust to the next line */
        glyph += bytesperline;
        offs  += scanline;
    }
}

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
uint32_t terminal_fg;
uint32_t terminal_bg;
uint16_t* terminal_buffer;

// void terminal_setcolor(uint8_t color) {
// 	terminal_color = color;
// }
 
// void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
// 	const size_t index = y * VGA_WIDTH + x;
// 	terminal_buffer[index] = vga_entry(c, color);
// }
 
void terminal_putchar(char c) {
	if (c == '\n')
		terminal_column = VGA_WIDTH - 1;
	else
		terminal_putentryat(c, terminal_column, terminal_row, terminal_fg, terminal_bg);
	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_row = 0;
	}
}

void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}
 
void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}

int vprintk(const char* fmt, va_list list) {
	char buffer[512];

	int result = vsnprintf(buffer, sizeof(buffer), fmt, list);

	if (result < 0) {
		terminal_writestring("Failed to format\n");
		return result;
	}
	else if (result == sizeof(buffer)) {
		terminal_writestring("String is too long\n");
		buffer[sizeof(buffer) - 1] = '\0';
	}

	terminal_writestring(buffer);

	return result;
}

int printk(const char* fmt, ...) {
	va_list list;

	va_start(list, fmt);
	int result = vprintk(fmt, list);
	va_end(list);

	return result;
}

[[noreturn]]
void panic(const char* fmt, ...) {
	va_list list;

	va_start(list, fmt);
	int result = vprintk(fmt, list);
	va_end(list);

	while (true)
		asm volatile("hlt"); // halt
}





void init_terminal(void) {
	psf_init();
	terminal_fg = 0xFF00FF;
	terminal_bg = 0xFFFFFF;

	printk("Hello\n");

	// terminal_row = 0;
	// terminal_column = 0;
	// terminal_buffer = (uint16_t*)0xC03FF000;
	// for (size_t y = 0; y < VGA_HEIGHT; y++) {
	// 	for (size_t x = 0; x < VGA_WIDTH; x++) {
	// 		const size_t index = y * VGA_WIDTH + x;
	// 		terminal_buffer[index] = vga_entry(' ', terminal_color);
	// 	}
	// }
}
