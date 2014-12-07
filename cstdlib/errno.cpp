/* string.h library for large systems - small embedded systems use clibrary.c instead */
#include <errno.h>
#include "../interpreter.h"

#ifndef BUILTIN_MINI_STDLIB

#ifdef EACCES
static int EACCESValue = EACCES;
#endif

#ifdef EADDRINUSE
static int EADDRINUSEValue = EADDRINUSE;
#endif

#ifdef EADDRNOTAVAIL
static int EADDRNOTAVAILValue = EADDRNOTAVAIL;
#endif

#ifdef EAFNOSUPPORT
static int EAFNOSUPPORTValue = EAFNOSUPPORT;
#endif

#ifdef EAGAIN
static int EAGAINValue = EAGAIN;
#endif

#ifdef EALREADY
static int EALREADYValue = EALREADY;
#endif

#ifdef EBADF
static int EBADFValue = EBADF;
#endif

#ifdef EBADMSG
static int EBADMSGValue = EBADMSG;
#endif

#ifdef EBUSY
static int EBUSYValue = EBUSY;
#endif

#ifdef ECANCELED
static int ECANCELEDValue = ECANCELED;
#endif

#ifdef ECHILD
static int ECHILDValue = ECHILD;
#endif

#ifdef ECONNABORTED
static int ECONNABORTEDValue = ECONNABORTED;
#endif

#ifdef ECONNREFUSED
static int ECONNREFUSEDValue = ECONNREFUSED;
#endif

#ifdef ECONNRESET
static int ECONNRESETValue = ECONNRESET;
#endif

#ifdef EDEADLK
static int EDEADLKValue = EDEADLK;
#endif

#ifdef EDESTADDRREQ
static int EDESTADDRREQValue = EDESTADDRREQ;
#endif

#ifdef EDOM
static int EDOMValue = EDOM;
#endif

#ifdef EDQUOT
static int EDQUOTValue = EDQUOT;
#endif

#ifdef EEXIST
static int EEXISTValue = EEXIST;
#endif

#ifdef EFAULT
static int EFAULTValue = EFAULT;
#endif

#ifdef EFBIG
static int EFBIGValue = EFBIG;
#endif

#ifdef EHOSTUNREACH
static int EHOSTUNREACHValue = EHOSTUNREACH;
#endif

#ifdef EIDRM
static int EIDRMValue = EIDRM;
#endif

#ifdef EILSEQ
static int EILSEQValue = EILSEQ;
#endif

#ifdef EINPROGRESS
static int EINPROGRESSValue = EINPROGRESS;
#endif

#ifdef EINTR
static int EINTRValue = EINTR;
#endif

#ifdef EINVAL
static int EINVALValue = EINVAL;
#endif

#ifdef EIO
static int EIOValue = EIO;
#endif

#ifdef EISCONN
static int EISCONNValue = EISCONN;
#endif

#ifdef EISDIR
static int EISDIRValue = EISDIR;
#endif

#ifdef ELOOP
static int ELOOPValue = ELOOP;
#endif

#ifdef EMFILE
static int EMFILEValue = EMFILE;
#endif

#ifdef EMLINK
static int EMLINKValue = EMLINK;
#endif

#ifdef EMSGSIZE
static int EMSGSIZEValue = EMSGSIZE;
#endif

#ifdef EMULTIHOP
static int EMULTIHOPValue = EMULTIHOP;
#endif

#ifdef ENAMETOOLONG
static int ENAMETOOLONGValue = ENAMETOOLONG;
#endif

#ifdef ENETDOWN
static int ENETDOWNValue = ENETDOWN;
#endif

#ifdef ENETRESET
static int ENETRESETValue = ENETRESET;
#endif

#ifdef ENETUNREACH
static int ENETUNREACHValue = ENETUNREACH;
#endif

#ifdef ENFILE
static int ENFILEValue = ENFILE;
#endif

#ifdef ENOBUFS
static int ENOBUFSValue = ENOBUFS;
#endif

#ifdef ENODATA
static int ENODATAValue = ENODATA;
#endif

#ifdef ENODEV
static int ENODEVValue = ENODEV;
#endif

