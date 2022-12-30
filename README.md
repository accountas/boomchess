# Boomchess

WIP Atomic chess engine written in C++

Quickly reaches depth of around 10ply in under a minute

Around 1.5M Nodes per second on i7-8750H. (Compiled with -O3)

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
- King safety
  - penalty for pieces attacking kings radius
  - small penalty for friendly pieces touching king