#ifndef __INCLUDE_LINUX_OOM_UEVENT_H
#define __INCLUDE_LINUX_OOM_UEVENT_H

#ifdef __KERNEL__

int oom_uevent_send(char *msg[]);

#endif /* __KERNEL__*/
#endif /* _INCLUDE_LINUX_OOM_H */
