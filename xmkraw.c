/*
 *  Copyright (c) 2014, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in
 *  the LICENSE file in the root directory of this source tree. An
 *  additional grant of patent rights can be found in the PATENTS file
 *  in the same directory.
 *
 */
#include <termios.h>
#include <errno.h>
#include "xmkraw.h"
#include "util.h"

struct xmkraw_save {
    int fd;
    struct termios attr;
};

void
xtcgetattr(int fd, struct termios* attr)
{
    int ret;

    do {
        ret = tcgetattr(fd, attr);
    } while (ret == -1 && errno == EINTR);

    if (ret < 0)
        die_errno("tcgetattr(%d)", fd);
}

void
xtcsetattr(int fd, struct termios* attr)
{
    int ret;

    do {
        ret = tcsetattr(fd, TCSADRAIN, attr);
    } while (ret == -1 && errno == EINTR);

    if (ret < 0)
        die_errno("tcsetattr(%d)", fd);
}

static void
xmkraw_cleanup(void* arg)
{
    struct xmkraw_save* save = arg;
    int ret;
    do {
        ret = tcsetattr(save->fd, TCSADRAIN, &save->attr);
    } while (ret == -1 && errno == EINTR);
    close(save->fd);
}

void
xmkraw(int fd, unsigned flags)
{
    struct termios attr;
    xtcgetattr(fd, &attr);

    if ((flags & XMKRAW_SKIP_CLEANUP) == 0) {
        struct xmkraw_save* save = xalloc(sizeof (*save));
        struct cleanup* cl = cleanup_allocate();
        save->fd = dup(fd);
        if (save->fd == -1)
            die_errno("dup");
        save->attr = attr;
        cleanup_commit(cl, xmkraw_cleanup, save);
    }

    cfmakeraw(&attr);
    xtcsetattr(fd, &attr);
}
