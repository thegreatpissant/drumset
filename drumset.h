/* Drumset include file *
 */
#ifndef __DRUMSET_H__
#define __DRUMSET_H__


typedef struct sdrum_event {
	int lbutton;
	int button;
} sdrum_event_t;
typedef struct sound_s {
	Uint8 *samples;
	Uint32 length;
} sound_t, *sound_p;

typedef struct playing_s {
	int active;
	sound_p sound;
	Uint32 position;
} playing_t, *playing_p;


#endif
