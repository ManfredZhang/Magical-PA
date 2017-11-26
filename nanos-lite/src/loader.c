#include "common.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();
extern int fs_open(const char *pathname, int flags, int mode);
extern ssize_t fs_read(int fd, void *buf, size_t len);
extern size_t fs_filesz(int fd);
extern int fs_close(int fd);

//调用loader()函数加载用户程序, 函数会返回用户程序的入口地址.
//目前的loader只需要做一件事情: 将ramdisk中从0开始的所有内容放置在 0x4000000 , 并把这个地址作为程序的入口返回即可. 我们把这个简化了的loader称为raw program loader.
uintptr_t loader(_Protect *as, const char *filename) {
  //TODO();
  //ramdisk_read(DEFAULT_ENTRY, 0, get_ramdisk_size());
  int fd = fs_open(filename, 0, 0);
  fs_read(fd, DEFAULT_ENTRY, fs_filesz(fd));									
  fs_close(fd);

  return (uintptr_t)DEFAULT_ENTRY;
}
