mmap_file_op: file_op.o mmap_file.o mmap_file_op.o mmap_file_op_test.cpp
	
mmap_file_op.o :file_op.o mmap_file.o mmap_file_op.cpp mmap_file_op.h

file_op_test :file_op.o file_op_test.cpp

file_op.o :file_op.cpp file_op.h

mmap_file_test : mmap_file.o mmap_file_test.cpp

mmap_file.o :mmap_file.cpp mmap_file.h



clean:
	rm mmap_file_test mmap_file.o file_op_test file_op.o


