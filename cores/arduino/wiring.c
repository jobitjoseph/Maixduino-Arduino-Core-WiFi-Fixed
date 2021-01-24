#include "Arduino.h"
#include "sysctl.h"
#include "sleep.h"

/* since sysctl_get_time_us() overflow at
 * cycle 18446744073888 =
 * 44343134792 usecs (cpu clock = 416 MHz),
 * 45773558495 usecs (cpu clock = 403 MHz),
 * we need to use read_cycle() instead.
 */
uint64_t millis(){
    uint64_t v_cycle = read_cycle();
    return v_cycle / (sysctl_clock_get_freq(SYSCTL_CLOCK_CPU)/1000);
}
uint64_t micros(){
    uint64_t v_cycle = read_cycle();
    return v_cycle / (sysctl_clock_get_freq(SYSCTL_CLOCK_CPU)/1000000);
}
void delay(uint64_t dwMs){
    msleep(dwMs);
    return;
}
void delayMicroseconds(uint64_t dwUs){
    usleep(dwUs);
    return;
}

void pll_init(){
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 300000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    return;
}