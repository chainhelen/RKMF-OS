#include "multiboot.h"
#include "elf.h"

unsigned char *videobuf = (unsigned char *)0xb8000;

/* 参考 https://www.zhihu.com/question/49580321 */

// the_cpu为每个cpu的数据，目前仅用于cpu栈
char the_cpu[4096] __attribute__((section(".percpu")));
void print(char *s, int len);
void memcpy(char *dst, char *src, int count);
void memset(char *addr, char data, int count);
Elf64_Addr loadGoELF(multiboot_info_t *info) ;
Elf64_Addr loadELF(char *elf_header);
typedef void (*go_entry_t)(uint32, uint32);

// The physical address of the multiboot header. On qemu for example this is typically at 0x9500.
// TODO？ 这里函数调约是反的，可能跟直接进入64位有关
void bp_main(multiboot_info_t *mbi, unsigned long magic)
{
    Elf64_Addr addr = loadGoELF(mbi);
    go_entry_t go_entry  = (go_entry_t)(addr);
    go_entry((uint32)magic, (uint32)(uint64)mbi);

    while(1){};
}

Elf64_Addr loadGoELF(multiboot_info_t *info) {
    if (info->mods_count < 1) {
        return 0;
    }
    multiboot_module_t *mod = (multiboot_module_t *)((uint64) (info->mods_addr));
//  这里没有重新在100M位置copy一份，直接使用multiboot载入的image
    return loadELF((char *)((uint64)mod->mod_start));
}

Elf64_Addr loadELF(char *elf_header) {
    struct elf64_hdr *hdr = (struct elf64_hdr *)elf_header;
    // magic 0x7f454C46
    if (!(hdr->e_ident[0] == 0x7f &&
            hdr->e_ident[1] == 0x45 &&
            hdr->e_ident[2] == 0x4C &&
            hdr->e_ident[3] == 0x46)) {
       return 0;
    }
    // golang 静态编译的
    struct elf64_phdr *phdr = (struct elf64_phdr *)(elf_header + hdr->e_phoff);

    for (int i = 0;i < hdr->e_phnum;i++) {
        struct elf64_phdr *phIter = (struct elf64_phdr *)(phdr + i);
        char *dstphAddr = (char *)(phIter->p_paddr);
        char *srcPhAddr = elf_header + (Elf64_Off)phIter->p_offset;
        int memCopyBsCount = (int)(phIter -> p_filesz);
        memcpy(dstphAddr, srcPhAddr, memCopyBsCount);

        if (phIter->p_memsz > phIter->p_filesz) {
            char *addr = (char *)((Elf64_Addr)phIter->p_paddr+ (Elf64_Off)phIter->p_filesz);
            int count = (int)(phIter->p_memsz - phIter->p_filesz);
            memset(addr, 0, count);
        }
    }

    return hdr->e_entry;
}

void memcpy(char *dst, char *src, int count) {
    for (int i = 0;i < count;i++) {
        *dst = *src;
        dst++;
        src++;
    }
}

void memset(char *addr, char data, int count) {
    for (int i = 0;i < count;i++) {
        *addr = data;
        addr++;
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