#ifdef ENOENT
static int ENOENTValue = ENOENT;
#endif

#ifdef ENOEXEC
static int ENOEXECValue = ENOEXEC;
#endif

#ifdef ENOLCK
static int ENOLCKValue = ENOLCK;
#endif

#ifdef ENOLINK
static int ENOLINKValue = ENOLINK;
#endif

#ifdef ENOMEM
static int ENOMEMValue = ENOMEM;
#endif

#ifdef ENOMSG
static int ENOMSGValue = ENOMSG;
#endif

#ifdef ENOPROTOOPT
static int ENOPROTOOPTValue = ENOPROTOOPT;
#endif

#ifdef ENOSPC
static int ENOSPCValue = ENOSPC;
#endif

#ifdef ENOSR
static int ENOSRValue = ENOSR;
#endif

#ifdef ENOSTR
static int ENOSTRValue = ENOSTR;
#endif

#ifdef ENOSYS
static int ENOSYSValue = ENOSYS;
#endif

#ifdef ENOTCONN
static int ENOTCONNValue = ENOTCONN;
#endif

#ifdef ENOTDIR
static int ENOTDIRValue = ENOTDIR;
#endif

#ifdef ENOTEMPTY
static int ENOTEMPTYValue = ENOTEMPTY;
#endif

#ifdef ENOTRECOVERABLE
static int ENOTRECOVERABLEValue = ENOTRECOVERABLE;
#endif

#ifdef ENOTSOCK
static int ENOTSOCKValue = ENOTSOCK;
#endif

#ifdef ENOTSUP
static int ENOTSUPValue = ENOTSUP;
#endif

#ifdef ENOTTY
static int ENOTTYValue = ENOTTY;
#endif

#ifdef ENXIO
static int ENXIOValue = ENXIO;
#endif

#ifdef EOPNOTSUPP
static int EOPNOTSUPPValue = EOPNOTSUPP;
#endif

#ifdef EOVERFLOW
static int EOVERFLOWValue = EOVERFLOW;
#endif

#ifdef EOWNERDEAD
static int EOWNERDEADValue = EOWNERDEAD;
#endif

#ifdef EPERM
static int EPERMValue = EPERM;
#endif

#ifdef EPIPE
static int EPIPEValue = EPIPE;
#endif

#ifdef EPROTO
static int EPROTOValue = EPROTO;
#endif

#ifdef EPROTONOSUPPORT
static int EPROTONOSUPPORTValue = EPROTONOSUPPORT;
#endif

#ifdef EPROTOTYPE
static int EPROTOTYPEValue = EPROTOTYPE;
#endif

#ifdef ERANGE
static int ERANGEValue = ERANGE;
#endif

#ifdef EROFS
static int EROFSValue = EROFS;
#endif

#ifdef ESPIPE
static int ESPIPEValue = ESPIPE;
#endif

#ifdef ESRCH
static int ESRCHValue = ESRCH;
#endif

#ifdef ESTALE
static int ESTALEValue = ESTALE;
#endif

#ifdef ETIME
static int ETIMEValue = ETIME;
#endif

#ifdef ETIMEDOUT
static int ETIMEDOUTValue = ETIMEDOUT;
#endif

#ifdef ETXTBSY
static int ETXTBSYValue = ETXTBSY;
#endif

#ifdef EWOULDBLOCK
static int EWOULDBLOCKValue = EWOULDBLOCK;
#endif

#ifdef EXDEV
static int EXDEVValue = EXDEV;
#endif


/* creates various system-dependent definitions */
void StdErrnoSetupFunc(Picoc *pc)
{
    /* defines */
#ifdef EACCES
    VariableDefinePlatformVar(pc, NULL, "EACCES", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EACCESValue), FALSE);
#endif

#ifdef EADDRINUSE
    VariableDefinePlatformVar(pc, NULL, "EADDRINUSE", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EADDRINUSEValue), FALSE);
#endif

#ifdef EADDRNOTAVAIL
    VariableDefinePlatformVar(pc, NULL, "EADDRNOTAVAIL", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EADDRNOTAVAILValue), FALSE);
