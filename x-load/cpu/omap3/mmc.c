/*
 * (C) Copyright 2008
 * Texas Instruments, <www.ti.com>
 * Syed Mohammed Khasim <khasim@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation's version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <common.h>
#include <part.h>
#include <fat.h>
#include <mmc.h>

#define __raw_readl(a)    (*(volatile unsigned int *)(a))
#define __raw_writel(v,a) (*(volatile unsigned int *)(a) = (v))

#define mmc_reg_out(addr, mask, val)\
		__raw_writel((__raw_readl(addr) & (~(mask))) | ((val) & (mask)), (addr))

#define MMC_SLOT_COUNT	3

#ifdef CFG_CMD_MMC

struct omap_hsmmc_data {
	struct hsmmc *base_addr;
	int cd_gpio;
	int wp_gpio;
};

const unsigned short mmc_transspeed_val[15][4] = {
	{CLKD(10, 1), CLKD(10, 10), CLKD(10, 100), CLKD(10, 1000)},
	{CLKD(12, 1), CLKD(12, 10), CLKD(12, 100), CLKD(12, 1000)},
	{CLKD(13, 1), CLKD(13, 10), CLKD(13, 100), CLKD(13, 1000)},
	{CLKD(15, 1), CLKD(15, 10), CLKD(15, 100), CLKD(15, 1000)},
	{CLKD(20, 1), CLKD(20, 10), CLKD(20, 100), CLKD(20, 1000)},
	{CLKD(26, 1), CLKD(26, 10), CLKD(26, 100), CLKD(26, 1000)},
	{CLKD(30, 1), CLKD(30, 10), CLKD(30, 100), CLKD(30, 1000)},
	{CLKD(35, 1), CLKD(35, 10), CLKD(35, 100), CLKD(35, 1000)},
	{CLKD(40, 1), CLKD(40, 10), CLKD(40, 100), CLKD(40, 1000)},
	{CLKD(45, 1), CLKD(45, 10), CLKD(45, 100), CLKD(45, 1000)},
	{CLKD(52, 1), CLKD(52, 10), CLKD(52, 100), CLKD(52, 1000)},
	{CLKD(55, 1), CLKD(55, 10), CLKD(55, 100), CLKD(55, 1000)},
	{CLKD(60, 1), CLKD(60, 10), CLKD(60, 100), CLKD(60, 1000)},
	{CLKD(70, 1), CLKD(70, 10), CLKD(70, 100), CLKD(70, 1000)},
	{CLKD(80, 1), CLKD(80, 10), CLKD(80, 100), CLKD(80, 1000)}
};

static block_dev_desc_t mmc_blk_dev[3];
static struct mmc hsmmc_dev[3];
static struct omap_hsmmc_data hsmmc_dev_data[3];

block_dev_desc_t *mmc_get_dev(int dev)
{
	// There are only MMC_SLOT_COUNT MMC devices
	if (dev >=0  && dev < MMC_SLOT_COUNT) {
		return &mmc_blk_dev[dev];
	}

	return NULL;
}

unsigned char mmc_board_init(void)
{
#if defined (CONFIG_OMAP34XX) && !defined (CONFIG_AM3517EVM)
	unsigned int value = 0;

	value = CONTROL_PBIAS_LITE;
	CONTROL_PBIAS_LITE = value | (1 << 2) | (1 << 1) | (1 << 9);

	value = CONTROL_DEV_CONF0;
	CONTROL_DEV_CONF0 = value | (1 << 24) | (1 << 6);
#endif
	return 1;
}

void mmc_init_stream(struct mmc * mmc)
{
	volatile unsigned int mmc_stat;
	struct hsmmc *mmc_base = ((struct omap_hsmmc_data *)mmc->priv)->base_addr;

	__raw_writel(__raw_readl(&mmc_base->con) | INIT_INITSTREAM, &mmc_base->con);

	__raw_writel(MMC_CMD0, &mmc_base->cmd);

	do {
		mmc_stat = __raw_readl(&mmc_base->stat);
	} while (!(mmc_stat & CC_MASK));

	__raw_writel(CC_MASK, &mmc_base->stat);

	__raw_writel(MMC_CMD0, &mmc_base->cmd);
	do {
		mmc_stat = __raw_readl(&mmc_base->stat);;
	} while (!(mmc_stat & CC_MASK));

	__raw_writel(OMAP_HSMMC_STAT, &mmc_base->stat);

	__raw_writel(__raw_readl(&mmc_base->con) & ~INIT_INITSTREAM, &mmc_base->con);
}

unsigned char mmc_clock_config(struct mmc * mmc, unsigned int iclk, unsigned short clk_div)
{
	unsigned int val;
	struct hsmmc *mmc_base = ((struct omap_hsmmc_data *)mmc->priv)->base_addr;

	mmc_reg_out(&mmc_base->sysctl, (ICE_MASK | DTO_MASK | CEN_MASK),
		    (ICE_STOP | DTO_15THDTO | CEN_DISABLE));

	switch (iclk) {
	case CLK_INITSEQ:
		val = MMC_INIT_SEQ_CLK / 2;
		break;
	case CLK_400KHZ:
		val = MMC_400kHz_CLK;
		break;
	case CLK_MISC:
		val = clk_div;
		break;
	default:
		return 0;
	}

	mmc_reg_out(&mmc_base->sysctl,
		    ICE_MASK | CLKD_MASK, (val << CLKD_OFFSET) | ICE_OSCILLATE);

	while ((__raw_readl(&mmc_base->sysctl) & ICS_MASK) == ICS_NOTREADY) {
	}

	__raw_writel(__raw_readl(&mmc_base->sysctl) | CEN_ENABLE, &mmc_base->sysctl);

	return 1;
}

unsigned char mmc_init_setup(struct mmc * mmc)
{
	volatile __u32 reg_val;
	struct hsmmc *mmc_base = ((struct omap_hsmmc_data *)mmc->priv)->base_addr;

	mmc_board_init();

	__raw_writel(__raw_readl(&mmc_base->sysconfig) | MMC_SOFTRESET, &mmc_base->sysconfig);

	// Check the status
	while ((__raw_readl(&mmc_base->sysstatus) & RESETDONE) == 0);

	__raw_writel(__raw_readl(&mmc_base->sysctl) | SOFTRESETALL, &mmc_base->sysctl);
	while ((__raw_readl(&mmc_base->sysctl) & SOFTRESETALL) != 0x0);

	__raw_writel(DTW_1_BITMODE | SDBP_PWROFF | SDVS_3V0, &mmc_base->hctl);

	__raw_writel(__raw_readl(&mmc_base->capa) | VS30_3V0SUP | VS18_1V8SUP, &mmc_base->capa);

	reg_val = __raw_readl(&mmc_base->con) & RESERVED_MASK;

	__raw_writel(CTPL_MMC_SD | reg_val | WPP_ACTIVEHIGH |
	    CDP_ACTIVEHIGH | MIT_CTO | DW8_1_4BITMODE | MODE_FUNC |
	    STR_BLOCK | HR_NOHOSTRESP | INIT_NOINIT | NOOPENDRAIN, &mmc_base->con);

	mmc_clock_config(mmc, CLK_INITSEQ, 0);

	__raw_writel(__raw_readl(&mmc_base->hctl) | SDBP_PWRON, &mmc_base->hctl);

	__raw_writel(0x307f0033, &mmc_base->ie);

	mmc_init_stream(mmc);
	return 1;
}

unsigned char mmc_send_cmd(struct mmc * mmc, unsigned int cmd, unsigned int arg,
			   unsigned int *response)
{
	volatile unsigned int mmc_stat;
	struct hsmmc *mmc_base = ((struct omap_hsmmc_data *)mmc->priv)->base_addr;

	while ((__raw_readl(&mmc_base->pstate) & DATI_MASK) == DATI_CMDDIS) {
	}

	__raw_writel(BLEN_512BYTESLEN | NBLK_STPCNT, &mmc_base->blk);
	__raw_writel(0xFFFFFFFF, &mmc_base->stat);
	__raw_writel(arg, &mmc_base->arg);
	__raw_writel(cmd | CMD_TYPE_NORMAL | CICE_NOCHECK |
		    CCCE_NOCHECK | MSBS_SGLEBLK | ACEN_DISABLE | BCE_DISABLE |
		    DE_DISABLE, &mmc_base->cmd);

	while (1) {
		do {
			mmc_stat = __raw_readl(&mmc_base->stat);
		} while (mmc_stat == 0);

		if ((mmc_stat & ERRI_MASK) != 0)
			return (unsigned char) mmc_stat;

		if (mmc_stat & CC_MASK) {
			__raw_writel(CC_MASK, &mmc_base->stat);
			response[0] = __raw_readl(&mmc_base->rsp10);
			if ((cmd & RSP_TYPE_MASK) == RSP_TYPE_LGHT136) {
				response[1] = __raw_readl(&mmc_base->rsp32);
				response[2] = __raw_readl(&mmc_base->rsp54);
				response[3] = __raw_readl(&mmc_base->rsp76);
			}
			break;
		}
	}
	return 1;
}

unsigned char mmc_read_data(struct mmc * mmc, unsigned int *output_buf)
{
	volatile unsigned int mmc_stat;
	unsigned int read_count = 0;
	struct hsmmc *mmc_base = ((struct omap_hsmmc_data *)mmc->priv)->base_addr;

	/*
	 * Start Polled Read
	 */
	while (1) {
		do {
			mmc_stat = __raw_readl(&mmc_base->stat);
		} while (mmc_stat == 0);

		if ((mmc_stat & ERRI_MASK) != 0)
			return (unsigned char) mmc_stat;

		if (mmc_stat & BRR_MASK) {
			unsigned int k;

			__raw_writel(__raw_readl(&mmc_base->stat) | BRR_MASK, &mmc_base->stat);
			for (k = 0; k < MMCSD_SECTOR_SIZE / 4; k++) {
				*output_buf = __raw_readl(&mmc_base->data);
				output_buf++;
				read_count += 4;
			}
		}

		if (mmc_stat & BWR_MASK)
			__raw_writel(__raw_readl(&mmc_base->stat) | BWR_MASK, &mmc_base->stat);

		if (mmc_stat & TC_MASK) {
			__raw_writel(__raw_readl(&mmc_base->stat) | TC_MASK, &mmc_base->stat);
			break;
		}
	}
	return 1;
}

