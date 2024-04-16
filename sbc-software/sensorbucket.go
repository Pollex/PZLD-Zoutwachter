package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"net/http"

	"github.com/pollex/pzld-zoutwachter/internal/cn0359"
)

type SensorBucketConfiguration struct {
	URL    string
	APIKey string
}

type SensorBucket struct {
	config SensorBucketConfiguration
}

type SensorbucketPacket struct {
	cn0359.Result
	Profile [4]uint8 `json:"profile"`
}

func NewSensorBucket(cfg SensorBucketConfiguration) *SensorBucket {
	return &SensorBucket{
		config: cfg,
	}
}

func (sb *SensorBucket) Push(profile Profile, result cn0359.Result) error {
	pkt := SensorbucketPacket{
		Result:  result,
		Profile: profile,
	}
	var buf bytes.Buffer
	if err := json.NewEncoder(&buf).Encode(pkt); err != nil {
		return fmt.Errorf("error encoding measurement result to json: %w", err)
	}

	// Build request
	httpReq, err := http.NewRequest("POST", sb.config.URL, &buf)
	if err != nil {
		return fmt.Errorf("error assembling http request: %w", err)
	}
	if sb.config.APIKey != "" {
		httpReq.Header.Set("Authorization", "Bearer "+sb.config.APIKey)
	}

	// Perform request
	httpRes, err := http.DefaultClient.Do(httpReq)
	if err != nil {
		return fmt.Errorf("error performing http request: %w", err)
	}
	if httpRes.StatusCode < 200 || httpRes.StatusCode > 299 {
		body, _ := io.ReadAll(httpRes.Body)
		return fmt.Errorf("remote responded with failure: %d\nBody: %s", httpRes.StatusCode, string(body))
	}
	return nil
}
