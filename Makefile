GXX = g++
FLAGS = -Wall --std=c++20 -Wextra 

tls_mod: tls_mod.cpp
	$(GXX) $(FLAGS)  tls_mod.cpp  -o tls_mod


clean:
	rm tls_mod