#include "main.h"


typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} pin_type;

typedef struct {
    pin_type digit_activators[4];
    pin_type BCD_input[4];
    uint32_t digits[4];
    uint32_t number;
} seven_segment_type;

seven_segment_type seven_segment = {.digit_activators={{.port=GPIOB, .pin=GPIO_PIN_0},
                                                       {.port=GPIOB, .pin=GPIO_PIN_1},
                                                       {.port=GPIOB, .pin=GPIO_PIN_2},
                                                       {.port=GPIOE, .pin=GPIO_PIN_7}},
        .BCD_input={{.port=GPIOC, .pin=GPIO_PIN_2},
                    {.port=GPIOC, .pin=GPIO_PIN_3},
                    {.port=GPIOF, .pin=GPIO_PIN_2},
                    {.port=GPIOA, .pin=GPIO_PIN_1}},
        .digits={0, 0, 0, 0},
        .number = 0};

void seven_segment_display_decimal(uint32_t n) {
    if (n < 10) {
        HAL_GPIO_WritePin(seven_segment.BCD_input[0].port, seven_segment.BCD_input[0].pin,
                          (n & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(seven_segment.BCD_input[1].port, seven_segment.BCD_input[1].pin,
                          (n & 2) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(seven_segment.BCD_input[2].port, seven_segment.BCD_input[2].pin,
                          (n & 4) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(seven_segment.BCD_input[3].port, seven_segment.BCD_input[3].pin,
                          (n & 8) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

void seven_segment_deactivate_digits(void) {
    for (int i = 0; i < 4; ++i) {
        HAL_GPIO_WritePin(seven_segment.digit_activators[i].port, seven_segment.digit_activators[i].pin,
                          GPIO_PIN_SET);
    }
}

void seven_segment_activate_digit(uint32_t d) {
    if (d < 4) {
        HAL_GPIO_WritePin(seven_segment.digit_activators[d].port, seven_segment.digit_activators[d].pin,
                          GPIO_PIN_RESET);
    }
}

void seven_segment_set_num(uint32_t n) {
    if (n < 10000) {
        seven_segment.number = n;
        for (uint32_t i = 0; i < 4; ++i) {
            seven_segment.digits[3 - i] = n % 10;
            n /= 10;
        }
    }
}

void seven_segment_refresh(void) {
    static uint32_t state = 0;
    static uint32_t last_time = 0;
    if (HAL_GetTick() - last_time > 5) {
        seven_segment_deactivate_digits();
        seven_segment_activate_digit(state);
        seven_segment_display_decimal(seven_segment.digits[state]);
        state = (state + 1) % 4;
        last_time = HAL_GetTick();
    }
}

void programInit(int x) {
    seven_segment_set_num(x);

}


void programLoop() {
    seven_segment_refresh();

}


//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
//    if (GPIO_Pin == GPIO_PIN_0) {
//        seven_segment_set_num(++seven_segment.number);
//        HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_9);
//    } else if (GPIO_Pin == GPIO_PIN_1) {
//
//    }
//}
