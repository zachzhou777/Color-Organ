;******************************************************************************
; NeoPixel (WS2812B) Driver for Tiva LaunchPad
; Usage: Connect data line on a NeoPixel strip to the specified GPIO pin. Also 
;   provide the address and length of an array of 32-bit integers whose values 
;   are 24-bit RGB codes of each of the NeoPixels, with the first element 
;   corresponding to the LED closest to the GPIO pin.
; Note: neopixels.s can be used independently of neopixels.c/.h, but for ease 
;   of use, do NOT directly call send_neopixels_data() from here; instead, use 
;   the wrapper function flash_neopixels().
; Author: Zachary Zhou
;******************************************************************************

    export send_neopixels_data

GREEN_MASK EQU 0x00008000
RED_MASK   EQU 0x80000000
BLUE_MASK  EQU 0x00800000

gpio_data_addr   RN r0
gpio_pin_m       RN r1
neopixel_data    RN r2
num_neopixels    RN r3
gpio_data        RN r4
inv_gpio_pin_m   RN r5
rgb_code         RN r6
color_mask        RN r7
color_loop_cnt   RN r8
bits_loop_cnt    RN r9
wait_cnt         RN r10

;******************************************************************************
; FLASH Segment
;******************************************************************************
    AREA    FLASH, CODE, READONLY
    align

;******************************************************************************
; EABI compliant function that sends data to the NeoPixel strip following the 
; protocol specified by the WS2812B datasheet.
; Parameters:
;   r0 - GPIO data register address
;   r1 - GPIO pin mask
;   r2 - NeoPixel data array address
;   r3 - Number of NeoPixels
;******************************************************************************
send_neopixels_data PROC
    ; Save R4 through R10
    PUSH {r4-r10}
    
    ; Transmission beginning; disable interrupts
    CPSID i
    
    ; Initialize 'gpio_data' and 'inv_gpio_pin_m'
    LDR gpio_data, [gpio_data_addr]
    MVN inv_gpio_pin_m, gpio_pin_m
    
    ; In case 'num_neopixels' is 0 from the get-go, return from function
    CMP num_neopixels, #0
    
send_neopixels_data_led_loop_begin
    BEQ send_neopixels_data_return
    
    ; Begin data transfer for an LED
    LDR rgb_code, [neopixel_data], #4
    MOV color_loop_cnt, #3
send_neopixels_data_color_loop_begin
    ; Set 'color_mask' based on 'color_loop_cnt'
    MOV color_mask, #GREEN_MASK
    CMP color_loop_cnt, #2
    MOVEQ color_mask, #RED_MASK
    CMP color_loop_cnt, #1
    MOVEQ color_mask, #BLUE_MASK
    
    ; Send 8 bits per color
    MOV bits_loop_cnt, #8
send_neopixels_data_bits_loop_begin
    ; Set appropriate bit high
    ORR gpio_data, gpio_data, gpio_pin_m
    STR gpio_data, [gpio_data_addr]
    
    ; If next bit to transfer is 0, write 0 to bit in 'gpio_data'
    TST rgb_code, color_mask
    ANDEQ gpio_data, gpio_data, inv_gpio_pin_m
    
    ; 'rgb_code' gets shifted, while 'color_mask' remains constant
    LSL rgb_code, rgb_code, #1
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    
    ; Assert value on pin, then put 0 in pin's bit in 'gpio_data'
    STR gpio_data, [gpio_data_addr]
    AND gpio_data, inv_gpio_pin_m
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    
    ; Unconditionally pull pin low
    STR gpio_data, [gpio_data_addr]
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    
    ; If branch taken, then still transmitting same color
    SUBS bits_loop_cnt, bits_loop_cnt, #1
    BGT send_neopixels_data_bits_loop_begin
    
    ; If branch taken, then moving onto next color
    SUBS color_loop_cnt, color_loop_cnt, #1
    BGT send_neopixels_data_color_loop_begin
    
    ; If branch taken, then moving onto next LED
    SUBS num_neopixels, num_neopixels, #1
    BGT send_neopixels_data_led_loop_begin

    ; Wait >50 microseconds, i.e., 250 * 0.2 microseconds
    MOV wait_cnt, #250
    
send_neopixels_data_wait
    ; 10 NOPs = 0.2 microseconds, and branching takes some time too
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    
    SUBS wait_cnt, wait_cnt, #1
    BGT send_neopixels_data_wait
    
send_neopixels_data_return
    ; Transmission completed; enable interrupts
    CPSIE i
    
    ; Restore R4 through R10
    POP     {r4-r10}
    
    BX LR
    ENDP
    ALIGN
    
    END
