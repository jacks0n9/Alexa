requires ti.mas.types[5,0,0,0];
requires ti.mas.fract[2,0,0,0];
requires ti.mas.util[4,0,0,0];
requires internal ti.mas.swtools[2,0,0,0];

/*! VPE package
 * 
 * @a(NAME)  `ti.mas.vpe`
 *
 * @a(DESCRIPTION)
 *
 *        This package creates the VPE package. It depends on the following packages:
 * @p(dlist)
 * - `ti.mas.types`
 * - `ti.mas.fract`
 * - `ti.mas.util`
 *
 * @a(CONTENTS)
 * @p(dlist)
 * - 'hlc' module
 *      provides high level compensation functionality
 * - 'slm' module
 *      provides signal limitation functionality
 * - `Version` module
 *      provides version numbering for VPE package and is described in
 *      `Version.xdc`. Note that `Version.xdc` is generated from the template file 
 *      `Version.xdt`.
 * @p
 * @a(TARGETS) The following architecture targets are supported:
 *
 * @p(dlist)
 * - `C55`
 * - `C55_large`
 * - `C64`
 * - `C64Plus`
 * @p
 * @a(BUILD)  The build options are implemented in the file `package.bld`.
 *
 * Build command: `xdc XDCARGS="option"`.
 * option1 = [c55s, c55l, c64le, c64be, c64Ple, c64Pbe, all]
 * option2 = [obj, src, docs]
 * option3 = []
 * If any other option other than above is used then the package is not built correctly
 * and will crash when used in conjunction with other packages.
 *
 *
 * Generated files:
 *
 * @p(dlist)
 * -  `Version.h`
 * @p
 *
 * Command to clean: `xdc clean`
 *
 * @a(Online API documentation)
 *
 * `xdc XDCARGS="option" gendocs`
 *
 * @a(RELEASES)
 *
 * Release creation: `xdc XDCARGS="option1 option2" [gendocs] release`
 * option1 = [c55s, c55l, c64le, c64be, c64Ple, c64Pbe]
 * option2 = [obj, src, docs]
 * option3 = []
 *
 * Using the option1 as given and option2="obj" creates the released tar file containing object code only:
 * `ti_mas_vpe_<arch>_src_<v>_<r>_<x>_<p>.tar` ???? src or obj ???
 * 
 * Using the option1 as given and option2="src" creates the released tar file containing source with object code:
 * `ti_mas_vpe_<arch>_src_<v>_<r>_<x>_<p>.tar`
 *
 * To generate document only the following should be used:
 * `xdc XDCARGS="docs" release` or `xdc XDCARGS="option1,docs"`
 * and `ti_mas_vpe_docs_<v>_<r>_<x>_<p>.tar` is created
 * 
 * If any other option other than the ones listed above is used, no release is created.
 * 
 *
 * 
 *
 * @a(Copyright)
 * @p(html)
 * &#169; 2008 Texas Instruments, Inc.
 * @p
 */

package ti.mas.vpe[2,0,3,0] {
   module Version;
}

/* nothing past this point */
