#include "fs.h"

extern void ramdisk_write(const void *buf, off_t offset, size_t len);
extern void ramdisk_read(void *buf, off_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;  // 文件在ramdisk中的偏移
  off_t open_offset;  // 文件被打开之后的读写指针
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

int fs_open(const char *pathname, int flags, int mode)
{//printf("open\n");
	//open(), openat(), and creat() return the new file  descriptor,  or  -1  if  anerror occurred (in which case, errno is set appropriately).
	for (int i = 0; i < NR_FILES; i++)
	{
		//printf("open\n");
		if (strcmp(file_table[i].name, pathname) == 0)
			return i;
	}
	return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len)
{//printf("read\n");
	//printf("read\nfd=%d\nlen=%d\n", fd, len);
	//printf("file_table[fd].open_offset=%d\n", file_table[fd].open_offset);
	//printf("file_table[fd].size=%d\n", file_table[fd].size);
	//printf("file_table[fd].disk_offset=%d\n", file_table[fd].disk_offset);

	switch (fd)
	{
		case FD_STDIN:
		case FD_STDOUT:
		case FD_STDERR:
			len = 0;
			break;
		case FD_FB:
		case FD_EVENTS:
		case FD_DISPINFO:
			TODO();
			break;
		default:
			if (file_table[fd].open_offset >= file_table[fd].size)
				return -1;
			else if(file_table[fd].open_offset + len > file_table[fd].size)
				len = file_table[fd].size - file_table[fd].open_offset;
			ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
	}
	//printf("readover\n");
	return len;
}

ssize_t fs_write(int fd, const void *buf, size_t len)
{//printf("write\n");
	switch (fd)
	{
		case FD_STDIN:
		case FD_STDOUT:
		case FD_STDERR:
			for (int i = 0; i < len; i++)
				_putc(*((char*)buf++));
			break;
		case FD_FB:
		case FD_EVENTS:
		case FD_DISPINFO:
			TODO();
			break;
		default:
			if (file_table[fd].open_offset >= file_table[fd].size)
				return -1;
			else if (file_table[fd].open_offset + len > file_table[fd].size)
				len = file_table[fd].size - file_table[fd].open_offset;
			ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
	}
	return len;
}

off_t fs_lseek(int fd, off_t offset, int whence)
{//printf("lseek\n");
	int ret = -1;
	switch(fd) {
		case FD_STDIN:
		case FD_STDOUT:
		case FD_STDERR:
		case FD_FB:
		case FD_EVENTS:
		case FD_DISPINFO:
			TODO();
			break;
		default:
			switch(whence) {
				case SEEK_SET:
					if (offset <= file_table[fd].size && offset >= 0) {
						file_table[fd].open_offset = offset;
						ret = file_table[fd].open_offset;
					}
					break;
				case SEEK_CUR:
					if((offset + file_table[fd].open_offset <= file_table[fd].size) && offset + file_table[fd].open_offset >= 0) {
						file_table[fd].open_offset = offset + file_table[fd].open_offset;
						ret = file_table[fd].open_offset;
					}
					break;
				case SEEK_END:
					if(1) {
						file_table[fd].open_offset = offset + file_table[fd].size;
						ret = file_table[fd].open_offset;
					}
					break;
			}
	}
	return (off_t)ret;
}

int fs_close(int fd)
{
	return 0;
}

size_t fs_filesz(int fd) {
	return file_table[fd].size;
}
