/*----------------------------------------------------------------------------
 * Name:    LED.c
 * Purpose: low level LED functions
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2013 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <stm32f30x.h>
#include "LED.h"

const unsigned long led_mask[] = {1UL <<  8, 1UL <<  9, 1UL << 10, 1UL << 11,
                                  1UL << 12, 1UL << 13, 1UL << 14, 1UL << 15 };

/*----------------------------------------------------------------------------
  initialize LED Pins (PE8..15)
 *----------------------------------------------------------------------------*/
void LED_Init (void) {

  RCC->AHBENR     |=  ((1UL << 21) );                 /* Enable GPIOE clock              */

  GPIOE->ODR     &= ~((1UL <<   8) | (1UL <<   9) |
                       (1UL <<  10) | (1UL <<  11) |
                       (1UL <<  12) | (1UL <<  13) |
                       (1UL <<  14) | (1UL <<  15)  ); /* switch LEDs off)               */

  GPIOE->MODER    &= ~((3UL << 2* 8) | (3UL << 2* 9) |
                       (3UL << 2*10) | (3UL << 2*11) |
                       (3UL << 2*12) | (3UL << 2*13) |
                       (3UL << 2*14) | (3UL << 2*15)  ); /* PE.8..15 is output           */
  GPIOE->MODER    |=  ((1UL << 2* 8) | (1UL << 2* 9) |
                       (1UL << 2*10) | (1UL << 2*11) |
                       (1UL << 2*12) | (1UL << 2*13) |
                       (1UL << 2*14) | (1UL << 2*15)  );
  GPIOE->OTYPER   &= ~((1UL <<    8) | (1UL <<    9) |
                       (1UL <<   10) | (1UL <<   11) |
                       (1UL <<   12) | (1UL <<   13) |
                       (1UL <<   14) | (1UL <<   15)  ); /* PE.8..15 is output Push-Pull */
  GPIOE->OSPEEDR  |=  ((3UL << 2* 8) | (3UL << 2* 9) |
                       (3UL << 2*10) | (3UL << 2*11) |
                       (3UL << 2*12) | (3UL << 2*13) |
                       (3UL << 2*14) | (3UL << 2*15)  ); /* PE.8..15 is High Speed       */
  GPIOE->PUPDR    &= ~((3UL << 2* 8) | (3UL << 2* 9) |
                       (3UL << 2*10) | (3UL << 2*11) |
                       (3UL << 2*12) | (3UL << 2*13) |
                       (3UL << 2*14) | (3UL << 2*15)  ); /* PE.8..15 is Pull up          */
  GPIOE->PUPDR    |=  ((1UL << 2* 8) | (1UL << 2* 9) |
                       (1UL << 2*10) | (1UL << 2*11) |
                       (1UL << 2*12) | (1UL << 2*13) |
                       (1UL << 2*14) | (1UL << 2*15)  );
}

/*----------------------------------------------------------------------------
  Function that turns on requested LED
 *----------------------------------------------------------------------------*/
void LED_On (unsigned int num) {

  if (num < LED_NUM) {
    GPIOE->BSRR = led_mask[num];
  }
}

/*----------------------------------------------------------------------------
  Function that turns off requested LED
 *----------------------------------------------------------------------------*/
void LED_Off (unsigned int num) {

  if (num < LED_NUM) {
    GPIOE->BSRR = led_mask[num] << 16;
  }
}

/*----------------------------------------------------------------------------
  Function that outputs value to LEDs
 *----------------------------------------------------------------------------*/
void LED_Out(unsigned int value) {
  int i;

  for (i = 0; i < LED_NUM; i++) {
    if (value & (1<<i)) {
      LED_On (i);
    } else {
      LED_Off(i);
    }
  }
}
