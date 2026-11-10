// game/include/compat.hpp
#pragma once

#ifdef __CUDACC__
    #define HOST_DEVICE __host__ __device__
    #define FORCE_INLINE __forceinline__
#else
    #define HOST_DEVICE
    #define FORCE_INLINE inline
#endif