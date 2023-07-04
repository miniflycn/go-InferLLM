package InferLLM

// #cgo CXXFLAGS: -I./build
// #cgo LDFLAGS: -L./ -lbinding -lm -lstdc++
// #cgo darwin LDFLAGS: -framework Accelerate
// #cgo darwin CXXFLAGS: -std=c++11
// #include "binding.h"
import "C"
import (
	"fmt"
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
func (l *InferLLM) AskSync(quz string) string {
	p := C.GoString(C.ask_sync(l.state, C.CString(quz)))
	return p
}
func (l *InferLLM) Ask(quz string) string {
	p := C.GoString(C.ask(l.state, C.CString(quz)))
	return p
}
func (l *InferLLM) GetContinue() string {
	p := C.GoString(C.get_continue(l.state))
	return p
}

func IsEnd(s string) bool {
	if len(s) > 3 {
		return string(s[len(s)-3:]) == "\u200b"
	} else {
		return false
	}
}

// func main() {
// 	l, err := New("../InferLLM-Model/chatglm-q4.bin")
// 	if err != nil {
// 		fmt.Println("Loading the model failed:", err.Error())
// 		os.Exit(1)
// 	}
// 	p := l.Ask("怎么提高睡眠?")
// 	print(p)
// 	for {
// 		p := l.GetContinue()
// 		print(p)
// 		if isEnd(p) {
// 			break
// 		}
// 	}
// 	// fmt.Println(l.AskSync("你好"))
// 	l.Free()
// }
