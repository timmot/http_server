package main

import (
	"bufio"
	"fmt"
	"net"
	"strings"
)

func main() {
	ln, err := net.Listen("tcp", ":8000")

	if err != nil {
		fmt.Println("error??", err)
	}

	fmt.Println("listening on port 8000")
	for {
		conn, err := ln.Accept()
		if err != nil {
			fmt.Println("ERRRRROR?", err)
		}

		fmt.Println("new client :)))")
		go handleConnection(conn)
	}
}

func handleConnection(con net.Conn) {
	// get request from http
	// b1 := make([]byte, 1024)
	// n1, err := con.Read(b1)

	reader := bufio.NewReader(con)
	request, _, _ := reader.ReadLine()
	lines := []string{string(request)}
	for {
		line, _, _ := reader.ReadLine()
		lines = append(lines, string(line))
		if len(line) == 0 {
			break
		}
	}

	http_parts := strings.Split(string(request), " ")

	if len(http_parts) == 3 && http_parts[1] == "/" {
		content := "hello world :))"
		output := fmt.Sprintf("HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n%s",len(content), content)

		con.Write([]byte(output))
	} else {
		fmt.Println("failed?");
		for i, line := range lines {
			fmt.Printf("i:%d line:%s\n", i, line)
		}
	}

	con.Close()
}
