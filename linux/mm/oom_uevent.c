/*
 *  linux/mm/oom_uevent.c
 *
 *  Copyright (C)  2019 Amazon, Inc
 *  Sayanna Chandula
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  The routines in this file are used to send uevents from oom killer.
 *  Since there was no kset defined for memory management kobjects,
 *  a new kset has been defined here and used to send uevents.
 *  Sample code at linux/samples/kobject/kset_example.c is used as
 *  reference for creating kset and kobject
 */

#include <linux/oom.h>
#include <linux/mm.h>
#include <linux/err.h>
#include <linux/gfp.h>
#include <linux/memcontrol.h>
#include <linux/mempolicy.h>
#include <linux/kobject.h>
#include <linux/printk.h>
#include <linux/oom_uevent.h>

static DEFINE_MUTEX(_oom_uevent_lock);

enum state {
	UNINITIALIZED = 0,
	INITIALIZED,
	ERROR
};

/*
 * This is our "object" that we will create and register them with sysfs
 */
struct oom_uevent_obj {
	struct kobject kobj;
};
#define to_oom_uevent_obj(x) container_of(x, struct oom_uevent_obj, kobj)

static struct {
	struct kset *oom_kset;
	struct oom_uevent_obj *cgrp_obj;
	enum state state;
} _oom_uevent_inst;

/*
 * The release function for our object.  This is REQUIRED by the kernel to
 * have.  We free the memory held in our object here.
 */
 static void oom_release(struct kobject *kobj)
{
	struct oom_uevent_obj *oom;

	oom = to_oom_uevent_obj(kobj);
	kfree(oom);
}

/*
 * Custom ktype for oom_uevent kobjects.  Here we specify release function
 * We have not specified custom sysfsops or attributes since we are not
 * using any attributes for now. This can be extended as needed
 */
static struct kobj_type oom_ktype = {
	.release = oom_release,
};

static struct oom_uevent_obj *create_oom_uevent_obj(const char *name)
{
	struct oom_uevent_obj *obj;
	int retval;

	/* allocate the memory for the whole oom object */
	obj = kzalloc(sizeof(*obj), GFP_KERNEL);
	if (!obj)
		return NULL;

	obj->kobj.kset = _oom_uevent_inst.oom_kset;

	retval = kobject_init_and_add(&obj->kobj, &oom_ktype, NULL, "%s", name);
	if (retval) {
		kobject_put(&obj->kobj);
		return NULL;
	}

	kobject_uevent(&obj->kobj, KOBJ_ADD);

	return obj;
}

static int check_initialization(void)
{
	if (_oom_uevent_inst.state == ERROR)
		return -EINVAL;

	if (_oom_uevent_inst.state == INITIALIZED)
		return 0;

	_oom_uevent_inst.oom_kset = kset_create_and_add("oom_uevent", NULL, mm_kobj);
	if (!_oom_uevent_inst.oom_kset)
		goto oom_error;

	_oom_uevent_inst.cgrp_obj = create_oom_uevent_obj("cgroup");
	if (!_oom_uevent_inst.cgrp_obj)
		goto oom_error;

	_oom_uevent_inst.state = INITIALIZED;
	return 0;

oom_error:
	_oom_uevent_inst.state = ERROR;
	return -EINVAL;
}

int oom_uevent_send(char *msg[])
{
	int ret = 0;

	mutex_lock(&_oom_uevent_lock);

	ret = check_initialization();
	if (ret)
		goto oom_uevent_error;

	kobject_uevent_env(&_oom_uevent_inst.cgrp_obj->kobj, KOBJ_CHANGE, msg);

	mutex_unlock(&_oom_uevent_lock);
	return 0;

oom_uevent_error:
	mutex_unlock(&_oom_uevent_lock);
	return ret;
}
EXPORT_SYMBOL_GPL(oom_uevent_send);
