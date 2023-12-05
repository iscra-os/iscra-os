#include "allocator.h"
#include "terminal.h"
#include "gdt.h"
#include "interrupts.h"
#include "vfs.h"
#include "processes.h"
#include "multiboot.h"
 
#if !defined(__i386__)
#error "This kernel needs to be compiled with a ix86-elf compiler"
#endif



void draw_blue_line(multiboot_info* mbi) {
	/* Draw diagonal blue line. */
      multiboot_uint32_t color;
      unsigned i;
      uint8_t* fb = reinterpret_cast<uint8_t*>(mbi->framebuffer_addr);

      switch (mbi->framebuffer_type)
        {
        case MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED:
          {
            unsigned best_distance, distance;
            struct multiboot_color *palette;
            
            palette = (struct multiboot_color *) mbi->framebuffer_palette_addr;

            color = 0;
            best_distance = 4*256*256;
            
            for (i = 0; i < mbi->framebuffer_palette_num_colors; i++)
              {
                distance = (0xff - palette[i].blue) * (0xff - palette[i].blue)
                  + palette[i].red * palette[i].red
                  + palette[i].green * palette[i].green;
                if (distance < best_distance)
                  {
                    color = i;
                    best_distance = distance;
                  }
              }
          }
          break;

        case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
          color = ((1 << mbi->framebuffer_blue_mask_size) - 1) 
            << mbi->framebuffer_blue_field_position;
          break;

        case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
          color = '#' | 0x0100;
          break;

        default:
          color = 0xffffffff;
          break;
        }
      for (i = 0; i < mbi->framebuffer_width
             && i < mbi->framebuffer_height; i++)
        {
          switch (mbi->framebuffer_bpp) // bits per pixel
            {
            case 8:
              {
                multiboot_uint8_t *pixel = fb + mbi->framebuffer_pitch * i + i;
                *pixel = color;
              }
              break;
            case 15:
            case 16:
              {
                multiboot_uint16_t *pixel
                  = reinterpret_cast<uint16_t*>(fb + mbi->framebuffer_pitch * i + 2 * i);
                *pixel = color;
              }
              break;
            case 24:
              {
                multiboot_uint32_t *pixel
                  = reinterpret_cast<uint32_t*>(fb + mbi->framebuffer_pitch * i + 3 * i);
                *pixel = (color & 0xffffff) | (*pixel & 0xff000000);
              }
              break;

            case 32:
              {
                multiboot_uint32_t *pixel
                  = reinterpret_cast<uint32_t*>(fb + mbi->framebuffer_pitch * i + 4 * i);
                *pixel = color;
              }
              break;
            }
        }
}

/* the linear framebuffer */
char *fb;
/* number of bytes in each line, it's possible it's not screen width * bytesperpixel! */
int scanline;
 

extern "C" void kernel_main(multiboot_info* mbi) {
	uint32_t framebuffer_addr = mbi->framebuffer_addr;
	init_allocator(framebuffer_addr);
	mbi->framebuffer_addr = framebuffer_addr;

	fb = reinterpret_cast<char*>(mbi->framebuffer_addr);
	scanline = mbi->framebuffer_pitch;

	init_gdt();

	init_interrupts();

    // terminal_setcolor(vga_entry_color(VGA_COLOR_MAGENTA, VGA_COLOR_WHITE));
	init_terminal();

	// init_vfs();

	// init_processes();

	// draw_blue_line(mbi);
	// printk("%llX\n", mbi->framebuffer_addr);

	// int* ptr = 0; 
	// *ptr = 1;

	// while(1) {
	// 	uint8_t status = 0;
	// 	while (!(status & 1)) {
	// 		status = inb(0x64);
	// 	}
	// 	char key = inb(0x60);
	// 	printk("%x\n", key);
	// }
}