package main

import (
	"encoding/csv"
	"errors"
	"fmt"
	"io"
	"os"
	"time"

	"github.com/pollex/pzld-zoutwachter/internal/cn0359"
)

type FileLoggerConfiguration struct {
	Directory string
}

type FileLogger struct {
	config FileLoggerConfiguration
}

func NewFileLogger(cfg FileLoggerConfiguration) *FileLogger {
	return &FileLogger{
		config: cfg,
	}
}

func (logger *FileLogger) Save(profile Profile, result cn0359.Result) error {
	now := time.Now()
	year, week := now.ISOWeek()
	filename := fmt.Sprintf("%s/%4d%2d.csv", logger.config.Directory, year, week)

	writer, file, err := logger.assertFile(filename)
	if err != nil {
		return fmt.Errorf("error asserting file at %s: %w", filename, err)
	}
	defer file.Close()

	err = writer.Write([]string{
		result.Timestamp.Format(time.RFC3339),
		fmt.Sprintf("%d|%d|%d|%d", profile[0], profile[1], profile[2], profile[3]),
		fmt.Sprint(profile[0]), fmt.Sprint(profile[1]), fmt.Sprint(profile[2]), fmt.Sprint(profile[3]),
		fmt.Sprint(result.ExcitationVoltage),
		fmt.Sprint(result.ExcitationFrequency),
		fmt.Sprint(result.ExcitationSetupTime),
		fmt.Sprint(result.ExcitationHoldtime),
		fmt.Sprint(result.TemperatureCoefficient),
		fmt.Sprint(result.KConstant),
		fmt.Sprint(result.ADC0Hits),
		fmt.Sprint(result.PositiveCurrentGain),
		fmt.Sprint(result.PositiveCurrentPeak),
		fmt.Sprint(result.NegativeCurrentGain),
		fmt.Sprint(result.NegativeCurrentPeak),
		fmt.Sprint(result.PositiveVoltageGain),
		fmt.Sprint(result.PositiveVoltagePeak),
		fmt.Sprint(result.NegativeVoltageGain),
		fmt.Sprint(result.NegativeVoltagePeak),
		fmt.Sprint(result.ADC1Hits),
		fmt.Sprint(result.Temperature),
		fmt.Sprint(result.Conductivity),
	})
	if err != nil {
		return fmt.Errorf("could not write CSV file: %w", err)
	}
	writer.Flush()

	return nil
}

func (logger *FileLogger) assertFile(filename string) (*csv.Writer, io.Closer, error) {
	createHeader := false
	if stat, err := os.Stat(filename); errors.Is(err, os.ErrNotExist) {
		createHeader = true
	} else if err != nil {
		return nil, nil, fmt.Errorf("could not stat csv file: %w", err)
	} else if stat.IsDir() {
		return nil, nil, fmt.Errorf("CSV file is a directory")
	}

	file, err := os.OpenFile(filename, os.O_CREATE|os.O_APPEND|os.O_RDWR, 0666)
	if err != nil {
		return nil, nil, fmt.Errorf("could not open/create csv file: %w", err)
	}

	writer := csv.NewWriter(file)
	if !createHeader {
		return writer, file, nil
	}

	// Only reached when file does not exist yet
	if err := writer.Write([]string{
		"Timestamp",
		"Elektrode Pair",
		"Elektrode 1",
		"Elektrode 2",
		"Elektrode 3",
		"Elektrode 4",
		"ExcitationVoltage",
		"ExcitationFrequency",
		"ExcitationSetupTime",
		"ExcitationHoldtime",
		"TemperatureCoefficient",
		"KConstant",
		"ADC0Hits",
		"PositiveCurrentGain",
		"PositiveCurrentPeak",
		"NegativeCurrentGain",
		"NegativeCurrentPeak",
		"PositiveVoltageGain",
		"PositiveVoltagePeak",
		"NegativeVoltageGain",
		"NegativeVoltagePeak",
		"ADC1Hits",
		"Temperature",
		"Conductivity",
	}); err != nil {
		return nil, nil, fmt.Errorf("could not write header to CSV file: %w", err)
	}

	return writer, file, nil
}
