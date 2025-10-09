# Real-Time IoT Data Processor

[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![MQTT](https://img.shields.io/badge/MQTT-Protocol-green.svg)](https://mqtt.org/)
[![InfluxDB](https://img.shields.io/badge/InfluxDB-Time%20Series-orange.svg)](https://www.influxdata.com/)
[![Docker](https://img.shields.io/badge/Docker-Compose-blue.svg)](https://www.docker.com/)
[![Grafana](https://img.shields.io/badge/Grafana-Visualization-brown.svg)](https://grafana.com/)

A high-performance, enterprise-grade real-time IoT data processing pipeline built in modern C++17. This system demonstrates end-to-end data flow from sensor simulation to cloud storage, featuring MQTT messaging, time-series database integration, and cloud streaming capabilities.

## 🏗️ System Architecture

```
┌─────────────────┐    MQTT      ┌──────────────┐    InfluxDB    ┌─────────────────┐
│  Sensor         │─────────────→│  Processor   │───────────────→│  Time Series    │
│  Simulator      │              │  Engine      │                │  Database       │
└─────────────────┘              └──────────────┘                └─────────────────┘
                                          │
                                          │ Google Cloud Pub/Sub
                                          ▼
                                 ┌─────────────────┐
                                 │  Cloud Storage  │
                                 └─────────────────┘
```

## 🚀 Key Features

### **High-Performance Data Processing**
- Real-time sensor data processing
- **Thread-safe message handling** with asynchronous MQTT client
- **Zero-copy memory optimization** for high-throughput scenarios

### **Enterprise-Grade Messaging**
- **MQTT Protocol Integration** using Eclipse Paho C++ client
- **Automatic reconnection** and connection resilience
- **JSON-based sensor data serialization** with nlohmann/json

### **Time-Series Data Storage**
- **InfluxDB integration** with native C++ client
- **High-frequency data ingestion** (1000+ points/minute)
- **Real-time query capabilities** for monitoring dashboards
- Monitor data in **Grafana** dashboard

### **Cloud-Native Architecture**
- **Google Cloud Pub/Sub** integration with OAuth2 authentication
- **Docker Compose** orchestration for local development

## 🛠️ Technology Stack

| Component | Technology | Purpose |
|-----------|------------|---------|
| **Core Language** | C++17 | High-performance, memory-efficient processing |
| **Messaging** | MQTT (Eclipse Paho) | Real-time sensor data communication |
| **Time-Series DB** | InfluxDB | Efficient storage and querying of sensor data |
| **Cloud Messaging** | Google Cloud Pub/Sub | Scalable cloud data streaming |
| **JSON Processing** | nlohmann/json | Fast JSON parsing and serialization |
| **Containerization** | Docker Compose | Development environment orchestration |
| **Visualization** | Grafana | Real-time monitoring dashboards |



### Configuration

`config.json` file with user settings:

```json
{
  "mqtt": {
    "broker_url": "tcp://localhost:1883",
    "client_id": "iot_processor",
    "topic": "sensors/telemetry",
    "qos": 1
  },
  "influxdb": {
    "url": "http://localhost:8086",
    "token": "user-influxdb-token",
    "org": "user-organization",
    "bucket": "sensor_data"
  },
  "gcp": {
    "project_id": "user-gcp-project",
    "topic_id": "sensor-updates",
    "credentials_path": "/path/to/credentials.json"
  }
}
```

## 🔍 Monitoring & Observability

Access monitoring dashboards:
- **Grafana**: http://localhost:3000 (admin/admin)
- **InfluxDB UI**: http://localhost:8086
- **MQTT Broker**: mqtt://localhost:1883

### Key Metrics Tracked
- Message processing rate
- Database write performance
- Memory and CPU utilization
- Network latency and errors
- System uptime and availability
