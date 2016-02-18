/*
 * Copyright (c) 2009-2014 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __MUSIC_PLAYER_H__
#define __MUSIC_PLAYER_H__

#define MUSIC_PLAYER_NAME "[music-minicontrol.LOCKSCREEN]"
#define SOUND_PLAYER_NAME "[sound-minicontrol.LOCKSCREEN]"
#define MUSIC_PLAYER "org.tizen.music-player"
#define SOUND_PLAYER "org.tizen.sound-player"
#define LOCK_MUSIC_PLAYER_MINICONTROL_NAME_KEY "__minicontrol_name__"

typedef enum {
	MUSIC_STATE_NO_MUSIC = 0,
	MUSIC_STATE_MUSIC_PLAYER_ON = 1,
	MUSIC_STATE_SOUND_PLAYER_ON = 2,
	MUSIC_STATE_MAX,
} music_state_e;

typedef enum {
	READ_FILE_ERROR_NONE = 0,
	READ_FILE_ERROR_EOF = 1,
	READ_FILE_ERROR_ERROR = 2,
	READ_FILE_MAX,
} read_file_error_e;

music_state_e lock_music_player_state_get(void);
Evas_Object *lock_music_player_minicontroller_get(void);

void lock_music_player_bg_set(Evas_Object *bg, music_state_e music_state);
void lock_music_player_bg_unset(void);
void lock_music_player_minicontroller_hide_event_send(void);
Evas_Object *lock_music_player_minicontroller_create(music_state_e state, Evas_Object *layout, const char *name);
void lock_music_player_minicontroller_destroy(void);

#endif
