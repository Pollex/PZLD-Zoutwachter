package main

import (
	"bytes"
	"context"
	"encoding/csv"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"net/http"
	"os"
	"os/signal"
	"strings"
	"time"

	"github.com/knadh/koanf/parsers/toml"
	"github.com/knadh/koanf/providers/env"
	"github.com/knadh/koanf/providers/file"
	"github.com/knadh/koanf/v2"
	"github.com/pollex/pzld-zoutwachter/internal/cn0359"
	"github.com/pollex/pzld-zoutwachter/internal/eswitch"
)

func main() {
	if err := Run(); err != nil {
		fmt.Fprintf(os.Stderr, "Error: %s\n", err.Error())
	}
}

type Config struct {
	ReadoutPath    string
	ReadoutBaud    int
	ReadoutAddress uint
	PushURL        string
	Authorization  string
	Interval       time.Duration
	SwitchPath     string
	Profile        [][4]uint8
	CSVFolder      string
}

var k = koanf.New(".")

var koanfLower = koanf.WithMergeFunc(func(src, dest map[string]interface{}) error {
	for k, v := range src {
		dest[strings.ToLower(k)] = v
	}
	return nil
})

func loadConfig() (Config, error) {
	var config Config
	if err := k.Load(file.Provider("config.toml"), toml.Parser(), koanfLower); err != nil {
		return config, fmt.Errorf("could not load config file: %w", err)
	}
	err := k.Load(env.Provider("ZW_", ".", func(s string) string {
		return strings.ReplaceAll(strings.ToLower(
			strings.TrimPrefix(s, "ZW_")), "_", ".")
	}), nil)
	if err != nil {
		return config, fmt.Errorf("could not load config from env: %w", err)
	}
	if err := k.Unmarshal("", &config); err != nil {
		return config, fmt.Errorf("config file invalid: %w", err)
	}

	if config.CSVFolder == "" {
		config.CSVFolder = "."
	}

	return config, nil
}

func Run() error {
	ctx, cancel := signal.NotifyContext(context.Background(), os.Interrupt)
	defer cancel()

	config, err := loadConfig()
	if err != nil {
		return fmt.Errorf("loading config: %w", err)
	}

	fmt.Printf(`Configuration loaded:
    Pushing data to
        %s
    every
        %s`+"\n", config.PushURL, config.Interval.String())

	fmt.Print("Connecting to CN0359...")
	cn, err := cn0359.New(config.ReadoutPath, config.ReadoutBaud, config.ReadoutAddress)
	if err != nil {
		return fmt.Errorf("creating cn0359 driver failed: %w", err)
	}
	fmt.Println("success")

	fmt.Print("Connecting to Switch...")
	elektrodes, err := eswitch.New(config.SwitchPath, 115200)
	if err != nil {
		return fmt.Errorf("creating switch driver failed: %w", err)
	}
	fmt.Println("success")

	fmt.Println("Performing first measurement in 2 seconds...")
	<-time.After(2 * time.Second)

	profileIndex := 0
	profileLength := len(config.Profile)

	// Initial measurement
	measure := func(tim time.Time) {
		profile := config.Profile[profileIndex]
		fmt.Println("\nSwitching elektrodes...")
		if err := elektrodes.Set(profile); err != nil {
			fmt.Printf("Failed to switch elektrodes: %s\n", err.Error())
			return
		}

		// Get measuremnet
		fmt.Println("\nStarting new measurement...")
		fmt.Print("Polling...")
		measurement, err := cn.Poll()
		if err != nil {
			fmt.Println("failed")
			fmt.Printf("polling cn0359 error: %s\n", err)
			return
		}
		fmt.Println("Success")
		fmt.Printf("Result:\n %+v\n", measurement)

		// Save to CSV
		if err := saveToCSV(config, measurement, profile); err != nil {
			fmt.Printf("could not write to CSV: %s", err)
		}

		// Send to SB
		if err := sendToSensorBucket(config, measurement, profile); err != nil {
			fmt.Printf("could not send to SensorBucket: %s", err)
		}

		// End
		fmt.Printf("Next measurement at: %s\n", tim.Add(config.Interval).Local().String())
		profileIndex = (profileIndex + 1) % profileLength
	}
	measure(time.Now())

	t := time.NewTicker(config.Interval)
outer_loop:
	for {
		select {
		case <-ctx.Done():
			break outer_loop
		case tickTime := <-t.C:
			measure(tickTime)
		}
	}

	return nil
}

func saveToCSV(cfg Config, result cn0359.Result, profile [4]uint8) error {
	fmt.Print("Writing to file...")
	now := time.Now()
	year, week := now.ISOWeek()
	filename := fmt.Sprintf("%s/%4d%2d.csv", cfg.CSVFolder, year, week)
	createHeader := false
	if stat, err := os.Stat(filename); errors.Is(err, os.ErrNotExist) {
		createHeader = true
	} else if err != nil {
		return fmt.Errorf("could not stat csv file: %w", err)
	} else if stat.IsDir() {
		return fmt.Errorf("CSV file is a directory")
	}

	file, err := os.OpenFile(filename, os.O_CREATE|os.O_APPEND|os.O_RDWR, 0666)
	if err != nil {
		return fmt.Errorf("could not open/create csv file: %w", err)
	}
	c := csv.NewWriter(file)
	defer file.Close()
	if createHeader {
		fmt.Println("Creating header for new CSV")
		err := c.Write([]string{
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
		})
		if err != nil {
			return fmt.Errorf("could not write CSV file: %w", err)
		}
	}
	fmt.Println("Writing line to CSV")
	err = c.Write([]string{
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
	c.Flush()

	fmt.Println("Success")
	return nil
}

type SensorbucketPacket struct {
	cn0359.Result
	Profile [4]uint8 `json:"profile"`
}

func sendToSensorBucket(cfg Config, result cn0359.Result, profile [4]uint8) error {
	pkt := SensorbucketPacket{
		Result:  result,
		Profile: profile,
	}
	fmt.Print("Pushing data...")
	var buf bytes.Buffer
	if err := json.NewEncoder(&buf).Encode(pkt); err != nil {
		fmt.Println("failed!")
		return fmt.Errorf("error encoding measurement result to json: %w", err)
	}
	httpReq, err := http.NewRequest("POST", cfg.PushURL, &buf)
	if err != nil {
		fmt.Println("failed!")
		return fmt.Errorf("error assembling http request: %w", err)
	}
	httpReq.Header.Set("Authorization", cfg.Authorization)
	httpRes, err := http.DefaultClient.Do(httpReq)
	if err != nil {
		fmt.Println("failed!")
		return fmt.Errorf("error performing http request: %w", err)
	}
	if httpRes.StatusCode < 200 || httpRes.StatusCode > 299 {
		fmt.Println("failed!")
		body, _ := io.ReadAll(httpRes.Body)
		return fmt.Errorf("remote responded with failure: %d\nBody: %s", httpRes.StatusCode, string(body))
	}
	fmt.Println("Success!")
	return nil
}
