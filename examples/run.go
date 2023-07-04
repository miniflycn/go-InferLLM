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
	// 问一个问题，并获取回答
	p := l.Ask("怎么提高睡眠?")
	print(p)
	for {
		// 如果回答没有结束继续获取
		p := l.GetContinue()
		print(p)
		// 回答结束了，退出循环
		if InferLLM.IsEnd(p) {
			break
		}
	}
	// fmt.Println(l.AskSync("你好"))
	l.Free()
}
