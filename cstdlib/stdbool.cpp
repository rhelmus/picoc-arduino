/* string.h library for large systems - small embedded systems use clibrary.c instead */
#include "../interpreter.h"

#ifndef BUILTIN_MINI_STDLIB

static int trueValue = 1;
static int falseValue = 0;


/* structure definitions */
const char StdboolDefs[] = "typedef int bool;";

/* creates various system-dependent definitions */
void StdboolSetupFunc(Picoc *pc)
{
    /* defines */
    VariableDefinePlatformVar(pc, NULL, "true", &pc->IntType, (TAnyValuePtr)ptrWrap(&trueValue), FALSE);
    VariableDefinePlatformVar(pc, NULL, "false", &pc->IntType, (TAnyValuePtr)ptrWrap(&falseValue), FALSE);
    VariableDefinePlatformVar(pc, NULL, "__bool_true_false_are_defined", &pc->IntType, (TAnyValuePtr)ptrWrap(&trueValue), FALSE);
}

#endif /* !BUILTIN_MINI_STDLIB */
