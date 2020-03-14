
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
#include "elf_int.h"

int main()
{
    return load_elf("g431.elf", "out.bin" , 1);
}
