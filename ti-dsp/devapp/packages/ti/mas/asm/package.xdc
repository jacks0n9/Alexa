requires internal ti.mas.swtools[2,0,4,0];

/*! asm package
 * 
 * @a(NAME)  `ti.mas.asm`
 *
 * @a(DESCRIPTION)
 *
 *        This package packages far.inc for c54.
 *
 * @a(CONTENTS)
 * @p(dlist)
 * - `far.inc`
 * - `version` module
 *      provides version numbering for the package and is described in
 *      `version.xdc`. Note that `version.xdc` is generated from the template file 
 *      `version.xdt`.
 * @p
 * @a(TARGETS) The following architecture targets are supported:
 *
 * @p(dlist)
 * - `C54`
 * @p
 * @a(BUILD)  The build options are implemented in the file `package.bld`.
 *
 * Build command: `xdc XDCARGS="option"`.
 * option = [src, all, docs]
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
 * Release creation: `xdc XDCARGS="option" release`
 * option = [src, all, docs]
 * 
 * Using the option "src" or "all" creates the released tar file containing source code:
 * `ti_mas_asm_<v>_<r>_<x>_<p>.tar`
 *
 * Using the option1 as given and option 2="docs" creates the released tar file containing documentation only:
 * `ti_mas_asm_docs_<v>_<r>_<x>_<p>.tar`
 *
 *
 * 
 * If any other option other than the ones listed above is used, no release is created.
 * 
 *
 * 
 *
 * @a(Copyright)
 * @p(html)
 * &#169; 2006-2007 Texas Instruments, Inc.
 * @p
 */


package ti.mas.asm[3,0,3,0]{
  module Version;
}

/* nothing past this point */
