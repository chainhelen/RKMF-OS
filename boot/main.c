#include "multiboot.h"
#include "elf.h"

unsigned char *videobuf = (unsigned char *)0xb8000;

/* 参考 https://www.zhihu.com/question/49580321 */

// the_cpu为每个cpu的数据，目前仅用于cpu栈
char the_cpu[4096] __attribute__((section(".percpu")));
void print(char *s, int len);
void memcpy(char *dst, char *src, int count);
uint64 loadGoELF(multiboot_info_t *info) ;
uint64 loadELF(char *elf_header);

// The physical address of the multiboot header. On qemu for example this is typically at 0x9500.
// TODO？ 这里函数调约是反的，可能跟直接进入64位有关
void bp_main(multiboot_info_t *mbi, unsigned long magic)
{
    if(loadGoELF(mbi) == 1) {
        print("1", 1);
    } else {
        print("0", 1);
    }
    while(1){};
}

uint64 loadGoELF(multiboot_info_t *info) {
    if (info->mods_count < 1) {
        return 0;
    }
    multiboot_module_t *mod = (multiboot_module_t *)((uint64) (info->mods_addr));
    int count = mod->mod_end - mod->mod_start + 1;
    char *image = (char *)(100 << 20);
    memcpy(image, (char *)((uint64)mod->mod_start), count);
    return loadELF(image);
}

uint64 loadELF(char *elf_header) {
    struct elf64_hdr *hdr = (struct elf64_hdr *)elf_header;
    // magic 0x7f454C46
    if (!(hdr->e_ident[0] == 0x7f &&
            hdr->e_ident[1] == 0x45 &&
            hdr->e_ident[2] == 0x4C &&
            hdr->e_ident[3] == 0x46)) {
       return 0;
    }

    return 1;
}

void memcpy(char *dst, char *src, int count) {
    for (int i = 0;i < count;i++) {
        *dst = *src;
        dst++;
        src++;
    }
}

void print(char *s, int len)
{
    int i ;
    for (i = 0;i < len;i++)
    {
        videobuf[i * 2 + 0] = s[i];
        videobuf[i * 2 + 1] = 0x17;
    }
    for (; i < 80 * 25;i++)
    {
        videobuf[i * 2 + 0] = ' ';
        videobuf[i * 2 + 1] = 0x17;
    }
}