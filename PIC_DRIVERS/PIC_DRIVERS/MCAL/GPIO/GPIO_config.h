#ifndef GPIO_CONFIG_H
#define GPIO_CONFIG_H

/*
 * ============================================================
 *  ADAS Project — PIC16F877A Pin Assignment  (@ 20 MHz)
 * ============================================================
 *  PORTA
 *    RA0 / AN0  : LDR sensor          (analog IN  — ADC CH0)
 *    RA1        : HC-SR04 TRIG        (digital OUT)
 *    RA2        : HC-SR04 ECHO        (digital IN)
 *    RA3–RA5    : unused              (kept as inputs for safety)
 *
 *  PORTB
 *    RB0 / INT  : Emergency-stop btn  (digital IN  — falling-edge EXT_INT)
 *    RB1        : Red LED 1 — Door 1  (digital OUT)
 *    RB2        : Red LED 2 — Door 2  (digital OUT)
 *    RB3        : Green LED — All OK  (digital OUT)
 *    RB4        : Door Sensor 1       (digital IN  — RBIF port-change)
 *    RB5        : Door Sensor 2       (digital IN  — RBIF port-change)
 *    RB6        : Seatbelt Button 1   (digital IN  — RBIF port-change)
 *    RB7        : Seatbelt Button 2   (digital IN  — RBIF port-change)
 *
 *  PORTC
 *    RC0        : Servo signal        (digital OUT — Timer0 soft-PWM)
 *    RC1 / CCP2 : Motor ENB (PWM)    (digital OUT — hardware PWM ch2)
 *    RC2 / CCP1 : Motor ENA (PWM)    (digital OUT — hardware PWM ch1)
 *    RC3 / SCL  : LCD I2C SCL        (I2C module — open-drain)
 *    RC4 / SDA  : LCD I2C SDA        (I2C module — open-drain)
 *    RC5        : Buzzer              (digital OUT — active HIGH)
 *    RC6 / TX   : UART TX to RPi     (digital OUT)
 *    RC7 / RX   : UART RX from RPi   (digital IN)
 *
 *  PORTD
 *    RD0        : Motor A IN1        (digital OUT)
 *    RD1        : Motor A IN2        (digital OUT)
 *    RD2        : Motor B IN3        (digital OUT)
 *    RD3        : Motor B IN4        (digital OUT)
 *    RD4        : White LED 1 (head) (digital OUT)
 *    RD5        : White LED 2 (head) (digital OUT)
 *    RD6        : White LED 3 (head) (digital OUT)
 *    RD7        : White LED 4 (head) (digital OUT)
 *
 *  PORTE
 *    RE0–RE2    : unused              (kept as inputs)
 * ============================================================
 *  TRIS: 1=input, 0=output
 */

/* RA0=in, RA1=out, RA2=in, RA3-5=in */
#define GPIO_PORTA_DIR      0x3D   /* 0b00111101 */

/* RB0=in, RB1-3=out, RB4-7=in */
#define GPIO_PORTB_DIR      0xF1   /* 0b11110001 */

/* RC7=in(RX), RC3/RC4 managed by I2C HW, rest=out */
#define GPIO_PORTC_DIR      0x80   /* 0b10000000 */

/* All outputs: motor direction + headlight LEDs */
#define GPIO_PORTD_DIR      0x00

/* RE0-RE2 kept as inputs (unused) */
#define GPIO_PORTE_DIR      0x07

/* Initial output values: everything off / low */
#define GPIO_PORTA_INIT_VAL 0x00
#define GPIO_PORTB_INIT_VAL 0x00
#define GPIO_PORTC_INIT_VAL 0x00
#define GPIO_PORTD_INIT_VAL 0x00
#define GPIO_PORTE_INIT_VAL 0x00

#endif /* GPIO_CONFIG_H */