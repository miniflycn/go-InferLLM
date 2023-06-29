package main

// #cgo CXXFLAGS: -I./build
// #cgo LDFLAGS: -L./ -lbinding -lm -lstdc++
// #cgo darwin LDFLAGS: -framework Accelerate
// #cgo darwin CXXFLAGS: -std=c++11
// #include "binding.h"
import "C"

type InferLLM struct {
	contextSize int
}

func main() {
	modelPath := C.CString("./xxx")
	result := C.load_model(modelPath)
	println(result)
}