#endif

#ifdef EAFNOSUPPORT
    VariableDefinePlatformVar(pc, NULL, "EAFNOSUPPORT", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EAFNOSUPPORTValue), FALSE);
#endif

#ifdef EAGAIN
    VariableDefinePlatformVar(pc, NULL, "EAGAIN", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EAGAINValue), FALSE);
#endif

#ifdef EALREADY
    VariableDefinePlatformVar(pc, NULL, "EALREADY", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EALREADYValue), FALSE);
#endif

#ifdef EBADF
    VariableDefinePlatformVar(pc, NULL, "EBADF", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EBADFValue), FALSE);
#endif

#ifdef EBADMSG
    VariableDefinePlatformVar(pc, NULL, "EBADMSG", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EBADMSGValue), FALSE);
#endif

#ifdef EBUSY
    VariableDefinePlatformVar(pc, NULL, "EBUSY", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EBUSYValue), FALSE);
#endif

#ifdef ECANCELED
    VariableDefinePlatformVar(pc, NULL, "ECANCELED", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ECANCELEDValue), FALSE);
#endif

#ifdef ECHILD
    VariableDefinePlatformVar(pc, NULL, "ECHILD", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ECHILDValue), FALSE);
#endif

#ifdef ECONNABORTED
    VariableDefinePlatformVar(pc, NULL, "ECONNABORTED", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ECONNABORTEDValue), FALSE);
#endif

#ifdef ECONNREFUSED
    VariableDefinePlatformVar(pc, NULL, "ECONNREFUSED", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ECONNREFUSEDValue), FALSE);
#endif

#ifdef ECONNRESET
    VariableDefinePlatformVar(pc, NULL, "ECONNRESET", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ECONNRESETValue), FALSE);
#endif

#ifdef EDEADLK
    VariableDefinePlatformVar(pc, NULL, "EDEADLK", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EDEADLKValue), FALSE);
#endif

#ifdef EDESTADDRREQ
    VariableDefinePlatformVar(pc, NULL, "EDESTADDRREQ", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EDESTADDRREQValue), FALSE);
#endif

#ifdef EDOM
    VariableDefinePlatformVar(pc, NULL, "EDOM", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EDOMValue), FALSE);
#endif

#ifdef EDQUOT
    VariableDefinePlatformVar(pc, NULL, "EDQUOT", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EDQUOTValue), FALSE);
#endif

#ifdef EEXIST
    VariableDefinePlatformVar(pc, NULL, "EEXIST", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EEXISTValue), FALSE);
#endif

#ifdef EFAULT
    VariableDefinePlatformVar(pc, NULL, "EFAULT", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EFAULTValue), FALSE);
#endif

#ifdef EFBIG
    VariableDefinePlatformVar(pc, NULL, "EFBIG", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EFBIGValue), FALSE);
#endif

#ifdef EHOSTUNREACH
    VariableDefinePlatformVar(pc, NULL, "EHOSTUNREACH", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EHOSTUNREACHValue), FALSE);
#endif

#ifdef EIDRM
    VariableDefinePlatformVar(pc, NULL, "EIDRM", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EIDRMValue), FALSE);
#endif

#ifdef EILSEQ
    VariableDefinePlatformVar(pc, NULL, "EILSEQ", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EILSEQValue), FALSE);
#endif

#ifdef EINPROGRESS
    VariableDefinePlatformVar(pc, NULL, "EINPROGRESS", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EINPROGRESSValue), FALSE);
#endif

#ifdef EINTR
    VariableDefinePlatformVar(pc, NULL, "EINTR", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EINTRValue), FALSE);
#endif

#ifdef EINVAL
    VariableDefinePlatformVar(pc, NULL, "EINVAL", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EINVALValue), FALSE);
#endif

#ifdef EIO
    VariableDefinePlatformVar(pc, NULL, "EIO", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EIOValue), FALSE);
#endif

#ifdef EISCONN
    VariableDefinePlatformVar(pc, NULL, "EISCONN", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EISCONNValue), FALSE);
#endif

