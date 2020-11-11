xdc.loadCapsule('xdc/om2.xs');

var $om = xdc.om;
var __CFG__ = $om.$name == 'cfg';
var __ROV__ = $om.$name == 'rov';
var $$pkgspec = xdc.$$ses.findPkg('ti.bios.power');

/* ======== IMPORTS ======== */

    xdc.loadPackage('xdc');
    xdc.loadPackage('xdc.corevers');

/* ======== OBJECTS ======== */

// package ti.bios.power
    var pkg = $om.$$bind('ti.bios.power.Package', $$PObj());
    $om.$$bind('ti.bios.power', $$VObj('ti.bios.power', pkg));
// interface ITMS320CDM644x
    var po = $om.$$bind('ti.bios.power.ITMS320CDM644x.Module', $$PObj());
    var vo = $om.$$bind('ti.bios.power.ITMS320CDM644x', $$VObj('ti.bios.power.ITMS320CDM644x', po));
    pkg.$$bind('ITMS320CDM644x', vo);
    $om.$$bind('ti.bios.power.ITMS320CDM644x.OpersysCfg', $$PEnm('ti.bios.power.ITMS320CDM644x.OpersysCfg'));
    $om.$$bind('ti.bios.power.ITMS320CDM644x.TransportCfg', $$PEnm('ti.bios.power.ITMS320CDM644x.TransportCfg'));
// module TMS320CDM6441GPP
    var po = $om.$$bind('ti.bios.power.TMS320CDM6441GPP.Module', $$PObj());
    var vo = $om.$$bind('ti.bios.power.TMS320CDM6441GPP', $$VObj('ti.bios.power.TMS320CDM6441GPP', po));
    pkg.$$bind('TMS320CDM6441GPP', vo);
    $om.$$bind('ti.bios.power.TMS320CDM6441GPP.OpersysCfg', $om['ti.bios.power.ITMS320CDM644x.OpersysCfg']);
    $om.$$bind('ti.bios.power.TMS320CDM6441GPP.TransportCfg', $om['ti.bios.power.ITMS320CDM644x.TransportCfg']);
// module TMS320CDM6441DSP
    var po = $om.$$bind('ti.bios.power.TMS320CDM6441DSP.Module', $$PObj());
    var vo = $om.$$bind('ti.bios.power.TMS320CDM6441DSP', $$VObj('ti.bios.power.TMS320CDM6441DSP', po));
    pkg.$$bind('TMS320CDM6441DSP', vo);
    $om.$$bind('ti.bios.power.TMS320CDM6441DSP.OpersysCfg', $om['ti.bios.power.ITMS320CDM644x.OpersysCfg']);
    $om.$$bind('ti.bios.power.TMS320CDM6441DSP.TransportCfg', $om['ti.bios.power.ITMS320CDM644x.TransportCfg']);
    $om.$$bind('ti.bios.power.TMS320CDM6441DSP$$RuntimeContext', $$PObj());
    $om.$$bind('ti.bios.power.TMS320CDM6441DSP.RuntimeContext', $$PStr($om['ti.bios.power.TMS320CDM6441DSP$$RuntimeContext'], true));
// module TMS320CDM6446GPP
    var po = $om.$$bind('ti.bios.power.TMS320CDM6446GPP.Module', $$PObj());
    var vo = $om.$$bind('ti.bios.power.TMS320CDM6446GPP', $$VObj('ti.bios.power.TMS320CDM6446GPP', po));
    pkg.$$bind('TMS320CDM6446GPP', vo);
    $om.$$bind('ti.bios.power.TMS320CDM6446GPP.OpersysCfg', $om['ti.bios.power.ITMS320CDM644x.OpersysCfg']);
    $om.$$bind('ti.bios.power.TMS320CDM6446GPP.TransportCfg', $om['ti.bios.power.ITMS320CDM644x.TransportCfg']);
// module TMS320CDM6446DSP
    var po = $om.$$bind('ti.bios.power.TMS320CDM6446DSP.Module', $$PObj());
    var vo = $om.$$bind('ti.bios.power.TMS320CDM6446DSP', $$VObj('ti.bios.power.TMS320CDM6446DSP', po));
    pkg.$$bind('TMS320CDM6446DSP', vo);
    $om.$$bind('ti.bios.power.TMS320CDM6446DSP.OpersysCfg', $om['ti.bios.power.ITMS320CDM644x.OpersysCfg']);
    $om.$$bind('ti.bios.power.TMS320CDM6446DSP.TransportCfg', $om['ti.bios.power.ITMS320CDM644x.TransportCfg']);
    $om.$$bind('ti.bios.power.TMS320CDM6446DSP$$RuntimeContext', $$PObj());
    $om.$$bind('ti.bios.power.TMS320CDM6446DSP.RuntimeContext', $$PStr($om['ti.bios.power.TMS320CDM6446DSP$$RuntimeContext'], true));
// module TMS320C3430GPP
    var po = $om.$$bind('ti.bios.power.TMS320C3430GPP.Module', $$PObj());
    var vo = $om.$$bind('ti.bios.power.TMS320C3430GPP', $$VObj('ti.bios.power.TMS320C3430GPP', po));
    pkg.$$bind('TMS320C3430GPP', vo);
    $om.$$bind('ti.bios.power.TMS320C3430GPP.OpersysCfg', $om['ti.bios.power.ITMS320CDM644x.OpersysCfg']);
    $om.$$bind('ti.bios.power.TMS320C3430GPP.TransportCfg', $om['ti.bios.power.ITMS320CDM644x.TransportCfg']);
// module TMS320C3430DSP
    var po = $om.$$bind('ti.bios.power.TMS320C3430DSP.Module', $$PObj());
    var vo = $om.$$bind('ti.bios.power.TMS320C3430DSP', $$VObj('ti.bios.power.TMS320C3430DSP', po));
    pkg.$$bind('TMS320C3430DSP', vo);
    $om.$$bind('ti.bios.power.TMS320C3430DSP.OpersysCfg', $om['ti.bios.power.ITMS320CDM644x.OpersysCfg']);
    $om.$$bind('ti.bios.power.TMS320C3430DSP.TransportCfg', $om['ti.bios.power.ITMS320CDM644x.TransportCfg']);
    $om.$$bind('ti.bios.power.TMS320C3430DSP$$RuntimeContext', $$PObj());
    $om.$$bind('ti.bios.power.TMS320C3430DSP.RuntimeContext', $$PStr($om['ti.bios.power.TMS320C3430DSP$$RuntimeContext'], true));
// module OMAP2530GPP
    var po = $om.$$bind('ti.bios.power.OMAP2530GPP.Module', $$PObj());
    var vo = $om.$$bind('ti.bios.power.OMAP2530GPP', $$VObj('ti.bios.power.OMAP2530GPP', po));
    pkg.$$bind('OMAP2530GPP', vo);
    $om.$$bind('ti.bios.power.OMAP2530GPP.OpersysCfg', $om['ti.bios.power.ITMS320CDM644x.OpersysCfg']);
    $om.$$bind('ti.bios.power.OMAP2530GPP.TransportCfg', $om['ti.bios.power.ITMS320CDM644x.TransportCfg']);
// module OMAP2530DSP
    var po = $om.$$bind('ti.bios.power.OMAP2530DSP.Module', $$PObj());
    var vo = $om.$$bind('ti.bios.power.OMAP2530DSP', $$VObj('ti.bios.power.OMAP2530DSP', po));
    pkg.$$bind('OMAP2530DSP', vo);
    $om.$$bind('ti.bios.power.OMAP2530DSP.OpersysCfg', $om['ti.bios.power.ITMS320CDM644x.OpersysCfg']);
    $om.$$bind('ti.bios.power.OMAP2530DSP.TransportCfg', $om['ti.bios.power.ITMS320CDM644x.TransportCfg']);
    $om.$$bind('ti.bios.power.OMAP2530DSP$$RuntimeContext', $$PObj());
    $om.$$bind('ti.bios.power.OMAP2530DSP.RuntimeContext', $$PStr($om['ti.bios.power.OMAP2530DSP$$RuntimeContext'], true));
// module OMAP3530GPP
    var po = $om.$$bind('ti.bios.power.OMAP3530GPP.Module', $$PObj());
    var vo = $om.$$bind('ti.bios.power.OMAP3530GPP', $$VObj('ti.bios.power.OMAP3530GPP', po));
    pkg.$$bind('OMAP3530GPP', vo);
    $om.$$bind('ti.bios.power.OMAP3530GPP.OpersysCfg', $om['ti.bios.power.ITMS320CDM644x.OpersysCfg']);
    $om.$$bind('ti.bios.power.OMAP3530GPP.TransportCfg', $om['ti.bios.power.ITMS320CDM644x.TransportCfg']);
