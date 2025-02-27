#include <reg51.h>
#define display_port P2

// Traffic Light LEDs
sbit red = P1^0;
sbit blue = P1^1;
sbit green = P1^2;

// 7-Segment Display Control
sbit digctrl1 = P1^3;
sbit digctrl2 = P1^4;

// LCD Control Pins
sbit rw = P3^3;
sbit e = P3^4;
sbit rs = P3^2;

// 7-Segment Display Lookup Table (Common Anode)
unsigned char code ADDUP[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};

// Traffic Light Timings
unsigned int red_time = 20;
unsigned int blue_time = 5;
unsigned int green_time = 15;

unsigned char present_state = 0;
unsigned int ticking_clock;

void Timer0_Init(void);
void lcd_init(void);
void lcd_cmd(unsigned char command);
void lcd_data(unsigned char disp_data);
void show_in_lcd(char *msg);
void show_counting(unsigned int count);
void Delay(unsigned int time);

// Timer0 Initialization
void Timer0_Init(void) {
    TMOD = 0x01;     // Timer0 Mode 1 (16-bit)
    TH0 = 0x3C;      // Load initial high byte
    TL0 = 0xB0;      // Load initial low byte
    EA = 1;          // Enable Global Interrupt
    ET0 = 1;         // Enable Timer0 Interrupt
    TR0 = 1;         // Start Timer0
}

// Timer0 ISR (Interrupt Service Routine)
void Timer0_ISR(void) interrupt 1 {
    TH0 = 0x3C;     
    TL0 = 0xB0;     

    if (ticking_clock > 0) {
        ticking_clock--;
        show_counting(ticking_clock);
    } else {
        // Traffic Light State Machine
        switch (present_state) {
            case 0:   
                red = 1; blue = 0; green = 0;
                lcd_cmd(0x80);   // Move cursor to first line
                show_in_lcd("SIGNAL: RED - STOP ");
                ticking_clock = red_time;
                present_state = 1;
                break;

            case 1:   
                red = 0; blue = 1; green = 0;
                lcd_cmd(0x80);
                show_in_lcd("SIGNAL: BLUE - WAIT ");
                ticking_clock = blue_time;
                present_state = 2;
                break;

            case 2:   
                red = 0; blue = 0; green = 1;
                lcd_cmd(0x80);
                show_in_lcd("SIGNAL: GREEN - GO  ");
                ticking_clock = green_time;
                present_state = 0;
                break;
        }
    }
}

// LCD Initialization
void lcd_init(void) {
    lcd_cmd(0x38);   // 8-bit mode, 2-line display
    Delay(10);
    lcd_cmd(0x0C);   // Display ON, Cursor OFF
    Delay(10);
    lcd_cmd(0x01);   // Clear Display
    Delay(10);
    lcd_cmd(0x80);   // Move Cursor to First Line
    Delay(10);
}

// Send Command to LCD
void lcd_cmd(unsigned char command) {
    display_port = command;
    rs = 0;
    rw = 0;
    e = 1;
    Delay(1);
    e = 0;
}

// Send Data to LCD
void lcd_data(unsigned char disp_data) {
    display_port = disp_data;
    rs = 1;
    rw = 0;
    e = 1;
    Delay(1);
    e = 0;
}

// Display String on LCD
void show_in_lcd(char *msg) {
    while (*msg) {
        lcd_data(*msg++);
    }
}

// Display Countdown Timer on 7-Segment Display
void show_counting(unsigned int count) {
    digctrl1 = 1;
    digctrl2 = 0;
    P0 = ADDUP[count / 10];   // Display tens place
    Delay(5);
    digctrl1 = 0;
    digctrl2 = 1;
    P0 = ADDUP[count % 10];   // Display ones place
    Delay(5);
}

// Small Delay Function
void Delay(unsigned int temp) {
    unsigned int i, j;
    for (i = 0; i < temp; i++)
        for (j = 0; j < 1275; j++);
}

// Main Function
void main() {
    P0 = 0x00;   // Initialize Ports
    P1 = 0x00;
    P2 = 0x00;
    
    lcd_init();   // Initialize LCD
    Timer0_Init();    // Start Timer0

    ticking_clock = red_time;   // Start with red light

    while (1) {
        Delay(10);  // Prevent CPU overuse
    }
}