#ifdef EISDIR
    VariableDefinePlatformVar(pc, NULL, "EISDIR", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EISDIRValue), FALSE);
#endif

#ifdef ELOOP
    VariableDefinePlatformVar(pc, NULL, "ELOOP", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ELOOPValue), FALSE);
#endif

#ifdef EMFILE
    VariableDefinePlatformVar(pc, NULL, "EMFILE", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EMFILEValue), FALSE);
#endif

#ifdef EMLINK
    VariableDefinePlatformVar(pc, NULL, "EMLINK", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EMLINKValue), FALSE);
#endif

#ifdef EMSGSIZE
    VariableDefinePlatformVar(pc, NULL, "EMSGSIZE", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EMSGSIZEValue), FALSE);
#endif

#ifdef EMULTIHOP
    VariableDefinePlatformVar(pc, NULL, "EMULTIHOP", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EMULTIHOPValue), FALSE);
#endif

#ifdef ENAMETOOLONG
    VariableDefinePlatformVar(pc, NULL, "ENAMETOOLONG", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENAMETOOLONGValue), FALSE);
#endif

#ifdef ENETDOWN
    VariableDefinePlatformVar(pc, NULL, "ENETDOWN", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENETDOWNValue), FALSE);
#endif

#ifdef ENETRESET
    VariableDefinePlatformVar(pc, NULL, "ENETRESET", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENETRESETValue), FALSE);
#endif

#ifdef ENETUNREACH
    VariableDefinePlatformVar(pc, NULL, "ENETUNREACH", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENETUNREACHValue), FALSE);
#endif

#ifdef ENFILE
    VariableDefinePlatformVar(pc, NULL, "ENFILE", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENFILEValue), FALSE);
#endif

#ifdef ENOBUFS
    VariableDefinePlatformVar(pc, NULL, "ENOBUFS", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENOBUFSValue), FALSE);
#endif

#ifdef ENODATA
    VariableDefinePlatformVar(pc, NULL, "ENODATA", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENODATAValue), FALSE);
#endif

#ifdef ENODEV
    VariableDefinePlatformVar(pc, NULL, "ENODEV", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENODEVValue), FALSE);
#endif

#ifdef ENOENT
    VariableDefinePlatformVar(pc, NULL, "ENOENT", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENOENTValue), FALSE);
#endif

#ifdef ENOEXEC
    VariableDefinePlatformVar(pc, NULL, "ENOEXEC", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENOEXECValue), FALSE);
#endif

#ifdef ENOLCK
    VariableDefinePlatformVar(pc, NULL, "ENOLCK", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENOLCKValue), FALSE);
#endif

#ifdef ENOLINK
    VariableDefinePlatformVar(pc, NULL, "ENOLINK", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENOLINKValue), FALSE);
#endif

#ifdef ENOMEM
    VariableDefinePlatformVar(pc, NULL, "ENOMEM", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENOMEMValue), FALSE);
#endif

#ifdef ENOMSG
    VariableDefinePlatformVar(pc, NULL, "ENOMSG", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENOMSGValue), FALSE);
#endif

#ifdef ENOPROTOOPT
    VariableDefinePlatformVar(pc, NULL, "ENOPROTOOPT", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENOPROTOOPTValue), FALSE);
#endif

#ifdef ENOSPC
    VariableDefinePlatformVar(pc, NULL, "ENOSPC", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENOSPCValue), FALSE);
#endif

#ifdef ENOSR
    VariableDefinePlatformVar(pc, NULL, "ENOSR", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENOSRValue), FALSE);
#endif

#ifdef ENOSTR
    VariableDefinePlatformVar(pc, NULL, "ENOSTR", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENOSTRValue), FALSE);
#endif

#ifdef ENOSYS
    VariableDefinePlatformVar(pc, NULL, "ENOSYS", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENOSYSValue), FALSE);
#endif

#ifdef ENOTCONN
    VariableDefinePlatformVar(pc, NULL, "ENOTCONN", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENOTCONNValue), FALSE);
#endif

#ifdef ENOTDIR
    VariableDefinePlatformVar(pc, NULL, "ENOTDIR", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENOTDIRValue), FALSE);
