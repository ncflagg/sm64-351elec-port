#if !defined(_WIN32) && !defined(_WIN64)

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include <SDL2/SDL.h>

#include <ultra64.h>

#include "controller_api.h"

#ifdef TARGET_RG351

#include "../rg351.h"
#include "../configfile.h"

#endif

#define DEADZONE 4960
#ifdef TARGET_RG351
static bool init_ok;
static SDL_GameController *sdl_cntrl;

static void controller_sdl_init(void) {
    if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0) {
        fprintf(stderr, "SDL init error: %s\n", SDL_GetError());
        return;
    }

    #ifdef TARGET_ELEC
        if (SDL_GameControllerAddMappingsFromFile("/storage/roms/ports/sm64/controller/gamecontrollerdb.txt") < 0) {
            fprintf(stderr, "SDL mapping error: %s\n", SDL_GetError());
            return;
        }
    #endif

    #ifdef TARGET_ARK
        if (SDL_GameControllerAddMappingsFromFile(SDL_getenv("SDL_GAMECONTROLLERCONFIG_FILE")) < 0) {
            fprintf(stderr, "SDL mapping error: %s\n", SDL_GetError());
            return;
        }
    #endif

    init_ok = true;
}

int strequals(const char* a, const char* b){
    unsigned int size1 = strlen(a);
    if (strlen(b) != size1)
        return 0;
    for (unsigned int i = 0; i < size1; i++)
        if (tolower(a[i]) != tolower(b[i]))
            return 0;
    return 1;
}

SDL_GameControllerButton getConfiguredButton(char *configuredButton) {

    SDL_GameControllerButton returnValue;

    if(strequals(configuredButton, "A")) {
        returnValue = SDL_CONTROLLER_BUTTON_A;
    }

    if(strequals(configuredButton, "B")) {
        returnValue = SDL_CONTROLLER_BUTTON_B;
    }

    if(strequals(configuredButton, "X")) {
        returnValue = SDL_CONTROLLER_BUTTON_X;
    }

    if(strequals(configuredButton, "Y")) {
        returnValue = SDL_CONTROLLER_BUTTON_Y;
    }

    if(strequals(configuredButton, "START")) {
        returnValue = SDL_CONTROLLER_BUTTON_START;
    }

    if(strequals(configuredButton, "L1")) {
        returnValue = SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
    }

    if(strequals(configuredButton, "R1")) {
        returnValue = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
    }

    return returnValue;

}