unsigned char mmc_detect_card(struct mmc *mmc)
{
	unsigned char err;
	unsigned int argument = 0;
	unsigned int ocr_value, ocr_recvd, ret_cmd41, hcs_val;
	unsigned int resp[4];
	unsigned short retry_cnt = 2000;
	struct hsmmc *mmc_base = ((struct omap_hsmmc_data *)mmc->priv)->base_addr;

	/* Set to Initialization Clock */
	err = mmc_clock_config(mmc, CLK_400KHZ, 0);
	if (err != 1)
		return err;

	mmc->RCA = MMC_RELATIVE_CARD_ADDRESS;
	argument = 0x00000000;

	ocr_value = (0x1FF << 15);
	err = mmc_send_cmd(mmc, MMC_CMD0, argument, resp);
	if (err != 1)
		return err;

	argument = SD_CMD8_CHECK_PATTERN | SD_CMD8_2_7_3_6_V_RANGE;
	err = mmc_send_cmd(mmc, MMC_SDCMD8, argument, resp);
	hcs_val = (err == 1) ?
	    MMC_OCR_REG_HOST_CAPACITY_SUPPORT_SECTOR :
	    MMC_OCR_REG_HOST_CAPACITY_SUPPORT_BYTE;

	argument = 0x0000 << 16;
	err = mmc_send_cmd(mmc, MMC_CMD55, argument, resp);
	if (err == 1) {
		mmc->card_type = SD_CARD;
		ocr_value |= hcs_val;
		ret_cmd41 = MMC_ACMD41;
	} else {
		mmc->card_type = MMC_CARD;
		ocr_value |= MMC_OCR_REG_ACCESS_MODE_SECTOR;
		ret_cmd41 = MMC_CMD1;
		__raw_writel(__raw_readl(&mmc_base->con) & ~OD, &mmc_base->con);
		__raw_writel(__raw_readl(&mmc_base->con) | OPENDRAIN, &mmc_base->con);
	}

	argument = ocr_value;
	err = mmc_send_cmd(mmc, ret_cmd41, argument, resp);
	if (err != 1)
		return err;

	ocr_recvd = ((mmc_resp_r3 *) resp)->ocr;

	while (!(ocr_recvd & (0x1 << 31)) && (retry_cnt > 0)) {
		retry_cnt--;
		if (mmc->card_type == SD_CARD) {
			argument = 0x0000 << 16;
			err = mmc_send_cmd(mmc, MMC_CMD55, argument, resp);
		}

		argument = ocr_value;
		err = mmc_send_cmd(mmc, ret_cmd41, argument, resp);
		if (err != 1)
			return err;
		ocr_recvd = ((mmc_resp_r3 *) resp)->ocr;
	}

	if (!(ocr_recvd & (0x1 << 31)))
		return 0;

	if (mmc->card_type == MMC_CARD) {
		if ((ocr_recvd & MMC_OCR_REG_ACCESS_MODE_MASK) ==
		    MMC_OCR_REG_ACCESS_MODE_SECTOR) {
			mmc->mode = SECTOR_MODE;
		} else {
			mmc->mode = BYTE_MODE;
		}

		ocr_recvd &= ~MMC_OCR_REG_ACCESS_MODE_MASK;
	} else {
		if ((ocr_recvd & MMC_OCR_REG_HOST_CAPACITY_SUPPORT_MASK)
		    == MMC_OCR_REG_HOST_CAPACITY_SUPPORT_SECTOR) {
			mmc->mode = SECTOR_MODE;
		} else {
			mmc->mode = BYTE_MODE;
		}
		ocr_recvd &= ~MMC_OCR_REG_HOST_CAPACITY_SUPPORT_MASK;
	}

	ocr_recvd &= ~(0x1 << 31);
	if (!(ocr_recvd & ocr_value))
		return 0;

	err = mmc_send_cmd(mmc, MMC_CMD2, argument, resp);
	if (err != 1)
		return err;

	if (mmc->card_type == MMC_CARD) {
		argument = mmc->RCA << 16;
		err = mmc_send_cmd(mmc, MMC_CMD3, argument, resp);
		if (err != 1)
			return err;
	} else {
		argument = 0x00000000;
		err = mmc_send_cmd(mmc, MMC_SDCMD3, argument, resp);
		if (err != 1)
			return err;

		mmc->RCA = ((mmc_resp_r6 *) resp)->newpublishedrca;
	}

	__raw_writel(__raw_readl(&mmc_base->con) & ~OD, &mmc_base->con);
	__raw_writel(__raw_readl(&mmc_base->con) | NOOPENDRAIN, &mmc_base->con);

	return 1;
}

