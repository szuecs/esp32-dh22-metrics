# esp32-dh22-metrics
Get data from
[AM2302](http://akizukidenshi.com/download/ds/aosong/AM2302.pdf)
(dh22) running on an
[esp32](https://www.espressif.com/en/products/hardware/esp32/overview)
and sends [Prometheus](https://github.com/prometheus/prometheus)
metrics to a [pushgateway](https://github.com/prometheus/pushgateway)
using WiFi.
Prometheus can scrape metrics from the pushgateway and you can use
[grafana](https://github.com/grafana/grafana) to show nice dashboards.

## Features

You can select different serial output and push data types. Both
supports CSV, JSON and Prometheus data.
Serial output defaults to CSV and push data defaults to Prometheus.

All data and functions exported by https://github.com/beegee-tokyo/DHTesp
library are used to provide the metrics.

## Requirements

Hardware:

- dev board: ESP32
- sensor: AM2302 (dh22)
- USB connector to connect your computer with ESP32 (power and flash)
- 3 cable connectors to connect sensor with ESP32 (power, ground, GPIO data), see also [page 2 picture and table top left](https://cdn-shop.adafruit.com/datasheets/Digital+humidity+and+temperature+sensor+AM2302.pdf)

Software:

- pushgateway should be running, for example: `docker run -d -hostnetwork -p 9091:9091 prom/pushgateway`
- C++ dev package to have [unordered_map](https://en.cppreference.com/w/cpp/container/unordered_map), should be available.

You can find a nice tutorial at [techtutorialsx](https://techtutorialsx.com/2018/04/18/esp32-arduino-getting-temperature-from-a-dht22-sensor/).

## Install

Install [Arduino IDE](https://www.arduino.cc/en/main/software) and start Arduino IDE.

Install dependencies:

1. Install in the IDE: Board "ESP32 Dev Module" (`Tools -> Manage Libraries..`)
1. Install in the IDE: beegee-tokyo DHTesp https://github.com/beegee-tokyo/DHTesp (`Tools -> Manage Libraries..`)

First steps:

1. If you have the hardware connected and all dependencies setup, clone this repository
1. Open `File -> Open...` dialog to open the cloned repository and select file `esp32-dh22-metrics.ino`
1. Change file `esp32-dh22-metrics.ino` line 10-13 to your own setup
1. click `Tools -> Port` and select your USB serial port to configure the connection from your computer to ESP32.
1. click the `Upload` button to flash your ESP32 and run the software
1. Open the `Tools -> Serial Monitor` to show the serial output of the software running on your ESP32, after a few seconds you should get some data output, if it was successful
1. Check metrics endpoint of the running pushgateway

```
% curl -s http://127.0.0.1:9091/metrics | grep dh22
absolute_humidity{instance="dc",job="dh22"} 9
comfort_status{instance="dc",job="dh22",state="Comfort_OK"} 1
dew_point{instance="dc",job="dh22"} 9.9
distance_too_cold{instance="dc",job="dh22"} 1
distance_too_dry{instance="dc",job="dh22"} -1624.1
distance_too_hot{instance="dc",job="dh22"} -7.7
distance_too_humid{instance="dc",job="dh22"} -1079.4
heat_index{instance="dc",job="dh22"} 19.7
humidity{instance="dc",job="dh22"} 51
push_failure_time_seconds{instance="dc",job="dh22"} 0
push_time_seconds{instance="dc",job="dh22"} 1.578165888371657e+09
temperature_celsius{instance="dc",job="dh22"} 20.3
too_cold{instance="dc",job="dh22"} 1
too_dry{instance="dc",job="dh22"} 0
too_hot{instance="dc",job="dh22"} 0
too_humid{instance="dc",job="dh22"} 0
```

## Development

Please create issues and Pull requests as you like.
