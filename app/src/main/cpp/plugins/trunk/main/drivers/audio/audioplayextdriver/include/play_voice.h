#ifndef _PLAY_VOICE_H__
#define _PLAY_VOICE_H__

#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

typedef struct Voice_queue{
    long Vsize;
    char *Vdata;
    struct Voice_queue *next;
}v_queue;

#ifdef __cplusplus
extern "C" {
#endif /* C++ */

int device_voide_init( void );
int play_voice( char *databuffer , int Dsize);
int device_close( void );
int start_play_voice( void );
int stop_play_voice( void );

void *Device_audio_Pthread( void *arg );

#ifdef __cplusplus
} /* extern "C" */	
#endif /* C++ */

#endif // _PLAY_VOICE_H__
