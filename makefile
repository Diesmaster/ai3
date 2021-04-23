# the compiler: gcc for C program, define as g++ for C++
  CC = g++
  #g++ -Wall -Wextra -O2 -o chess chess2021.cc
  # compiler flags:
  #  -g     - this flag adds debugging information to the executable file
  #  -Wall  - this flag is used to turn on most compiler warnings
  CFLAGS  =  -Wall -O2 #-Wextra 

  # The build target
  TARGET = chess2021

  all: $(TARGET)

  $(TARGET): $(TARGET).cc
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cc

  clean:
	$(RM) $(TARGET)
