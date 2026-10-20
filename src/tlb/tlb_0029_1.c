#include "tlb.h"
#include "tlb/tlb_0029.h"

/* Ends the running scenario and starts scenario n. */
void mach2DemoScenarioStart(s32 n) {
    scRunEnd();
    scRunStart(n, 0, 0);
}
