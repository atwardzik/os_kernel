//
// Created by Artur Twardzik on 28/05/2026.
//

#include "loader.h"
#include "libc.h"

#include <stdint.h>

#include "memory.h"

uint32_t get_dyn(const char *) { return 0xdeadbeef; }

constexpr int ELF_NIDENT = 16;
constexpr int E_TYPE_REL = 1;
constexpr int E_TYPE_EXEC = 2;
constexpr int E_MACHINE_ARM = 0x28;
constexpr int SST_FUNC = 2;

typedef uint16_t HalfWord;
typedef uint32_t Word;
typedef uint32_t Address;
typedef uint32_t Offset;

typedef struct {
        uint8_t e_ident[ELF_NIDENT];
        HalfWord e_type;
        HalfWord e_machine;
        Word e_version;
        Address e_entry;
        Offset e_phoff;
        Offset e_shoff;
        Word e_flags;
        HalfWord e_ehsize;
        HalfWord e_phentsize;
        HalfWord e_phnum;
        HalfWord e_shentsize;
        HalfWord e_shnum;
        HalfWord e_shstrndx; //index of names field in section headers table
} Elf32_EHdr;

typedef struct {
        Word sh_name;
        Word sh_type;
        Word sh_flags;
        Address sh_addr;
        Offset sh_offset;
        uint32_t sh_size;
        Word sh_link;
        Word sh_info;
        Word sh_addralign;
        Word sh_entsize;
} Elf32_SHdr;

typedef struct {
        Word p_type;
        Offset p_offset;
        Address p_vaddr;
        Address p_paddr;
        Word p_filesz;
        Word p_memsz;
        Word p_flags;
        Word p_align;
} Elf32_PHdr;

struct Elf32_Sym {
        uint32_t st_name;
        uint32_t st_value;
        uint32_t st_size;
        unsigned char st_info;
        unsigned char st_other;
        uint16_t st_shndx;
};

#define ELF32_ST_BIND(i)   ((i)>>4)
#define ELF32_ST_TYPE(i)   ((i)&0xf)
#define ELF32_ST_INFO(b,t) (((b)<<4)+((t)&0xf))

void *read_exec(const char *filename) {
        int fd = open(filename, O_RDONLY, 0);
        if (fd < 0) {
                dprintf(2, "File not found.\n");
                return nullptr;
        }

        const auto len = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);

        void *fbytes = kmalloc(len);
        if (!fbytes) {
                return nullptr;
        }

        const size_t n = read(fd, fbytes, len);
        if (n < 1) {
                kfree(fbytes);
                return nullptr;
        }

        return fbytes;
}

bool validate_elf(const void *fbytes) {
        const Elf32_EHdr *elf_header = fbytes;

        constexpr uint8_t magic[8] = {0x7f, 0x45, 0x4c, 0x46, 0x01, 0x01, 0x01, 0x00};
        if (memcmp(elf_header->e_ident, magic, 8) != 0) {
                dprintf(2, "Not an elf file. Magic number did not match.\n");
                return false;
        }

        if (elf_header->e_type != E_TYPE_EXEC) {
                dprintf(2, "File type not supported. Supported type is currently EXEC only.\n");
                return false;
        }

        if (elf_header->e_machine != E_MACHINE_ARM) {
                dprintf(2, "Machine not supported, only ARM!\n");
                return false;
        }

        if (elf_header->e_version != 1) {
                dprintf(2, "Version is not current.\n");
                return false;
        }

        return true;
}

int load_exec(const char *path) {
        void *fbytes = read_exec(path);
        if (!fbytes) {
                dprintf(2, "File could not be opened.\n");
                return -1;
        }

        if (!validate_elf(fbytes)) {
                goto cleanup_error;
        }

        const Elf32_EHdr *elf_header = fbytes;
        const Elf32_PHdr *phdr_table = fbytes + elf_header->e_phoff;
        const Elf32_SHdr *shdr_table = fbytes + elf_header->e_shoff;
        const Elf32_SHdr *shstrtab = &shdr_table[elf_header->e_shstrndx];

        size_t text_offset = 0;
        const struct Elf32_Sym *symtab = nullptr;
        size_t symtab_len = 0;
        const char *strtab = nullptr;
        int stubs_index = -1;

        printf("\nSection Headers:\n");
        printf("Indx %-20s Addr Offs  Size\n", "Section name");
        for (size_t i = 0; i < elf_header->e_shnum; ++i) {
                const Elf32_SHdr *shdr = &shdr_table[i];
                const char *section_name = (char *) (fbytes + shstrtab->sh_offset + shdr->sh_name);
                const uint32_t address = shdr->sh_addr;
                const uint32_t offset = shdr->sh_offset;
                const int size = shdr->sh_size;

                printf("[%2zu] %-20s 0x%02x 0x%03x %3i\n", i, section_name, address, offset, size);

                if (strcmp(section_name, ".text") == 0) {
                        text_offset = shdr->sh_offset;
                }
                else if (strncmp(section_name, ".text.stubs", strlen(".text.stubs")) == 0) {
                        stubs_index = i;
                }
                else if (strcmp(section_name, ".symtab") == 0) {
                        symtab = (struct Elf32_Sym *) (fbytes + shdr->sh_offset);
                        symtab_len = shdr->sh_size / sizeof(struct Elf32_Sym);
                }
                else if (strcmp(section_name, ".strtab") == 0) {
                        strtab = (char *) (fbytes + shdr->sh_offset);
                }
        }


        if (!text_offset || !symtab || !strtab) {
                dprintf(2, "ELF find not parsed properly.\n");
                goto cleanup_error;
        }


        constexpr int bufsz = 4096;
        unsigned char *buffer = kmalloc(bufsz);
        if (!buffer) {
                goto cleanup_error;
        }

        size_t index = 0;
        for (size_t i = 0; i < elf_header->e_phnum; ++i) {
                const Elf32_PHdr *phdr = &phdr_table[i];
                if (index + phdr->p_memsz >= bufsz) {
                        dprintf(2, "Not enough memory.\n");
                        kfree(buffer);
                        goto cleanup_error;
                }

                memcpy(buffer + index, fbytes + phdr->p_offset, phdr->p_filesz);

                index += phdr->p_memsz;
        }

        printf("\n.symtab:\n");
        printf("Indx Name\n");
        for (size_t i = 0; i < symtab_len; ++i) {
                const char *name;
                if (symtab[i].st_name == 0) {
                        const Elf32_SHdr *section = &shdr_table[i];
                        name = (const char *) (fbytes + shstrtab->sh_offset + section->sh_name);
                }
                else {
                        name = strtab + symtab[i].st_name;
                }

                printf("[%2zu] %s\n", i, name);

                if (ELF32_ST_TYPE(symtab[i].st_info) == SST_FUNC && symtab[i].st_shndx == stubs_index) {
                        const uint32_t dest_offset = symtab[i].st_value & 0xffff'fffe;

                        const uint32_t dyn_address_replacement = get_dyn(name);
                        memcpy(buffer + dest_offset + 4, (char *) &dyn_address_replacement, 4); //endianess?
                }
        }


        for (size_t i = 0; i < index; ++i) {
                printf("%02x ", buffer[i]);
        }

        kfree(fbytes);
        kfree(buffer);
        return 0;

cleanup_error:
        kfree(fbytes);
        return -1;
}