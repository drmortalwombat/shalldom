#ifndef GAMEMUSIC_H
#define GAMEMUSIC_H

void music_init(char tune);

void music_play(void);

void music_patch_voice3(bool enable);

#pragma compile("gamemusic.c")

#endif
