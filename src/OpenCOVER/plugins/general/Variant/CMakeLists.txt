#OpenCOVER plugins general Variant
using(VRML)
SET(HEADERS
  coVRBoxOfInterest.h
  Variant.h
  VariantPlugin.h
  VariantUI.h
  VrmlNodeVariant.h
)

SET(SOURCES
  coVRBoxOfInterest.cpp
  Variant.cpp
  VariantPlugin.cpp
  VariantUI.cpp
  VrmlNodeVariant.cpp
)

cover_add_plugin(Variant)

if(COVISE_USE_QT)
    qt_use_modules(Variant Xml)
endif()
