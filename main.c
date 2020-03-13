#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#define ELF_DATA_MAX_SIZE   1024*2048
#define OUT_DATA_MAX_SIZE   1024*128

#define MAX_SECTION         32
//https://cirosantilli.com/elf-hello-world
#define SHT_NULL	0
#define SHT_PROGBITS	1
#define SHT_SYMTAB	2
#define SHT_STRTAB	3
#define SHT_RELA	4
#define SHT_HASH	5
#define SHT_DYNAMIC	6
#define SHT_NOTE	7
#define SHT_NOBITS	8
#define SHT_REL		9
#define SHT_SHLIB	10
#define SHT_DYNSYM	11
#define SHT_NUM		12
#define SHT_LOPROC	0x70000000
#define SHT_HIPROC	0x7fffffff
#define SHT_LOUSER	0x80000000
#define SHT_HIUSER	0xffffffff

#define     EI_NIDENT 16
/* 32-bit ELF base types. */
typedef unsigned int	Elf32_Addr;
typedef unsigned short	Elf32_Half;
typedef unsigned int	Elf32_Off;
typedef unsigned int	Elf32_Sword;
typedef unsigned int    Elf32_Word;

typedef struct elf32_hdr
{
  unsigned char	e_ident[EI_NIDENT];
  Elf32_Half	e_type;
  Elf32_Half	e_machine;
  Elf32_Word	e_version;
  Elf32_Addr	e_entry;  /* Entry point */
  Elf32_Off	    e_phoff;
  Elf32_Off	    e_shoff;
  Elf32_Word	e_flags;
  Elf32_Half	e_ehsize;
  Elf32_Half	e_phentsize;
  Elf32_Half	e_phnum;
  Elf32_Half	e_shentsize;
  Elf32_Half	e_shnum;
  Elf32_Half	e_shstrndx;
} Elf32_Ehdr;

typedef struct elf32_shdr
{
  Elf32_Word	sh_name;
  Elf32_Word	sh_type;
  Elf32_Word	sh_flags;
  Elf32_Addr	sh_addr;
  Elf32_Off	    sh_offset;
  Elf32_Word	sh_size;
  Elf32_Word	sh_link;
  Elf32_Word	sh_info;
  Elf32_Word	sh_addralign;
  Elf32_Word	sh_entsize;
} Elf32_Shdr;


unsigned char file_data[ELF_DATA_MAX_SIZE], out_data[OUT_DATA_MAX_SIZE];
Elf32_Ehdr      *elf_header;
Elf32_Shdr      *elf_sections[32];

int main()
{
FILE *fp;
int len,i;
char    filename[64];
unsigned char section_name[256];
unsigned int addr, offset  , entaddr , initial_shoffset , shoffset , shsize;

    sprintf(filename,"g431.elf");
    fp = fopen(filename,"rb");
    if ( fp )
    {
        len = fread(file_data,1,ELF_DATA_MAX_SIZE,fp);
        fclose(fp);
    }
    else
    {
        printf("Could not open file %s for reading\n", filename);
        return 0;
    }

    printf("%s is %d bytes long\n", filename , len);
    elf_header = (Elf32_Ehdr *)file_data;

    printf("e_ident             0x%02x %c%c%c\n", elf_header->e_ident[0],elf_header->e_ident[1],elf_header->e_ident[2],elf_header->e_ident[3]);
    printf("e_type              0x%04x\n", elf_header->e_type );
    printf("e_machine           0x%04x\n", elf_header->e_machine );
    printf("e_version           0x%08x\n", elf_header->e_version );
    printf("e_entry             0x%08x\n", elf_header->e_entry );
    printf("e_phoff             0x%08x\n", elf_header->e_phoff );
    printf("e_shoff             0x%08x ( %d )\n", elf_header->e_shoff , elf_header->e_shoff );
    printf("e_flags             0x%08x\n", elf_header->e_flags );
    printf("e_ehsize            0x%04x ( %d bytes )\n", elf_header->e_ehsize, elf_header->e_ehsize );
    printf("e_phentsize         0x%04x\n", elf_header->e_phentsize );
    printf("e_phnum             0x%04x\n", elf_header->e_phnum );
    printf("e_shentsize         0x%04x\n", elf_header->e_shentsize );
    printf("e_shnum             0x%04x ( %d )\n", elf_header->e_shnum, elf_header->e_shnum );
    printf("e_shstrndx          0x%04x ( %d )\n", elf_header->e_shstrndx, elf_header->e_shstrndx );
    printf("Section header table starts @ 0x%08x , %d entries each %d bytes long\n",elf_header->e_shoff,elf_header->e_shnum,elf_header->e_shentsize);

    for(i=0;i<elf_header->e_shnum;i++)
        elf_sections[i] = (Elf32_Shdr *)(file_data+elf_header->e_shoff+(i*elf_header->e_shentsize));

    addr = elf_sections[elf_header->e_shstrndx]->sh_addr;
    offset = elf_sections[elf_header->e_shstrndx]->sh_offset;
    shoffset = shsize = 0;
    initial_shoffset = 0xffffffff;
    for(i=0;i<elf_header->e_shnum;i++)
    {
        if (( elf_sections[i]->sh_addr & 0x08000000) == 0x08000000)
        {
            entaddr = &file_data[0]+addr+offset+elf_sections[i]->sh_name; // pointer to name
            memcpy(section_name,(unsigned char*)entaddr,elf_sections[elf_header->e_shstrndx]->sh_size);
            printf("****** Section %d : %s ***********\n",i,section_name);
            printf("sh_name         0x%08x\n", elf_sections[i]->sh_name );
            printf("sh_type         0x%08x\n", elf_sections[i]->sh_type );
            printf("sh_flags        0x%08x\n", elf_sections[i]->sh_flags );
            printf("sh_addr         0x%08x\n", elf_sections[i]->sh_addr );
            printf("sh_offset       0x%08x\n", elf_sections[i]->sh_offset );
            printf("sh_size         0x%08x\n", elf_sections[i]->sh_size );
            printf("sh_link         0x%08x\n", elf_sections[i]->sh_link );
            printf("sh_info         0x%08x\n", elf_sections[i]->sh_info );
            printf("sh_addralign    0x%08x\n", elf_sections[i]->sh_addralign );
            printf("sh_entsize      0x%08x\n\n", elf_sections[i]->sh_entsize );
            if ( initial_shoffset == 0xffffffff)
                initial_shoffset = elf_sections[i]->sh_offset;
            shoffset = elf_sections[i]->sh_offset + elf_sections[i]->sh_size;
        }
    }
    printf ("initial_shoffset = 0x%08x shoffset = 0x%08x total %d bytes\n" , initial_shoffset,shoffset,shoffset-initial_shoffset);
    printf ("from file = offset 0x%08x number of bytes %d\n" , &file_data[0]+initial_shoffset,shoffset-initial_shoffset);
    memcpy(out_data, &file_data[0]+initial_shoffset,shoffset-initial_shoffset);
    fp = fopen("out.bin","wb");
    if ( !fp )
    {
        printf("File out.bin can't be opened\n");
        return -1;
    }
    fwrite(out_data,1,shoffset-initial_shoffset,fp);
    fclose(fp);
    return 0;
}
