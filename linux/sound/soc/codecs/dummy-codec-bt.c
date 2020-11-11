/*
 * ALSA SoC dummy codec driver for bluetooth
 *
 * Copyright (c) 2013 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>

/* The so-called "dummy-codec-bt" is a dummy codec that allows you to
 * use a CPU DAI (e.g. McBSPn) without having a specific codec driver
 * (e.g. a codec that's always configured and ready to go).  It's
 * intended for use with Bluetooth.
 */

static struct snd_soc_dai_driver dummy_codec_dai_drv = {
	.name = "Bluetooth",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 1,
		.rates = SNDRV_PCM_RATE_8000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,
	},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 1,
		.rates = SNDRV_PCM_RATE_8000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,
	},
};

static struct snd_soc_codec_driver dummy_codec_codec_driver = {
};

MODULE_ALIAS("platform:dummy-codec-bt");

static int __devinit dummy_codec_probe(struct platform_device *pdev)
{
	int ret;

	ret = snd_soc_register_codec(&pdev->dev,
				     &dummy_codec_codec_driver,
				     &dummy_codec_dai_drv,
				     1);
	if (ret < 0)
		printk(KERN_ERR "Could not register dummy-codec-bt with ASoC %d\n", ret);

	return ret;
}

static int __devexit dummy_codec_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

static struct platform_driver dummy_codec_driver = {
	.probe = dummy_codec_probe,
	.remove = __devexit_p(dummy_codec_remove),
	.driver = {
		.name = "dummy-codec-bt",
		.owner = THIS_MODULE,
	}
};

static int __init dummy_codec_init(void)
{
	int ret;

	ret = platform_driver_register(&dummy_codec_driver);
	if (ret < 0)
		printk(KERN_ERR "Could not register driver dummy-codec-bt %d\n", ret);

	return ret;
}
module_init(dummy_codec_init);

static void __exit dummy_codec_exit(void)
{
	platform_driver_unregister(&dummy_codec_driver);
}
module_exit(dummy_codec_exit);

MODULE_DESCRIPTION("ASoC dummy-codec-bt driver");
MODULE_AUTHOR("Amazon.com, Inc.");
MODULE_LICENSE("GPL");
