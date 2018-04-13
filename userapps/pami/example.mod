
set A;
param B {A};
var x {A} >= 0 <= 1;

minimize obj: sum{a in A} B[a]*x[a];

# PICO SYMBOL: A B



