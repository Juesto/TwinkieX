// This file is for checking that the current build configuration is valid for the target game.
#pragma once

// _M_X64 is defined for x64
#ifdef _M_X64
#define X64
#else
#define X86
#endif

// TMCN (TrackmaniaChina) is an x64 game
#if defined(TMCN) and defined(X86)
#error "TMCN is only supported on x64 architecture."
#endif

// Define GAMEBOX for TM1, TMO, TMS, TMSX and ESWC, since those are all Gamebox games
// Technically, TMCN is also built on Gamebox, but we separate it to Maniaplanet because of its differences to the rest
#if defined(TM1) || defined(TMO) || defined(TMS) || defined(TMSX) || defined(ESWC) || defined(TMU)
// GAMEBOX is for TM1, TMO, TMS, TMSX and ESWC
#define GAMEBOX
#elif defined(TMCN)
// MANIAPLANET is for TMCN
#define MANIAPLANET
#endif

// TM1, TMO, TMS, TMSX and ESWC are x86 games
#if defined(GAMEBOX) and defined(X64)
#error "TM1, TMO, TMS, TMSX and ESWC are only supported on x86 architecture."
#endif

#if !defined(TMCN) && !defined(TM1) && !defined(TMO) && !defined(TMS) && !defined(TMSX) && !defined(ESWC) && !defined(TMU)
#error "No target game defined. Please define one of the following: TMCN, TM1, TMO, TMS, TMSX, ESWC."
#endif

#ifdef TMCN
#define TWINKIEX_GAME "TMCN"
#define TWINKIEX_GAMELOWER "tmcn"
#define TWINKIEX_GAMEFULL "TrackmaniaChina"
#elif defined(TM1)
#define TWINKIEX_GAME "TM1"
#define TWINKIEX_GAMELOWER "tm1"
#define TWINKIEX_GAMEFULL "TrackMania (2003)"
#elif defined(TMO)
#define TWINKIEX_GAME "TMO"
#define TWINKIEX_GAMELOWER "tmo"
#define TWINKIEX_GAMEFULL "TrackMania Original"
#elif defined(TMS)
#define TWINKIEX_GAME "TMS"
#define TWINKIEX_GAMELOWER "tms"
#define TWINKIEX_GAMEFULL "TrackMania Sunrise"
#elif defined(TMSX)
#define TWINKIEX_GAME "TMSX"
#define TWINKIEX_GAMELOWER "tmsx"
#define TWINKIEX_GAMEFULL "TrackMania Sunrise eXtreme"
#elif defined(ESWC)
#define TWINKIEX_GAME "ESWC"
#define TWINKIEX_GAMELOWER "eswc"
#define TWINKIEX_GAMEFULL "TrackMania Nations ESWC"
#elif defined(TMU)
#define TWINKIEX_GAME "TMU"
#define TWINKIEX_GAMELOWER "tmu"
#define TWINKIEX_GAMEFULL "Trackmania United"
#else
#error "No target game defined."
#endif

#define TWINKIEX_DLL ("Twinkie" TWINKIEX_GAME ".dll")