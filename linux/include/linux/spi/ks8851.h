#ifndef __DRIVER_NET_SPI_PDATA_KS8851_H__
#define __DRIVER_NET_SPI_PDATA_KS8851_H__

#include <asm/system.h>

struct ks8851_platform_data {
	u8 have_mac;
	unsigned char mac[MAC_ADDR_SIZE];
};

#endif /* __DRIVER_NET_SPI_PDATA_KS8851_H__ */
