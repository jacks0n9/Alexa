requires internal ti.mas.swtools[2,0,4,0];

/*! Basic data types for portable components in ti.mas.*
 * 
 * @a(NAME)  `ti.mas.types`
 *
 * @a(DESCRIPTION)
 *
 *        This package provides definitions for all data types
 *        to be used by packages in ti.mas.*
 * 
 * 
 * @a(CONTENTS)
 * @p(dlist)
 * - `types` module
 *      provides definitions for the data types and is described in `types.xdc`.
 * - `const` module
 *      provides definitions for system constants and is described in `const.xdc`.
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
 * - `C55_large`
 * - `C64`
 * - `C64_big_endian`
 * - `C64P`
 * - `C64P_big_endian`
 * @p
 * @a(BUILD)  The build options are implemented in the file `package.bld`.
 *
 * Build command: `xdc XDCARGS="option"`
 * option = [c54, c55, c55L, c64, c64_be, c64P, c64P_be, all]
 * If any other option other than above is used then the package is not built correctly
 * and will crash when used in conjunction with other packages. 
 *
 *
 * Generated files:
 *
 * @p(dlist)
 * -  `types.h`
 * -  `<target>/types.h` 
 * -  `types__prologue.h`
 * -  `const.h`
 * -  `version.h`
 * @p
 *
 * Command to clean: `xdc clean`
 *
 * @a(RELEASES)
 *
 * Release creation: `xdc XDCARGS="option1 option2" release`
 * option1 = [c54, c55, c55L, c64, c64_be, c64P, c64P_be, all]
 * option2 = [src, docs]
 *
 * Using the option1 properly and option 2="src" creates the released tar file of the form `ti_mas_types_<arch>_src_<v>_<r>_<x>_<p>.tar`
 * or `ti_mas_types_all_src_<v>_<r>_<x>_<p>.tar` if option 1="all"
 *
 * If option2 = "obj" or empty then `ti_mas_types_all_<v>_<r>_<x>_<p>.tar` is created as release
 *
 * To generate document only the following should be used:
 * `xdc XDCARGS="docs" release` or `xdc XDCARGS="option1,docs"`
 * and `ti_mas_types_docs_<v>_<r>_<x>_<p>.tar` is created
 * 
 * If any other option other than the ones listed above is used, no release is created.
 * 
 *
 * 
 *
 * @a(Copyright)
 * @p(html)
 * &#169; 2006, 2007 Texas Instruments, Inc.
 * @p
 */

/* had to change S key due to change in tbool which may result in code
 * not being binary compatible! */

package ti.mas.types[5,0,7,0] {
  module Version;
}
/* nothing past this point */
