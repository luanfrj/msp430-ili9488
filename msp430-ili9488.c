/* 
 * File:   msp430-ili9488.c
 * Author: luan
 *
 * Created on 14 de Abril de 2017, 20:11
 */

#include <msp430g2553.h>
#include "font_table.h"

#define LCD_RST	BIT0
#define LCD_CS	BIT1
#define LCD_RS	BIT2
#define LCD_WR	BIT3
#define LCD_RD	BIT4

// configure pins
void configure_pins()
{
    P1DIR |= 0xFF;
    P1OUT |= 0xFF;
    P2DIR |= LCD_RST | LCD_CS | LCD_RS | LCD_WR | LCD_RD;
    P2OUT |= LCD_RST | LCD_CS | LCD_RS | LCD_WR | LCD_RD;
}

// configure clocks
void configure_clocks()
{
    // Stop watchdog
    WDTCTL = WDTPW + WDTHOLD;
    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL = CALDCO_16MHZ;
}

// delay in microseconds
void delay_us(unsigned int us)
{
    while (us)
    {
        // 1 for 1 Mhz set 16 for 16 MHz
        __delay_cycles(16); 
        us--;
    }
}

// delay in miliseconds
void delay_ms(unsigned int ms)
{
    while (ms)
    {
        // 1000 for 1MHz and 16000 for 16MHz
        __delay_cycles(16000);
        ms--;
    }
}

// send command
void send_command(unsigned char cmd)
{
    P2OUT = 0x11;
    P1OUT = cmd;
    P2OUT |= (LCD_WR);
}

// send data
void send_data(unsigned char data)
{
    P2OUT = 0x15;
    P1OUT = data;
    P2OUT |= (LCD_WR);
}

// set bg collor
void set_bgcolor(unsigned char r, unsigned char g, unsigned char b)
{
    unsigned short i, j;
     // write data command
    send_command(0x2C);
    
    // send color
    for (i = 0; i < 480; i++)
    {
        for (j = 0; j < 320; j++)
        {
            send_data(r);
            send_data(g);
            send_data(b);
        }
    }
}

// set address
void set_address(unsigned short x, unsigned short y)
{
    unsigned char xt, xb, yt, yb;
    
    xt = (x >> 8) & 0xFF;
    xb = x & 0xFF;
    
    yt = (y >> 8) & 0xFF;
    yb = y & 0xFF;
    
    P2OUT &= (~LCD_CS);
    
    // set cursor
    send_command(0x2A);
    // set start x
    send_data(xt);
    send_data(xb);
    // set end x
    send_data(xt);
    send_data(xb);
    send_command(0x00);
    
    send_command(0x2B);
    // set start y
    send_data(yt);
    send_data(yb);
    // set end y
    send_data(yt);
    send_data(yb);
    send_command(0x00);
}

// write pixel at position
void write_pixel(unsigned short x, unsigned short y, unsigned char r, unsigned char g, unsigned char b)
{
    set_address(x, y);
    P2OUT &= (~LCD_CS);
    send_command(0x2C);
    send_data(r);
    send_data(g);
    send_data(b); 
}

// write char row
void write_row(unsigned char rt, unsigned char rb, 
        unsigned short x, unsigned short y, 
        unsigned char r, unsigned char g, unsigned char b)
{
    unsigned short row;
    unsigned char i;
    row = (rt << 8) | (rb);
    
    P2OUT &= (~LCD_CS);
    for (i = 0; i < 10; i++)
    {
        if (row > 32767)
        {
            write_pixel(x, y, r, g, b);
        }
        x++;
        row = row << 1;
    }
}

// write character at position
void write_char(char c, unsigned short x, unsigned short y, 
        unsigned char r, unsigned char g, unsigned char b)
{
    unsigned short base = (c - ' ') * 30;
    unsigned char i, rt, rb;
    
    for (i = 0; i < 15; i++)
    {
        rt = courierNew_12ptBitmaps[base + 2*i];
        rb = courierNew_12ptBitmaps[base + 2*i + 1];
        write_row(rt, rb, x, y, r, g, b);
        y++;
    }
    
}

// write string
void write_string(char *str, 
        unsigned short x, unsigned short y,
        unsigned char r, unsigned char g, unsigned char b)
{
    while(*str)
    {
        write_char(*str, x, y, r, g, b);
        x += 10;
        str++;
    }
}

// abs
int abs(int val)
{
    if (val < 0)
        return -val;
    else
        return val;
}

