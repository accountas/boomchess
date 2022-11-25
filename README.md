# Boomchess

WIP Atomic chess engine

Quickly reaches depth 8, able to reach depth 10 in under few minutes
Around 1.1M Nodes per second on i7-8750H. (Compiled with -O3)

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
  - Null move heuristic
  - Move ordering:
    - Best move from transposition table
    - MVV-LVA
    - Killer heuristic
    
Evaluation:
  - Material
  - Piece-square tables
