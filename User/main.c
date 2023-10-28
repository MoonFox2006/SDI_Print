#include "debug.h"

int main(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();

    init();
    printf("ChipID: %08X\r\n", DBGMCU_GetCHIPID());

    while(1) {
        printf("\r%lu sec.", millis() / 1000);
        delay(1000);
    }
}
