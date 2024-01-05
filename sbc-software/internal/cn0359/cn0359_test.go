package cn0359_test

import (
	"bytes"
	"io"
	"testing"

	"github.com/pollex/pzld-zoutwachter/internal/cn0359"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

type customReadWriteCloser struct {
	reader io.Reader
	writer io.Writer
}

// Read delegates to the Reader part of customReadWriteCloser.
func (rw *customReadWriteCloser) Read(p []byte) (n int, err error) {
	return rw.reader.Read(p)
}

// Write delegates to the Writer part of customReadWriteCloser.
func (rw *customReadWriteCloser) Write(p []byte) (n int, err error) {
	return rw.writer.Write(p)
}

// Close performs the necessary cleanup. This method can be modified
// to suit the specific requirements of your program.
func (rw *customReadWriteCloser) Close() error {
	// Implement any necessary cleanup here.
	// Since bytes.Buffer does not require explicit closing,
	// this is left empty. Adjust as needed for your use case.
	return nil
}

func TestParsing(t *testing.T) {
	readBuf := bytes.NewBufferString(`EXC V: 3.000000V
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
`)
	var writeBuf bytes.Buffer

	conn := &customReadWriteCloser{
		reader: readBuf,
		writer: &writeBuf,
	}
	cn := cn0359.NewFromStream(conn, 44)

	result, err := cn.Poll()
	require.NoError(t, err)
	requestString, err := writeBuf.ReadString('\n')
	assert.NoError(t, err)
	assert.Equal(t, "44 poll\n", requestString)

	assert.Equal(t, cn0359.Result{
		ExcitationVoltage:      3.0,
		ExcitationFrequency:    100,
		ExcitationSetupTime:    10,
		ExcitationHoldtime:     1,
		TemperatureCoefficient: 2,
		KConstant:              1,
		ADC0Hits:               15,
		PositiveCurrentGain:    100,
		PositiveCurrentPeak:    8.781249e-5,
		NegativeCurrentGain:    100,
		NegativeCurrentPeak:    -8.778367e-5,
		PositiveVoltageGain:    1,
		PositiveVoltagePeak:    5.961617,
		NegativeVoltageGain:    1,
		NegativeVoltagePeak:    -5.960855,
		ADC1Hits:               0,
		Temperature:            25,
		Conductivity:           1.472817e-05,
	}, result)
}
