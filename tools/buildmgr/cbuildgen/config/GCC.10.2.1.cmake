# Version: 0.10.1
# Date: 2021-04-08
# This file maps the CMSIS project options to toolchain settings.
#
#   - Applies to toolchain: GNU Arm Embedded Toolchain 10-2020-q4-major 10.2.1

############### EDIT BELOW ###############
# Set base directory of toolchain
set(TOOLCHAIN_ROOT)
set(PREFIX arm-none-eabi-)
set(EXT)

############ DO NOT EDIT BELOW ###########

set(AS ${TOOLCHAIN_ROOT}/${PREFIX}as${EXT})
set(CC ${TOOLCHAIN_ROOT}/${PREFIX}gcc${EXT})
set(CXX ${TOOLCHAIN_ROOT}/${PREFIX}g++${EXT})
set(OC ${TOOLCHAIN_ROOT}/${PREFIX}objcopy${EXT})

# Assembler

if(CPU STREQUAL "Cortex-M0")
  set(GNUASM_CPU "-mcpu=cortex-m0")
elseif(CPU STREQUAL "Cortex-M0+")
  set(GNUASM_CPU "-mcpu=cortex-m0plus")
elseif(CPU STREQUAL "Cortex-M1")
  set(GNUASM_CPU "-mcpu=cortex-m1")
elseif(CPU STREQUAL "Cortex-M3")
  set(GNUASM_CPU "-mcpu=cortex-m3")
elseif(CPU STREQUAL "Cortex-M4")
  if(FPU STREQUAL "SP_FPU")
    set(GNUASM_CPU "-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard")
  else()
    set(GNUASM_CPU "-mcpu=cortex-m4")
  endif()
elseif(CPU STREQUAL "Cortex-M7")
  if(FPU STREQUAL "DP_FPU")
    set(GNUASM_CPU "-mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard")
  elseif(FPU STREQUAL "SP_FPU")
    set(GNUASM_CPU "-mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -mfloat-abi=hard")
  else()
    set(GNUASM_CPU "-mcpu=cortex-m7")
  endif()
elseif(CPU STREQUAL "Cortex-M23")
  set(GNUASM_CPU "-mcpu=cortex-m23")
elseif(CPU STREQUAL "Cortex-M33")
  if(FPU STREQUAL "SP_FPU")
    set(GNUASM_CPU "-mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard")
  elseif(DSP STREQUAL "DSP")
    set(GNUASM_CPU "-mcpu=cortex-m33")
  else()
    set(GNUASM_CPU "-mcpu=cortex-m33+nodsp")
  endif()
elseif(CPU STREQUAL "Cortex-M35P")
  if(FPU STREQUAL "SP_FPU")
    if(DSP STREQUAL "DSP")
      set(GNUASM_CPU "-mcpu=cortex-m35p -mfpu=fpv5-sp-d16 -mfloat-abi=hard")
    else()
      set(GNUASM_CPU "-mcpu=cortex-m35p+nodsp -mfpu=fpv5-sp-d16 -mfloat-abi=hard")
    endif()
  else()
    if(DSP STREQUAL "DSP")
      set(GNUASM_CPU "-mcpu=cortex-m35p")
    else()
      set(GNUASM_CPU "-mcpu=cortex-m35p+nodsp")
    endif()
  endif()
elseif(CPU STREQUAL "Cortex-M55")
  if(FPU STREQUAL "NO_FPU")
    if(MVE STREQUAL "NO_MVE")
      set(GNUASM_CPU "-mcpu=cortex-m55+nofp+nomve")
    else()
      set(GNUASM_CPU "-mcpu=cortex-m55+nofp")
    endif()
  else()
    if(MVE STREQUAL "NO_MVE")
      set(GNUASM_CPU "-mcpu=cortex-m55+nomve -mfloat-abi=hard")
    elseif(MVE STREQUAL "MVE")
      set(GNUASM_CPU "-mcpu=cortex-m55+nomve.fp -mfloat-abi=hard")
    else()
      set(GNUASM_CPU "-mcpu=cortex-m55 -mfloat-abi=hard")
    endif()
  endif()
endif()
if(NOT DEFINED GNUASM_CPU)
  message(FATAL_ERROR "Error: CPU is not supported!")
endif()

# Supported Assembly Variants:
#   AS_LEG: gas
#   AS_GNU: gcc + GNU syntax
#   ASM: gcc + pre-processing

set(AS_LEG_CPU ${GNUASM_CPU})
set(AS_GNU_CPU ${GNUASM_CPU})
set(ASM_CPU ${GNUASM_CPU})

set(AS_LEG_FLAGS)
set(AS_GNU_FLAGS "-c")
set(ASM_FLAGS "-c")

foreach(ENTRY ${DEFINES})
  string(REGEX REPLACE "=.*" "" KEY ${ENTRY})
  if (KEY STREQUAL ENTRY)
    set(VALUE "1")
  else()
    string(REGEX REPLACE ".*=" "" VALUE ${ENTRY})
  endif()
  string(APPEND AS_LEG_DEFINES "--defsym ${KEY}=${VALUE} ")
  string(APPEND AS_GNU_DEFINES "-Wa,-defsym,\"${KEY}=${VALUE}\" ")
  string(APPEND ASM_DEFINES "-D${ENTRY} ")