// module OMAP3530DSP
    var po = $om.$$bind('ti.bios.power.OMAP3530DSP.Module', $$PObj());
    var vo = $om.$$bind('ti.bios.power.OMAP3530DSP', $$VObj('ti.bios.power.OMAP3530DSP', po));
    pkg.$$bind('OMAP3530DSP', vo);
    $om.$$bind('ti.bios.power.OMAP3530DSP.OpersysCfg', $om['ti.bios.power.ITMS320CDM644x.OpersysCfg']);
    $om.$$bind('ti.bios.power.OMAP3530DSP.TransportCfg', $om['ti.bios.power.ITMS320CDM644x.TransportCfg']);
    $om.$$bind('ti.bios.power.OMAP3530DSP$$RuntimeContext', $$PObj());
    $om.$$bind('ti.bios.power.OMAP3530DSP.RuntimeContext', $$PStr($om['ti.bios.power.OMAP3530DSP$$RuntimeContext'], true));

/* ======== CONSTS ======== */

// interface ITMS320CDM644x
    $om.$$bind('ti.bios.power.ITMS320CDM644x.Opersys_PrOS_DaVinci', $$Enum($om['ti.bios.power.ITMS320CDM644x.OpersysCfg'], 'ti.bios.power.ITMS320CDM644x.Opersys_PrOS_DaVinci', 0));
    $om.$$bind('ti.bios.power.ITMS320CDM644x.Opersys_Linux', $$Enum($om['ti.bios.power.ITMS320CDM644x.OpersysCfg'], 'ti.bios.power.ITMS320CDM644x.Opersys_Linux', 1));
    $om.$$bind('ti.bios.power.ITMS320CDM644x.Opersys_DSPBIOS', $$Enum($om['ti.bios.power.ITMS320CDM644x.OpersysCfg'], 'ti.bios.power.ITMS320CDM644x.Opersys_DSPBIOS', 2));
    $om.$$bind('ti.bios.power.ITMS320CDM644x.Transport_Link_PrOS_DaVinci', $$Enum($om['ti.bios.power.ITMS320CDM644x.TransportCfg'], 'ti.bios.power.ITMS320CDM644x.Transport_Link_PrOS_DaVinci', 0));
    $om.$$bind('ti.bios.power.ITMS320CDM644x.Transport_Link_Linux', $$Enum($om['ti.bios.power.ITMS320CDM644x.TransportCfg'], 'ti.bios.power.ITMS320CDM644x.Transport_Link_Linux', 1));
// module TMS320CDM6441GPP
// module TMS320CDM6441DSP
// module TMS320CDM6446GPP
// module TMS320CDM6446DSP
// module TMS320C3430GPP
// module TMS320C3430DSP
// module OMAP2530GPP
// module OMAP2530DSP
// module OMAP3530GPP
// module OMAP3530DSP

/* ======== CREATES ======== */

// interface ITMS320CDM644x
// module TMS320CDM6441GPP
// module TMS320CDM6441DSP
// module TMS320CDM6446GPP
// module TMS320CDM6446DSP
// module TMS320C3430GPP
// module TMS320C3430DSP
// module OMAP2530GPP
// module OMAP2530DSP
// module OMAP3530GPP
// module OMAP3530DSP

/* ======== FUNCTIONS ======== */


/* ======== SIZES ======== */


/* ======== TYPES ======== */

// interface ITMS320CDM644x
    var cap = $om.$$bind('ti.bios.power.ITMS320CDM644x$$capsule', xdc.loadCapsule('ti/bios/power/ITMS320CDM644x.xs'));
    var po = $om['ti.bios.power.ITMS320CDM644x.Module'].$$init('ti.bios.power.ITMS320CDM644x.Module', $$Module);
        po.$$fld('$hostonly', $$T_Num, 1, 'r');
        po.$$fld('opersys', $om['ti.bios.power.ITMS320CDM644x.OpersysCfg'], undefined, 'wh');
        po.$$fld('transport', $om['ti.bios.power.ITMS320CDM644x.TransportCfg'], undefined, 'wh');
        po.$$fld('proxyFor', $$T_Str, undefined, 'wh');
        var fxn = cap['module$meta$init'];
        if (fxn) $om.$$bind('ti.bios.power.ITMS320CDM644x$$module$meta$init', true);
        if (fxn) po.$$fxn('module$meta$init', $$T_Met, fxn);
        var fxn = cap['module$validate'];
        if (fxn) $om.$$bind('ti.bios.power.ITMS320CDM644x$$module$validate', true);
        if (fxn) po.$$fxn('module$validate', $$T_Met, fxn);
// module TMS320CDM6441GPP
    var cap = $om.$$bind('ti.bios.power.TMS320CDM6441GPP$$capsule', xdc.loadCapsule('ti/bios/power/TMS320CDM6441GPP.xs'));
    var po = $om['ti.bios.power.TMS320CDM6441GPP.Module'].$$init('ti.bios.power.TMS320CDM6441GPP.Module', $om['ti.bios.power.ITMS320CDM644x.Module']);
        po.$$fld('$hostonly', $$T_Num, 1, 'r');
        po.$$fld('enableVicpAtPowerOn', $$T_Bool, undefined, 'wh');
        po.$$fld('CPU_NAME', $$T_Str, "GPP", 'rh');
        var fxn = cap['module$meta$init'];
        if (fxn) $om.$$bind('ti.bios.power.TMS320CDM6441GPP$$module$meta$init', true);
        if (fxn) po.$$fxn('module$meta$init', $$T_Met, fxn);
        var fxn = cap['module$validate'];
        if (fxn) $om.$$bind('ti.bios.power.TMS320CDM6441GPP$$module$validate', true);
        if (fxn) po.$$fxn('module$validate', $$T_Met, fxn);
// module TMS320CDM6441DSP
    var cap = $om.$$bind('ti.bios.power.TMS320CDM6441DSP$$capsule', xdc.loadCapsule('ti/bios/power/TMS320CDM6441DSP.xs'));
    var po = $om['ti.bios.power.TMS320CDM6441DSP.Module'].$$init('ti.bios.power.TMS320CDM6441DSP.Module', $om['ti.bios.power.ITMS320CDM644x.Module']);
        po.$$fld('$hostonly', $$T_Num, 1, 'r');
        po.$$fld('CONTEXT_BUFFER_MAX_SIZE', $$T_CNum('(xdc_UInt)'), 0x40000, 'rh');
        po.$$fld('dspRC', $om['ti.bios.power.TMS320CDM6441DSP.RuntimeContext'], $$SO({len: 0x40000, useUserFxns: false, saveFxn: "", restoreFxn: ""}), 'wh');
        po.$$fld('imcopRC', $om['ti.bios.power.TMS320CDM6441DSP.RuntimeContext'], $$SO({len: 0x40000, useUserFxns: false, saveFxn: "", restoreFxn: ""}), 'wh');
        po.$$fld('edmaChannel', $$T_CNum('(xdc_UInt32)'), 45, 'wh');
        po.$$fld('targetMemoryRangeName', $$T_Str, "", 'wh');
        po.$$fld('CPU_NAME', $$T_Str, "DSP", 'rh');
        var fxn = cap['module$meta$init'];
        if (fxn) $om.$$bind('ti.bios.power.TMS320CDM6441DSP$$module$meta$init', true);
        if (fxn) po.$$fxn('module$meta$init', $$T_Met, fxn);
        var fxn = cap['module$validate'];
        if (fxn) $om.$$bind('ti.bios.power.TMS320CDM6441DSP$$module$validate', true);
        if (fxn) po.$$fxn('module$validate', $$T_Met, fxn);
// struct TMS320CDM6441DSP.RuntimeContext
    var po = $om['ti.bios.power.TMS320CDM6441DSP$$RuntimeContext'].$$init('ti.bios.power.TMS320CDM6441DSP.RuntimeContext', null);
        po.$$fld('$hostonly', $$T_Num, 1, 'r');
        po.$$fld('len', $$T_CNum('(xdc_UInt32)'), undefined, 'w');
        po.$$fld('useUserFxns', $$T_Bool, undefined, 'w');
        po.$$fld('saveFxn', $$T_Str, undefined, 'w');
        po.$$fld('restoreFxn', $$T_Str, undefined, 'w');
// module TMS320CDM6446GPP
    var cap = $om.$$bind('ti.bios.power.TMS320CDM6446GPP$$capsule', xdc.loadCapsule('ti/bios/power/TMS320CDM6446GPP.xs'));
    var po = $om['ti.bios.power.TMS320CDM6446GPP.Module'].$$init('ti.bios.power.TMS320CDM6446GPP.Module', $om['ti.bios.power.ITMS320CDM644x.Module']);
        po.$$fld('$hostonly', $$T_Num, 1, 'r');
        po.$$fld('enableVicpAtPowerOn', $$T_Bool, undefined, 'wh');
        po.$$fld('CPU_NAME', $$T_Str, "GPP", 'rh');
        var fxn = cap['module$meta$init'];
        if (fxn) $om.$$bind('ti.bios.power.TMS320CDM6446GPP$$module$meta$init', true);
        if (fxn) po.$$fxn('module$meta$init', $$T_Met, fxn);
        var fxn = cap['module$validate'];
        if (fxn) $om.$$bind('ti.bios.power.TMS320CDM6446GPP$$module$validate', true);
        if (fxn) po.$$fxn('module$validate', $$T_Met, fxn);
