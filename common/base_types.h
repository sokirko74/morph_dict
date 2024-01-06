#pragma once 

#include <cstdint>

typedef unsigned char BYTE;

enum RegisterEnum { AnyRegister = 0, LowLow = 1, UpLow = 2, UpUp = 3 };

typedef enum {
	morphUnknown = 0,
	morphRussian = 1,
	morphEnglish = 2,
	morphGerman = 3,
	morphGeneric = 4,
	morphDigits = 6,
	morphFioDisclosures = 7,
} MorphLanguageEnum;

typedef enum { DontKillHomonyms = 0, CoverageKillHomonyms = 1 } KillHomonymsEnum;

typedef enum { LocThes = 0, FinThes, CompThes, OmniThes, NoneThes } EThesType;
typedef enum { EClause = 0, EWord, EGroup, ENoneType } EUnitType;

