#ifndef HEADER_DDA54DE9045949D18DB52427B057B311 // -*- mode:c++ -*-
#define HEADER_DDA54DE9045949D18DB52427B057B311

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <string>
#include <array>

#include <beeb/roms.h>

#include <shared/enum_decl.h>
#include "roms.inl"
#include <shared/enum_end.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct BeebROM {
    std::string path;
    std::string name; //shown in UI
    StandardROM rom;

    std::string GetAssetPath() const;
};

//static const size_t ROM_SIZE = 16384;

// Ugh... maddening inconsistent capitalization!
//typedef std::array<uint8_t, ROM_SIZE> BeebRomData;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

extern const BeebROM BEEB_ROM_OS12;
extern const BeebROM BEEB_ROM_BPLUS_MOS;

extern const BeebROM BEEB_ROM_BASIC2;
extern const BeebROM BEEB_ROM_ACORN_DFS;
extern const BeebROM BEEB_ROM_WATFORD_DDFS_DDB2;
extern const BeebROM BEEB_ROM_WATFORD_DDFS_DDB3;
extern const BeebROM BEEB_ROM_OPUS_DDOS;
extern const BeebROM BEEB_ROM_OPUS_CHALLENGER;

extern const BeebROM BEEB_ROM_MOS320_SIDEWAYS_ROM_9;
extern const BeebROM BEEB_ROM_MOS320_SIDEWAYS_ROM_A;
extern const BeebROM BEEB_ROM_MOS320_SIDEWAYS_ROM_B;
extern const BeebROM BEEB_ROM_MOS320_SIDEWAYS_ROM_C;
extern const BeebROM BEEB_ROM_MOS320_SIDEWAYS_ROM_D;
extern const BeebROM BEEB_ROM_MOS320_SIDEWAYS_ROM_E;
extern const BeebROM BEEB_ROM_MOS320_SIDEWAYS_ROM_F;
extern const BeebROM BEEB_ROM_MOS320_MOS_ROM;

extern const BeebROM BEEB_ROM_MOS350_SIDEWAYS_ROM_9;
extern const BeebROM BEEB_ROM_MOS350_SIDEWAYS_ROM_A;
extern const BeebROM BEEB_ROM_MOS350_SIDEWAYS_ROM_B;
extern const BeebROM BEEB_ROM_MOS350_SIDEWAYS_ROM_C;
extern const BeebROM BEEB_ROM_MOS350_SIDEWAYS_ROM_D;
extern const BeebROM BEEB_ROM_MOS350_SIDEWAYS_ROM_E;
extern const BeebROM BEEB_ROM_MOS350_SIDEWAYS_ROM_F;
extern const BeebROM BEEB_ROM_MOS350_MOS_ROM;

// all ROMs, one after the other, terminated by nullptr.
extern const BeebROM *const BEEB_ROMS[];

// Non-sideways ROMs.
extern const BeebROM BEEB_ROM_MASTER_TURBO_PARASITE;
extern const BeebROM BEEB_ROM_TUBE110;

const BeebROM *FindBeebROM(StandardROM rom);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#endif