// module TMS320CDM6446DSP
    var cap = $om.$$bind('ti.bios.power.TMS320CDM6446DSP$$capsule', xdc.loadCapsule('ti/bios/power/TMS320CDM6446DSP.xs'));
    var po = $om['ti.bios.power.TMS320CDM6446DSP.Module'].$$init('ti.bios.power.TMS320CDM6446DSP.Module', $om['ti.bios.power.ITMS320CDM644x.Module']);
        po.$$fld('$hostonly', $$T_Num, 1, 'r');
        po.$$fld('CONTEXT_BUFFER_MAX_SIZE', $$T_CNum('(xdc_UInt)'), 0x40000, 'rh');
        po.$$fld('dspRC', $om['ti.bios.power.TMS320CDM6446DSP.RuntimeContext'], $$SO({len: 0x40000, useUserFxns: false, saveFxn: "", restoreFxn: ""}), 'wh');
        po.$$fld('imcopRC', $om['ti.bios.power.TMS320CDM6446DSP.RuntimeContext'], $$SO({len: 0x40000, useUserFxns: false, saveFxn: "", restoreFxn: ""}), 'wh');
        po.$$fld('edmaChannel', $$T_CNum('(xdc_UInt32)'), 45, 'wh');
        po.$$fld('targetMemoryRangeName', $$T_Str, "", 'wh');
        po.$$fld('CPU_NAME', $$T_Str, "DSP", 'rh');
        var fxn = cap['module$meta$init'];
        if (fxn) $om.$$bind('ti.bios.power.TMS320CDM6446DSP$$module$meta$init', true);
        if (fxn) po.$$fxn('module$meta$init', $$T_Met, fxn);
        var fxn = cap['module$validate'];
        if (fxn) $om.$$bind('ti.bios.power.TMS320CDM6446DSP$$module$validate', true);
        if (fxn) po.$$fxn('module$validate', $$T_Met, fxn);
// struct TMS320CDM6446DSP.RuntimeContext
    var po = $om['ti.bios.power.TMS320CDM6446DSP$$RuntimeContext'].$$init('ti.bios.power.TMS320CDM6446DSP.RuntimeContext', null);
        po.$$fld('$hostonly', $$T_Num, 1, 'r');
        po.$$fld('len', $$T_CNum('(xdc_UInt32)'), undefined, 'w');
        po.$$fld('useUserFxns', $$T_Bool, undefined, 'w');
        po.$$fld('saveFxn', $$T_Str, undefined, 'w');
        po.$$fld('restoreFxn', $$T_Str, undefined, 'w');
// module TMS320C3430GPP
    var cap = $om.$$bind('ti.bios.power.TMS320C3430GPP$$capsule', xdc.loadCapsule('ti/bios/power/TMS320C3430GPP.xs'));
    var po = $om['ti.bios.power.TMS320C3430GPP.Module'].$$init('ti.bios.power.TMS320C3430GPP.Module', $om['ti.bios.power.ITMS320CDM644x.Module']);
        po.$$fld('$hostonly', $$T_Num, 1, 'r');
        po.$$fld('CPU_NAME', $$T_Str, "GPP", 'rh');
        var fxn = cap['module$meta$init'];
        if (fxn) $om.$$bind('ti.bios.power.TMS320C3430GPP$$module$meta$init', true);
        if (fxn) po.$$fxn('module$meta$init', $$T_Met, fxn);
        var fxn = cap['module$validate'];
        if (fxn) $om.$$bind('ti.bios.power.TMS320C3430GPP$$module$validate', true);
        if (fxn) po.$$fxn('module$validate', $$T_Met, fxn);
// module TMS320C3430DSP
    var cap = $om.$$bind('ti.bios.power.TMS320C3430DSP$$capsule', xdc.loadCapsule('ti/bios/power/TMS320C3430DSP.xs'));
    var po = $om['ti.bios.power.TMS320C3430DSP.Module'].$$init('ti.bios.power.TMS320C3430DSP.Module', $om['ti.bios.power.ITMS320CDM644x.Module']);
        po.$$fld('$hostonly', $$T_Num, 1, 'r');
        po.$$fld('CONTEXT_BUFFER_MAX_SIZE', $$T_CNum('(xdc_UInt)'), 0x40000, 'rh');
        po.$$fld('dspRC', $om['ti.bios.power.TMS320C3430DSP.RuntimeContext'], $$SO({len: 0x40000, useUserFxns: false, saveFxn: "", restoreFxn: ""}), 'wh');
        po.$$fld('edmaChannel', $$T_CNum('(xdc_UInt32)'), 45, 'wh');
        po.$$fld('targetMemoryRangeName', $$T_Str, "", 'wh');
        po.$$fld('CPU_NAME', $$T_Str, "DSP", 'rh');
        po.$$fld('stopBiosClockOnHibernate', $$T_Bool, undefined, 'wh');
        po.$$fld('stopCpuLoadMonitorOnHibernate', $$T_Bool, true, 'wh');
        po.$$fld('wugenEventMask', $$PArr($$T_CNum('(xdc_UInt32)'), false, 3), $$DEFAULT, 'wh');
        var fxn = cap['module$meta$init'];
        if (fxn) $om.$$bind('ti.bios.power.TMS320C3430DSP$$module$meta$init', true);
        if (fxn) po.$$fxn('module$meta$init', $$T_Met, fxn);
        var fxn = cap['module$validate'];
        if (fxn) $om.$$bind('ti.bios.power.TMS320C3430DSP$$module$validate', true);
        if (fxn) po.$$fxn('module$validate', $$T_Met, fxn);
// struct TMS320C3430DSP.RuntimeContext
    var po = $om['ti.bios.power.TMS320C3430DSP$$RuntimeContext'].$$init('ti.bios.power.TMS320C3430DSP.RuntimeContext', null);
        po.$$fld('$hostonly', $$T_Num, 1, 'r');
        po.$$fld('len', $$T_CNum('(xdc_UInt32)'), undefined, 'w');
        po.$$fld('useUserFxns', $$T_Bool, undefined, 'w');
        po.$$fld('saveFxn', $$T_Str, undefined, 'w');
        po.$$fld('restoreFxn', $$T_Str, undefined, 'w');
// module OMAP2530GPP
    var cap = $om.$$bind('ti.bios.power.OMAP2530GPP$$capsule', xdc.loadCapsule('ti/bios/power/OMAP2530GPP.xs'));
    var po = $om['ti.bios.power.OMAP2530GPP.Module'].$$init('ti.bios.power.OMAP2530GPP.Module', $om['ti.bios.power.ITMS320CDM644x.Module']);
        po.$$fld('$hostonly', $$T_Num, 1, 'r');
        po.$$fld('CPU_NAME', $$T_Str, "GPP", 'rh');
        var fxn = cap['module$meta$init'];
        if (fxn) $om.$$bind('ti.bios.power.OMAP2530GPP$$module$meta$init', true);
        if (fxn) po.$$fxn('module$meta$init', $$T_Met, fxn);
        var fxn = cap['module$validate'];
        if (fxn) $om.$$bind('ti.bios.power.OMAP2530GPP$$module$validate', true);
        if (fxn) po.$$fxn('module$validate', $$T_Met, fxn);
// module OMAP2530DSP
    var cap = $om.$$bind('ti.bios.power.OMAP2530DSP$$capsule', xdc.loadCapsule('ti/bios/power/OMAP2530DSP.xs'));
    var po = $om['ti.bios.power.OMAP2530DSP.Module'].$$init('ti.bios.power.OMAP2530DSP.Module', $om['ti.bios.power.ITMS320CDM644x.Module']);
        po.$$fld('$hostonly', $$T_Num, 1, 'r');
        po.$$fld('CONTEXT_BUFFER_MAX_SIZE', $$T_CNum('(xdc_UInt)'), 0x40000, 'rh');
        po.$$fld('dspRC', $om['ti.bios.power.OMAP2530DSP.RuntimeContext'], $$SO({len: 0x40000, useUserFxns: false, saveFxn: "", restoreFxn: ""}), 'wh');
        po.$$fld('edmaChannel', $$T_CNum('(xdc_UInt32)'), 45, 'wh');
        po.$$fld('targetMemoryRangeName', $$T_Str, "", 'wh');
        po.$$fld('CPU_NAME', $$T_Str, "DSP", 'rh');
        po.$$fld('stopBiosClockOnHibernate', $$T_Bool, undefined, 'wh');
        po.$$fld('stopCpuLoadMonitorOnHibernate', $$T_Bool, true, 'wh');
        po.$$fld('wugenEventMask', $$PArr($$T_CNum('(xdc_UInt32)'), false, 3), $$DEFAULT, 'wh');
        var fxn = cap['module$meta$init'];
        if (fxn) $om.$$bind('ti.bios.power.OMAP2530DSP$$module$meta$init', true);
        if (fxn) po.$$fxn('module$meta$init', $$T_Met, fxn);
        var fxn = cap['module$validate'];
        if (fxn) $om.$$bind('ti.bios.power.OMAP2530DSP$$module$validate', true);
        if (fxn) po.$$fxn('module$validate', $$T_Met, fxn);
