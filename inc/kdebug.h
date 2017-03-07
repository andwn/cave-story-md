/**
 *  \file kdebug.h
 *  \brief Gens KMod debug methods
 *  \author Kaneda
 *  \date XX/20XX
 *
 * This unit provides Gens KMod debug methods support.
 */

extern void KDebug_Halt();
extern void KDebug_Alert(const char *str);
extern void KDebug_AlertNumber(uint32_t nVal);
extern void KDebug_StartTimer();
extern void KDebug_StopTimer();