#endif

#ifdef ENOTEMPTY
    VariableDefinePlatformVar(pc, NULL, "ENOTEMPTY", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENOTEMPTYValue), FALSE);
#endif

#ifdef ENOTRECOVERABLE
    VariableDefinePlatformVar(pc, NULL, "ENOTRECOVERABLE", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENOTRECOVERABLEValue), FALSE);
#endif

#ifdef ENOTSOCK
    VariableDefinePlatformVar(pc, NULL, "ENOTSOCK", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENOTSOCKValue), FALSE);
#endif

#ifdef ENOTSUP
    VariableDefinePlatformVar(pc, NULL, "ENOTSUP", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENOTSUPValue), FALSE);
#endif

#ifdef ENOTTY
    VariableDefinePlatformVar(pc, NULL, "ENOTTY", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENOTTYValue), FALSE);
#endif

#ifdef ENXIO
    VariableDefinePlatformVar(pc, NULL, "ENXIO", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ENXIOValue), FALSE);
#endif

#ifdef EOPNOTSUPP
    VariableDefinePlatformVar(pc, NULL, "EOPNOTSUPP", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EOPNOTSUPPValue), FALSE);
#endif

#ifdef EOVERFLOW
    VariableDefinePlatformVar(pc, NULL, "EOVERFLOW", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EOVERFLOWValue), FALSE);
#endif

#ifdef EOWNERDEAD
    VariableDefinePlatformVar(pc, NULL, "EOWNERDEAD", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EOWNERDEADValue), FALSE);
#endif

#ifdef EPERM
    VariableDefinePlatformVar(pc, NULL, "EPERM", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EPERMValue), FALSE);
#endif

#ifdef EPIPE
    VariableDefinePlatformVar(pc, NULL, "EPIPE", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EPIPEValue), FALSE);
#endif

#ifdef EPROTO
    VariableDefinePlatformVar(pc, NULL, "EPROTO", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EPROTOValue), FALSE);
#endif

#ifdef EPROTONOSUPPORT
    VariableDefinePlatformVar(pc, NULL, "EPROTONOSUPPORT", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EPROTONOSUPPORTValue), FALSE);
#endif

#ifdef EPROTOTYPE
    VariableDefinePlatformVar(pc, NULL, "EPROTOTYPE", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EPROTOTYPEValue), FALSE);
#endif

#ifdef ERANGE
    VariableDefinePlatformVar(pc, NULL, "ERANGE", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ERANGEValue), FALSE);
#endif

#ifdef EROFS
    VariableDefinePlatformVar(pc, NULL, "EROFS", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EROFSValue), FALSE);
#endif

#ifdef ESPIPE
    VariableDefinePlatformVar(pc, NULL, "ESPIPE", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ESPIPEValue), FALSE);
#endif

#ifdef ESRCH
    VariableDefinePlatformVar(pc, NULL, "ESRCH", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ESRCHValue), FALSE);
#endif

#ifdef ESTALE
    VariableDefinePlatformVar(pc, NULL, "ESTALE", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ESTALEValue), FALSE);
#endif

#ifdef ETIME
    VariableDefinePlatformVar(pc, NULL, "ETIME", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ETIMEValue), FALSE);
#endif

#ifdef ETIMEDOUT
    VariableDefinePlatformVar(pc, NULL, "ETIMEDOUT", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ETIMEDOUTValue), FALSE);
#endif

#ifdef ETXTBSY
    VariableDefinePlatformVar(pc, NULL, "ETXTBSY", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ETXTBSYValue), FALSE);
#endif

#ifdef EWOULDBLOCK
    VariableDefinePlatformVar(pc, NULL, "EWOULDBLOCK", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EWOULDBLOCKValue), FALSE);
#endif

#ifdef EXDEV
    VariableDefinePlatformVar(pc, NULL, "EXDEV", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&EXDEVValue), FALSE);
#endif

    VariableDefinePlatformVar(pc, NULL, "errno", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&errno), TRUE);
}

#endif /* !BUILTIN_MINI_STDLIB */
