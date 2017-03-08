/**
 *  \file xgm.h
 *  \brief XGM sound driver
 *  \author Stephane Dallongeville
 *  \date 08/2015
 *
 * This unit provides methods to use the XGM (eXtended Genesis Music) sound driver.<br>
 * This driver takes VGM (or XGM) file as input to play music.<br>
 * It supports 4 PCM channels at a fixed 14 Khz and allows to play SFX through PCM with 16 level of priority.<br>
 * The driver is designed to avoid DMA contention when possible (depending CPU load).
 */

/**
 * \brief
 *      Returns play music state (XGM music player driver).
 */
uint8_t XGM_isPlaying();
/**
 *  \brief
 *      Start playing the specified XGM track (XGM music player driver).
 *
 *  \param song
 *      XGM track address.
 *
 *  \see XGM_stopPlay
 *  \see XGM_pausePlay
 *  \see XGM_nextFrame
 */
void XGM_startPlay(const uint8_t *song);
/**
 *  \brief
 *      Stop playing music (XGM music player driver).
 *
 *  \see XGM_pausePlay
 */
void XGM_stopPlay();
/**
 * \brief
 *      Pause playing music, music can be resumed by calling #XGM_resumePlay (XGM music player driver).
 *
 *  \see XGM_resumePlay
 *  \see XGM_stopPlay
 */
void XGM_pausePlay();
/**
 * \brief
 *      Resume playing music after pausing with XGM_pausePlay (XGM music player driver).
 *
 *  \see XGM_pausePlay
 *  \see XGM_nextFrame
 */
void XGM_resumePlay();

/**
 *  \brief
 *      Return play status of specified PCM channel (XGM music player driver).
 *
 *  \param channel_mask
 *      Channel(s) we want to retrieve play state.<br>
 *      #SOUND_PCM_CH1_MSK    = channel 1<br>
 *      #SOUND_PCM_CH2_MSK    = channel 2<br>
 *      #SOUND_PCM_CH3_MSK    = channel 3<br>
 *      #SOUND_PCM_CH4_MSK    = channel 4<br>
 *      <br>
 *      You can combine mask to retrieve state of severals channels at once:<br>
 *      <code>isPlayingPCM(SOUND_PCM_CH1_MSK | SOUND_PCM_CH2_MSK)</code><br>
 *      will actually return play state for channel 1 and channel 2.
 *
 *  \return
 *      Return non zero if specified channel(s) is(are) playing.
 */
uint8_t XGM_isPlayingPCM(const uint16_t channel_mask);
/**
 *  \brief
 *      Declare a new PCM sample (maximum = 255) for the XGM music player driver.<br>
 *      Sample id < 64 are reserved for music while others are used for SFX
 *      so if you want to declare a new SFX PCM sample use an id >= 64
 *
 *  \param id
 *      Sample id:<br>
 *      value 0 is not allowed<br>
 *      values from 1 to 63 are used for music
 *      values from 64 to 255 are used for SFX
 *  \param sample
 *      Sample address, should be 256 bytes boundary aligned<br>
 *      SGDK automatically align sample resource as needed
 *  \param len
 *      Size of sample in bytes, should be a multiple of 256<br>
 *      SGDK automatically adjust resource size as needed
 */
void XGM_setPCM(const uint8_t id, const uint8_t *sample, const uint32_t len);
/**
 *  \brief
 *      Same as #XGM_setPCM but fast version.<br>
 *      This method assume that XGM driver is loaded and that 68000 has access to Z80 bus
 *
 *  \param id
 *      Sample id:<br>
 *      value 0 is not allowed<br>
 *      values from 1 to 63 are used for music
 *      values from 64 to 255 are used for SFX
 *  \param sample
 *      Sample address, should be 256 bytes boundary aligned<br>
 *      SGDK automatically align sample resource as needed
 *  \param len
 *      Size of sample in bytes, should be a multiple of 256<br>
 *      SGDK automatically adjust resource size as needed
 */
