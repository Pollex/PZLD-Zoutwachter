package main

import (
	"fmt"
	"os"

	"github.com/pollex/pzld-zoutwachter/internal/cn0359"
)

func main() {
	if err := Run(); err != nil {
		fmt.Fprintf(os.Stderr, "Error: %s\n", err.Error())
	}
}

func Run() error {
	fmt.Println("Establishing connection")
	cn, err := cn0359.New("/dev/ttyUSB0", 19200, 44)
	if err != nil {
		return err
	}
	fmt.Println("Polling...")
	res, err := cn.Poll()
	if err != nil {
		return err
	}
	fmt.Printf("res: %+v\n", res)
	return nil
}
