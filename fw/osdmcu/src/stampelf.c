/*
    DRACO - Copyright (C) 2013-2014 Daniel Strnad

    This file is part of DRACO project.

    DRACO is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    DRACO is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

/**
 * @file    stampelf.c
 * @brief   ELF CRC stamping utility
 *
 */

/*
This program will stamp elf binary with CRC, application size and timestamp
so final ROM image (done by objcopy) will look according prescription below.

!!! Please note, that every fixed size section should be already
defined in linker script and pre-filled with some dummy stuff
of the same size. Name of these sections should be adjusted below!!!

Cortex-M3 executable image format (parsed by target bootloader)

  EEDOFF + 40   +---------------------+
                |        CRC          |
  EEDOFF + 36   +---------------------+
                |      Timestamp      |
  EEDOFF + 32   +---------------------+
                |   Version string    |
  EEDOFF        +---------------------+
                |                     |
                |                     |
                |   Executable code   |
                |      and data       |
                |                     |
                |                     |
  BOFF + 516    +---------------------+
                |  Application size   |
  BOFF + 512    +---------------------+
                | Cortex ISR vectors  |
  BOFF          +---------------------+

BOFF: Begin OFFset - should be the end address of bootloader + 512 B alignment(min. alignment for SCB->VTORS)
EEDOFF: End of Executable and Data OFFset - variable depending on size of application code
Cortex ISR Vectors: interrupt vectors for Cortex-M3 (fixed size field 512B)
Application size: size of whole image incl. ISR vectors and CRC (fixed size field 4B, little endian)
Timestamp: time of image generation in UNIX time (fixed size field 4B, little endian)
Version string: application version string ended by 0x00 (fixed size field 32B)
CRC: CRC-32 polynom 0x04C11DB7, initial value 0xFFFFFFFF (fixed size field 4B, little endian)
     cover whole image except CRC itself

*/

#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

// for adjusting
const char vectSectionName[] = ".vectors";
const char sizeSectionName[] =".app_size";
const char timeStampSectionName [] = ".app_time_stamp";
const char crcSectionName[] = ".app_crc";

#define APP_SIZE_OFFSET         512
#define APP_TIMESTAMP_E_OFFSET  8
#define APP_CRC_E_OFFSET        4

//#define VERBOSE

#define PERROR(...) {fprintf(stderr, "Error: " __VA_ARGS__);fflush(stderr);}
#define PSTD(...) {fprintf(stdout, __VA_ARGS__);fflush(stdout);}

#ifdef VERBOSE
    #define PVERB PSTD
#else
    #define PVERB(...)
#endif

char *filename;
uint8_t *fcontent = NULL;
uint8_t *conSections = NULL;
uint32_t conSectionsLen = 0;
int fsize = 0;

// ELF definitions
static const char elf_ei_magic[] = {0x7f, 'E', 'L', 'F'};

#define ELF_ET_EXEC                 0x02
#define ELF_EM_ARM                  0x28

#define ELF_PT_LOAD                 0x01
#define ELF_SHN_UNDEF               0x00
#define ELF_SHT_PROGBITS            0x01

#define ELF_OFF_E_TYPE              0x00000010
#define ELF_OFF_E_MACHINE           0x00000012
#define ELF_OFF_E_PHOFF             0x0000001C
#define ELF_OFF_E_SHOFF             0x00000020
#define ELF_OFF_E_PHENTSIZE         0x0000002A
#define ELF_OFF_E_PHNUM             0x0000002C
#define ELF_OFF_E_SHENTSIZE         0x0000002E
#define ELF_OFF_E_SHNUM             0x00000030
#define ELF_OFF_E_SHSTRNDX          0x00000032

#define ELF_OFF_P_TYPE              0x00000000
#define ELF_OFF_P_OFFSET            0x00000004
#define ELF_OFF_P_VADDR             0x00000008
#define ELF_OFF_P_PADDR             0x0000000C
#define ELF_OFF_P_FILESZ            0x00000010
#define ELF_OFF_P_MEMSZ             0x00000014
#define ELF_OFF_P_FLAGS             0x00000018
#define ELF_OFF_P_ALIGN             0x0000001C