endforeach()

if(BYTE_ORDER STREQUAL "Little-endian")
  set(ASM_BYTE_ORDER "-mlittle-endian")
elseif(BYTE_ORDER STREQUAL "Big-endian")
  set(ASM_BYTE_ORDER "-mbig-endian")
endif()
set(AS_LEG_BYTE_ORDER "${AS_BYTE_ORDER}")
set(AS_GNU_BYTE_ORDER "${AS_BYTE_ORDER}")

# C Compiler

set(CC_CPU "${GNUASM_CPU}")
set(CC_FLAGS)
set(CC_BYTE_ORDER ${ASM_BYTE_ORDER})
set(_PI "-include ")
set(_ISYS "-isystem ")

foreach(ENTRY ${DEFINES})
  string(APPEND CC_DEFINES "-D${ENTRY} ")
endforeach()

if(SECURE STREQUAL "Secure")
  set(CC_SECURE "-mcmse")
else()
  set(CC_SECURE "")
endif()

set (CC_SYS_INC_PATHS_LIST
  "${TOOLCHAIN_ROOT}/../lib/gcc/arm-none-eabi/10.2.1/include"
  "${TOOLCHAIN_ROOT}/../lib/gcc/arm-none-eabi/10.2.1/include-fixed"
  "${TOOLCHAIN_ROOT}/../arm-none-eabi/include"
)
foreach(ENTRY ${CC_SYS_INC_PATHS_LIST})
  string(APPEND CC_SYS_INC_PATHS "${_ISYS}\"${ENTRY}\" ")
endforeach()

# C++ Compiler

set(CXX_CPU "${CC_CPU}")
set(CXX_DEFINES "${CC_DEFINES}")
set(CXX_BYTE_ORDER "${CC_BYTE_ORDER}")
set(CXX_SECURE "${CC_SECURE}")
set(CXX_FLAGS "${CC_FLAGS}")

set (CXX_SYS_INC_PATHS_LIST
  "${TOOLCHAIN_ROOT}/../arm-none-eabi/include/c++/10.2.1"
  "${TOOLCHAIN_ROOT}/../arm-none-eabi/include/c++/10.2.1/arm-none-eabi"
  "${TOOLCHAIN_ROOT}/../arm-none-eabi/include/c++/10.2.1/backward"
  "${CC_SYS_INC_PATHS_LIST}"
)
foreach(ENTRY ${CXX_SYS_INC_PATHS_LIST})
  string(APPEND CXX_SYS_INC_PATHS "${_ISYS}\"${ENTRY}\" ")
endforeach()

# Linker

set(LD_CPU ${GNUASM_CPU})
if(LD_SCRIPT)
  set(LD_SCRIPT "-T \"${LD_SCRIPT}\"")
endif()

if(SECURE STREQUAL "Secure")
  set(LD_SECURE "-Wl,--cmse-implib -Wl,--out-implib=\"${OUT_DIR}/${TARGET}_CMSE_Lib.o\"")
endif()

set(LD_FLAGS "")

# Target Output

set (LIB_PREFIX "lib")
set (LIB_SUFFIX ".a")
set (EXE_SUFFIX ".elf")

# ELF to HEX conversion
set (ELF2HEX -O ihex "${OUT_DIR}/${TARGET}${EXE_SUFFIX}" "${OUT_DIR}/${TARGET}.hex")

# ELF to BIN conversion
set (ELF2BIN -O binary "${OUT_DIR}/${TARGET}${EXE_SUFFIX}" "${OUT_DIR}/${TARGET}.bin")

# Set CMake variables for toolchain initialization
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_ASM_COMPILER "${CC}")
set(CMAKE_AS_LEG_COMPILER "${AS}")
set(CMAKE_AS_GNU_COMPILER "${CC}")
set(CMAKE_C_COMPILER "${CC}")
set(CMAKE_CXX_COMPILER "${CXX}")
set(CMAKE_OBJCOPY "${OC}")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMakeASM")

# Set CMake variables for skipping compiler identification
set(CMAKE_ASM_COMPILER_FORCED TRUE)
set(CMAKE_C_COMPILER_ID "GNU")
set(CMAKE_C_COMPILER_ID_RUN TRUE)
set(CMAKE_C_COMPILER_VERSION "10.2.1")
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_ID "${CMAKE_C_COMPILER_ID}")
set(CMAKE_CXX_COMPILER_ID_RUN "${CMAKE_C_COMPILER_ID_RUN}")
set(CMAKE_CXX_COMPILER_VERSION "${CMAKE_C_COMPILER_VERSION}")
set(CMAKE_CXX_COMPILER_FORCED "${CMAKE_C_COMPILER_FORCED}")
set(CMAKE_CXX_COMPILER_WORKS "${CMAKE_C_COMPILER_WORKS}")
