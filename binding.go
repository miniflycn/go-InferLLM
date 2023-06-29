package main

// #cgo CXXFLAGS: -I./build
// #cgo LDFLAGS: -L./ -lbinding -lm -lstdc++
// #cgo darwin LDFLAGS: -framework Accelerate
// #cgo darwin CXXFLAGS: -std=c++11
// #include "binding.h"
import "C"
import (
	"fmt"
	"os"
	"unsafe"
)

type InferLLM struct {
	state unsafe.Pointer
}

func New(model string) (*InferLLM, error) {
	modelPath := C.CString(model)
	result := C.load_model(modelPath)
	if result == nil {
		return nil, fmt.Errorf("failed loading model")
	}
	ll := &InferLLM{state: result}

	return ll, nil
}

func (l *InferLLM) Free() {
	C.llm_binding_free_model(l.state)
}

func main() {
	l, err := New("../InferLLM-Model/chatglm-q4.bin")
	if err != nil {
		fmt.Println("Loading the model failed:", err.Error())
		os.Exit(1)
	}
	l.Free()
}