unsigned char mmc_read_cardsize(struct mmc *mmc_dev_data,
				mmc_csd_reg_t *cur_csd)
{
	mmc_extended_csd_reg_t ext_csd;
	unsigned int size, count, blk_len, blk_no, card_size, argument;
	unsigned char err;
	unsigned int resp[4];

	if (mmc_dev_data->mode == SECTOR_MODE) {
		if (mmc_dev_data->card_type == SD_CARD) {
			card_size =
			    (((mmc_sd2_csd_reg_t *) cur_csd)->
			     c_size_lsb & MMC_SD2_CSD_C_SIZE_LSB_MASK) |
			    ((((mmc_sd2_csd_reg_t *) cur_csd)->
			      c_size_msb & MMC_SD2_CSD_C_SIZE_MSB_MASK)
			     << MMC_SD2_CSD_C_SIZE_MSB_OFFSET);
			mmc_dev_data->size = card_size * 1024;
			if (mmc_dev_data->size == 0)
				return 0;
		} else {
			argument = 0x00000000;
			err = mmc_send_cmd(mmc_dev_data, MMC_CMD8, argument, resp);
			if (err != 1)
				return err;
			err = mmc_read_data(mmc_dev_data, (unsigned int *) &ext_csd);
			if (err != 1)
				return err;
			mmc_dev_data->size = ext_csd.sectorcount;

			if (mmc_dev_data->size == 0)
				mmc_dev_data->size = 8388608;
		}
	} else {
		if (cur_csd->c_size_mult >= 8)
			return 0;

		if (cur_csd->read_bl_len >= 12)
			return 0;

		/* Compute size */
		count = 1 << (cur_csd->c_size_mult + 2);
		card_size = (cur_csd->c_size_lsb & MMC_CSD_C_SIZE_LSB_MASK) |
		    ((cur_csd->c_size_msb & MMC_CSD_C_SIZE_MSB_MASK)
		     << MMC_CSD_C_SIZE_MSB_OFFSET);
		blk_no = (card_size + 1) * count;
		blk_len = 1 << cur_csd->read_bl_len;
		size = blk_no * blk_len;
		mmc_dev_data->size = size / MMCSD_SECTOR_SIZE;
		if (mmc_dev_data->size == 0)
			return 0;
	}
	return 1;
}

