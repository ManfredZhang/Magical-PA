#include "common.h"
#include "syscall.h"

extern int fs_open(const char *pathname, int flags, int mode);
extern ssize_t fs_read(int fd, void *buf, size_t len);
extern ssize_t fs_write(int fd, const void *buf, size_t len);
extern int fs_close(int fd);
extern off_t fs_lseek(int fd, off_t offset, int whence);

_RegSet* do_syscall(_RegSet *r) {
  uint32_t eeax = SYSCALL_ARG1(r);
  uint32_t eebx = SYSCALL_ARG2(r);
  uint32_t eecx = SYSCALL_ARG3(r);
  uint32_t eedx = SYSCALL_ARG4(r);
  
  //printf("eeax=%d\n",eeax);

  switch (eeax) {
	case SYS_none:
		//printf("none\n");
		SYSCALL_ARG1(r) = 1;
		break;
	case SYS_exit:
		//printf("exit\n");
		_halt(eebx);
		break;
	/*case SYS_write:
	{
		if (eebx == 1 || eebx == 2)
		{
			char *buf = (void*)eecx;
			int length = eedx;
			char ch;
			while (length--) 
			{
				ch = *(buf++);
				if (ch == '\0') 
					break;
				_putc(ch);
			}
			SYSCALL_ARG1(r) = length;
			//printf("\n");
		}
		break;
	}*/
	case SYS_write:
		SYSCALL_ARG1(r) = fs_write(eebx, (void *)eecx, eedx);
		break;
	case SYS_brk:
	{/*
		uint32_t increment = eebx;
		if (increment >= 0x8000000 || increment < 0x400533c)
			eeax = -1;
		else
		{
			_heap.end = (void*)increment;
		}*/
		SYSCALL_ARG1(r) = 0;
		break;
	}
	case SYS_open:
		SYSCALL_ARG1(r) = fs_open((char *)eebx, eecx, eedx);
		break;
	case SYS_read:
		SYSCALL_ARG1(r) = fs_read(eebx, (void *)eecx, eedx);
		break;
	case SYS_close:
		SYSCALL_ARG1(r) = fs_close(eebx);
		break;
	case SYS_lseek:
		SYSCALL_ARG1(r) = fs_lseek(eebx, eecx, eedx);
		break;
    default: panic("Unhandled syscall ID = %d", eeax);
  }

  return NULL;
}