// struct OMAP2530DSP.RuntimeContext
    var po = $om['ti.bios.power.OMAP2530DSP$$RuntimeContext'].$$init('ti.bios.power.OMAP2530DSP.RuntimeContext', null);
        po.$$fld('$hostonly', $$T_Num, 1, 'r');
        po.$$fld('len', $$T_CNum('(xdc_UInt32)'), undefined, 'w');
        po.$$fld('useUserFxns', $$T_Bool, undefined, 'w');
        po.$$fld('saveFxn', $$T_Str, undefined, 'w');
        po.$$fld('restoreFxn', $$T_Str, undefined, 'w');
// module OMAP3530GPP
    var cap = $om.$$bind('ti.bios.power.OMAP3530GPP$$capsule', xdc.loadCapsule('ti/bios/power/OMAP3530GPP.xs'));
    var po = $om['ti.bios.power.OMAP3530GPP.Module'].$$init('ti.bios.power.OMAP3530GPP.Module', $om['ti.bios.power.ITMS320CDM644x.Module']);
        po.$$fld('$hostonly', $$T_Num, 1, 'r');
        po.$$fld('CPU_NAME', $$T_Str, "GPP", 'rh');
        var fxn = cap['module$meta$init'];
        if (fxn) $om.$$bind('ti.bios.power.OMAP3530GPP$$module$meta$init', true);
        if (fxn) po.$$fxn('module$meta$init', $$T_Met, fxn);
        var fxn = cap['module$validate'];
        if (fxn) $om.$$bind('ti.bios.power.OMAP3530GPP$$module$validate', true);
        if (fxn) po.$$fxn('module$validate', $$T_Met, fxn);
// module OMAP3530DSP
    var cap = $om.$$bind('ti.bios.power.OMAP3530DSP$$capsule', xdc.loadCapsule('ti/bios/power/OMAP3530DSP.xs'));
    var po = $om['ti.bios.power.OMAP3530DSP.Module'].$$init('ti.bios.power.OMAP3530DSP.Module', $om['ti.bios.power.ITMS320CDM644x.Module']);
        po.$$fld('$hostonly', $$T_Num, 1, 'r');
        po.$$fld('CONTEXT_BUFFER_MAX_SIZE', $$T_CNum('(xdc_UInt)'), 0x40000, 'rh');
        po.$$fld('dspRC', $om['ti.bios.power.OMAP3530DSP.RuntimeContext'], $$SO({len: 0x40000, useUserFxns: false, saveFxn: "", restoreFxn: ""}), 'wh');
        po.$$fld('edmaChannel', $$T_CNum('(xdc_UInt32)'), 45, 'wh');
        po.$$fld('targetMemoryRangeName', $$T_Str, "", 'wh');
        po.$$fld('CPU_NAME', $$T_Str, "DSP", 'rh');
        po.$$fld('stopBiosClockOnHibernate', $$T_Bool, undefined, 'wh');
        po.$$fld('stopCpuLoadMonitorOnHibernate', $$T_Bool, true, 'wh');
        po.$$fld('wugenEventMask', $$PArr($$T_CNum('(xdc_UInt32)'), false, 3), $$DEFAULT, 'wh');
        var fxn = cap['module$meta$init'];
        if (fxn) $om.$$bind('ti.bios.power.OMAP3530DSP$$module$meta$init', true);
        if (fxn) po.$$fxn('module$meta$init', $$T_Met, fxn);
        var fxn = cap['module$validate'];
        if (fxn) $om.$$bind('ti.bios.power.OMAP3530DSP$$module$validate', true);
        if (fxn) po.$$fxn('module$validate', $$T_Met, fxn);
// struct OMAP3530DSP.RuntimeContext
    var po = $om['ti.bios.power.OMAP3530DSP$$RuntimeContext'].$$init('ti.bios.power.OMAP3530DSP.RuntimeContext', null);
        po.$$fld('$hostonly', $$T_Num, 1, 'r');
        po.$$fld('len', $$T_CNum('(xdc_UInt32)'), undefined, 'w');
        po.$$fld('useUserFxns', $$T_Bool, undefined, 'w');
        po.$$fld('saveFxn', $$T_Str, undefined, 'w');
        po.$$fld('restoreFxn', $$T_Str, undefined, 'w');

/* ======== ROV ======== */

if (__ROV__) {


} // __ROV__

/* ======== SINGLETONS ======== */

// package ti.bios.power
    var po = $om['ti.bios.power.Package'].$$init('ti.bios.power.Package', $om['xdc.IPackage.Module']);
    var cap = $om.$$bind('xdc.IPackage$$capsule', xdc.loadCapsule('ti/bios/power/package.xs'));
        if (cap['init']) po.$$fxn('init', $om['xdc.IPackage$$init'], cap['init']);
        if (cap['close']) po.$$fxn('close', $om['xdc.IPackage$$close'], cap['close']);
        if (cap['validate']) po.$$fxn('validate', $om['xdc.IPackage$$validate'], cap['validate']);
        if (cap['exit']) po.$$fxn('exit', $om['xdc.IPackage$$exit'], cap['exit']);
        if (cap['getLibs']) po.$$fxn('getLibs', $om['xdc.IPackage$$getLibs'], cap['getLibs']);
        if (cap['getSects']) po.$$fxn('getSects', $om['xdc.IPackage$$getSects'], cap['getSects']);
    po.$$bind('$capsule', cap);
    var pkg = $om['ti.bios.power'].$$init(po, 'ti.bios.power', $$DEFAULT, false);
    pkg.$$bind('$name', 'ti.bios.power');
    pkg.$$bind('$category', 'Package');
    pkg.$$bind('$$qn', 'ti.bios.power.');
    pkg.$$bind('$spec', $$pkgspec);
    pkg.$$bind('$vers', [1, 1, 1]);
    pkg.$attr.$seal('length');
    pkg.$$bind('$imports', [
    ]);
    if (pkg.$vers.length >= 3) {
        pkg.$vers.push(Packages.xdc.services.global.Vers.getDate(xdc.csd() + '/..'));
    }
    
    pkg.build.libraries = [
        'lib/release/lpm.av5T',
        'lib/debug/lpm.av4TCE',
        'lib/debug/pwrm_DaVinci.a64P',
        'lib/release/pwrm_DaVinci.a64P',
        'lib/debug/lpm.a470uC',
        'lib/release/tal_dsplink_bios_2530.a64P',
        'lib/debug/tal_dsplink_bios_3530.a64P',
        'lib/release/lpm.av4TCE',
        'lib/debug/lpm.av5T',
        'lib/debug/tal_dsplink_bios_dm6446.a64P',
        'lib/release/lpm.a470uC',
        'lib/debug/tal_dsplink_bios_2530.a64P',
        'lib/release/tal_dsplink_bios_3530.a64P',
        'lib/release/tal_dsplink_bios_dm6446.a64P',
    ];
    
    pkg.build.libDesc = [
        [
            'lib/release/lpm.av5T',
            {
                target: 'gnu.targets.arm.GCArmv5T'
            }
        ],
        [
            'lib/debug/lpm.av4TCE',
            {
                target: 'microsoft.targets.arm.WinCE'
            }
        ],
        [
            'lib/debug/pwrm_DaVinci.a64P',
            {
                target: 'ti.targets.C64P'
            }
        ],
        [
            'lib/release/pwrm_DaVinci.a64P',
            {
                target: 'ti.targets.C64P'
            }
        ],
        [
            'lib/debug/lpm.a470uC',
            {
                target: 'gnu.targets.UCArm9'
            }
        ],
        [
            'lib/release/tal_dsplink_bios_2530.a64P',
            {
                target: 'ti.targets.C64P'
            }
        ],
        [
            'lib/debug/tal_dsplink_bios_3530.a64P',
            {
                target: 'ti.targets.C64P'
            }
        ],
        [
            'lib/release/lpm.av4TCE',
            {
                target: 'microsoft.targets.arm.WinCE'
            }
        ],
        [
            'lib/debug/lpm.av5T',
            {
                target: 'gnu.targets.arm.GCArmv5T'
            }
        ],
        [
            'lib/debug/tal_dsplink_bios_dm6446.a64P',
            {
                target: 'ti.targets.C64P'
            }
        ],
        [
            'lib/release/lpm.a470uC',
            {
                target: 'gnu.targets.UCArm9'
            }
        ],
        [
            'lib/debug/tal_dsplink_bios_2530.a64P',
            {
                target: 'ti.targets.C64P'
            }
        ],
        [
            'lib/release/tal_dsplink_bios_3530.a64P',
            {
                target: 'ti.targets.C64P'
            }
        ],
        [
            'lib/release/tal_dsplink_bios_dm6446.a64P',
            {
                target: 'ti.targets.C64P'
            }
        ],
    ];
    if ('suffix' in xdc.om['xdc.IPackage$$LibDesc']) {
        pkg.build.libDesc['lib/release/lpm.av5T'].suffix = 'v5T';
        pkg.build.libDesc['lib/debug/lpm.av4TCE'].suffix = 'v4TCE';
        pkg.build.libDesc['lib/debug/pwrm_DaVinci.a64P'].suffix = '64P';
        pkg.build.libDesc['lib/release/pwrm_DaVinci.a64P'].suffix = '64P';
        pkg.build.libDesc['lib/debug/lpm.a470uC'].suffix = '470uC';
        pkg.build.libDesc['lib/release/tal_dsplink_bios_2530.a64P'].suffix = '64P';
        pkg.build.libDesc['lib/debug/tal_dsplink_bios_3530.a64P'].suffix = '64P';
        pkg.build.libDesc['lib/release/lpm.av4TCE'].suffix = 'v4TCE';
        pkg.build.libDesc['lib/debug/lpm.av5T'].suffix = 'v5T';
        pkg.build.libDesc['lib/debug/tal_dsplink_bios_dm6446.a64P'].suffix = '64P';
        pkg.build.libDesc['lib/release/lpm.a470uC'].suffix = '470uC';
        pkg.build.libDesc['lib/debug/tal_dsplink_bios_2530.a64P'].suffix = '64P';
        pkg.build.libDesc['lib/release/tal_dsplink_bios_3530.a64P'].suffix = '64P';
        pkg.build.libDesc['lib/release/tal_dsplink_bios_dm6446.a64P'].suffix = '64P';
    }
    
