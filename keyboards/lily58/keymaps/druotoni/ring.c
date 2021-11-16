#include QMK_KEYBOARD_H
#include "gui_state.h"
#include "ring.h"

#include "fast_random.h"
#include "draw_helper.h"

char tListeTotal[SIZE_ARRAY_1]  = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P'};
char tListeTotal2[SIZE_ARRAY_1] = {'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '1', '2', '3', '4', '5', '6'};

static char tRefArc[SIZE_ARRAY_1]  = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P'};
static char tRefArc2[SIZE_ARRAY_1] = {'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '1', '2', '3', '4', '5', '6'};

const char code_to_name[60] = {' ', ' ', ' ', ' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'R', 'E', 'B', 'T', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ';', '\'', ' ', ',', '.', '/', ' ', ' ', ' '};

#define ANIM_CENTER_FRAME_NUMBER 5
uint32_t anim_center_timer = 0;
int anim_center_current_frame = 0;

#define ANIM_KEYLOG_FRAME_NUMBER 8
int      anim_keylog_current_frame = 0;
uint32_t anim_keylog_timer         = 0;

uint32_t circle_timer = 0;
char     c_target     = 'A';
char     c_target2    = 'Q';

char c_last     = ' ';
char c_previous = ' ';

static void rotate_right(char str[]) {
    int  iSize = SIZE_ARRAY_1;
    char cFist = str[0];
    for (int i = 0; i < iSize - 1; i++) {
        str[i] = str[i + 1];
    }
    str[iSize - 1] = cFist;
}

static void rotate_left(char str[]) {
    int  iSize = SIZE_ARRAY_1;
    char cLast = str[iSize - 1];
    for (int i = iSize - 1; i > 0; i--) {
        str[i] = str[i - 1];
    }
    str[0] = cLast;
}

static int GetPosition(char c, char tListe[]) {
    int iSize = SIZE_ARRAY_1;

    for (int i = 0; i < iSize; i++) {
        if (tListe[i] == c) return i;
    }

    return -1;
}

static signed int GetDistance(char cNew, char tListe[]) {
    int iPositionTarget = CURSOR_1;

    int iPositionNew = GetPosition(cNew, tListe);
    if (iPositionNew == -1) return 0;

    signed iReturn = iPositionNew - iPositionTarget;
    return iReturn;
}

// static char GetCharCursor(char tListe[]) { return tListe[CURSOR_1]; }


static int TesterEstDansListe(char cChar, char tListe[]) {
    int iSize = SIZE_ARRAY_1;

    for (int i = 0; i < iSize; i++) {
        if (tListe[i] == cChar) return 1;
    }

    return 0;
}

static void SmartRotation(char c, char tListe[]) {
    int signed i = GetDistance(c, tListe);
    if (i == 0) return;

    if (i < 0) {
        rotate_left(tListe);
        return;
    }

    if (i > 0) {
        rotate_right(tListe);
        return;
    }
}

static void update_list(char cNouveau, char tListe[]) {
    if (!TesterEstDansListe(cNouveau, tListe)) {
        // unknowed input
        return;
    }

    signed int iDistance = GetDistance(cNouveau, tListe);
    if (iDistance != 0) {
        SmartRotation(cNouveau, tListe);
    }
}

static void draw_arc_sector_16(uint8_t x, uint8_t y, uint8_t radius, int position, int color) {
    unsigned int s = 1;
    s              = s << (position / 2);

    if (position % 4 == 0 || position % 4 == 3) {
        draw_arc_sector(x, y, radius, s, 1, color);
    } else {
        draw_arc_sector(x, y, radius, s, 2, color);
    }
}

static void render_set(int x, int y, int r, int p, int color) {
    draw_arc_sector_16(x, y, r, p, color);
    draw_arc_sector_16(x, y, r - 1, p, color);
}

static void draw_letter_circle(char t[], char tRef[], char ct, int x, int y, int r, bool invert) {
    int  iPositionTarget = CURSOR_1;
    char c               = t[iPositionTarget];

    int p  = GetPosition(c, tRef);
    int pt = GetPosition(ct, tRef);

    bool w = 1;
    bool b = 0;

    if (!invert) {
        draw_fill_circle(x, y, r, b);
        draw_circle(x, y, r, b);
        draw_circle(x, y, r - 1, b);
        draw_circle(x, y, r - 2, b);

        // draw_circle(x, y, r, b);
        // draw_circle(x, y, r - 1, b);

        r -= 4;
        draw_circle(x, y, r, w);
        draw_circle(x, y, r - 1, w);
        r += 4;
    }

    int pafter  = (pt + 1) % SIZE_ARRAY_1;
    int pbefore = (pt + SIZE_ARRAY_1 - 1) % SIZE_ARRAY_1;
    render_set(x, y, r, pt, w);
    render_set(x, y, r, pafter, w);
    render_set(x, y, r, pbefore, w);

    pafter  = (pt + 2) % SIZE_ARRAY_1;
    pbefore = (pt + SIZE_ARRAY_1 - 2) % SIZE_ARRAY_1;
    render_set(x, y, r, pafter, w);
    render_set(x, y, r, pbefore, w);

    r -= 4;

    pafter  = (p + 1) % SIZE_ARRAY_1;
    pbefore = (p + SIZE_ARRAY_1 - 1) % SIZE_ARRAY_1;

    render_set(x, y, r, p, b);
    render_set(x, y, r, pafter, b);
    render_set(x, y, r, pbefore, b);

    draw_circle(x, y, r - 6, w);
}

static const char PROGMEM raw_circle[4][128] = {{
                                                    0, 0, 0, 192, 32, 16, 8, 8, 4, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 4, 8, 8, 16, 32, 192, 0, 0, 0, 240, 14, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 129, 238, 240, 15, 112, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 192, 240, 255, 127, 15, 0, 0, 0, 3, 4, 8, 16, 16, 32, 64, 64, 64, 128, 128, 192, 192, 224, 224, 224, 240, 112, 120, 124, 60, 30, 30, 15, 7, 3, 0, 0, 0,
                                                },
                                                {
                                                    0, 0, 0, 192, 32, 16, 8, 8, 4, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 4, 8, 8, 48, 224, 192, 0, 0, 0, 240, 14, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 255, 255, 254, 240, 15, 112, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 128, 192, 224, 240, 248, 248, 254, 255, 255, 127, 15, 0, 0, 0, 3, 7, 15, 31, 30, 62, 126, 126, 126, 254, 254, 254, 254, 254, 255, 255, 255, 127, 127, 127, 63, 31, 31, 15, 7, 3, 0, 0, 0,
                                                },
                                                {
                                                    0, 0, 0, 192, 32, 16, 8, 8, 4, 2, 2, 2, 1, 1, 1, 1, 3, 15, 255, 255, 254, 254, 254, 252, 248, 248, 240, 224, 192, 0, 0, 0, 240, 14, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 128, 192, 240, 248, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 254, 240, 15, 126, 252, 248, 248, 248, 248, 248, 252, 252, 254, 254, 255, 255, 255, 255, 255, 255, 255, 127, 63, 31, 15, 7, 7, 3, 3, 3, 7, 143, 127, 15, 0, 0, 0, 3, 7, 15, 31, 31, 63, 127, 127, 127, 255, 255, 255, 255, 255, 255, 195, 128, 64, 64, 64, 32, 16, 16, 8, 4, 3, 0, 0, 0,
                                                },
                                                {
                                                    0, 0, 0, 192, 224, 240, 248, 248, 124, 62, 30, 14, 15, 7, 7, 3, 3, 3, 1, 1, 2, 2, 2, 4, 8, 8, 16, 32, 192, 0, 0, 0, 240, 254, 255, 31, 15, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 14, 240, 15, 115, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 112, 15, 0, 0, 0, 3, 4, 8, 16, 16, 32, 64, 64, 64, 128, 128, 128, 128, 128, 128, 128, 128, 64, 64, 64, 32, 16, 16, 8, 4, 3, 0, 0, 0,
                                                }};

static void draw_center_circle_frame(uint8_t x, uint8_t y, uint8_t r, int f) {
    draw_fill_circle(x, y, r, 0);
    draw_circle(x, y, r, 0);

    if (f == 0) {
        draw_circle(x, y, r, 1);
    } else {
        oled_set_cursor(0, 11);
        oled_write_raw_P(raw_circle[f - 1], sizeof(raw_circle[f - 1]));
    }
}

static void render_anim_center_circle(uint8_t x, uint8_t y, uint8_t r) {
    if (anim_center_current_frame == ANIM_CENTER_FRAME_NUMBER) return;

    if (timer_elapsed32(anim_center_timer) > 40) {
        anim_center_timer = timer_read32();

        draw_center_circle_frame(x, y, r, anim_center_current_frame);

        anim_center_current_frame++;
    }
}


static void write_char(char c) {
    oled_set_cursor(2, 6);
    oled_write_char(c, false);
    move_block(12, 48, 6, 8, 2);
}

static void render_keylog(void) {
    if (anim_keylog_current_frame != ANIM_KEYLOG_FRAME_NUMBER) {
        if (timer_elapsed32(anim_keylog_timer) > 20) {
            anim_keylog_timer = timer_read32();
            anim_keylog_current_frame++;
        }

        draw_rectangle_fill(7, 46, 21, 11, false);

        if (anim_keylog_current_frame < ANIM_KEYLOG_FRAME_NUMBER / 2) {
            write_char(c_previous);
            draw_glitch_comb(9, 6 * 8, 18, 8, anim_keylog_current_frame + 1, true);
        } else {
            write_char(c_last);
            draw_glitch_comb(9, 6 * 8, 18, 8, ANIM_KEYLOG_FRAME_NUMBER - anim_keylog_current_frame, false);
        }

        return;
    }

    write_char(c_last);
}

#define ANIM_SLEEP_RING_FRAME_NUMBER 4

uint32_t anim_sleep_ring_timer = 0;
uint8_t current_sleep_ring_frame     = 0;
uint8_t sleep_ring_frame_destination = ANIM_SLEEP_RING_FRAME_NUMBER - 1;

void reset_ring(bool bNeedOpen) {
    // allume : doit s'eteindre
    current_sleep_ring_frame = 0;

    if (bNeedOpen) {
        // eteint : doit s'allumer
        anim_sleep_ring_timer    = timer_read32();
        current_sleep_ring_frame = ANIM_SLEEP_RING_FRAME_NUMBER - 1;
    }

    sleep_ring_frame_destination = (ANIM_SLEEP_RING_FRAME_NUMBER - 1) - current_sleep_ring_frame;
}


static void render_tv_circle(uint8_t x, uint8_t y, uint8_t r, int f) {
    switch (f) {
        case 0:
            draw_fill_circle(x, y, r, 1);
            break;

        case 1:
            draw_fill_circle(x, y, r - 5, 1);
            break;

        case 2:
            // cross
            drawline_hr(x - 1, y + 1, 4, true);
            drawline_hr(x - 1, y - 1, 4, true);

            // central line
            drawline_hr(x - r / 2, y, r, true);
            break;

        case 3:
            draw_fill_circle(x, y, r, 0);
            break;
    }
}

static void render_ring_sleep(void) {
    if (timer_elapsed32(anim_sleep_ring_timer) > 30) {
        //  RenderScopeBlack();

        if (current_sleep_ring_frame != sleep_ring_frame_destination) {
            anim_sleep_ring_timer = timer_read32();

            render_tv_circle(15, 103, 11, current_sleep_ring_frame);

            current_sleep_ring_frame += (sleep_ring_frame_destination > current_sleep_ring_frame) ? 1 : -1;
        }
    }
}

static const char PROGMEM raw_bottom[] = {
    127, 192, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 192, 127,
};

static const char PROGMEM raw_middle[] = {
    240, 8, 4, 226, 241, 248, 124, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 56, 0, 1, 62, 255, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 28, 28, 28, 0, 255, 0, 0, 127, 127, 70, 70, 126, 70, 70, 126, 70, 70, 126, 126, 62, 30, 142, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 64, 64, 62, 1, 2, 114, 114, 2, 2, 114, 114, 2, 2, 114, 114, 2, 2, 2, 2, 1, 0, 0, 0, 128, 64, 64, 64, 64, 64, 64, 64, 64, 64, 128, 0, 0, 128, 131, 131, 132, 136, 179, 131, 132, 184, 131, 131, 188, 128, 128, 128, 128, 128, 128, 128, 143, 144, 149, 144, 149, 144, 149, 144, 149, 144, 143, 0,
};

void render_circle(gui_state_t t) {
    // gear and frame
    oled_set_cursor(0, 5);
    oled_write_raw_P(raw_middle, sizeof(raw_middle));
    drawline_hr(5, 39, 25, 1);
    draw_rectangle_fill(0, 88, 32, 32, false);
    drawline_vb(0, 80, 8, 1);
    drawline_vb(31, 80, 8, 1);
    oled_write_pixel(1, 80, true);
    oled_write_pixel(30, 80, true);
    oled_set_cursor(0, 15);
    oled_write_raw_P(raw_bottom, sizeof(raw_bottom));

    if (timer_elapsed32(circle_timer) > CIRCLE_ANIM_FRAME_DURATION) {
        circle_timer = timer_read32();

        update_list(c_target, tListeTotal);
        update_list(c_target2, tListeTotal2);

        if (t == _WAKINGUP) {
            render_ring_sleep();
            return;
        }

        if (t == _IDLE) {
            sleep_ring_frame_destination = ANIM_SLEEP_RING_FRAME_NUMBER - 1;
            render_ring_sleep();
            return;
        }

        if (anim_center_current_frame == ANIM_CENTER_FRAME_NUMBER) {
            draw_letter_circle(tListeTotal, tRefArc, c_target, 15, 103, 15, false);
            draw_letter_circle(tListeTotal2, tRefArc2, c_target2, 15, 103, 15, true);
        }
    }

    if (anim_center_current_frame < ANIM_CENTER_FRAME_NUMBER) {
        // center circle
        render_anim_center_circle(15, 103, 15 - 4);
    }

    render_keylog();
}

void update_circle(uint16_t keycode) {
    if (keycode == KC_ESC || keycode == KC_SPACE || keycode == KC_ENTER) {
        anim_center_timer         = timer_read32();
        anim_center_current_frame = 0;
        return;
    }
    anim_center_current_frame = ANIM_CENTER_FRAME_NUMBER;

    if (keycode >= 60) return;

    char c = code_to_name[keycode];

    c_previous                = c_last;
    c_last                    = c;
    anim_keylog_current_frame = 0;
    if (TesterEstDansListe(c, tListeTotal)) {
        c_target = c;
        return;
    }

    if (TesterEstDansListe(c, tListeTotal2)) {
        c_target2 = c;
        return;
    }
}
