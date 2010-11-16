SET (MXA_HDF5_SRCS
  ${MXA_SOURCE_DIR}/MXA/HDF5/H5Lite.cpp
  ${MXA_SOURCE_DIR}/MXA/HDF5/H5Utilities.cpp
)

SET (MXA_HDF5_HDRS
  ${MXA_SOURCE_DIR}/MXA/HDF5/H5Lite.h
  ${MXA_SOURCE_DIR}/MXA/HDF5/H5Utilities.h
)
cmp_IDE_SOURCE_PROPERTIES( "MXA/HDF5" "${MXA_HDF5_HDRS}" "${MXA_HDF5_SRCS}" "0")
IF (MSVC)
    SET (MXA_HDF5_SRCS ${MXA_HDF5_SRCS} ${MXA_SOURCE_DIR}/MXA/HDF5/MXADirent.c )
    SET (MXA_HDF5_HDRS ${MXA_HDF5_HDRS} ${MXA_SOURCE_DIR}/MXA/HDF5/MXADirent.h )
endif()

if ( ${MXA_INSTALL_FILES} EQUAL 1 )
    INSTALL (FILES ${MXA_HDF5_HDRS}
            DESTINATION include/MXA/HDF5
            COMPONENT Headers   )
endif()
cmp_IDE_SOURCE_PROPERTIES( "MXA/HDF5" "${MXA_HDF5_HDRS}" "${MXA_HDF5_SRCS}" "0")