// interface ITMS320CDM644x
    var vo = $om['ti.bios.power.ITMS320CDM644x'];
    var po = $om['ti.bios.power.ITMS320CDM644x.Module'];
    vo.$$init(po, 'ti.bios.power.ITMS320CDM644x', $$DEFAULT, false);
    vo.$$bind('Module', po);
    vo.$$bind('$category', 'Interface');
    vo.$$bind('$spec', xdc.$$ses.findUnit('ti.bios.power.ITMS320CDM644x'));
    vo.$$bind('$capsule', $om['ti.bios.power.ITMS320CDM644x$$capsule']);
    vo.$$bind('$package', $om['ti.bios.power']);
    vo.$$bind('$$tdefs', []);
    vo.$$bind('$$proxies', []);
    vo.$$bind('OpersysCfg', $om['ti.bios.power.ITMS320CDM644x.OpersysCfg']);
    vo.$$bind('TransportCfg', $om['ti.bios.power.ITMS320CDM644x.TransportCfg']);
    vo.$$bind('Opersys_PrOS_DaVinci', $om['ti.bios.power.ITMS320CDM644x.Opersys_PrOS_DaVinci']);
    vo.$$bind('Opersys_Linux', $om['ti.bios.power.ITMS320CDM644x.Opersys_Linux']);
    vo.$$bind('Opersys_DSPBIOS', $om['ti.bios.power.ITMS320CDM644x.Opersys_DSPBIOS']);
    vo.$$bind('Transport_Link_PrOS_DaVinci', $om['ti.bios.power.ITMS320CDM644x.Transport_Link_PrOS_DaVinci']);
    vo.$$bind('Transport_Link_Linux', $om['ti.bios.power.ITMS320CDM644x.Transport_Link_Linux']);
    pkg.$interfaces.$add(vo);
    pkg.$$bind('ITMS320CDM644x', vo);
    pkg.$unitNames.$add('ITMS320CDM644x');
    vo.$seal();
// module TMS320CDM6441GPP
    var vo = $om['ti.bios.power.TMS320CDM6441GPP'];
    var po = $om['ti.bios.power.TMS320CDM6441GPP.Module'];
    vo.$$init(po, 'ti.bios.power.TMS320CDM6441GPP', $$DEFAULT, false);
    vo.$$bind('Module', po);
    vo.$$bind('$category', 'Module');
    vo.$$bind('$spec', xdc.$$ses.findUnit('ti.bios.power.TMS320CDM6441GPP'));
    vo.$$bind('$capsule', $om['ti.bios.power.TMS320CDM6441GPP$$capsule']);
    vo.$$bind('$package', $om['ti.bios.power']);
    vo.$$bind('$$tdefs', []);
    vo.$$bind('$$proxies', []);
    vo.$$bind('$$mcfgs', []);
    vo.$$bind('OpersysCfg', $om['ti.bios.power.ITMS320CDM644x.OpersysCfg']);
    vo.$$bind('TransportCfg', $om['ti.bios.power.ITMS320CDM644x.TransportCfg']);
    vo.$$bind('Opersys_PrOS_DaVinci', $om['ti.bios.power.ITMS320CDM644x.Opersys_PrOS_DaVinci']);
    vo.$$bind('Opersys_Linux', $om['ti.bios.power.ITMS320CDM644x.Opersys_Linux']);
    vo.$$bind('Opersys_DSPBIOS', $om['ti.bios.power.ITMS320CDM644x.Opersys_DSPBIOS']);
    vo.$$bind('Transport_Link_PrOS_DaVinci', $om['ti.bios.power.ITMS320CDM644x.Transport_Link_PrOS_DaVinci']);
    vo.$$bind('Transport_Link_Linux', $om['ti.bios.power.ITMS320CDM644x.Transport_Link_Linux']);
    pkg.$modules.$add(vo);
    $om.$modules.$add(vo);
    vo.$$bind('$$instflag', 0);
    vo.$$bind('$$iobjflag', 1);
    vo.$$bind('$$sizeflag', 1);
    vo.$$bind('$$dlgflag', 0);
    vo.$$bind('$$iflag', 1);
    vo.$$bind('$$romcfgs', '|');
    if ('Module_State' in vo) vo.$$bind('$object', new vo.Module_State);
    vo.$$bind('$$meta_iobj', 0 + ('ti.bios.power.TMS320CDM6441GPP$$instance$static$init' in $om));
    vo.$$bind('$$fxntab', []);
    vo.$$bind('$$logEvtCfgs', []);
    vo.$$bind('$$errorDescCfgs', []);
    vo.$$bind('$$assertDescCfgs', []);
    vo.$attr['@Template:"./config.xdt"'] = "./config.xdt";
    vo.$attr.$seal('length');
    vo.$$bind('TEMPLATE$', './config.xdt');
    pkg.$$bind('TMS320CDM6441GPP', vo);
    pkg.$unitNames.$add('TMS320CDM6441GPP');
// module TMS320CDM6441DSP
    var vo = $om['ti.bios.power.TMS320CDM6441DSP'];
    var po = $om['ti.bios.power.TMS320CDM6441DSP.Module'];
    vo.$$init(po, 'ti.bios.power.TMS320CDM6441DSP', $$DEFAULT, false);
    vo.$$bind('Module', po);
    vo.$$bind('$category', 'Module');
    vo.$$bind('$spec', xdc.$$ses.findUnit('ti.bios.power.TMS320CDM6441DSP'));
    vo.$$bind('$capsule', $om['ti.bios.power.TMS320CDM6441DSP$$capsule']);
    vo.$$bind('$package', $om['ti.bios.power']);
    vo.$$bind('$$tdefs', []);
    vo.$$bind('$$proxies', []);
    vo.$$bind('$$mcfgs', []);
    vo.$$bind('OpersysCfg', $om['ti.bios.power.ITMS320CDM644x.OpersysCfg']);
    vo.$$bind('TransportCfg', $om['ti.bios.power.ITMS320CDM644x.TransportCfg']);
    vo.$$bind('RuntimeContext', $om['ti.bios.power.TMS320CDM6441DSP.RuntimeContext']);
    vo.$$tdefs.push($om['ti.bios.power.TMS320CDM6441DSP.RuntimeContext']);
    vo.$$bind('Opersys_PrOS_DaVinci', $om['ti.bios.power.ITMS320CDM644x.Opersys_PrOS_DaVinci']);
    vo.$$bind('Opersys_Linux', $om['ti.bios.power.ITMS320CDM644x.Opersys_Linux']);
    vo.$$bind('Opersys_DSPBIOS', $om['ti.bios.power.ITMS320CDM644x.Opersys_DSPBIOS']);
    vo.$$bind('Transport_Link_PrOS_DaVinci', $om['ti.bios.power.ITMS320CDM644x.Transport_Link_PrOS_DaVinci']);
    vo.$$bind('Transport_Link_Linux', $om['ti.bios.power.ITMS320CDM644x.Transport_Link_Linux']);
    pkg.$modules.$add(vo);
    $om.$modules.$add(vo);
    vo.$$bind('$$instflag', 0);
    vo.$$bind('$$iobjflag', 1);
    vo.$$bind('$$sizeflag', 1);
    vo.$$bind('$$dlgflag', 0);
    vo.$$bind('$$iflag', 1);
    vo.$$bind('$$romcfgs', '|');
    if ('Module_State' in vo) vo.$$bind('$object', new vo.Module_State);
    vo.$$bind('$$meta_iobj', 0 + ('ti.bios.power.TMS320CDM6441DSP$$instance$static$init' in $om));
    vo.$$bind('$$fxntab', []);
    vo.$$bind('$$logEvtCfgs', []);
    vo.$$bind('$$errorDescCfgs', []);
    vo.$$bind('$$assertDescCfgs', []);
    vo.$attr['@Template:"./config.xdt"'] = "./config.xdt";
    vo.$attr.$seal('length');
    vo.$$bind('TEMPLATE$', './config.xdt');
    pkg.$$bind('TMS320CDM6441DSP', vo);
    pkg.$unitNames.$add('TMS320CDM6441DSP');
