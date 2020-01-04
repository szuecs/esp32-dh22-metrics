#ifndef PROMETHEUS_METRIC_H
#define PROMETHEUS_METRIC_H

#include "Arduino.h"
#include <unordered_map>

enum MetricType : short {
  untyped = 0, counter, gauge, histogram, summary
};

class Metric {
private:
  String name;
  String helpText;
  std::unordered_map<std::string, std::string> labels;
  MetricType type;
  float value;
  int precision;

public:
  String getString();

  Metric(MetricType type, String name, String helpText, int precision);

  Metric(MetricType type, String name, String helpText, int precision, std::unordered_map<std::string, std::string> labels);

  void setValue(float val) {
    this->value = val;
  }
  void setValue(float val, std::unordered_map<std::string, std::string> labels) {
    this->value = val;
    this->labels = labels;
  }
};

#endif
