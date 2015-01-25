/* Drumset for rockband integration to a sound system
 * TODO:
 * 	Add an interface to choose what sounds you would like to load
 * 	Add an interface to choose that the drum set you want is selected.
 * 	Add diferent types of trigers - loops start stop sounds on multiple hits.
 * 	Add a midi interface for something other than wav file playing.
 * 	Add a recording interface for wav or midi files.
 * 		- using jack.
 * 		- What else?
 *
 * 	What would adding the guitar interface to this do?
 */
#include <SDL/SDL.h>
#include <signal.h>

#include "drumset.h"

#define MAX_PADS 5
#define MAX_PLAYING_SOUNDS 10
#define VOLUME_PER_SOUND	SDL_MIX_MAXVOLUME / 2

playing_t playing[MAX_PLAYING_SOUNDS];

void AudioCallback( void *user_data, Uint8 * audio, int length);
int PlaySound(sound_p sound);
int LoadAndConvertSound(char * filename, SDL_AudioSpec * spec, sound_p sound);
void ClearPlayingSounds( void );
SDL_Joystick * set_drumset( void );



int main( int argc, char * argv[]) 
{
	SDL_Surface * screen;
	SDL_Joystick * drumset;
	SDL_Event event;
	int quit = 0;

	SDL_AudioSpec desired, obtained;

	sound_t pads[MAX_PADS];
	/* Start SDL */
	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0 ) {
		fprintf( stdout, "Faild to initialize SDL: %s\n", SDL_GetError() );
		exit(1);
	}
	atexit(SDL_Quit);
	atexit(SDL_CloseAudio);

	/* Load the screen */
	screen = SDL_SetVideoMode(640,480,24,SDL_RESIZABLE);
	if( screen == NULL ) {
		fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError() );
		exit(1);
	}

	/* Load drumset controller */
	drumset = SDL_JoystickOpen(0);

	desired.freq = 44100;
	desired.format = AUDIO_S16;
	desired.samples = 4096;
	desired.channels = 2;
	desired.callback = AudioCallback;
	desired.userdata = NULL;
	if( SDL_OpenAudio(&desired, &obtained) < 0) {
		fprintf(stderr, "Unable to open audio device: %s\n", SDL_GetError() );
		exit(1);
	}

	/* load the drumset sounds */
	if( LoadAndConvertSound("./sounds/top.wav", &obtained, &pads[0]) != 0) {
		fprintf(stderr," unable to load sound. \n");
		exit(1);
	}
	if( LoadAndConvertSound("./sounds/left.wav", &obtained, &pads[1]) != 0) {
		fprintf(stderr," unable to load sound. \n");
		exit(1);
	}


	if( LoadAndConvertSound("./sounds/laser.wav", &obtained, &pads[2]) != 0) {
		fprintf(stderr," unable to load sound. \n");
		exit(1);
	}


	if( LoadAndConvertSound("./sounds/ups.wav", &obtained, &pads[3]) != 0) {
		fprintf(stderr," unable to load sound. \n");
		exit(1);
	}


	if( LoadAndConvertSound("./sounds/cow.wav", &obtained, &pads[4]) != 0) {
		fprintf(stderr," unable to load sound. \n");
		exit(1);
	}
	
	ClearPlayingSounds();

	SDL_PauseAudio(0);


	SDL_keysym keysym;
	sdrum_event_t drum_event;
	while(!quit) {
		if( SDL_PollEvent(&event) ) {
			switch(event.type) {
				case SDL_KEYDOWN:
					keysym = event.key.keysym;
					if(keysym.sym == SDLK_q) {
						fprintf(stdout, "Exiting.\n");
						quit = 1;
					}
					break; 
					case SDL_JOYBUTTONDOWN: 
						drum_event.lbutton = drum_event.button; 
						drum_event.button = (int)event.jbutton.button; 
						if(event.jbutton.button < MAX_PADS)
							 PlaySound(&pads[event.jbutton.button]);
						fprintf(stdout, "Button#%d pushed\n", (int)event.jbutton.button); 
						break; 
					case SDL_QUIT: 
						fprintf(stdout, "Exiting.\n");
						quit = 1;
					break;
			}
		}
	}
				
	SDL_PauseAudio(1);
	SDL_LockAudio();

	int i = 0;
	for(i = 0; i < MAX_PADS; i++) {
		free(pads[i].samples);
	}

	SDL_UnlockAudio();

	exit(0);
}
int PlaySound( sound_p sound)
{
	int i = 0;
	for(i = 0; i < MAX_PLAYING_SOUNDS; i++) {
		if(playing[i].active == 0) {
			break;
		}
	}

	if( i == MAX_PLAYING_SOUNDS )
		return 1;
	
	/* obtain a lock for the structures that are accessed by the threaded
	 * Audio_Callback
	 */
	SDL_LockAudio();
	playing[i].active = 1;
	playing[i].sound = sound;
	playing[i].position = 0;
	SDL_UnlockAudio();

	return 0;
}


