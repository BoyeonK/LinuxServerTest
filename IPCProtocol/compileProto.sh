echo "Generating C++ files..."

protoc -I=./ --cpp_out=./Compiled ./IPCProtocol.proto