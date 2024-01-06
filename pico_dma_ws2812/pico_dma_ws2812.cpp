// Copyright (c) 2022, Takuya Urakawa (Dm9Records 5z6p.com)
// SPDX-License-Identifier: MIT

// modified by alex mole:
//   * switched to RGB channel ordering
//   * removed mutex and made all calls blocking
//   * add support for two different sets of WS2812 strings with different DMA channels

#include "pico_dma_ws2812.hpp"

#include <cstring>


static int gDmaChannelPerIrq[2] = { -1, -1 };

static void dmaIrq0Complete() {
    if (gDmaChannelPerIrq[0] >= 0)
        dma_hw->ints0 = 1u << gDmaChannelPerIrq[0];
}
static void dmaIrq1Complete() {
    if (gDmaChannelPerIrq[1] >= 0)
        dma_hw->ints1 = 1u << gDmaChannelPerIrq[1];
}


WS2812::WS2812(uint num_leds, PIO pio, uint sm, uint irq, uint pin, uint freq)
    : num_leds(num_leds)
    , pio(pio)
    , sm(sm)
    , irq(irq)
{
    buffer = new RGB *[BUFFER_COUNT];
    for (uint i = 0; i < BUFFER_COUNT; i++) {
        buffer[i] = new RGB[num_leds];
    }

    pio_program_offset = pio_add_program(pio, &ws2812_program);

    pio_gpio_init(pio, pin);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);

    pio_sm_config c = ws2812_program_get_default_config(pio_program_offset);
    sm_config_set_sideset_pins(&c, pin);

    sm_config_set_out_shift(&c, false, true, 24);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);

    int cycles_per_bit = ws2812_T1 + ws2812_T2 + ws2812_T3;
    float div          = clock_get_hz(clk_sys) / (freq * cycles_per_bit);
    sm_config_set_clkdiv(&c, div);

    pio_sm_init(pio, sm, pio_program_offset, &c);
    pio_sm_set_enabled(pio, sm, true);

    dma_channel               = dma_claim_unused_channel(true);
    dma_channel_config config = dma_channel_get_default_config(dma_channel);
    channel_config_set_bswap(&config, true);
    channel_config_set_dreq(&config, pio_get_dreq(pio, sm, true));
    channel_config_set_transfer_data_size(&config, DMA_SIZE_32);
    channel_config_set_read_increment(&config, true);
    dma_channel_set_trans_count(dma_channel, num_leds, false);
    dma_channel_set_read_addr(dma_channel, (uint32_t *)buffer[BUFFER_OUT],
                              false);
    dma_channel_configure(dma_channel, &config, &pio->txf[sm], NULL, 0, false);

    // DMA complete irq setting
    gDmaChannelPerIrq[irq] = dma_channel;
    if (irq == 0) {
        dma_channel_set_irq0_enabled(dma_channel, true);
        irq_set_exclusive_handler(DMA_IRQ_0, dmaIrq0Complete);
        irq_set_enabled(DMA_IRQ_0, true);
    }
    else {
        dma_channel_set_irq1_enabled(dma_channel, true);
        irq_set_exclusive_handler(DMA_IRQ_1, dmaIrq1Complete);
        irq_set_enabled(DMA_IRQ_1, true);
    }
}

WS2812::~WS2812() {
    clear();
    update();
    dma_channel_unclaim(dma_channel);
    gDmaChannelPerIrq[irq] = -1;
    pio_sm_set_enabled(pio, sm, false);
    pio_remove_program(pio, &ws2812_program, pio_program_offset);
#ifndef MICROPY_BUILD_TYPE
    // pio_sm_unclaim seems to hardfault in MicroPython
    pio_sm_unclaim(pio, sm);
#endif
    // Only delete buffers we have allocated ourselves.
    for (uint i = 0; i < BUFFER_COUNT; i++) {
        delete[] buffer[i];
    }
    delete[] buffer;
}


bool WS2812::update() {
    if (!request_send) return false;

    // copy buffer to out
    memcpy(buffer[BUFFER_OUT], buffer[BUFFER_IN], sizeof(RGB) * num_leds);
    request_send = false;

    add_alarm_in_us(LED_RESET_TIME, reset_time_alert_callback, (void *)this,
                    false);

    for (;;)
    {
        if (dma_channel_is_busy(dma_channel))
            break;
    }

    return true;
}

void WS2812::send() {
    dma_channel_set_trans_count(dma_channel, num_leds, false);
    dma_channel_set_read_addr(dma_channel, buffer[BUFFER_OUT], true);
}

void WS2812::set_request_send() {
    request_send = true;
}

int64_t WS2812::reset_time_alert_callback(alarm_id_t id, void *user_data) {
    WS2812 *ws2812 = static_cast<WS2812 *>(user_data);
    ws2812->send();
    return 0;
}

void WS2812::clear() {
    for (auto i = 0u; i < num_leds; ++i) {
        set_rgb(i, 0, 0, 0);
    }
}

void WS2812::set_hsv(uint32_t index, float h, float s, float v) {
    float i = floor(h * 6.0f);
    float f = h * 6.0f - i;
    v *= 255.0f;
    uint8_t p = v * (1.0f - s);
    uint8_t q = v * (1.0f - f * s);
    uint8_t t = v * (1.0f - (1.0f - f) * s);

    switch (int(i) % 6) {
        case 0:
            set_rgb(index, v, t, p);
            break;
        case 1:
            set_rgb(index, q, v, p);
            break;
        case 2:
            set_rgb(index, p, v, t);
            break;
        case 3:
            set_rgb(index, p, q, v);
            break;
        case 4:
            set_rgb(index, t, p, v);
            break;
        case 5:
            set_rgb(index, v, p, q);
            break;
    }
}

void WS2812::set_hsv_scaled(uint32_t index, float h, float s, float v, float rScale, float gScale, float bScale)
{
    using u8 = uint8_t;

    float i = floor(h * 6.0f);
    float f = h * 6.0f - i;
    v *= 255.0f;
    float p = v * (1.0f - s);
    float q = v * (1.0f - f * s);
    float t = v * (1.0f - (1.0f - f) * s);

    switch (int(i) % 6) {
        case 0:
            set_rgb(index, u8(v * rScale), u8(t * gScale), u8(p * bScale));
            break;
        case 1:
            set_rgb(index, u8(q * rScale), u8(v * gScale), u8(p * bScale));
            break;
        case 2:
            set_rgb(index, u8(p * rScale), u8(v * gScale), u8(t * bScale));
            break;
        case 3:
            set_rgb(index, u8(p * rScale), u8(q * gScale), u8(v * bScale));
            break;
        case 4:
            set_rgb(index, u8(t * rScale), u8(p * gScale), u8(v * bScale));
            break;
        case 5:
            set_rgb(index, u8(v * rScale), u8(p * gScale), u8(q * bScale));
            break;
    }
}


void WS2812::set_rgb(uint32_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index >= num_leds) return;

    buffer[BUFFER_IN][index].rgb(r, g, b);
    request_send = true;
}
