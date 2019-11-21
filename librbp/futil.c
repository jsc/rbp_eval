#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "futil.h"

int futil_is_writeable_dir(char * path) {
    struct stat st;
    uid_t euid;
    gid_t egid;
    int perm = 1;

    if (stat(path, &st) < 0) {
        return 0;
    }
    if (!S_ISDIR(st.st_mode)) {
        errno = EINVAL;
        return 0;
    }
    euid = geteuid();
    egid = getegid();

    if (st.st_uid == euid) {
        if (!(S_IWUSR & st.st_mode)) {
            perm = 0;
        }
    } else {
        if (st.st_gid == egid) {
            if (!(S_IWGRP & st.st_mode)) {
                perm = 0;
            }
        } else {
            if (!(S_IWOTH & st.st_mode)) {
                perm = 0;
            }
        }
    }
    if (perm == 0) {
        errno = EACCES;
        return 0;
    }

    return 1;
}

int futil_is_readable_file(char * path) {
    struct stat st;
    uid_t euid;
    gid_t egid;
    int perm = 1;

    if (stat(path, &st) < 0) {
        return 0;
    }
    if (S_ISDIR(st.st_mode)) {
        errno = EISDIR;
        return 0;
    }
    euid = geteuid();
    egid = getegid();

    if (st.st_uid == euid) {
        if (!(S_IRUSR & st.st_mode)) {
            perm = 0;
        }
    } else {
        if (st.st_gid == egid) {
            if (!(S_IRGRP & st.st_mode)) {
                perm = 0;
            }
        } else {
            if (!(S_IROTH & st.st_mode)) {
                perm = 0;
            }
        }
    }
    if (perm == 0) {
        errno = EACCES;
        return 0;
    }

    return 1;
}


