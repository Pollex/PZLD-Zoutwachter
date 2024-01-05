package main

import (
	"bytes"
	"context"
	"encoding/json"
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
)

func main() {
	if err := Run(); err != nil {
		fmt.Fprintf(os.Stderr, "Error: %s\n", err.Error())
	}
}

type Config struct {
	PushURL       string
	Authorization string
	Interval      time.Duration
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
	cn, err := cn0359.New("/dev/ttyUSB0", 19200, 44)
	if err != nil {
		return fmt.Errorf("creating cn0359 driver failed: %w", err)
	}
	fmt.Println("success")

	fmt.Println("Performing first measurement in 2 seconds...")
	<-time.After(2 * time.Second)

	// Initial measurement
	measure := func(tim time.Time) {
		fmt.Println("\nStarting new measurement...")
		if err := doMeasurement(config, cn); err != nil {
			fmt.Printf("Failed to perform measurement: %s\n", err.Error())
		}
		fmt.Printf("Next measurement at: %s\n", tim.Add(config.Interval).Local().String())
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

func doMeasurement(cfg Config, cn *cn0359.CN0359) error {
	fmt.Print("Polling...")
	measurement, err := cn.Poll()
	if err != nil {
		fmt.Println("failed")
		return fmt.Errorf("polling cn0359 error: %w", err)
	}
	fmt.Println("Success")
	fmt.Printf("Result:\n %+v\n", measurement)

	fmt.Print("Pushing data...")
	var buf bytes.Buffer
	if err := json.NewEncoder(&buf).Encode(measurement); err != nil {
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
