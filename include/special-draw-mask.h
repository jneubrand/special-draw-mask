#pragma once
#include <pebble.h>

typedef struct {
    GBitmap * mask;
} GSpecialSessionMaskModifierData;

GSpecialSessionModifier * graphics_special_draw_create_mask_modifier();
void graphics_special_draw_mask_modifier_update(
        GSpecialSession * session,
        GSpecialSessionModifier * modifier,
        bool use_true_background);
