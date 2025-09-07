

#include "../../lib/stdio.h"
#include "../../driver/io/ports.h"
#include "../../arch/interrupt/apic/apic.h"
#include "../../memory/kheap.h"

#include "rtc.h"

#define RTC_VECTOR  40
#define RTC_IRQ 8   // 40 - 32
// Ports for RTC
#define RTC_COMMAND_PORT 0x70
#define RTC_DATA_PORT 0x71

// RTC Registers
#define RTC_SECONDS     0x00
#define RTC_MINUTES     0x02
#define RTC_HOURS       0x04
#define RTC_DAY         0x07
#define RTC_MONTH       0x08
#define RTC_YEAR        0x09
#define RTC_REG_A       0x0A
#define RTC_REG_B       0x0B
#define RTC_REG_C       0x0C

volatile uint32_t rtc_ticks = 0;

rtc_time_t *rtc_time;

static void rtc_enable() {
    outb(RTC_COMMAND_PORT, RTC_REG_B);  
    uint8_t prev = inb(RTC_DATA_PORT);   // Read current value of Register B
    outb(RTC_COMMAND_PORT, RTC_REG_B);  
    outb(RTC_DATA_PORT, prev | 0x40);    // Enable Periodic Interrupt (PIE)
}

/*
Rate    |   Frequency (Hz)
6       |   1024
7       |   512
8       |   256
9       |   128
10      |   64
11      |   32
12      |   16
13      |   8
*/

// rtc_set_frequency(6); for 1024 Hz.
static void rtc_set_frequency(uint8_t rate) {
    outb(RTC_COMMAND_PORT, RTC_REG_A);
    uint8_t prev = inb(RTC_DATA_PORT);

    outb(RTC_COMMAND_PORT, RTC_REG_A);
    outb(RTC_DATA_PORT, (prev & 0xF0) | (rate & 0x0F));  // Set new frequency
}

static void enable_rtc_irq() {
    outb(0x21, inb(0x21) & ~(1 << 2));  // Unmask IRQ 2 (RTC cascaded via PIC)
    outb(0xA1, inb(0xA1) & ~(1 << 8));  // Unmask IRQ 8 (RTC IRQ)
}



static void rtc_interrupt_handler() {
    // Acknowledge the interrupt
    outb(RTC_COMMAND_PORT, RTC_REG_C);
    inb(RTC_DATA_PORT);  // Read Register C to acknowledge the interrupt

    // Increment the tick count
    rtc_ticks++;

    // Print every 100 ticks (adjust as needed)
    if (rtc_ticks % 100 == 0) {
        printf("RTC Tick: %u\n", rtc_ticks);
    }

    apic_send_eoi();
}

void rtc_init() {
    rtc_set_frequency(6);   // Set frequency to 1024 Hz
    rtc_enable();           // Enable RTC periodic interrupt
    // enable_rtc_irq();       // Enable IRQ 8 in PIC

    rtc_time = (rtc_time_t *) kheap_alloc(sizeof(rtc_time_t), ALLOCATE_DATA);
    if(!rtc_time){
        printf("rtc_time allocation failed!\n");
    }

    printf("[Info] RTC Timer initialized\n");
}


// Helper function to read a byte from the RTC
static uint8_t read_rtc_register(uint8_t reg) {
    outb(RTC_COMMAND_PORT, reg);  // Write register number to RTC command port
    return inb(RTC_DATA_PORT);    // Read the value from the data port
}


// Convert BCD to binary (if needed)
static uint8_t bcd_to_bin(uint8_t value) {
    return ((value >> 4) * 10) + (value & 0x0F);
}


// Store Current RTC time into year, mon, day, hour, min and sec variables
rtc_time_t *get_rtc_time(){
    uint8_t seconds = read_rtc_register(RTC_SECONDS);
    uint8_t minutes = read_rtc_register(RTC_MINUTES);
    uint8_t hours   = read_rtc_register(RTC_HOURS);
    uint8_t day     = read_rtc_register(RTC_DAY);
    uint8_t month   = read_rtc_register(RTC_MONTH);
    uint8_t year    = read_rtc_register(RTC_YEAR);

    // Convert BCD to binary (RTC typically uses BCD format)
    rtc_time->seconds = bcd_to_bin(seconds);
    rtc_time->minutes = bcd_to_bin(minutes);
    rtc_time->hours = bcd_to_bin(hours);
    rtc_time->days  = bcd_to_bin(day);
    rtc_time->months = bcd_to_bin(month);
    rtc_time->years = bcd_to_bin(year);

    return rtc_time;
}


// Function to read and print the current time
void print_current_rtc_time() {
    rtc_time_t *time = get_rtc_time();
    uint8_t seconds = time->seconds;
    uint8_t minutes = time->minutes;
    uint8_t hours   = time->hours;
    uint8_t day     = time->days;
    uint8_t month   = time->months;
    uint8_t year    = time->years;

    // RTC only provides the last two digits of the year, assume 20xx
    // year += 2000; // make uint16_t year

    // Print the UTC time in HH:MM:SS DD/MM/YYYY format
    printf("Current Time(UTC): %d:%d:%d %d/%d/%d\n", hours, minutes, seconds, day, month, year);
}