// module TMS320CDM6446GPP
    var vo = $om['ti.bios.power.TMS320CDM6446GPP'];
    var po = $om['ti.bios.power.TMS320CDM6446GPP.Module'];
    vo.$$init(po, 'ti.bios.power.TMS320CDM6446GPP', $$DEFAULT, false);
    vo.$$bind('Module', po);
    vo.$$bind('$category', 'Module');
    vo.$$bind('$spec', xdc.$$ses.findUnit('ti.bios.power.TMS320CDM6446GPP'));
    vo.$$bind('$capsule', $om['ti.bios.power.TMS320CDM6446GPP$$capsule']);
    vo.$$bind('$package', $om['ti.bios.power']);
    vo.$$bind('$$tdefs', []);
    vo.$$bind('$$proxies', []);
    vo.$$bind('$$mcfgs', []);
    vo.$$bind('OpersysCfg', $om['ti.bios.power.ITMS320CDM644x.OpersysCfg']);
    vo.$$bind('TransportCfg', $om['ti.bios.power.ITMS320CDM644x.TransportCfg']);
    vo.$$bind('Opersys_PrOS_DaVinci', $om['ti.bios.power.ITMS320CDM644x.Opersys_PrOS_DaVinci']);
    vo.$$bind('Opersys_Linux', $om['ti.bios.power.ITMS320CDM644x.Opersys_Linux']);
    vo.$$bind('Opersys_DSPBIOS', $om['ti.bios.power.ITMS320CDM644x.Opersys_DSPBIOS']);
    vo.$$bind('Transport_Link_PrOS_DaVinci', $om['ti.bios.power.ITMS320CDM644x.Transport_Link_PrOS_DaVinci']);
    vo.$$bind('Transport_Link_Linux', $om['ti.bios.power.ITMS320CDM644x.Transport_Link_Linux']);
    pkg.$modules.$add(vo);
    $om.$modules.$add(vo);
    vo.$$bind('$$instflag', 0);
    vo.$$bind('$$iobjflag', 1);
    vo.$$bind('$$sizeflag', 1);
    vo.$$bind('$$dlgflag', 0);
    vo.$$bind('$$iflag', 1);
    vo.$$bind('$$romcfgs', '|');
    if ('Module_State' in vo) vo.$$bind('$object', new vo.Module_State);
    vo.$$bind('$$meta_iobj', 0 + ('ti.bios.power.TMS320CDM6446GPP$$instance$static$init' in $om));
    vo.$$bind('$$fxntab', []);
    vo.$$bind('$$logEvtCfgs', []);
    vo.$$bind('$$errorDescCfgs', []);
    vo.$$bind('$$assertDescCfgs', []);
    vo.$attr['@Template:"./config.xdt"'] = "./config.xdt";
    vo.$attr.$seal('length');
    vo.$$bind('TEMPLATE$', './config.xdt');
    pkg.$$bind('TMS320CDM6446GPP', vo);
    pkg.$unitNames.$add('TMS320CDM6446GPP');
// module TMS320CDM6446DSP
    var vo = $om['ti.bios.power.TMS320CDM6446DSP'];
    var po = $om['ti.bios.power.TMS320CDM6446DSP.Module'];
    vo.$$init(po, 'ti.bios.power.TMS320CDM6446DSP', $$DEFAULT, false);
    vo.$$bind('Module', po);
    vo.$$bind('$category', 'Module');
    vo.$$bind('$spec', xdc.$$ses.findUnit('ti.bios.power.TMS320CDM6446DSP'));
    vo.$$bind('$capsule', $om['ti.bios.power.TMS320CDM6446DSP$$capsule']);
    vo.$$bind('$package', $om['ti.bios.power']);
    vo.$$bind('$$tdefs', []);
    vo.$$bind('$$proxies', []);
    vo.$$bind('$$mcfgs', []);
    vo.$$bind('OpersysCfg', $om['ti.bios.power.ITMS320CDM644x.OpersysCfg']);
    vo.$$bind('TransportCfg', $om['ti.bios.power.ITMS320CDM644x.TransportCfg']);
    vo.$$bind('RuntimeContext', $om['ti.bios.power.TMS320CDM6446DSP.RuntimeContext']);
    vo.$$tdefs.push($om['ti.bios.power.TMS320CDM6446DSP.RuntimeContext']);
    vo.$$bind('Opersys_PrOS_DaVinci', $om['ti.bios.power.ITMS320CDM644x.Opersys_PrOS_DaVinci']);
    vo.$$bind('Opersys_Linux', $om['ti.bios.power.ITMS320CDM644x.Opersys_Linux']);
    vo.$$bind('Opersys_DSPBIOS', $om['ti.bios.power.ITMS320CDM644x.Opersys_DSPBIOS']);
    vo.$$bind('Transport_Link_PrOS_DaVinci', $om['ti.bios.power.ITMS320CDM644x.Transport_Link_PrOS_DaVinci']);
    vo.$$bind('Transport_Link_Linux', $om['ti.bios.power.ITMS320CDM644x.Transport_Link_Linux']);
    pkg.$modules.$add(vo);
    $om.$modules.$add(vo);
    vo.$$bind('$$instflag', 0);
    vo.$$bind('$$iobjflag', 1);
    vo.$$bind('$$sizeflag', 1);
    vo.$$bind('$$dlgflag', 0);
    vo.$$bind('$$iflag', 1);
    vo.$$bind('$$romcfgs', '|');
    if ('Module_State' in vo) vo.$$bind('$object', new vo.Module_State);
    vo.$$bind('$$meta_iobj', 0 + ('ti.bios.power.TMS320CDM6446DSP$$instance$static$init' in $om));
    vo.$$bind('$$fxntab', []);
    vo.$$bind('$$logEvtCfgs', []);
    vo.$$bind('$$errorDescCfgs', []);
    vo.$$bind('$$assertDescCfgs', []);
    vo.$attr['@Template:"./config.xdt"'] = "./config.xdt";
    vo.$attr.$seal('length');
    vo.$$bind('TEMPLATE$', './config.xdt');
    pkg.$$bind('TMS320CDM6446DSP', vo);
    pkg.$unitNames.$add('TMS320CDM6446DSP');
// module TMS320C3430GPP
    var vo = $om['ti.bios.power.TMS320C3430GPP'];
    var po = $om['ti.bios.power.TMS320C3430GPP.Module'];
    vo.$$init(po, 'ti.bios.power.TMS320C3430GPP', $$DEFAULT, false);
    vo.$$bind('Module', po);
    vo.$$bind('$category', 'Module');
    vo.$$bind('$spec', xdc.$$ses.findUnit('ti.bios.power.TMS320C3430GPP'));
    vo.$$bind('$capsule', $om['ti.bios.power.TMS320C3430GPP$$capsule']);
    vo.$$bind('$package', $om['ti.bios.power']);
    vo.$$bind('$$tdefs', []);
    vo.$$bind('$$proxies', []);
    vo.$$bind('$$mcfgs', []);
    vo.$$bind('OpersysCfg', $om['ti.bios.power.ITMS320CDM644x.OpersysCfg']);
    vo.$$bind('TransportCfg', $om['ti.bios.power.ITMS320CDM644x.TransportCfg']);
    vo.$$bind('Opersys_PrOS_DaVinci', $om['ti.bios.power.ITMS320CDM644x.Opersys_PrOS_DaVinci']);
    vo.$$bind('Opersys_Linux', $om['ti.bios.power.ITMS320CDM644x.Opersys_Linux']);
    vo.$$bind('Opersys_DSPBIOS', $om['ti.bios.power.ITMS320CDM644x.Opersys_DSPBIOS']);
    vo.$$bind('Transport_Link_PrOS_DaVinci', $om['ti.bios.power.ITMS320CDM644x.Transport_Link_PrOS_DaVinci']);
    vo.$$bind('Transport_Link_Linux', $om['ti.bios.power.ITMS320CDM644x.Transport_Link_Linux']);
    pkg.$modules.$add(vo);
    $om.$modules.$add(vo);
    vo.$$bind('$$instflag', 0);
    vo.$$bind('$$iobjflag', 1);
    vo.$$bind('$$sizeflag', 1);
    vo.$$bind('$$dlgflag', 0);
    vo.$$bind('$$iflag', 1);
    vo.$$bind('$$romcfgs', '|');
    if ('Module_State' in vo) vo.$$bind('$object', new vo.Module_State);
    vo.$$bind('$$meta_iobj', 0 + ('ti.bios.power.TMS320C3430GPP$$instance$static$init' in $om));
    vo.$$bind('$$fxntab', []);
    vo.$$bind('$$logEvtCfgs', []);
    vo.$$bind('$$errorDescCfgs', []);
    vo.$$bind('$$assertDescCfgs', []);
    vo.$attr['@Template:"./config.xdt"'] = "./config.xdt";
    vo.$attr.$seal('length');
    vo.$$bind('TEMPLATE$', './config.xdt');
    pkg.$$bind('TMS320C3430GPP', vo);
    pkg.$unitNames.$add('TMS320C3430GPP');
