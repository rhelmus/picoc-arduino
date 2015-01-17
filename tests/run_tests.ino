#include <SdFat.h>
#include <virtmem.h>

#include <picoc.h>

namespace {

enum
{
    SD_SELECTPIN = 9,
    DISABLE_SELECTPIN = 10, // Make sure ethernet doesn't interfere on ethernet shield
    SPI_SPEED = SPI_FULL_SPEED
};

Picoc pc;
SdFat sd;

#if 0
const char *tests[] =
{
    "00_assignment.c",
    "01_comment.c",
    "02_printf.c",
    "03_struct.c",
    "04_for.c",
    "05_array.c",
    "06_case.c",
    "07_function.c",
    "08_while.c",
    "09_do_while.c",
    "10_pointer.c",
    "11_precedence.c",
    "12_hashdefine.c",
    "13_integer_literals.c",
    "14_if.c",
    "15_recursion.c",
    "16_nesting.c",
    "17_enum.c",
    "18_include.c",
    "19_pointer_arithmetic.c",
    "20_pointer_comparison.c",
    "21_char_array.c",
    "23_type_coercion.c",
    "25_quicksort.c",
    "26_character_constants.c",
    "28_strings.c",
    "29_array_address.c",
    "30_hanoi.c",
    "31_args.c",
    "32_led.c",
    "33_ternary_op.c",
    "34_array_assignment.c",
    "35_sizeof.c",
    "36_array_initialisers.c",
    "37_sprintf.c",
    "38_multiple_array_index.c",
    "39_typedef.c",
    "41_hashif.c",
    "43_void_param.c",
    "44_scoped_declarations.c",
    "45_empty_for.c",
    "47_switch_return.c",
    "48_nested_break.c",
    "49_bracket_evaluation.c",
    "50_logical_second_arg.c",
    "51_static.c",
    "52_unnamed_enum.c",
    "54_goto.c",
    "55_array_initialiser.c",
    "56_cross_structure.c",
    "57_macro_bug.c",
    "58_return_outside.c",
    "59_break_before_loop.c",
    "60_local_vars.c",
    "61_initializers.c",
    "62_float.c",
    "64_double_prefix_op.c",
    "66_printf_undefined.c",
//    "67_macro_crash.c",
    "68_return.c"
};
#else
const char *tests[] =
{
    "00_ASS~1.C",
    "01_COM~1.C",
    "02_PRI~1.C",
    "03_STR~1.C",
    "04_FOR.C",
    "05_ARRAY.C",
    "06_CASE.C",
    "07_FUN~1.C",
    "08_WHILE.C",
    "09_DO_~1.C",
    "10_POI~1.C",
    "11_PRE~1.C",
    "12_HAS~1.C",
    "13_INT~1.C",
    "14_IF.C",
    "15_REC~1.C",
    "16_NES~1.C",
    "17_ENUM.C",
    /*"18_INC~1.C",*/
    "19_POI~1.C",
    "20_POI~1.C",
    "21_CHA~1.C",
    "23_TYP~1.C",
    "25_QUI~1.C",
    "26_CHA~1.C",
    "27_SIZ~1.C",
    /*"28_STR~1.C",*/
    /*"29_ARR~1.C",*/
    "30_HANOI.C",
    "31_ARGS.C",
    "32_LED.C",
    "33_TER~1.C",
    "34_ARR~1.C",
    "35_SIZ~1.C",
    "36_ARR~1.C",
    "37_SPR~1.C",
    "38_MUL~1.C",
    "39_TYP~1.C",
    "41_HAS~1.C",
    "43_VOI~1.C",
    "44_SCO~1.C",
    "45_EMP~1.C",
    "47_SWI~1.C",
    "48_NES~1.C",
    "49_BRA~1.C",
    "50_LOG~1.C",
    "51_STA~1.C",
    "52_UNN~1.C",
    "54_GOTO.C",
    "55_ARR~1.C",
    "56_CRO~1.C",
    "57_MAC~1.C",
    /*"58_RET~1.C",*/
    "59_BRE~1.C",
    /*"60_LOC~1.C",*/
    "62_FLOAT.C",
    "64_DOU~1.C",
    "66_PRI~1.C",
    /*"67_MAC~1.C",*/
    "68_RET~1.C"
};

#endif

void runTest(const char *filename)
{
    PicocInitialise(&pc, 1024 * 1024);
    PicocPlatformScanFileByLine(&pc, filename);
//    PicocPlatformScanFile(&pc, filename);
    char *fn = (char *)filename; // HACK
    PicocCallMain(&pc, 1, &fn);
    PicocCleanup(&pc);
}

}

void setup()
{
    if (DISABLE_SELECTPIN != -1)
    {
        pinMode(DISABLE_SELECTPIN, OUTPUT);
        digitalWrite(DISABLE_SELECTPIN, HIGH);
    }

    while (!Serial)
        ;

    delay(3000);

    Serial.begin(9600);

    if (!sd.begin(SD_SELECTPIN, SPI_SPEED))
        sd.initErrorHalt();

    if (!sd.chdir("/tests", true))
        sd.errorHalt("failed to enter tests directory (did you copy the files?)");

    for (unsigned i=0; i<sizeof(tests) / sizeof(tests[0]); ++i)
    {
        Serial.print("running "); Serial.print(tests[i]); Serial.println("...");
        const uint32_t curtime = millis();
        runTest(tests[i]);
        Serial.print("\n\nFinished! ("); Serial.print(millis() - curtime); Serial.print(" ms, ");
        Serial.print(MaxStackMemUsed(&pc)); Serial.print("/"); Serial.print(MaxHeapMemUsed(&pc)); Serial.println(" bytes)");
    }

    Serial.println("All tests finished!");
}

void loop()
{
}

