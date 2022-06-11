# Causal Block Diagram simulator in C
A simle block diagram simulator written in C.

### Building
For building TUP is needed. Run `tup init` in the root directory followed by a `tup` which builds the project. The executable can be found at `out/main`. Running it needs nothing special except for python with matplotlib installed for CSV exports and generating plots and graphviz with dot for block diagram generation (only with the `sim_viz(&state)` function).