// module TMS320C3430DSP
    var vo = $om['ti.bios.power.TMS320C3430DSP'];
    var po = $om['ti.bios.power.TMS320C3430DSP.Module'];
    vo.$$init(po, 'ti.bios.power.TMS320C3430DSP', $$DEFAULT, false);
    vo.$$bind('Module', po);
    vo.$$bind('$category', 'Module');
    vo.$$bind('$spec', xdc.$$ses.findUnit('ti.bios.power.TMS320C3430DSP'));
    vo.$$bind('$capsule', $om['ti.bios.power.TMS320C3430DSP$$capsule']);
    vo.$$bind('$package', $om['ti.bios.power']);
    vo.$$bind('$$tdefs', []);
    vo.$$bind('$$proxies', []);
    vo.$$bind('$$mcfgs', []);
    vo.$$bind('OpersysCfg', $om['ti.bios.power.ITMS320CDM644x.OpersysCfg']);
    vo.$$bind('TransportCfg', $om['ti.bios.power.ITMS320CDM644x.TransportCfg']);
    vo.$$bind('RuntimeContext', $om['ti.bios.power.TMS320C3430DSP.RuntimeContext']);
    vo.$$tdefs.push($om['ti.bios.power.TMS320C3430DSP.RuntimeContext']);
    vo.$$bind('Opersys_PrOS_DaVinci', $om['ti.bios.power.ITMS320CDM644x.Opersys_PrOS_DaVinci']);
    vo.$$bind('Opersys_Linux', $om['ti.bios.power.ITMS320CDM644x.Opersys_Linux']);
    vo.$$bind('Opersys_DSPBIOS', $om['ti.bios.power.ITMS320CDM644x.Opersys_DSPBIOS']);
    vo.$$bind('Transport_Link_PrOS_DaVinci', $om['ti.bios.power.ITMS320CDM644x.Transport_Link_PrOS_DaVinci']);
    vo.$$bind('Transport_Link_Linux', $om['ti.bios.power.ITMS320CDM644x.Transport_Link_Linux']);
    pkg.$modules.$add(vo);
    $om.$modules.$add(vo);
    vo.$$bind('$$instflag', 0);
    vo.$$bind('$$iobjflag', 1);
    vo.$$bind('$$sizeflag', 1);
    vo.$$bind('$$dlgflag', 0);
    vo.$$bind('$$iflag', 1);
    vo.$$bind('$$romcfgs', '|');
    if ('Module_State' in vo) vo.$$bind('$object', new vo.Module_State);
    vo.$$bind('$$meta_iobj', 0 + ('ti.bios.power.TMS320C3430DSP$$instance$static$init' in $om));
    vo.$$bind('$$fxntab', []);
    vo.$$bind('$$logEvtCfgs', []);
    vo.$$bind('$$errorDescCfgs', []);
    vo.$$bind('$$assertDescCfgs', []);
    vo.$attr['@Template:"./config.xdt"'] = "./config.xdt";
    vo.$attr.$seal('length');
    vo.$$bind('TEMPLATE$', './config.xdt');
    pkg.$$bind('TMS320C3430DSP', vo);
    pkg.$unitNames.$add('TMS320C3430DSP');
// module OMAP2530GPP
    var vo = $om['ti.bios.power.OMAP2530GPP'];
    var po = $om['ti.bios.power.OMAP2530GPP.Module'];
    vo.$$init(po, 'ti.bios.power.OMAP2530GPP', $$DEFAULT, false);
    vo.$$bind('Module', po);
    vo.$$bind('$category', 'Module');
    vo.$$bind('$spec', xdc.$$ses.findUnit('ti.bios.power.OMAP2530GPP'));
    vo.$$bind('$capsule', $om['ti.bios.power.OMAP2530GPP$$capsule']);
    vo.$$bind('$package', $om['ti.bios.power']);
    vo.$$bind('$$tdefs', []);
    vo.$$bind('$$proxies', []);
    vo.$$bind('$$mcfgs', []);
    vo.$$bind('OpersysCfg', $om['ti.bios.power.ITMS320CDM644x.OpersysCfg']);
    vo.$$bind('TransportCfg', $om['ti.bios.power.ITMS320CDM644x.TransportCfg']);
    vo.$$bind('Opersys_PrOS_DaVinci', $om['ti.bios.power.ITMS320CDM644x.Opersys_PrOS_DaVinci']);
    vo.$$bind('Opersys_Linux', $om['ti.bios.power.ITMS320CDM644x.Opersys_Linux']);
    vo.$$bind('Opersys_DSPBIOS', $om['ti.bios.power.ITMS320CDM644x.Opersys_DSPBIOS']);
    vo.$$bind('Transport_Link_PrOS_DaVinci', $om['ti.bios.power.ITMS320CDM644x.Transport_Link_PrOS_DaVinci']);
    vo.$$bind('Transport_Link_Linux', $om['ti.bios.power.ITMS320CDM644x.Transport_Link_Linux']);
    pkg.$modules.$add(vo);
    $om.$modules.$add(vo);
    vo.$$bind('$$instflag', 0);
    vo.$$bind('$$iobjflag', 1);
    vo.$$bind('$$sizeflag', 1);
    vo.$$bind('$$dlgflag', 0);
    vo.$$bind('$$iflag', 1);
    vo.$$bind('$$romcfgs', '|');
    if ('Module_State' in vo) vo.$$bind('$object', new vo.Module_State);
    vo.$$bind('$$meta_iobj', 0 + ('ti.bios.power.OMAP2530GPP$$instance$static$init' in $om));
    vo.$$bind('$$fxntab', []);
    vo.$$bind('$$logEvtCfgs', []);
    vo.$$bind('$$errorDescCfgs', []);
    vo.$$bind('$$assertDescCfgs', []);
    vo.$attr['@Template:"./config.xdt"'] = "./config.xdt";
    vo.$attr.$seal('length');
    vo.$$bind('TEMPLATE$', './config.xdt');
    pkg.$$bind('OMAP2530GPP', vo);
    pkg.$unitNames.$add('OMAP2530GPP');
// module OMAP2530DSP
    var vo = $om['ti.bios.power.OMAP2530DSP'];
    var po = $om['ti.bios.power.OMAP2530DSP.Module'];
    vo.$$init(po, 'ti.bios.power.OMAP2530DSP', $$DEFAULT, false);
    vo.$$bind('Module', po);
    vo.$$bind('$category', 'Module');
    vo.$$bind('$spec', xdc.$$ses.findUnit('ti.bios.power.OMAP2530DSP'));
    vo.$$bind('$capsule', $om['ti.bios.power.OMAP2530DSP$$capsule']);
    vo.$$bind('$package', $om['ti.bios.power']);
    vo.$$bind('$$tdefs', []);
    vo.$$bind('$$proxies', []);
    vo.$$bind('$$mcfgs', []);
    vo.$$bind('OpersysCfg', $om['ti.bios.power.ITMS320CDM644x.OpersysCfg']);
    vo.$$bind('TransportCfg', $om['ti.bios.power.ITMS320CDM644x.TransportCfg']);
    vo.$$bind('RuntimeContext', $om['ti.bios.power.OMAP2530DSP.RuntimeContext']);
    vo.$$tdefs.push($om['ti.bios.power.OMAP2530DSP.RuntimeContext']);
    vo.$$bind('Opersys_PrOS_DaVinci', $om['ti.bios.power.ITMS320CDM644x.Opersys_PrOS_DaVinci']);
    vo.$$bind('Opersys_Linux', $om['ti.bios.power.ITMS320CDM644x.Opersys_Linux']);
    vo.$$bind('Opersys_DSPBIOS', $om['ti.bios.power.ITMS320CDM644x.Opersys_DSPBIOS']);
    vo.$$bind('Transport_Link_PrOS_DaVinci', $om['ti.bios.power.ITMS320CDM644x.Transport_Link_PrOS_DaVinci']);
    vo.$$bind('Transport_Link_Linux', $om['ti.bios.power.ITMS320CDM644x.Transport_Link_Linux']);
    pkg.$modules.$add(vo);
    $om.$modules.$add(vo);
    vo.$$bind('$$instflag', 0);
    vo.$$bind('$$iobjflag', 1);
    vo.$$bind('$$sizeflag', 1);
    vo.$$bind('$$dlgflag', 0);
    vo.$$bind('$$iflag', 1);
    vo.$$bind('$$romcfgs', '|');
    if ('Module_State' in vo) vo.$$bind('$object', new vo.Module_State);
    vo.$$bind('$$meta_iobj', 0 + ('ti.bios.power.OMAP2530DSP$$instance$static$init' in $om));
    vo.$$bind('$$fxntab', []);
    vo.$$bind('$$logEvtCfgs', []);
    vo.$$bind('$$errorDescCfgs', []);
    vo.$$bind('$$assertDescCfgs', []);
    vo.$attr['@Template:"./config.xdt"'] = "./config.xdt";
    vo.$attr.$seal('length');
    vo.$$bind('TEMPLATE$', './config.xdt');
    pkg.$$bind('OMAP2530DSP', vo);
    pkg.$unitNames.$add('OMAP2530DSP');
