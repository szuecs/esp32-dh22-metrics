#include "prometheus_metric.h"
#include "Arduino.h"

String Metric::getString() {
  String result = "";
  if (isnan(this->value)) {
    return result;
  }

  result += String("# HELP ") + this->name + String(" ") + this->helpText + String("\n");
  switch (this->type) {
  case counter:
    result += String("# TYPE ") + this->name + String(" counter\n");
    break;
  case gauge:
    result += String("# TYPE ") + this->name + String(" gauge\n");
    break;
  case histogram:
    result += String("# TYPE ") + this->name + String(" histogram\n");
    break;
  case summary:
    result += String("# TYPE ") + this->name + String(" summary\n");
    break;
  }

  result += this->name;
  int size = this->labels.size();
  if (size > 0) {
      result += "{";
      int i = 0;
      for( const auto& h : this->labels ) {
        i++;
        result += String(h.first.c_str()) + "=\"" + String(h.second.c_str()) + "\"";
        if (i != size) {
            result += ",";
        }
      }
      result += "}";
  }
  result += String(" ") + String(this->value, this->precision) + String("\n");
  return result;
}

Metric::Metric(MetricType type, String name, String helpText, int precision, std::unordered_map<std::string, std::string> labels) {
  this->type = type;
  this->name = name;
  this->helpText = helpText;
  this->precision = precision;
  this->labels = labels;
}

Metric::Metric(MetricType type, String name, String helpText, int precision) {
  this->type = type;
  this->name = name;
  this->helpText = helpText;
  this->precision = precision;
}
