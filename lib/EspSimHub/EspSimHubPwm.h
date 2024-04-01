#pragma once
#include <Arduino.h>

#ifdef ESP32
#include "driver/ledc.h"

ledc_timer_bit_t getResolutionFromBits(uint8_t dutyCycleRangeInBits) {
    if (dutyCycleRangeInBits > SOC_LEDC_TIMER_BIT_WIDE_NUM || dutyCycleRangeInBits < 1) {
        throw -1;
    }

    switch(dutyCycleRangeInBits) {
        case 1:
            return LEDC_TIMER_1_BIT;
        case 2:
            return LEDC_TIMER_2_BIT;
        case 3:
            return LEDC_TIMER_3_BIT;
        case 4:
            return LEDC_TIMER_4_BIT;
        case 5:
            return LEDC_TIMER_5_BIT;
        case 6:
            return LEDC_TIMER_6_BIT;
        case 7:
            return LEDC_TIMER_7_BIT;
        case 8:
            return LEDC_TIMER_8_BIT;
        case 9:
            return LEDC_TIMER_9_BIT;
        case 10:
            return LEDC_TIMER_10_BIT;
        case 11:
            return LEDC_TIMER_11_BIT;
        case 12:
            return LEDC_TIMER_12_BIT;
        case 13:
            return LEDC_TIMER_13_BIT;
        case 14:
            return LEDC_TIMER_14_BIT;
#if SOC_LEDC_TIMER_BIT_WIDE_NUM > 14
        case 15:
            return LEDC_TIMER_15_BIT;
        case 16:
            return LEDC_TIMER_16_BIT;
        case 17:
            return LEDC_TIMER_17_BIT;
        case 18:
            return LEDC_TIMER_18_BIT;
        case 19:
            return LEDC_TIMER_19_BIT;
        case 20:
            return LEDC_TIMER_20_BIT;
#endif
        default:
            return LEDC_TIMER_8_BIT;
    } 
}

ledc_channel_t getChannelFromNumber(uint8_t channel) {
    if (channel > SOC_LEDC_CHANNEL_NUM || channel < 0) {
        throw -1;
    }
    
    switch(channel) {
        case 0:
            return LEDC_CHANNEL_0;
        case 1:
            return LEDC_CHANNEL_1;
        case 2:
            return LEDC_CHANNEL_2;
        case 3:
            return LEDC_CHANNEL_3;
        case 4:
            return LEDC_CHANNEL_4;
        case 5:
            return LEDC_CHANNEL_5;
#if SOC_LEDC_CHANNEL_NUM > 6
        case 6:
            return LEDC_CHANNEL_6;
        case 7:
            return LEDC_CHANNEL_7;
#endif
#if SOC_LEDC_CHANNEL_NUM > 8
        case 8:
            return LEDC_CHANNEL_8;
        case 9:
            return LEDC_CHANNEL_9;
        case 10:
            return LEDC_CHANNEL_10;
        case 11:
            return LEDC_CHANNEL_11;
        case 12:
            return LEDC_CHANNEL_12;
        case 13:
            return LEDC_CHANNEL_13;
        case 14:
            return LEDC_CHANNEL_14;
        case 15:
            return LEDC_CHANNEL_15;
#endif
        default:
            return LEDC_CHANNEL_0;
    }
}

ledc_timer_t getTimerFromNumber(uint8_t timer) {
    if (timer > (LEDC_TIMER_MAX - 1) || timer < 0) {
        throw -1;
    }
    
    switch(timer) {
        case 0:
            return LEDC_TIMER_0;
        case 1:
            return LEDC_TIMER_1;
        case 2:
            return LEDC_TIMER_2;
        case 3:
            return LEDC_TIMER_3;
        default:
            return LEDC_TIMER_0;
    }
}

/**
 * A struct that contains timer and channel structs from LEDC
 */
struct ledc_components {
    uint8_t pin;
    ledc_timer_config_t timer;
    ledc_channel_config_t channel;
};


/**
 * Reuses the timer and creates a new channel using it
 */
ledc_components makePwmFromTimer(
    ledc_timer_config_t timerConfig,
    ledc_channel_t channel,
    uint32_t initialDutyCycleForResolution,
    uint8_t outputPin
) {
    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .gpio_num       = outputPin,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = channel,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = timerConfig.timer_num,
        .duty           = initialDutyCycleForResolution,
        .hpoint         = 0,
    };
    if(ledc_channel_config(&ledc_channel) != ESP_OK) {
      throw -2;
    }

    ledc_components components = {
        .pin = outputPin,
        .timer = timerConfig,
        .channel = ledc_channel,
    };

    return components;
} 


/**
 * Configure a new timer and channel for PWM
 */