unsigned char omap_mmc_read_sect(unsigned int start_sec, unsigned int num_bytes,
				 struct mmc *mmc_c,
				 unsigned long *output_buf)
{
	unsigned char err;
	unsigned int argument;
	unsigned int resp[4];
	unsigned int num_sec_val =
	    (num_bytes + (MMCSD_SECTOR_SIZE - 1)) / MMCSD_SECTOR_SIZE;
	unsigned int sec_inc_val;

	if (num_sec_val == 0)
		return 1;

	if (mmc_c->mode == SECTOR_MODE) {
		argument = start_sec;
		sec_inc_val = 1;
	} else {
		argument = start_sec * MMCSD_SECTOR_SIZE;
		sec_inc_val = MMCSD_SECTOR_SIZE;
	}

	while (num_sec_val) {
		err = mmc_send_cmd(mmc_c, MMC_CMD17, argument, resp);
		if (err != 1)
			return err;

		err = mmc_read_data(mmc_c, (unsigned int *) output_buf);
		if (err != 1)
			return err;

		output_buf += (MMCSD_SECTOR_SIZE / 4);
		argument += sec_inc_val;
		num_sec_val--;
	}
	return 1;
}

unsigned char configure_mmc(struct mmc * mmc)
{
	unsigned char ret_val;
	unsigned int argument;
	unsigned int resp[4];
	unsigned int trans_clk, trans_fact, trans_unit, retries = 2;
	mmc_csd_reg_t Card_CSD;
	unsigned char trans_speed;


	ret_val = mmc_init_setup(mmc);
	if (ret_val != 1)
		return ret_val;

	do {
		ret_val = mmc_detect_card(mmc);
		retries--;
	} while ((retries > 0) && (ret_val != 1));

	argument = mmc->RCA << 16;
	ret_val = mmc_send_cmd(mmc, MMC_CMD9, argument, resp);
	if (ret_val != 1)
		return ret_val;

	((unsigned int *) &Card_CSD)[3] = resp[3];
	((unsigned int *) &Card_CSD)[2] = resp[2];
	((unsigned int *) &Card_CSD)[1] = resp[1];
	((unsigned int *) &Card_CSD)[0] = resp[0];

	if (mmc->card_type == MMC_CARD)
		mmc->version = Card_CSD.spec_vers;

	trans_speed = Card_CSD.tran_speed;

	ret_val = mmc_send_cmd(mmc, MMC_CMD4, MMC_DSR_DEFAULT << 16, resp);
	if (ret_val != 1)
		return ret_val;

	trans_unit = trans_speed & MMC_CSD_TRAN_SPEED_UNIT_MASK;
	trans_fact = trans_speed & MMC_CSD_TRAN_SPEED_FACTOR_MASK;

	if (trans_unit > MMC_CSD_TRAN_SPEED_UNIT_100MHZ)
		return 0;

	if ((trans_fact < MMC_CSD_TRAN_SPEED_FACTOR_1_0) ||
	    (trans_fact > MMC_CSD_TRAN_SPEED_FACTOR_8_0))
		return 0;

	trans_unit >>= 0;
	trans_fact >>= 3;

	trans_clk = mmc_transspeed_val[trans_fact - 1][trans_unit] * 2;
	ret_val = mmc_clock_config(mmc, CLK_MISC, trans_clk);
	if (ret_val != 1)
		return ret_val;

	argument = mmc->RCA << 16;
	ret_val = mmc_send_cmd(mmc, MMC_CMD7_SELECT, argument, resp);
	if (ret_val != 1)
		return ret_val;

	/* Configure the block length to 512 bytes */
	argument = MMCSD_SECTOR_SIZE;
	ret_val = mmc_send_cmd(mmc, MMC_CMD16, argument, resp);
	if (ret_val != 1)
		return ret_val;

	/* get the card size in sectors */
	ret_val = mmc_read_cardsize(mmc, &Card_CSD);
	if (ret_val != 1)
		return ret_val;

	return 1;
}

