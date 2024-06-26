IF (CUDA_FOUND)
	PROJECT(nvtt LANGUAGES CXX CUDA)
ELSE (CUDA_FOUND)
	PROJECT(nvtt LANGUAGES CXX)
ENDIF (CUDA_FOUND)

ADD_SUBDIRECTORY(squish)

SET(NVTT_SRCS
    nvtt.h nvtt.cpp
    nvtt_wrapper.h nvtt_wrapper.cpp
    ClusterFit.h ClusterFit.cpp
    Compressor.h
    BlockCompressor.h BlockCompressor.cpp
    CompressorDX9.h CompressorDX9.cpp
    CompressorDX10.h CompressorDX10.cpp
    CompressorDX11.h CompressorDX11.cpp
    CompressorDXT1.h CompressorDXT1.cpp
    CompressorDXT5_RGBM.h CompressorDXT5_RGBM.cpp
    CompressorETC.h CompressorETC.cpp
    CompressorRGB.h CompressorRGB.cpp
    Context.h Context.cpp
    QuickCompressDXT.h QuickCompressDXT.cpp
    OptimalCompressDXT.h OptimalCompressDXT.cpp
    SingleColorLookup.h SingleColorLookup.cpp
    CompressionOptions.h CompressionOptions.cpp
    InputOptions.h InputOptions.cpp
    OutputOptions.h OutputOptions.cpp
    TaskDispatcher.h #TaskDispatcher.cpp
    Surface.h Surface.cpp
    CubeSurface.h CubeSurface.cpp
    cuda/CudaUtils.h cuda/CudaUtils.cpp
    cuda/CudaMath.h
    cuda/BitmapTable.h
    cuda/CudaCompressorDXT.h cuda/CudaCompressorDXT.cpp)

IF (CUDA_FOUND)
    ADD_DEFINITIONS(-DHAVE_CUDA)
    SET(CUDA_SRCS cuda/CompressKernel.cu)
    SET(NVTT_SRCS ${NVTT_SRCS} ${CUDA_SRCS})
    SET(LIBS ${LIBS} ${CUDA_LIBRARIES})
    INCLUDE_DIRECTORIES(${CUDA_INCLUDE_DIRS})
ENDIF (CUDA_FOUND)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${NV_SOURCE_DIR}/extern/rg_etc1_v104)

ADD_DEFINITIONS(-DNVTT_EXPORTS)

IF(NVTT_SHARED)	
    ADD_LIBRARY(nvtt SHARED ${NVTT_SRCS})
ELSE(NVTT_SHARED)
    ADD_LIBRARY(nvtt ${NVTT_SRCS})
ENDIF(NVTT_SHARED)

TARGET_LINK_LIBRARIES(nvtt ${LIBS} nvcore nvimage nvthread squish bc6h bc7 nvmath rg_etc1)
ADD_LIBRARY(Nvtt::Nvtt ALIAS nvtt)

INSTALL(TARGETS nvtt 
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static)

INSTALL(FILES nvtt.h DESTINATION include/nvtt)

IF(NVTT_TOOLS)
    ADD_SUBDIRECTORY(tools)
ENDIF()
IF(NVTT_TESTING)
    ADD_SUBDIRECTORY(tests)
ENDIF()
