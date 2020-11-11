requires ti.mas.types[5,0,7,0];
requires internal ti.mas.swtools[2,0,4,0];

/*! fract package
 * 
 * @a(NAME)  `ti.mas.fract`
 *
 * @a(DESCRIPTION)
 *        This package creates fract. It depends on the following packages:
 * @p(dlist)
 * - `ti.mas.types`
 * - `ti.mas.fract.c54`
 * - `ti.mas.fract.c55`
 * - `ti.mas.fract.c64`
 * 
 * @a(CONTENTS)
 * @p(dlist)
 * - `version` module
 *      provides version numbering for the package and is described in
 *      `version.xdc`. Note that `version.xdc` is generated from the template file 
 *      `version.xdt`.
 * @p
 * @a(TARGETS) The following architecture targets are supported:
 *
 * @p(dlist)
 * - `C54`
 * - `C55`
 * - `C64`
 * @p
 * @a(BUILD)  The build options are implemented in the file `package.bld`.
 *
 * Build command: `xdc XDCARGS="option"`.
 * option = [c54, c55, c64, all]
 * If any other option other than above is used then the package is not built correctly
 * and will crash when used in conjunction with other packages. 
 *
 *
 * Generated files:
 *
 * @p(dlist)
 * -  `version.h`
 * @p
 *
 * Command to clean: `xdc clean`
 *
 * @a(RELEASES)
 *
 * Release creation: `xdc XDCARGS="option1 option2" release`
 * option1 = [c54, c55, c64, all]
 * option2 = [src, docs]
 *
 * Using the option1 properly and option 2="src" creates the released tar file of the form `ti_mas_fract_<arch>_src_<v>_<r>_<x>_<p>.tar`
 * or `ti_mas_fract_all_src_<v>_<r>_<x>_<p>.tar` if option 1="all"
 *
 * If option2 = "obj" or empty then `ti_mas_fract_all_<v>_<r>_<x>_<p>.tar` is created as release
 *
 * If option2 = "obj" or empty then `ti_mas_fract_all_<v>_<r>_<x>_<p>.tar` is created containing only object code
 *
 * To generate document only the following should be used:
 * `xdc XDCARGS="docs" release` or `xdc XDCARGS="option1,docs"`
 * and `ti_mas_fract_docs_<v>_<r>_<x>_<p>.tar` is created
 * 
 * If any other option other than the ones listed above is used, no release is created.
 * 
 *
 * 
 *
 * @a(Copyright)
 * @p(html)
 * &#169; 2006 Texas Instruments, Inc.
 * @p
 */

package ti.mas.fract[2,0,7,0] {
  module Version;
}

/* nothing past this point */
