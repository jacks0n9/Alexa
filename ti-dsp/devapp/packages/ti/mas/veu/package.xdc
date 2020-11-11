requires ti.mas.types[5,0,0,0];
requires ti.mas.util[4,0,0,0];
requires ti.mas.fract[2,0,0,0];
requires internal ti.mas.swtools[2,0,0,0];

/*! VEU package
 * 
 * @a(NAME)  `ti.mas.veu`
 *
 * @a(DESCRIPTION)
 *
 *        This package creates the VEU system. It depends on the following packages:
 * @p(dlist)
 * - `ti.mas.types`
 * - `ti.mas.util`
 * - `ti.mas.fract`
 `
 * 
 * @a(CONTENTS)
 * @p(dlist)
 * - `VEU` module
 *      provides data type definitions, macros, and function APIs and is described in `veu.xdc`.
 * - `Version` module
 *      provides Version numbering for the package and is described in
 *      `Version.xdc`. Note that `Version.xdc` is generated from the template file 
 *      `Version.xdt`.
 * @p
 * @a(TARGETS) The following architecture targets are supported:
 *
 * @p(dlist)
 * - `C64`
 * - `C64_big_endian`
 * - `C64P`
 * - `C64P_big_endian`
 * @p
 * @a(BUILD)  The build options are implemented in the file `package.bld`.
 *
 * Build command: `xdc XDCARGS="option"`.
 * option = [c64, c64_be, c64P, c64P_be, all]
 * If any other option other than above is used then the package is not built correctly
 * and will crash when used in conjunction with other packages. 
 *
 *
 * Generated files:
 *
 * @p(dlist)
 * -  `veu.h`
 * -  `Version.h`
 * @p
 *
 * Command to clean: `xdc clean`
 *
 * @a(RELEASES)
 *
 * Release creation: `xdc XDCARGS="option1 option2" release`
 * option1 = [c64, c64_be, c64P, c64P_be, all]
 * option2 = [obj, src, docs]
 *
 * Using the option1 as given and option 2="obj" creates the released tar file containing object code only:
 * `ti_mas_veu_<arch>_src_<v>_<r>_<x>_<p>.tar`
 * or `ti_mas_veu_all_src_<v>_<r>_<x>_<p>.tar` if option 1="all"
 * 
 * Using the option1 as given and option 2="src" creates the released tar file containing source with object code:
 * `ti_mas_veu_<arch>_src_<v>_<r>_<x>_<p>.tar`
 * or `ti_mas_veu_all_src_<v>_<r>_<x>_<p>.tar` if option 1="all"
 *
 * If option2 = "obj" or empty then `ti_mas_veu_all_<v>_<r>_<x>_<p>.tar` is created containing only object code
 *
 * To generate document only the following should be used:
 * `xdc XDCARGS="docs" release` or `xdc XDCARGS="option1,docs"`
 * and `ti_mas_veu_docs_<v>_<r>_<x>_<p>.tar` is created
 * 
 * If any other option other than the ones listed above is used, no release is created.
 * 
 *
 * 
 *
 * @a(Copyright)
 * @p(html)
 * &#169; 2009 Texas Instruments, Inc.
 * @p
 */

package ti.mas.veu[4,3,0,0] {
   module Version;
}

/* nothing past this point */
