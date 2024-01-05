package cn0359

import (
	"bufio"
	"fmt"
	"io"
	"regexp"
	"strconv"
	"strings"
	"time"

	"github.com/tarm/serial"
)

/*
   EXC V: 3.000000V
   EXC FREQ: 100.000000Hz
   EXC setup time: 10.000000%
   EXC hold time: 1.000000%
   TEMP COEF: 2.000000%/'C
   cell K: 1.000000/cm
   ADC0 hits: 15
   +I gain: 100
   +Ip-p: 8.781249e-05A
   -I gain: 100
   -Ip-p: -8.778367e-05A
   +V gain: 1
   +Vp-p: 5.961617e+00V
   -V gain: 1
   -Vp-p: -5.960855e+00V
   ADC1 hits: 0
   RTD: PT-1
   RTD wire: -1 wire
   TEMP: 25.000000'C

   conductivity: 1.472817e-05S/cm
*/

type Result struct {
	ExcitationVoltage      float64
	ExcitationFrequency    float64
	ExcitationSetupTime    float64
	ExcitationHoldtime     float64
	TemperatureCoefficient float64
	KConstant              float64
	ADC0Hits               float64
	PositiveCurrentGain    float64
	PositiveCurrentPeak    float64
	NegativeCurrentGain    float64
	NegativeCurrentPeak    float64
	PositiveVoltageGain    float64
	PositiveVoltagePeak    float64
	NegativeVoltageGain    float64
	NegativeVoltagePeak    float64
	ADC1Hits               float64
	Temperature            float64
	Conductivity           float64
}

type CN0359 struct {
	conn io.ReadWriteCloser
	addr uint
}

func NewFromStream(conn io.ReadWriteCloser, addr uint) *CN0359 {
	return &CN0359{
		conn: conn,
		addr: addr,
	}
}

func New(portStr string, baud int, addr uint) (*CN0359, error) {
	c := &serial.Config{Name: portStr, Baud: baud, ReadTimeout: 1 * time.Second}
	port, err := serial.OpenPort(c)
	if err != nil {
		return nil, fmt.Errorf("opening serial port: %w", err)
	}
	return NewFromStream(port, addr), nil
}

func (c *CN0359) SetAddr(addr uint) {
	c.addr = addr
}

var r_float = regexp.MustCompile(`-?\d+(\.\d+)?(e[-+]?[\d]+)?`)

func (c *CN0359) Poll() (Result, error) {
	var result Result

	scanner := bufio.NewScanner(c.conn)

	_, err := fmt.Fprintf(c.conn, "%d poll\n", c.addr)
	if err != nil {
		return result, fmt.Errorf("could not write to connection: %w", err)
	}

	count := 0
	for scanner.Scan() {
		line := scanner.Text()
		splits := strings.Split(line, ":")
		if len(splits) != 2 {
			continue
		}
		prop := splits[0]
		valueStr := r_float.FindString(splits[1])
		value, err := strconv.ParseFloat(valueStr, 64)
		if err != nil {
			return result, fmt.Errorf("error parsing float value: %w", err)
		}
		result.setProperty(prop, value)
		if err != nil {
			return result, fmt.Errorf("could not read line from connection: %w", err)
		}
		count++
	}
	if err := scanner.Err(); err != nil {
		return result, fmt.Errorf("reading from stream: %w", err)
	}
	if count != 20 {
		return result, fmt.Errorf("partial response: %d lines out of expected 22", count)
	}

	return result, nil
}

func (r *Result) setProperty(prop string, value float64) {
	switch prop {
	case "EXC V":
		r.ExcitationVoltage = value
	case "EXC FREQ":
		r.ExcitationFrequency = value
	case "EXC setup time":
		r.ExcitationSetupTime = value
	case "EXC hold time":
		r.ExcitationHoldtime = value
	case "TEMP COEF":
		r.TemperatureCoefficient = value
	case "cell K":
		r.KConstant = value
	case "ADC0 hits":
		r.ADC0Hits = value
	case "+I gain":
		r.PositiveCurrentGain = value
	case "+Ip-p":
		r.PositiveCurrentPeak = value
	case "-I gain":
		r.NegativeCurrentGain = value
	case "-Ip-p":
		r.NegativeCurrentPeak = value
	case "+V gain":
		r.PositiveVoltageGain = value
	case "+Vp-p":
		r.PositiveVoltagePeak = value
	case "-V gain":
		r.NegativeVoltageGain = value
	case "-Vp-p":
		r.NegativeVoltagePeak = value
	case "ADC1 hits":
		r.ADC1Hits = value
	case "TEMP":
		r.Temperature = value
	case "conductivity":
		r.Conductivity = value
	}
}
