package main

import (
	"fmt"
	"os"

	"github.com/miniflycn/InferLLM-go"
)

func main() {
	l, err := InferLLM.New("../InferLLM-Model/chatglm-q4.bin")
	if err != nil {
		fmt.Println("Loading the model failed:", err.Error())
		os.Exit(1)
	}
	p := l.Ask("怎么提高睡眠?")
	print(p)
	for {
		p := l.GetContinue()
		print(p)
		if InferLLM.IsEnd(p) {
			break
		}
	}
	// fmt.Println(l.AskSync("你好"))
	l.Free()
}
