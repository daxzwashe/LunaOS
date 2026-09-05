#include <stdint.h>
#include <stddef.h>
#include "../include/framebuffer.h"

extern Framebuffer* GlobalFB;
extern const uint8_t* GetCharBitmap(char c);
extern void sleep_ms(uint32_t ms);
extern void ClearScreen(uint32_t color);

extern const uint32_t BAD_APPLE_WIDTH;
extern const uint32_t BAD_APPLE_HEIGHT;
extern const uint32_t BAD_APPLE_FRAMES;
extern const uint8_t bad_apple_frames[];

extern volatile int StopVideoFlag;

static const char ASCII_RAMP[] = " .:-=+*#%@";

static inline void DrawGlyph(int px_x, int px_y, char c, uint32_t fg_color, uint32_t bg_color) {
    uint32_t* screen = (uint32_t*)GlobalFB->BaseAddress;
    const uint8_t* bitmap = GetCharBitmap(c);

    for (int y = 0; y < 16; y++) {
        uint32_t screen_y = px_y + y;
        if (screen_y >= GlobalFB->Height) break;

        uint32_t line_offset = screen_y * GlobalFB->PixelsPerScanLine;
        uint8_t row = bitmap[y];

        for (int x = 0; x < 8; x++) {
            uint32_t screen_x = px_x + x;
            if (screen_x >= GlobalFB->Width) break;

            uint32_t color = ((row >> (7 - x)) & 1) ? fg_color : bg_color;
            screen[line_offset + screen_x] = color;
        }
    }
}

void RunAppleApp(void) {
    StopVideoFlag = 0;

    // Вычисляем размер текстовой сетки на весь экран (символ 8x16)
    uint32_t screen_cols = GlobalFB->Width / 8;
    uint32_t screen_rows = GlobalFB->Height / 16;

    uint32_t frame_size = BAD_APPLE_WIDTH * BAD_APPLE_HEIGHT;

    for (uint32_t f = 0; f < BAD_APPLE_FRAMES; f++) {
        // Прерывание видео при нажатии Ctrl+C
        if (StopVideoFlag) {
            break;
        }

        const uint8_t* current_frame = &bad_apple_frames[f * frame_size];

        for (uint32_t row = 0; row < screen_rows; row++) {
            // Маппинг текущей строки экрана на строку видеокадра
            uint32_t src_y = (row * BAD_APPLE_HEIGHT) / screen_rows;

            for (uint32_t col = 0; col < screen_cols; col++) {
                // Маппинг текущей колонки экрана на колонку видеокадра
                uint32_t src_x = (col * BAD_APPLE_WIDTH) / screen_cols;

                uint8_t ramp_index = current_frame[src_y * BAD_APPLE_WIDTH + src_x];
                char ch = ASCII_RAMP[ramp_index % 10];

                DrawGlyph(
                    col * 8,
                    row * 16,
                    ch,
                    0xFFFFFFFF,
                    0xFF000000
                );
            }
        }

        sleep_ms(66); // ~15 FPS
    }

    // Восстановление экрана после выхода
    ClearScreen(0xFF000000);
}