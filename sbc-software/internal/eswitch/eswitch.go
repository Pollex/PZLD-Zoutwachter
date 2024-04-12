package eswitch

import (
	"fmt"
	"io"
	"time"

	"github.com/tarm/serial"
)

type Eswitch struct {
	conn io.ReadWriteCloser
}

func NewFromStream(conn io.ReadWriteCloser) *Eswitch {
	return &Eswitch{
		conn: conn,
	}
}

func New(portStr string, baud int) (*Eswitch, error) {
	c := &serial.Config{Name: portStr, Baud: baud, ReadTimeout: 1 * time.Second}
	port, err := serial.OpenPort(c)
	if err != nil {
		return nil, fmt.Errorf("opening serial port: %w", err)
	}
	return NewFromStream(port), nil
}

func (e *Eswitch) Set(elektrodes [4]uint8) error {
	_, err := fmt.Fprintf(e.conn, "configure %d %d %d %d\r\n",
		elektrodes[0],
		elektrodes[1],
		elektrodes[2],
		elektrodes[3],
	)
	if err != nil {
		return fmt.Errorf("could not set elektrodes: %w", err)
	}
	return nil
}
