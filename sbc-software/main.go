package main

import (
	"context"
	"errors"
	"fmt"
	"log"
	"os"
	"os/signal"
	"strings"
	"time"

	"github.com/knadh/koanf/parsers/toml/v2"
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

type (
	Profile [4]uint8

	Config struct {
		Readout struct {
			Path     string
			Address  uint
			BaudRate uint
		}
		Switch struct {
			Path     string
			BaudRate uint
		}
		SensorBucket SensorBucketConfiguration
		FileLogger   FileLoggerConfiguration
		Profile      struct {
			CycleInterval time.Duration
			SettleTime    time.Duration
			Configuration []Profile
		}
	}
)

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

	if config.FileLogger.Directory == "" {
		config.FileLogger.Directory = "."
	} else {
		// Ensure it exists
		if stat, err := os.Stat(config.FileLogger.Directory); errors.Is(err, os.ErrNotExist) {
			if err := os.Mkdir(config.FileLogger.Directory, 0660); err != nil {
				return config, fmt.Errorf("could not create FileLogger Directory: %w", err)
			}
		} else if !stat.IsDir() {
			return config, fmt.Errorf("FileLogger Directory is not a directory")
		}
	}

	return config, nil
}

var (
	config       Config
	readout      *cn0359.CN0359
	elektrodes   *eswitch.Eswitch
	sensorbucket *SensorBucket
	csvlog       FileLogger
)

func Run() error {
	var err error

	log.Printf("Starting ZoutWachter software. Press CTRL+C any time to initiate a shutdown\n")
	ctx, cancel := signal.NotifyContext(context.Background(), os.Interrupt)
	defer cancel()

	log.Println("Loading configuration file...")
	config, err = loadConfig()
	if err != nil {
		return fmt.Errorf("loading config: %w", err)
	}

	sensorbucket = NewSensorBucket(config.SensorBucket)
	csvlog = *NewFileLogger(config.FileLogger)

	log.Println("Connecting to CN0359...")
	readout, err = cn0359.New(config.Readout.Path, config.Readout.BaudRate, config.Readout.Address)
	if err != nil {
		return fmt.Errorf("creating cn0359 driver failed: %w", err)
	}

	log.Println("Connecting to Switch...")
	elektrodes, err = eswitch.New(config.Switch.Path, config.Switch.BaudRate)
	if err != nil {
		return fmt.Errorf("creating switch driver failed: %w", err)
	}

	log.Println("Performing first measurement in 2 seconds...")
	<-time.After(2 * time.Second)
	PerformMeasurementCycle()

	// Main loop
	t := time.NewTicker(config.Profile.CycleInterval)
outer_loop:
	for {
		select {
		case <-ctx.Done():
			break outer_loop
		case tickTime := <-t.C:
			PerformMeasurementCycle()
			log.Printf("Next cycle at %s\n", tickTime.Add(config.Profile.CycleInterval).Format(time.RFC3339))
		}
	}

	log.Println("Shutting down...")
	t.Stop()

	return nil
}

func PerformMeasurementCycle() {
	profileCount := len(config.Profile.Configuration)
	log.Printf("Starting measurement cycle, will measure at %d different profiles...\n", profileCount)
	for profileIndex := 0; profileIndex < profileCount; profileIndex++ {
		if err := PerformProfileMeasure(config.Profile.Configuration[profileIndex]); err != nil {
			log.Printf("error performing profile measurement for profile %d(%v): %v\n", profileIndex, config.Profile.Configuration[profileIndex], err)
		}
	}
	log.Println("Measurement cycle completed")
}

func PerformProfileMeasure(profile Profile) error {
	log.Printf("Switching elektrodes to: %v...\n", profile)
	if err := elektrodes.Set(profile); err != nil {
		return fmt.Errorf("error changing elektrodes: %w", err)
	}

	log.Println("Waiting for readout to settle...")
	time.Sleep(config.Profile.SettleTime)

	log.Println("Starting readout...")
	result, err := readout.Poll()
	if err != nil {
		return fmt.Errorf("error reading measurement from readout: %w", err)
	}

	log.Println("Pushing to SensorBucket...")
	if err := sensorbucket.Push(profile, result); err != nil {
		log.Printf("error pushing to SensorBucket: %v\n", err)
	}
	log.Println("Logging to file...")
	if err := csvlog.Save(profile, result); err != nil {
		log.Printf("error saving to file: %v\n", err)
	}

	return nil
}