unsigned long mmc_bread(int dev_num, unsigned long blknr, lbaint_t blkcnt,
			void *dst)
{
	if (dev_num >= 0 && dev_num < 3) {
		omap_mmc_read_sect(blknr, (blkcnt * MMCSD_SECTOR_SIZE), &hsmmc_dev[dev_num],
			   (unsigned long *) dst);

		return 1;
	}

	return 0;
}

int mmc_init(int dev_index)
{
	unsigned char ret;
	struct mmc *mmc = &hsmmc_dev[dev_index];
	struct omap_hsmmc_data *priv_data = &hsmmc_dev_data[dev_index];

	if(dev_index < 0 || dev_index >= MMC_SLOT_COUNT)
		return 0;

	if (mmc->init_done)
		return 1;

	mmc->priv = priv_data;

	switch (dev_index) {
	case 0:
		priv_data->base_addr = (struct hsmmc *)OMAP_HSMMC1_BASE;
		break;
#ifdef OMAP_HSMMC2_BASE
	case 1:
		priv_data->base_addr = (struct hsmmc *)OMAP_HSMMC2_BASE;
		break;
#endif
#ifdef OMAP_HSMMC3_BASE
	case 2:
		priv_data->base_addr = (struct hsmmc *)OMAP_HSMMC3_BASE;
		break;
#endif
	default:
		return 0;
	}

	mmc->dev = dev_index;
	ret = configure_mmc(mmc);

	if (ret == 1) {
		mmc_blk_dev[dev_index].if_type = IF_TYPE_MMC;
		mmc_blk_dev[dev_index].part_type = PART_TYPE_CUSTOM;
		mmc_blk_dev[dev_index].dev = dev_index;
		mmc_blk_dev[dev_index].lun = 0;
		mmc_blk_dev[dev_index].type = 0;

		/* FIXME fill in the correct size (is set to 32MByte) */
		mmc_blk_dev[dev_index].blksz = MMCSD_SECTOR_SIZE;
		mmc_blk_dev[dev_index].lba = 0x10000;
		mmc_blk_dev[dev_index].removable = 0;
		mmc_blk_dev[dev_index].block_read = mmc_bread;
		mmc->init_done = 1;

		return 1;
	}
	else 
		return 0;
}

int mmc_read(ulong src, uchar *dst, int size)
{
	/*
	 * NOT Implemented
	 */
	return 0;
}

int mmc_write(uchar *src, ulong dst, int size)
{
	/*
	 * NOT Implemented
	 */
	return 0;
}

int mmc2info(ulong addr)
{
	/*
	 * NOT Implemented
	 */
	return 0;
}
#endif