ledc_components makePwm(
    ledc_timer_bit_t resolution,
    ledc_timer_t timer,
    ledc_channel_t channel,
    uint32_t freq,
    uint32_t initialDutyCycleForResolution,
    uint8_t outputPin
) {
     // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE, // the old ESP32 is the only one able to use non-low speed modes
        .duty_resolution  = resolution,
        .timer_num        = timer,
        .freq_hz          = freq,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    if (ledc_timer_config(&ledc_timer) != ESP_OK) {
      throw -1;
    }

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .gpio_num       = outputPin,
        .speed_mode     = LEDC_LOW_SPEED_MODE, // the old ESP32 is the only one able to use non-low speed modes
        .channel        = channel,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = timer,
        .duty           = initialDutyCycleForResolution,
        .hpoint         = 0,
    };
    if(ledc_channel_config(&ledc_channel) != ESP_OK) {
      throw -2;
    }

    ledc_components components = {
        .pin = outputPin,
        .timer = ledc_timer,
        .channel = ledc_channel,
    };

    return components;
}

char currentTimer = 0;
char currentChannel = 0;


/**
 * Allocate a number of channels for static frequency and varying the duty cycle.
 * 
 *  This will create a single timer attached to multiple channels, and each channel
 *  can vary its duty cycle independently.
 * 
 * dutyCycleRangeInBits is how many steps will the system divide duty cycle into, 8 bits -> 0-255; 10 bits; 0-1023 and so on.
 *  We need to tweak this number based on how we get our duty cycle updates (number range), but also it's necessary to be tweaked 
 *  in order to achieve different ranges of frequencies due to dividers in the device, clock etc.
 *  We need higher bits resolution for lower frequencies and lower bits for higher frequencies, or LEDC won't work at all. 
 *   This isn't matter of YOU requiring more resolution, but the hardware timers being able to count properly to achieve the target.
 * 
 * This function will throw if you've allocated more channels or timers than available in the hardware, so make sure you don't have
 *  many pwm features on at the same time (fans, motors, gauges, meters).
 * 
 *  This function modifies the passed array of ledc_components (pointer) with outputPinSize valid elements
 *   with a maximum of SOC_LEDC_CHANNEL_NUM elements (6-16): 
 *   https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/ledc.html
 *   (please notice that we're NOT using Arduino framework for esp32 to handle PWM)
 */
void createPwmForDutyCycleControl(
    struct ledc_components *components,
    uint8_t outputPinSize,
    uint32_t staticFrequency, 
    uint8_t dutyCycleResolutionBits
) {
    if (currentChannel + outputPinSize > SOC_LEDC_CHANNEL_NUM) {
        // We can't allocate the requested number of channels
        throw -1;
    }
    if (currentTimer + 1 > (LEDC_TIMER_MAX - 1)) {
        // We can't allocate the requested number of timers (1 more)
        throw -2;
    }

    for (int i = 0; i < outputPinSize; i++) {
        if (i == 0) {
            components[i] = makePwm(
                getResolutionFromBits(dutyCycleResolutionBits),
                getTimerFromNumber(currentTimer++),
                getChannelFromNumber(currentChannel++),
                staticFrequency,
                0,
                components[i].pin
            );
        } else {
            ledc_timer_config_t timer = components[0].timer;
            components[i] = makePwmFromTimer(
                timer,
                getChannelFromNumber(currentChannel++),
                0,
                components[i].pin
            );
        }
    }
}

/**
 * Create a channel and timer for each of the passed pins
 * 
 * ESPs have a maximum of (LEDC_TIMER_MAX - 1)[usually 4] timers, and those are shared among all LEDC outputs.
 * Thus function will take 1 to (LEDC_TIMER_MAX - 1) pins for varying frequency and create a channel and timer per each
 *  You will deplete the timers quickly with this function, so plan ahead which features will be enabled
 */
void createPwmForFrequencyControl(
    struct ledc_components *components,
    uint8_t outputPinSize,
    uint32_t initialFrequency, 
    uint8_t dutyCycleResolutionBits,
    uint32_t initialDutyCycleForResolution // if resolution is 8 bits this parameter is expected to be between 0-255; if 10 bits then 0-1023, etc
) {
    if (currentChannel + outputPinSize > SOC_LEDC_CHANNEL_NUM) {
        // We can't allocate the requested number of channels
        throw -1;
    }
    if (currentTimer + outputPinSize > (LEDC_TIMER_MAX - 1)) {
        // We can't allocate the requested number of timers 
        throw -2;
    }

    for (int i = 0; i < outputPinSize; i++) {
        auto component = makePwm(
            getResolutionFromBits(dutyCycleResolutionBits),
            getTimerFromNumber(currentTimer++),
            getChannelFromNumber(currentChannel++),
            initialFrequency,
            initialDutyCycleForResolution,
            components[i].pin
        );
        components[i].channel = component.channel;
        components[i].timer = component.timer;
    }
}
#endif
