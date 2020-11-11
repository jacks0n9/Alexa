xdc.loadCapsule('xdc/om2.xs');

var $om = xdc.om;
var __CFG__ = $om.$name == 'cfg';
var __ROV__ = $om.$name == 'rov';
var $$pkgspec = xdc.$$ses.findPkg('local_power_manager_linux_1_24_03_10_eng');

/* ======== IMPORTS ======== */

    xdc.loadPackage('ti.bios.power');
    xdc.loadPackage('ti.bios.power.utils');
    xdc.loadPackage('xdc');
    xdc.loadPackage('xdc.corevers');

/* ======== OBJECTS ======== */

// package local_power_manager_linux_1_24_03_10_eng
    var pkg = $om.$$bind('local_power_manager_linux_1_24_03_10_eng.Package', $$PObj());
    $om.$$bind('local_power_manager_linux_1_24_03_10_eng', $$VObj('local_power_manager_linux_1_24_03_10_eng', pkg));

/* ======== CONSTS ======== */


/* ======== CREATES ======== */


/* ======== FUNCTIONS ======== */


/* ======== SIZES ======== */


/* ======== TYPES ======== */


/* ======== ROV ======== */

if (__ROV__) {


} // __ROV__

/* ======== SINGLETONS ======== */

// package local_power_manager_linux_1_24_03_10_eng
    var po = $om['local_power_manager_linux_1_24_03_10_eng.Package'].$$init('local_power_manager_linux_1_24_03_10_eng.Package', $om['xdc.IPackage.Module']);
    po.$$bind('$capsule', undefined);
    var pkg = $om['local_power_manager_linux_1_24_03_10_eng'].$$init(po, 'local_power_manager_linux_1_24_03_10_eng', $$DEFAULT, false);
    pkg.$$bind('$name', 'local_power_manager_linux_1_24_03_10_eng');
    pkg.$$bind('$category', 'Package');
    pkg.$$bind('$$qn', 'local_power_manager_linux_1_24_03_10_eng.');
    pkg.$$bind('$spec', $$pkgspec);
    pkg.$$bind('$vers', []);
    pkg.$attr.$seal('length');
    pkg.$$bind('$imports', [
        ['ti.bios.power', []],
        ['ti.bios.power.utils', []],
    ]);
    if (pkg.$vers.length >= 3) {
        pkg.$vers.push(Packages.xdc.services.global.Vers.getDate(xdc.csd() + '/..'));
    }
    
    pkg.build.libraries = [
    ];
    
    pkg.build.libDesc = [
    ];
    if ('suffix' in xdc.om['xdc.IPackage$$LibDesc']) {
    }
    

/* ======== INITIALIZATION ======== */

if (__CFG__) {
} // __CFG__
    pkg.init();
    $om.$packages.$add(pkg);
