requires internal ti.mas.swtools[2,0,2,0];
requires ti.mas.types[5,0,3,0];

/*! AFE package
 * 
 * @a(NAME)  `ti.mas.afe`
 *
 * @a(DESCRIPTION)
 *
 *   This package creates the Audio Front End. 
 * 
 * @a(CONTENTS)
 * @p(dlist)
 * - `afe` module
 *      Provides the Audio Front End Driver functionality.
 * - `version` module
 *      Provides version numbering for AFE package and is described in
 *      `version.xdc`. Note that `version.xdc` is generated from the template
 *      file `version.xdt`.
 * @p
 * @a(TARGETS) The following architecture targets are supported:
 *
 * @p(dlist)
 * - `C64P`
 * @p
 * @a(BUILD)  The build options are implemented in the file `package.bld`.
 *
 * Build command: `xdc XDCARGS="option"`
 * option = [c64P]
 * If any other options other than the above is used then the package will not
 * be built correctly and will crash when used in conjunction with other 
 * packages. 
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
 * Release creation: `xdc XDCARGS="option1 option2" gendocs [genapidocs] release`
 * option1 = [c64P]
 * option2 = [obj, src]
 *
 * Using the option1="c64P" and option 2="obj" creates the released tar 
 * file containing object code only: `ti_mas_afe_c64P_obj_<v>_<r>_<x>_<p>.tar`
 * 
 * Using the option1="c64P" and option 2="src" creates the released tar 
 * file containing object code only: `ti_mas_afe_c64P_src_<v>_<r>_<x>_<p>.tar`
 *
 * If option2="obj" or empty then `ti_mas_afe_all_<v>_<r>_<x>_<p>.tar` is 
 * created containing only object code
 *
 * To generate document only the following should be used:
 * `xdc gendocs` or `xdc genapidocs`. gendocs creates doxygen documentation for
 * whole AFE implementation, while genapidocs generation doxygen documentation
 * only for external customer consumption (does not include implementation 
 * details)
 * 
 * If any other option other than the ones listed above is used, no release 
 * is created.
 * 
 *
 * 
 *
 * @a(Copyright)
 * @p(html)
 * &#169; 2007 Texas Instruments, Inc.
 * @p
 */

package ti.mas.afe[1,0,0,0] {
   module AFE;
   module Version;
}