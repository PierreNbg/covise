ADD_SUBDIRECTORY(ReadCellTree)
ADD_SUBDIRECTORY(ReadZPR)
ADD_SUBDIRECTORY(ReadAstroPh1)
ADD_SUBDIRECTORY(ReadAccretion)
ADD_SUBDIRECTORY(ReadMeteoCosmo)
ADD_SUBDIRECTORY(ReadVois)
ADD_SUBDIRECTORY(ReadFlash)

if (COVISE_USE_VIRVO)
   ADD_SUBDIRECTORY(ReadAstroOld)
   ADD_SUBDIRECTORY(ReadMeteo)
endif()
