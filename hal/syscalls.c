 #include <errno.h>
 #include <sys/stat.h>
 
/*int _read(int file, char *ptr, int len) 
{
     return 0;
}*/

int _fstat(int file, struct stat *st)
{
    (void)file;
    st->st_mode = S_IFCHR; // character device
    return 0;
}

int _fstat_r(void *reent, int file, struct stat *st) 
{
     return _fstat(file, st);
}

int _close(int file)
{
    (void)file;
    return -1; // always fail
}

int _close_r(void *reent, int file) 
{
     return _close(file);
}

int _isatty(int file)
{
    (void)file;
    return 1; // yes, it's a tty
}


int _lseek(int file, int ptr, int dir)
{
    (void)file;
    (void)ptr;
    (void)dir;
    return -1; // always fail (no seeking on TTY)
}

int _getpid(void)
{
    return 1; // single process
}

int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    return -1; // always fail (no signal support)
}


