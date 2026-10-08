# Sarcina specifica a e^{-}
Quick helper for physics lab also playground to test some ranges

Ref: https://www.youtube.com/watch?v=QoaVRQvA6hI

# Compile
Do
clang++ -std=c++23 exp_electron.cpp -o exp_electron -lfmt

# Test
Good values:
./exp_electron < good_values.txt

Bad values:
./exp_electron < bad_values.txt