void ClearPlayingSounds( void )
{
	int i = 0;
	for(i = 0; i < MAX_PLAYING_SOUNDS; i++) {
		playing[i].active =0;
	}
}

int LoadAndConvertSound(char * filename, SDL_AudioSpec * spec, sound_p sound)
{
	SDL_AudioCVT cvt;	/* Format conversion struct */
	SDL_AudioSpec loaded;	/* Format of the loaded data */
	Uint8 *new_buf;

	/* load the WAV file in its original sample format. */
	if(SDL_LoadWAV(filename, &loaded, &sound->samples, &sound->length) == NULL) {
		fprintf(stderr, "unable to load sound: %s\n", SDL_GetError() );
		return 1;
	}

	/* Build the conversion structure for converting the samples.
	 * this structure containg the data SDL needs to quickly
	 * convert between sample formats. */
	if( SDL_BuildAudioCVT( &cvt, loaded.format, loaded.channels, loaded.freq,
				spec->format, spec->channels, spec->freq) < 0 ) {
		fprintf(stderr, "Unable to convert sound: %s\n", SDL_GetError() );
		return 1;
	}
	/* since converting PCM samples can result in more data (for instance,
	 * converting 8-bit mono to 16-bit stereo), we need to allocate a new
	 * buffer for the converted data. Fortunately SDL_BuildAudioCVT supplied
	 * the necessary information.
	 */
	cvt.len = sound->length;
	new_buf = (Uint8 *) malloc(cvt.len * cvt.len_mult);
	if(new_buf == NULL) {
		fprintf(stderr, "Memory allocation failed.\n");
		SDL_FreeWAV(sound->samples);
		return 1;
	}

	/* Copy the sound samples into the new buffer */
	memcpy(new_buf, sound->samples, sound->length);

	/* perform the conversion on the new buffer. */
	cvt.buf = new_buf;
	if(SDL_ConvertAudio(&cvt) < 0 ) {
		fprintf(stderr, "Audio conversion error: %s\n", SDL_GetError());
		free(new_buf);
		SDL_FreeWAV(sound->samples);
		return 1;
	}

	/* swap the converted data for the original. */
	SDL_FreeWAV(sound->samples);
	sound->samples = new_buf;
	sound->length = sound->length * cvt.len_mult;

	fprintf(stdout, "\"%s\" was loaded and converted successfully.\n", filename);
	return 0;
}


void AudioCallback( void *user_data, Uint8 * audio, int length)
{
	int i;

	memset(audio, 0, length);

	for(i = 0; i < MAX_PLAYING_SOUNDS; i++) {
		if( playing[i].active) {
			Uint8 *sound_buf;
			Uint32 sound_len;

			sound_buf = playing[i].sound->samples;
			sound_buf += playing[i].position;

			if( (playing[i].position + length) > playing[i].sound->length) {
				sound_len = playing[i].sound->length - playing[i].position;
			} else {
				sound_len = length;
			}

			SDL_MixAudio(audio, sound_buf, sound_len, VOLUME_PER_SOUND);
			playing[i].position += length;
			if( playing[i].position >= playing [i].sound->length) {
				playing[i].active = 0;
			}
		}
	}
}

SDL_Joystick * set_drumset( void )
{
	int i = 0;
	fprintf(stdout, "%i joysticks were found.\n", SDL_NumJoysticks() );
	fprintf(stdout, "They are named:\n");
	for(i = 0; i < SDL_NumJoysticks(); i++ ) {
		fprintf(stdout ,"#%d: \t%s\n", i, SDL_JoystickName(i) );
	}
	
	return SDL_JoystickOpen(0);
}	