#define ELF_OFF_SH_NAME             0x00000000
#define ELF_OFF_SH_TYPE             0x00000004
#define ELF_OFF_SH_FLAGS            0x00000008
#define ELF_OFF_SH_ADDR             0x0000000C
#define ELF_OFF_SH_OFFSET           0x00000010
#define ELF_OFF_SH_SIZE             0x00000014
#define ELF_OFF_SH_LINK             0x00000018
#define ELF_OFF_SH_INFO             0x0000001C
#define ELF_OFF_SH_ADDRALIGN        0x00000020
#define ELF_OFF_SH_ENTSIZE          0x00000024

#define CON_SECTIONS_ALLOC          1048576

typedef struct elfHeaderStruct
{
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} tElfHeader;

typedef struct elfProgHeaderStruct
{
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} tElfProgHeader;

typedef struct elfSecHeaderStruct
{
    uint32_t sh_name;
    uint32_t sh_type;
    uint32_t sh_flags;
    uint32_t sh_addr;
    uint32_t sh_offset;
    uint32_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint32_t sh_addralign;
    uint32_t sh_entsize;
} tElfSecHeader;

tElfHeader elfHeader;
tElfSecHeader elfStrSecHeader;
tElfSecHeader elfVectSecHeader;
tElfSecHeader elfCrcSecHeader;
tElfSecHeader elfSizeSecHeader;
tElfSecHeader elfTimeStampSecHeader;

tElfSecHeader *sortedSHeaders = 0;
tElfProgHeader *sortedPHeaders = 0;

#define STR_BUFF_SIZE   128
char strbuff[STR_BUFF_SIZE];