// module OMAP3530GPP
    var vo = $om['ti.bios.power.OMAP3530GPP'];
    var po = $om['ti.bios.power.OMAP3530GPP.Module'];
    vo.$$init(po, 'ti.bios.power.OMAP3530GPP', $$DEFAULT, false);
    vo.$$bind('Module', po);
    vo.$$bind('$category', 'Module');
    vo.$$bind('$spec', xdc.$$ses.findUnit('ti.bios.power.OMAP3530GPP'));
    vo.$$bind('$capsule', $om['ti.bios.power.OMAP3530GPP$$capsule']);
    vo.$$bind('$package', $om['ti.bios.power']);
    vo.$$bind('$$tdefs', []);
    vo.$$bind('$$proxies', []);
    vo.$$bind('$$mcfgs', []);
    vo.$$bind('OpersysCfg', $om['ti.bios.power.ITMS320CDM644x.OpersysCfg']);
    vo.$$bind('TransportCfg', $om['ti.bios.power.ITMS320CDM644x.TransportCfg']);
    vo.$$bind('Opersys_PrOS_DaVinci', $om['ti.bios.power.ITMS320CDM644x.Opersys_PrOS_DaVinci']);
    vo.$$bind('Opersys_Linux', $om['ti.bios.power.ITMS320CDM644x.Opersys_Linux']);
    vo.$$bind('Opersys_DSPBIOS', $om['ti.bios.power.ITMS320CDM644x.Opersys_DSPBIOS']);
    vo.$$bind('Transport_Link_PrOS_DaVinci', $om['ti.bios.power.ITMS320CDM644x.Transport_Link_PrOS_DaVinci']);
    vo.$$bind('Transport_Link_Linux', $om['ti.bios.power.ITMS320CDM644x.Transport_Link_Linux']);
    pkg.$modules.$add(vo);
    $om.$modules.$add(vo);
    vo.$$bind('$$instflag', 0);
    vo.$$bind('$$iobjflag', 1);
    vo.$$bind('$$sizeflag', 1);
    vo.$$bind('$$dlgflag', 0);
    vo.$$bind('$$iflag', 1);
    vo.$$bind('$$romcfgs', '|');
    if ('Module_State' in vo) vo.$$bind('$object', new vo.Module_State);
    vo.$$bind('$$meta_iobj', 0 + ('ti.bios.power.OMAP3530GPP$$instance$static$init' in $om));
    vo.$$bind('$$fxntab', []);
    vo.$$bind('$$logEvtCfgs', []);
    vo.$$bind('$$errorDescCfgs', []);
    vo.$$bind('$$assertDescCfgs', []);
    vo.$attr['@Template:"./config.xdt"'] = "./config.xdt";
    vo.$attr.$seal('length');
    vo.$$bind('TEMPLATE$', './config.xdt');
    pkg.$$bind('OMAP3530GPP', vo);
    pkg.$unitNames.$add('OMAP3530GPP');
// module OMAP3530DSP
    var vo = $om['ti.bios.power.OMAP3530DSP'];
    var po = $om['ti.bios.power.OMAP3530DSP.Module'];
    vo.$$init(po, 'ti.bios.power.OMAP3530DSP', $$DEFAULT, false);
    vo.$$bind('Module', po);
    vo.$$bind('$category', 'Module');
    vo.$$bind('$spec', xdc.$$ses.findUnit('ti.bios.power.OMAP3530DSP'));
    vo.$$bind('$capsule', $om['ti.bios.power.OMAP3530DSP$$capsule']);
    vo.$$bind('$package', $om['ti.bios.power']);
    vo.$$bind('$$tdefs', []);
    vo.$$bind('$$proxies', []);
    vo.$$bind('$$mcfgs', []);
    vo.$$bind('OpersysCfg', $om['ti.bios.power.ITMS320CDM644x.OpersysCfg']);
    vo.$$bind('TransportCfg', $om['ti.bios.power.ITMS320CDM644x.TransportCfg']);
    vo.$$bind('RuntimeContext', $om['ti.bios.power.OMAP3530DSP.RuntimeContext']);
    vo.$$tdefs.push($om['ti.bios.power.OMAP3530DSP.RuntimeContext']);
    vo.$$bind('Opersys_PrOS_DaVinci', $om['ti.bios.power.ITMS320CDM644x.Opersys_PrOS_DaVinci']);
    vo.$$bind('Opersys_Linux', $om['ti.bios.power.ITMS320CDM644x.Opersys_Linux']);
    vo.$$bind('Opersys_DSPBIOS', $om['ti.bios.power.ITMS320CDM644x.Opersys_DSPBIOS']);
    vo.$$bind('Transport_Link_PrOS_DaVinci', $om['ti.bios.power.ITMS320CDM644x.Transport_Link_PrOS_DaVinci']);
    vo.$$bind('Transport_Link_Linux', $om['ti.bios.power.ITMS320CDM644x.Transport_Link_Linux']);
    pkg.$modules.$add(vo);
    $om.$modules.$add(vo);
    vo.$$bind('$$instflag', 0);
    vo.$$bind('$$iobjflag', 1);
    vo.$$bind('$$sizeflag', 1);
    vo.$$bind('$$dlgflag', 0);
    vo.$$bind('$$iflag', 1);
    vo.$$bind('$$romcfgs', '|');
    if ('Module_State' in vo) vo.$$bind('$object', new vo.Module_State);
    vo.$$bind('$$meta_iobj', 0 + ('ti.bios.power.OMAP3530DSP$$instance$static$init' in $om));
    vo.$$bind('$$fxntab', []);
    vo.$$bind('$$logEvtCfgs', []);
    vo.$$bind('$$errorDescCfgs', []);
    vo.$$bind('$$assertDescCfgs', []);
    vo.$attr['@Template:"./config.xdt"'] = "./config.xdt";
    vo.$attr.$seal('length');
    vo.$$bind('TEMPLATE$', './config.xdt');
    pkg.$$bind('OMAP3530DSP', vo);
    pkg.$unitNames.$add('OMAP3530DSP');

/* ======== INITIALIZATION ======== */

if (__CFG__) {
} // __CFG__
    $om['ti.bios.power.TMS320CDM6441GPP'].module$meta$init();
    $om['ti.bios.power.TMS320CDM6441DSP'].module$meta$init();
    $om['ti.bios.power.TMS320CDM6446GPP'].module$meta$init();
    $om['ti.bios.power.TMS320CDM6446DSP'].module$meta$init();
    $om['ti.bios.power.TMS320C3430GPP'].module$meta$init();
    $om['ti.bios.power.TMS320C3430DSP'].module$meta$init();
    $om['ti.bios.power.OMAP2530GPP'].module$meta$init();
    $om['ti.bios.power.OMAP2530DSP'].module$meta$init();
    $om['ti.bios.power.OMAP3530GPP'].module$meta$init();
    $om['ti.bios.power.OMAP3530DSP'].module$meta$init();
    pkg.init();
    $om['ti.bios.power.ITMS320CDM644x'].$$bless();
    $om['ti.bios.power.TMS320CDM6441GPP'].$$bless();
    $om['ti.bios.power.TMS320CDM6441DSP'].$$bless();
    $om['ti.bios.power.TMS320CDM6446GPP'].$$bless();
    $om['ti.bios.power.TMS320CDM6446DSP'].$$bless();
    $om['ti.bios.power.TMS320C3430GPP'].$$bless();
    $om['ti.bios.power.TMS320C3430DSP'].$$bless();
    $om['ti.bios.power.OMAP2530GPP'].$$bless();
    $om['ti.bios.power.OMAP2530DSP'].$$bless();
    $om['ti.bios.power.OMAP3530GPP'].$$bless();
    $om['ti.bios.power.OMAP3530DSP'].$$bless();
    $om.$packages.$add(pkg);