void XGM_setPCMFast(const uint8_t id, const uint8_t *sample, const uint32_t len);
/**
 *  \brief
 *      Play a PCM sample on specified channel (XGM music player driver).<br>
 *      If a sample was currently playing on this channel then priority of the newer sample should be are compared then it's stopped and the new sample is played instead.<br>
 *      Note that music may use the first PCM channel so it's better to use channel 2 to 4 for SFX.
 *
 *  \param id
 *      Sample id (set #XGM_setPCM method)
 *  \param priority
 *      Value should go from 0 to 15 where 0 is lowest priority and 15 the highest one.<br>
 *      If the channel was already playing the priority is used to determine if the new SFX should replace the current one (new priority >= old priority).
 *  \param channel
 *      Channel where we want to play sample.<br>
 *      #SOUND_PCM_CH1    = channel 1 (usually used by music)<br>
 *      #SOUND_PCM_CH2    = channel 2<br>
 *      #SOUND_PCM_CH3    = channel 3<br>
 *      #SOUND_PCM_CH4    = channel 4<br>
 */
void XGM_startPlayPCM(const uint8_t id, const uint8_t priority, const uint16_t channel);
/**
 *  \brief
 *      Stop play PCM on specified channel (XGM music player driver).<br>
 *      No effect if no sample was currently playing on this channel.
 *
 *  \param channel
 *      Channel we want to stop.<br>
 *      #SOUND_PCM_CH1    = channel 1<br>
 *      #SOUND_PCM_CH2    = channel 2<br>
 *      #SOUND_PCM_CH3    = channel 3<br>
 *      #SOUND_PCM_CH4    = channel 4<br>
 */
void XGM_stopPlayPCM(const uint16_t channel);

/**
 *  \brief
 *      Return the elapsed play time since the last #XGM_startPlay(..) call.<br>
 *      The returned value is in music frame which can be 50/60 per second depending the base music play rate (NTSC/PAL).
 *
 *  \see XGM_startPlay(..)
 *  \see XGM_setMusicTempo()
 */
uint32_t XGM_getElapsed();

/**
 *  \brief
 *      Set the music tempo (in tick per second).<br>
 *      Default value is 60 or 50 depending the system is NTSC or PAL.
 *      This method is meaningful only if you use the automatic music sync mode (see XGM_setManualSync() method)
 *      which is the default mode.<br>
 *      Note that using specific tempo (not 60 or 50) can completely distord FM instruments sound and affect
 *      performance of DMA contention and external command parsing so it's recommended to stand with default one.
 *
 *  \see XGM_setManualSync()
 *  \see XGM_getMusicTempo()
 */
void XGM_setMusicTempo(uint16_t value);

/**
 *  \brief
 *      Notify the Z80 a new frame just happened (XGM music player driver).
 *
 *  Sound synchronization was initially 100% done by Z80 itself using the V-Interrupt but
 *  if the Z80 is stopped right at V-Int time (bus request from 68000 or DMA stall) then
 *  the V-Int can be missed by the Z80 and music timing affected.<br>
 *  To fix that issue and also to offer more flexibility the music timing should now be handled by the 68k.<br>
 *  By default this method is called automatically by SGDK at V-Int time but you can decide to handle sync
 *  manually (see XGM_setManualSync(..) method).<br>
 *  When you are in manual sync you normally should call this method once per frame (in the V-Int callback for instance)
 *  but you are free to play with it to increase or decrease music tempo.<br>
 *  Note that it's better to call this method a bit before (3/4 scanlines should be fine) doing DMA operation for best
 *  main bus contention protection (see #XGM_set68KBUSProtection() and #XGM_setForceDelayDMA() methods).
 *
 * \see XGM_setManualSync(..)
 * \see XGM_nextXFrame(..)
 * \see XGM_set68KBUSProtection(..)
 * \see XGM_setForceDelayDMA(..)
 */
#define XGM_nextFrame()  XGM_nextXFrame(1)
/**
 *  \brief
 *      Same as XGM_nextFrame() except you can specify the numer of frame.
 *
 * \see XGM_nextFrame(..)
 */
void XGM_nextXFrame(uint16_t num);

void XGM_set68KBUSProtection(uint8_t value);

void XGM_doVBlankProcess();
