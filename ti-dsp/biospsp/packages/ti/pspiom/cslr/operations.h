/* 

Register operations 

Version 0.01 Under Construction

*/

#define IN_REGL(offSet)          (*(volatile unsigned int*)(offSet))
#define OUT_REGL(offSet, value)  (*(volatile unsigned int*)(offSet) = (unsigned int)(value))
#define SETBIT_REGL(dest, mask)  (OUT_REGL((dest), IN_REGL((dest)) | (mask)))
#define CLRBIT_REGL(dest, mask)  (OUT_REGL((dest), IN_REGL((dest)) & ~(mask)))
#define TGLBIT_REGL(dest, mask)  (OUT_REGL((dest), IN_REGL((dest)) ^ (mask)))
#define CHKBIT_REGL(dest, mask)  (IN_REGL((dest)) & (mask))
