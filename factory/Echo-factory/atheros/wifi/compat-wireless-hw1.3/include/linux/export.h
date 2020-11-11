#ifndef _COMPAT_LINUX_EXPORT_H
#define _COMPAT_LINUX_EXPORT_H 1

#include <linux/version.h>

#if defined(CONFIG_X86)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0))
#include_next <linux/export.h>
#else
#include <linux/module.h>
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)) */

#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,8))
#include_next <linux/export.h>
#else
#include <linux/module.h>
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,8)) */

#endif /* CONFIG_X86 */

#endif	/* _COMPAT_LINUX_EXPORT_H */