int fileToMem(const char *filename, uint8_t **content)
{
    int size = 0;
    FILE *f = fopen(filename, "rb");
    if (f == NULL)
    {
        *content = NULL;
        PERROR("fileToMem: Can't fopen()! =%d\n", errno);
        return -1;
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    *content = (uint8_t *)malloc(size);
    if (*content  == NULL)
    {
        PERROR("Can't malloc()! =%d\n", *content);
    }
    if (size != fread(*content, sizeof(char), size, f))
    {
        free(*content);
        PERROR("Can't fread()!\n");
        fclose(f);
        return -2; // -2 means file reading fail
    }
    fclose(f);
    return size;
}

int memToFile(const char *filename, uint8_t **content, uint32_t size)
{
    uint32_t res = 0;
    FILE *f = fopen(filename, "wb");
    if (f == NULL)
    {
        PERROR("memToFile: Can't fopen()! = %d \n", errno);
        return -1;
    }
    res = fwrite(*content, 1, size, f);
    if (res != size) {
        PERROR("memToFile: Can't fwrite()! = %d\n", errno);
        fclose(f);
        return -2;
    }
    fclose(f);
    return res;
}

void cleanup()
{
    if (fcontent != NULL) free(fcontent);
    if (sortedSHeaders != 0) free (sortedSHeaders);
    if (conSections != 0) free (conSections);
    if (sortedPHeaders != 0) free(sortedPHeaders);
}

uint32_t _CRC32(const uint32_t oldCRC, const uint8_t new_byte)
{
    uint32_t  crc;
    uint8_t i;

    crc = oldCRC ^ new_byte;
    i = 8;
    do {
        if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320L; // CRC-32 reverse polynom
        else crc = crc >> 1;
    } while(--i);
    return (crc);
}

uint32_t CalcCRC32(uint8_t *block, const uint32_t count)
{
  uint32_t crc=0xffffffff;
  uint32_t cnt=count;
  uint8_t *uk;
  uk=block;
  while(cnt--) {
      crc=_CRC32(crc,*uk++);
  }
  crc ^= 0xffffffff;
  return(crc);
}

// Read file content little endian 32-bit
uint32_t readLE32(uint32_t offset)
{
    uint32_t result = 0;
    if ((offset+4) > fsize)
    {
        PERROR("Unexpected end of file\n");
        cleanup();
        exit(-1);
        return 0;
    }
    result = fcontent[offset];
    result |= fcontent[offset+1] << 8;
    result |= fcontent[offset+2] << 16;
    result |= fcontent[offset+3] << 24;
    return result;
}

// Read file content little endian 16-bit
uint16_t readLE16(uint32_t offset)
{
    uint16_t result = 0;
    if ((offset+2) > fsize)
    {
        PERROR("Unexpected end of file\n");
        cleanup();
        exit(-1);
        return 0;
    }
    result = fcontent[offset];
    result |= fcontent[offset+1] << 8;
    return result;
}

// Read file content 8-bit
uint8_t readLE8(uint32_t offset)
{
    uint8_t result = 0;
    if ((offset+1) > fsize)
    {
        PERROR("Unexpected end of file\n");
        cleanup();
        exit(-1);
        return 0;
    }
    result = fcontent[offset];
    return result;
}

// put LE 32-bit value to the desired memory positon with check
void putLE32(uint8_t *p, uint32_t pSize, uint32_t offset, uint32_t value)
{
    if ((offset+4) > pSize)
    {
        PERROR("Unexpected end of file\n");
        cleanup();
        exit(-1);
    }

    p[offset] =   (uint8_t) (value & 0xff);
    p[offset+1] = (uint8_t) ((value & 0xff00) >> 8);
    p[offset+2] = (uint8_t) ((value & 0xff0000) >> 16);
    p[offset+3] = (uint8_t) ((value & 0xff000000) >> 24);
}

void elfGetHeader(tElfHeader *header)
{
    header->e_type = readLE16(ELF_OFF_E_TYPE);
    header->e_machine = readLE16(ELF_OFF_E_MACHINE);
    header->e_phoff = readLE32(ELF_OFF_E_PHOFF);
    header->e_shoff = readLE32(ELF_OFF_E_SHOFF);
    header->e_phentsize = readLE16(ELF_OFF_E_PHENTSIZE);
    header->e_phnum = readLE16(ELF_OFF_E_PHNUM);
    header->e_shentsize = readLE16(ELF_OFF_E_SHENTSIZE);
    header->e_shnum = readLE16(ELF_OFF_E_SHNUM);
    header->e_shstrndx = readLE16(ELF_OFF_E_SHSTRNDX);

    PVERB("===ELF header===\n");
    PVERB("e_type: 0x%x\n", header->e_type);
    PVERB("e_machine: 0x%x\n", header->e_machine);
    PVERB("e_phoff: 0x%x\n", header->e_phoff);
    PVERB("e_shoff: 0x%x\n", header->e_shoff);
    PVERB("e_phentsize: 0x%x\n", header->e_phentsize);
    PVERB("e_phnum: 0x%x\n", header->e_phnum);
    PVERB("e_shentsize: 0x%x\n", header->e_shentsize);
    PVERB("e_shnum: 0x%x\n", header->e_shnum);
    PVERB("e_shstrndx: 0x%x\n", header->e_shstrndx);
}

void elfGetProgHeader(tElfProgHeader *progHeader, tElfHeader *elfHeader, uint16_t index)
{
    uint32_t offset = elfHeader->e_phoff + (elfHeader->e_phentsize * index);
    progHeader->p_type = readLE32(offset + ELF_OFF_P_TYPE);
    progHeader->p_offset = readLE32(offset + ELF_OFF_P_OFFSET);
    progHeader->p_vaddr = readLE32(offset + ELF_OFF_P_VADDR);
    progHeader->p_paddr = readLE32(offset + ELF_OFF_P_PADDR);
    progHeader->p_filesz = readLE32(offset + ELF_OFF_P_FILESZ);
    progHeader->p_memsz = readLE32(offset + ELF_OFF_P_MEMSZ);
    progHeader->p_flags = readLE32(offset + ELF_OFF_P_FLAGS);
    progHeader->p_align = readLE32(offset + ELF_OFF_P_ALIGN);

    PVERB("===program header %u===\n", index);
    PVERB("Base offset: 0x%x\n", offset);
    PVERB("p_type: 0x%x\n", progHeader->p_type);
    PVERB("p_offset: 0x%x\n", progHeader->p_offset);
    PVERB("p_vaddr: 0x%x\n", progHeader->p_vaddr);
    PVERB("p_paddr: 0x%x\n", progHeader->p_paddr);
    PVERB("p_filesz: 0x%x\n", progHeader->p_filesz);
    PVERB("p_memsz: 0x%x\n", progHeader->p_memsz);
    PVERB("p_align: 0x%x\n", progHeader->p_align);
}

void elfGetSecHeader(tElfSecHeader *secHeader, tElfHeader *elfHeader, uint16_t index)
{
    uint32_t offset = elfHeader->e_shoff + (elfHeader->e_shentsize * index);
    secHeader->sh_name = readLE32(offset + ELF_OFF_SH_NAME);
    secHeader->sh_type = readLE32(offset + ELF_OFF_SH_TYPE);
    secHeader->sh_flags = readLE32(offset + ELF_OFF_SH_FLAGS);
    secHeader->sh_addr = readLE32(offset + ELF_OFF_SH_ADDR);
    secHeader->sh_offset = readLE32(offset + ELF_OFF_SH_OFFSET);
    secHeader->sh_size = readLE32(offset + ELF_OFF_SH_SIZE);
    secHeader->sh_link = readLE32(offset + ELF_OFF_SH_LINK);
    secHeader->sh_info = readLE32(offset + ELF_OFF_SH_INFO);
    secHeader->sh_addralign = readLE32(offset + ELF_OFF_SH_ADDRALIGN);
    secHeader->sh_entsize = readLE32(offset + ELF_OFF_SH_ENTSIZE);

    PVERB("===Section header %u===\n", index);
    PVERB("Base offset: 0x%x\n", offset);
    PVERB("e_shoff = 0x%x\n", elfHeader->e_shoff);
    PVERB("e_shentsize = 0x%x\n", elfHeader->e_shentsize);
    PVERB("sh_name: 0x%x\n", secHeader->sh_name);
    PVERB("sh_type: 0x%x\n", secHeader->sh_type);
    PVERB("sh_flags: 0x%x\n", secHeader->sh_flags);
    PVERB("sh_addr: 0x%x\n", secHeader->sh_addr);
    PVERB("sh_offset: 0x%x\n", secHeader->sh_offset);
    PVERB("sh_size: 0x%x\n", secHeader->sh_size);
    PVERB("sh_link: 0x%x\n", secHeader->sh_link);
    PVERB("sh_info: 0x%x\n", secHeader->sh_info);
    PVERB("sh_addralign: 0x%x\n", secHeader->sh_addralign);
    PVERB("sh_entsize: 0x%x\n", secHeader->sh_entsize);
}

char *elfGetSectionName(char *buff, uint16_t buffSize ,tElfSecHeader *stringSecHeader,  uint16_t strindex)
{
    uint32_t offset = stringSecHeader->sh_offset;
    uint8_t i;
    #define MIN(x,y) ((y)>(x))?x:y
    if (strindex > stringSecHeader->sh_size-1) {
        PVERB("String index out of string table!\n");
        cleanup();
        exit(-1);
    }
    strncpy(buff, &fcontent[offset + strindex], MIN(stringSecHeader->sh_size - strindex, buffSize));
    return buff;
}

int main(int argc, char *argv[])
{
    uint32_t i = 0;
    if (argc<2) {
        PERROR("No file given!\n");
        PERROR("usage: stampelf filename.elf\n");
        cleanup();
        exit(-1);
    }

    filename = argv[1];
    PSTD("Stamping %s\n", filename);
    fsize=fileToMem(filename, &fcontent);
    if (fsize<4) {
        PERROR("Cant't fileToMem()!\n");
        cleanup();
        exit(-1);
    }

    // test for ELF magic sequence
    if (memcmp(fcontent, elf_ei_magic, 4) != 0)
    {
        PERROR("Not valid elf file!\n");
        cleanup();
        exit(-1);
    }
    elfGetHeader(&elfHeader);

    // test for executable ARM32 ELF type
    if ((elfHeader.e_type != ELF_ET_EXEC) || (elfHeader.e_machine != ELF_EM_ARM))
    {
        PERROR("Not executable ARM32 ELF!");
        cleanup();
        exit(-1);
    }

    /* !!! search for all LOAD memory segments, we will assume
     * segments are already sorted in ELF from the lowest LMA
     * to the highest one !!!
     */

    uint8_t segFound = 0;
    sortedPHeaders = malloc(sizeof(tElfProgHeader) * elfHeader.e_phnum);
    for (i=0; i < elfHeader.e_phnum; i++)
    {
        elfGetProgHeader(&sortedPHeaders[i], &elfHeader, i);
        if (sortedPHeaders[i].p_type == ELF_PT_LOAD) {
            segFound = 1;
        }
    }
    if (!segFound)
    {
        PERROR("No LOAD program segment header found :(\n");
        cleanup();
        exit(-1);
    }

    // find string table section header
    if (elfHeader.e_shstrndx == ELF_SHN_UNDEF)
    {
        PERROR("No string table section header in ELF!\n");
        cleanup();
        exit(-1);
    }
    elfGetSecHeader(&elfStrSecHeader, &elfHeader, elfHeader.e_shstrndx);

    // find vector, crc, appsize and timestamp section headers
    uint8_t vectHeaderFound=0;
    uint8_t crcHeaderFound=0;
    uint8_t sizeHeaderFound=0;
    uint8_t timeStampHeaderFound=0;
    for (i=1; i < elfHeader.e_shnum; i++)
    {
        tElfSecHeader secHeader;
        elfGetSecHeader(&secHeader, &elfHeader, i);
        elfGetSectionName(strbuff, STR_BUFF_SIZE, &elfStrSecHeader, secHeader.sh_name);
        PVERB("Section name: %s\n", strbuff);

        if (secHeader.sh_type != ELF_SHT_PROGBITS) continue;
        if (strncmp(strbuff, vectSectionName, STR_BUFF_SIZE) ==  0) {
            PVERB("Application ISR vectors section found\n");
            vectHeaderFound = 1;
            elfVectSecHeader = secHeader;
        }
        if (strncmp(strbuff, crcSectionName, STR_BUFF_SIZE) ==  0) {
            PVERB("Application CRC section found\n");
            crcHeaderFound = 1;
            elfCrcSecHeader = secHeader;
        }
        if (strncmp(strbuff, sizeSectionName, STR_BUFF_SIZE) ==  0) {
            PVERB("Application size section found\n");
            sizeHeaderFound = 1;
            elfSizeSecHeader = secHeader;
        }
        if (strncmp(strbuff, timeStampSectionName, STR_BUFF_SIZE) ==  0) {
            PVERB("Application CRC section found\n");
            timeStampHeaderFound = 1;
            elfTimeStampSecHeader = secHeader;
        }
    }

    if (!((crcHeaderFound)&&(vectHeaderFound)&&(timeStampHeaderFound)&&(sizeHeaderFound)))
    {
        PERROR("Required sections not found! \n" \
                "This utility may require to adjust, read description!");
        cleanup();
        exit(-1);
    }

    /*
     * Sort section headers according their address
     * from the lowest to the highest one
     */
    sortedSHeaders = malloc(sizeof(tElfSecHeader) * elfHeader.e_shnum);
    for (i=0; i < elfHeader.e_shnum; i++) {
        elfGetSecHeader(&sortedSHeaders[i], &elfHeader, i);
    }
    uint8_t sorted = 0;
    do {
        sorted = 1;
        for(i=0;i < (elfHeader.e_shnum-1); i++) {
            if (sortedSHeaders[i].sh_addr > sortedSHeaders[i+1].sh_addr) {
                sorted = 0;
                tElfSecHeader tmp;
                tmp = sortedSHeaders[i];
                sortedSHeaders[i] = sortedSHeaders[i+1];
                sortedSHeaders[i+1] = tmp;
            }
        }
    } while (!sorted);

    /*
     * Concentate content of all sections
     * in PT_LOAD program headers together at their LMA's...
     * conSections then should contain the same
     * as output of objcopy -O binary file.elf output.bin
     */

    conSections = malloc(CON_SECTIONS_ALLOC);
    conSectionsLen = 0;
    for (i=0;i<CON_SECTIONS_ALLOC;i++) conSections[i] = 0x00;
    #define IN_RANGE(a,x,y) ((((a) >= (x)) && ((a)<=(y)))?1:0)

    uint32_t firstLma = 0;
    for (i=0;i<elfHeader.e_phnum; i++) {
        // iterate LOAD non-empty segment headers
        if ((sortedPHeaders[i].p_type != ELF_PT_LOAD) || (sortedPHeaders[i].p_filesz == 0)) {
            continue;
        }
        uint32_t j;
        for (j=0;j<elfHeader.e_shnum;j++) {
            if (IN_RANGE(sortedSHeaders[j].sh_addr, sortedPHeaders[i].p_vaddr, (sortedPHeaders[i].p_vaddr + sortedPHeaders[i].p_filesz - 1))) {
                if (!firstLma) firstLma = sortedPHeaders[i].p_paddr + (sortedSHeaders[j].sh_addr - sortedPHeaders[i].p_vaddr);
                uint32_t k;
                for (k=0;k<sortedSHeaders[j].sh_size;k++) {
                    conSectionsLen = (sortedSHeaders[j].sh_addr - sortedPHeaders[i].p_vaddr) + (sortedPHeaders[i].p_paddr - firstLma) + k;
                    uint32_t elfOffset = k + sortedSHeaders[j].sh_offset;
                    if (conSectionsLen > (CON_SECTIONS_ALLOC - 1)) {
                        PERROR("CON_SECTIONS_ALLOC exceeded!!!\n");
                        cleanup();
                        exit(-1);
                    }
                    conSections[conSectionsLen] = fcontent[elfOffset];
                }
            }
        }
    }
    conSectionsLen++;
    //memToFile("consections.bin", &conSections, conSectionsLen);

    // stamp with application size
    uint32_t appSize;
    appSize = conSectionsLen;

    PSTD("Stamping app size to 0x%x\n", appSize);
    putLE32(fcontent, fsize, elfSizeSecHeader.sh_offset, appSize);
    putLE32(conSections, conSectionsLen, APP_SIZE_OFFSET ,appSize);

    // stamp with actual time
    time_t timeStamp = time(NULL);
    PSTD("Stamping app time to %s", ctime(&timeStamp));
    putLE32(fcontent, fsize, elfTimeStampSecHeader.sh_offset,  (uint32_t)timeStamp);
    putLE32(conSections, conSectionsLen,conSectionsLen - APP_TIMESTAMP_E_OFFSET, (uint32_t)timeStamp);

    // stamp with crc
    uint32_t crc = CalcCRC32(conSections, appSize - APP_CRC_E_OFFSET);
    PSTD("Stamping app crc to 0x%00000000x\n", crc);
    putLE32(fcontent, fsize, elfCrcSecHeader.sh_offset, crc);

    // save back to file.
    if (memToFile(filename, &fcontent, fsize) < 0)
    {
        PERROR("Can't write to %s\n", filename);
        cleanup();

        exit(-1);
    }
    cleanup();
    return 0;
}

