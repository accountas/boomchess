cmake_minimum_required(VERSION 3.10)
project(BoomChess)

set(CMAKE_CXX_STANDARD 17)


set(COMMON_SOURCES src/main.cpp src/Driver.cpp src/Driver.h src/Board.cpp src/Board.h src/Piece.cpp src/Piece.h src/Common.h src/Move.h src/MoveGenerator.cpp src/MoveGenerator.h src/FenParsing.cpp src/Search.cpp src/Search.h src/Evaluator.cpp src/Evaluator.h src/Timer.h src/ZobristKey.cpp src/ZobristKey.h src/TranspositionTable.cpp src/TranspositionTable.h src/Metrics.h src/Perft.cpp src/UCI.cpp src/UCI.h src/Experiments.cpp)
set(ALL_SEARCH USE_MVV_LVA USE_KILLER USE_HISTORY USE_TT USE_ID USE_PVS USE_NULL_MOVE)

#add_executable(F_baseline ${COMMON_SOURCES})
#target_compile_definitions(F_baseline PUBLIC NO_AB)

#add_executable(S_A ${COMMON_SOURCES})
#target_compile_definitions(S_A PUBLIC USE_MVV_LVA)

#add_executable(S_B ${COMMON_SOURCES})
#target_compile_definitions(S_B PUBLIC USE_MVV_LVA USE_KILLER)

#add_executable(S_C ${COMMON_SOURCES})
#target_compile_definitions(S_C PUBLIC USE_MVV_LVA USE_KILLER USE_HISTORY)
#
#add_executable(S_D ${COMMON_SOURCES})
#target_compile_definitions(S_D PUBLIC USE_MVV_LVA USE_KILLER USE_HISTORY USE_TT)
#
#add_executable(S_E ${COMMON_SOURCES})
#target_compile_definitions(S_E PUBLIC USE_MVV_LVA USE_KILLER USE_HISTORY USE_TT USE_ID)
#
#add_executable(S_F ${COMMON_SOURCES})
#target_compile_definitions(S_F PUBLIC USE_MVV_LVA USE_KILLER USE_HISTORY USE_TT USE_ID USE_PVS)
#
#add_executable(S_G ${COMMON_SOURCES})
#target_compile_definitions(S_G PUBLIC USE_MVV_LVA USE_KILLER USE_HISTORY USE_TT USE_ID USE_PVS USE_NULL_MOVE)
#
add_executable(F_qs ${COMMON_SOURCES})
target_compile_definitions(F_qs PUBLIC USE_QSEARCH ${ALL_SEARCH})

#add_executable(F_qs_pws ${COMMON_SOURCES})
#target_compile_definitions(F_qs_pws PUBLIC USE_QSEARCH PAPER_WEIGHTS)
#
#add_executable(F_qs_mws ${COMMON_SOURCES})
#target_compile_definitions(F_qs_mws PUBLIC USE_QSEARCH MIX_WEIGHTS ${ALL_SEARCH})
#
#add_executable(F_qs_sws ${COMMON_SOURCES})
#target_compile_definitions(F_qs_sws PUBLIC USE_QSEARCH STANDARD_WEIGHTS ${ALL_SEARCH})

add_executable(F_qs_mob ${COMMON_SOURCES})
target_compile_definitions(F_qs_mob PUBLIC USE_QSEARCH USE_MOBILITY ${ALL_SEARCH})

add_executable(F_qs_mob_ks ${COMMON_SOURCES})
target_compile_definitions(F_qs_mob_ks PUBLIC USE_QSEARCH USE_MOBILITY USE_KING_TOUCH_PENALTY USE_KING_ATTACK_PENALTY ${ALL_SEARCH})

add_executable(F_qs_mob_ks_pp ${COMMON_SOURCES})
target_compile_definitions(F_qs_mob_ks_pp PUBLIC USE_QSEARCH USE_PASSED_PAWNS USE_KING_ATTACK_PENALTY USE_KING_TOUCH_PENALTY USE_MOBILITY ${ALL_SEARCH})

#add_executable(F_qs_mob_ks_pp_kd ${COMMON_SOURCES})
#target_compile_definitions(F_qs_mob_ks_pp_kd PUBLIC USE_QSEARCH USE_PASSED_PAWNS USE_KING_ATTACK_PENALTY USE_KING_TOUCH_PENALTY USE_MOBILITY USE_KING_DISTANCE ${ALL_SEARCH})

add_executable(F_qs_mob_ks_pp_sf ${COMMON_SOURCES})
target_compile_definitions(F_qs_mob_ks_pp_sf PUBLIC USE_QSEARCH USE_PASSED_PAWNS USE_KING_ATTACK_PENALTY USE_KING_TOUCH_PENALTY USE_MOBILITY USE_SAFE_SQUARE ${ALL_SEARCH})

add_executable(F_qs_mob_ks_pp_sf_kd ${COMMON_SOURCES})
target_compile_definitions(F_qs_mob_ks_pp_sf_kd PUBLIC USE_QSEARCH USE_PASSED_PAWNS USE_KING_ATTACK_PENALTY USE_KING_TOUCH_PENALTY USE_MOBILITY USE_SAFE_SQUARE USE_KING_DISTANCE ${ALL_SEARCH})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -g -pthread")
else ()
    MATH(EXPR stack_size "32 * 1024 * 1024") # 32 Mb
    set(CMAKE_EXE_LINKER_FLAGS "-Wl,--stack,${stack_size}")
    set(CMAKE_CXX_FLAGS "-pthread -static-libstdc++ -static-libgcc -static -O3")
endif ()

