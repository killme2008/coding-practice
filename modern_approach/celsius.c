//
// Created by 庄晓丹 on 2017/1/4.
//

#include <stdio.h>

#define FREEZING_PT 32.0f
#define SCALE_FACTOR (5.0f / 9.0f)

int main(void) {
    float f, c;

    printf("Enter Fahrenheit temmperature: ");
    scanf("%f", &f);

    c = (f - FREEZING_PT) * SCALE_FACTOR;

    printf("Celsius equivalent: %.1f", c);

    return 0;
}
