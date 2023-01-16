# Boomchess

WIP Atomic chess engine written in C++

Reached depths of around 10 in under 5s.

Supports UCI

Around 1.1M (1.8M for perft) Nodes per second on i7-8750H. (Compiled with -O3)

# Features
Board:

- 0x88 representation
- Optimized with piece lists
- Incremental move make and unmake
- Incremental Zobrist hashing

Search:

- Negamax with alpha beta pruning
- Transposition table
- Iterative deepening
- Principal variation search
- Quiescence search
- Null move heuristic
- Move ordering:
    - Best move from transposition table
    - MVV-LVA
    - Killer heuristic
    - History heuristic

Evaluation:

- Material
- Piece-square tables
- Mobility
- Protected pieces (capturing would loose opponent material from explosion)
- King safety
  - penalty for pieces attacking kings radius
  - small penalty for friendly pieces touching king
  - touching kings
  
P.S. The code neeeds heavy refactoring