// draw line
void draw_line(unsigned short x1, unsigned short y1,
        unsigned short x2, unsigned short y2,
        unsigned char r, unsigned char g, unsigned char b)
{
    int w = x2 - x1;
    int h = y2 - y1;
    int dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;

    if (w<0)
        dx1 = -1;
    else
        if (w>0)
            dx1 = 1;
    
    if (h<0)
        dy1 = -1;
    else
        if (h>0)
            dy1 = 1;
    if (w<0)
        dx2 = -1;
    else
        if (w>0)
            dx2 = 1;

    int longest = abs(w) ;
    int shortest = abs(h) ;
    if (!(longest>shortest))
    {
        longest = abs(h) ;
        shortest = abs(w) ;
        if (h<0)
            dy2 = -1;
        else
            if (h>0)
                dy2 = 1;
        dx2 = 0 ;
    }
    int numerator = longest >> 1;
    for (int i = 0; i <= longest; i++)
    {
        write_pixel(x1, y1, r, g, b);

        numerator += shortest ;
        if (!(numerator<longest))
        {
            numerator -= longest;
            x1 += dx1;
            y1 += dy1;
        }
        else
        {
            x1 += dx2;
            y1 += dy2;
        }
    }
}

void draw_circle(unsigned short x0, unsigned short y0, unsigned short radius,
        unsigned char r, unsigned char g, unsigned char b)
{
    unsigned short x = radius;
    unsigned short y = 0;
    short err = 0;

    while (x >= y)
    {
        write_pixel(x0 + x, y0 + y, r, g, b);
        write_pixel(x0 + y, y0 + x, r, g, b);
        write_pixel(x0 - y, y0 + x, r, g, b);
        write_pixel(x0 - x, y0 + y, r, g, b);
        write_pixel(x0 - x, y0 - y, r, g, b);
        write_pixel(x0 - y, y0 - x, r, g, b);
        write_pixel(x0 + y, y0 - x, r, g, b);
        write_pixel(x0 + x, y0 - y, r, g, b);

        if (err <= 0)
        {
            y += 1;
            err += 2*y + 1;
        }
        if (err > 0)
        {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}

// init lcd
void init_lcd()
{
    // reset LCD
    P2OUT &= (~LCD_RST);
    delay_ms(100);
    P2OUT |= (LCD_RST);
    
    // software reset
    send_command(0x01);
    delay_ms(100);
    
    // sleep out
    send_command(0x11);
    delay_ms(100);
    
    // memory acces control
    send_command(0x36);
    send_data(0xE8);
    delay_ms(100);
    
    // set dbi
    send_command(0x3A);
    send_data(0x06);
    delay_ms(100);
    
    // partial mode on
    send_command(0x12);
    delay_ms(100);
    
    // display on
    send_command(0x29);
    delay_ms(100);
    
    // set cursor
    send_command(0x2A);
    // set start x
    send_data(0x00);
    send_data(0x00);
    // set end x
    send_data(0x01);
    send_data(0xDF);
    send_command(0x00);
    
    send_command(0x2B);
    // set start y
    send_data(0x00);
    send_data(0x00);
    // set end y
    send_data(0x01);
    send_data(0x3F);
    send_command(0x00);
    
    delay_ms(100);
   
    // set brightness
    send_command(0x51);
    send_data(0x0F);
    delay_ms(100);
    
    // set brightness control
    send_command(0x53);
    send_data(0x2C);
    delay_ms(100);
    
    // set framerate
    send_command(0xB1);
    send_data(0xB0);
    send_data(0x11);
    delay_ms(50);
    
    set_bgcolor(0, 0, 0);
    
    P2OUT |= (LCD_CS);
    delay_ms(100);
    
}

// Main function
int main() 
{
    int i;
    
    configure_clocks();
    delay_ms(100);
    
    configure_pins();
    delay_ms(100);
    
    init_lcd();
    
    P2OUT &= (~LCD_CS);
    
    for (i = 1; i < 48; i ++)
    {
        draw_line(0, 0, 10*i - 1, 319, 0, 235, 155);
    }
    
    draw_circle(240, 160, 80, 255, 0, 0);
    draw_circle(240, 160, 100, 0, 255, 0);
    draw_circle(240, 160, 120, 0, 0, 255);
    
    write_string("Hello world!", 200, 20, 255, 255, 255);
 
    P2OUT |= (LCD_CS);
    P1OUT = 0x00;
    
    while (1)
    {
        
    }
    
}