static void controller_sdl_read(OSContPad *pad) {
    if (!init_ok) {
        return;
    }

    SDL_GameControllerUpdate();

    if (sdl_cntrl != NULL && !SDL_GameControllerGetAttached(sdl_cntrl)) {
        SDL_GameControllerClose(sdl_cntrl);
        sdl_cntrl = NULL;
    }
    if (sdl_cntrl == NULL) {
        for (int i = 0; i < SDL_NumJoysticks(); i++) {
            if (SDL_IsGameController(i)) {
                sdl_cntrl = SDL_GameControllerOpen(i);
                if (sdl_cntrl != NULL) {
                    break;
                }
            }
        }
        if (sdl_cntrl == NULL) {
            return;
        }
    }

    // Reverting RG351 Back, Start button swap for R36S
    if (SDL_GameControllerGetButton(sdl_cntrl, SDL_CONTROLLER_BUTTON_START)) pad->button |= START_BUTTON;
    if (SDL_GameControllerGetButton(sdl_cntrl, getConfiguredButton(buttonZ))) pad->button |= Z_TRIG;
    if (SDL_GameControllerGetButton(sdl_cntrl, getConfiguredButton(buttonR))) pad->button |= R_TRIG;
    if (SDL_GameControllerGetButton(sdl_cntrl, getConfiguredButton(buttonA))) pad->button |= A_BUTTON;
    if (SDL_GameControllerGetButton(sdl_cntrl, getConfiguredButton(buttonB))) pad->button |= B_BUTTON;

    #ifdef TARGET_RG351

    // Exit game by pressing SELECT + START
    if (SDL_GameControllerGetButton(sdl_cntrl, SDL_CONTROLLER_BUTTON_BACK) && SDL_GameControllerGetButton(sdl_cntrl, SDL_CONTROLLER_BUTTON_START)) {
        exitGame = 1;
    }    

    #endif

    int16_t leftx = SDL_GameControllerGetAxis(sdl_cntrl, SDL_CONTROLLER_AXIS_LEFTX);
    int16_t lefty = SDL_GameControllerGetAxis(sdl_cntrl, SDL_CONTROLLER_AXIS_LEFTY);
    int16_t rightx = SDL_GameControllerGetAxis(sdl_cntrl, SDL_CONTROLLER_AXIS_RIGHTX);
    int16_t righty = SDL_GameControllerGetAxis(sdl_cntrl, SDL_CONTROLLER_AXIS_RIGHTY);

    int16_t ltrig = SDL_GameControllerGetAxis(sdl_cntrl, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
    int16_t rtrig = SDL_GameControllerGetAxis(sdl_cntrl, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

#ifdef TARGET_WEB
    // Firefox has a bug: https://bugzilla.mozilla.org/show_bug.cgi?id=1606562
    // It sets down y to 32768.0f / 32767.0f, which is greater than the allowed 1.0f,
    // which SDL then converts to a int16_t by multiplying by 32767.0f, which overflows into -32768.
    // Maximum up will hence never become -32768 with the current version of SDL2,
    // so this workaround should be safe in compliant browsers.
    if (lefty == -32768) {
        lefty = 32767;
    }
    if (righty == -32768) {
        righty = 32767;
    }
#endif

    if (rightx < -0x4000) pad->button |= L_CBUTTONS;
    if (rightx > 0x4000) pad->button |= R_CBUTTONS;
    if (righty < -0x4000) pad->button |= U_CBUTTONS;
    if (righty > 0x4000) pad->button |= D_CBUTTONS;

    if (ltrig > 30 * 256) pad->button |= Z_TRIG;
    if (rtrig > 30 * 256) pad->button |= R_TRIG;

    uint32_t magnitude_sq = (uint32_t)(leftx * leftx) + (uint32_t)(lefty * lefty);
    if (magnitude_sq > (uint32_t)(DEADZONE * DEADZONE)) {
        pad->stick_x = leftx / 0x100;
        int stick_y = -lefty / 0x100;
        pad->stick_y = stick_y == 128 ? 127 : stick_y;
    }
}

#else /* TARGET_RG351 */

static bool init_ok;
static SDL_Joystick *sdl_joy;

static void controller_sdl_init(void) {
    if (SDL_Init(SDL_INIT_JOYSTICK) != 0) {
        fprintf(stderr, "SDL init error: %s\n", SDL_GetError());
        return;
    }

    init_ok = true;
}

static void controller_sdl_read(OSContPad *pad) {
    if (!init_ok) {
        return;
    }

    SDL_JoystickUpdate();

    if (sdl_joy != NULL && !SDL_JoystickGetAttached(sdl_joy)) {
        SDL_JoystickClose(sdl_joy);
        sdl_joy = NULL;
    }
    if (sdl_joy == NULL) {
        for (int i = 0; i < SDL_NumJoysticks(); i++) {
            if (!SDL_IsGameController(i)) {
                sdl_joy = SDL_JoystickOpen(i);
                if (sdl_joy != NULL) {
                    break;
                }
            }
        }
        if (sdl_joy == NULL) {
            return;
        }
    }

    int16_t plop = SDL_JoystickNumButtons(sdl_joy);

    if (SDL_JoystickGetButton(sdl_joy, SDL_CONTROLLER_BUTTON_START)) pad->button |= START_BUTTON;
    if (SDL_JoystickGetButton(sdl_joy, SDL_CONTROLLER_BUTTON_LEFTSHOULDER)) pad->button |= Z_TRIG;
    if (SDL_JoystickGetButton(sdl_joy, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) pad->button |= R_TRIG;
    if (SDL_JoystickGetButton(sdl_joy, SDL_CONTROLLER_BUTTON_A)) pad->button |= A_BUTTON;
    if (SDL_JoystickGetButton(sdl_joy, SDL_CONTROLLER_BUTTON_X)) pad->button |= B_BUTTON;

    int16_t leftx = SDL_JoystickGetAxis(sdl_joy, 0);
    int16_t lefty = SDL_JoystickGetAxis(sdl_joy, 1);
    int16_t rightx = SDL_JoystickGetAxis(sdl_joy, 2);
    int16_t righty = SDL_JoystickGetAxis(sdl_joy, 3);
    
    if (rightx < -0x4000) pad->button |= L_CBUTTONS;
    if (rightx > 0x4000) pad->button |= R_CBUTTONS;
    if (righty < -0x4000) pad->button |= U_CBUTTONS;
    if (righty > 0x4000) pad->button |= D_CBUTTONS;

    uint32_t magnitude_sq = (uint32_t)(leftx * leftx) + (uint32_t)(lefty * lefty);
    if (magnitude_sq > (uint32_t)(DEADZONE * DEADZONE)) {
        pad->stick_x = leftx / 0x100;
        int stick_y = -lefty / 0x100;
        pad->stick_y = stick_y == 128 ? 127 : stick_y;
    }
}
#endif

struct ControllerAPI controller_sdl = {
    controller_sdl_init,
    controller_sdl_read
};

#endif
