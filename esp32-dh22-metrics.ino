
#include <WiFi.h>
#include <WiFiMulti.h>
WiFiMulti WiFiMulti;

#include <DHTesp.h>
DHTesp dht;

// change values to your own setup
const char SSID[]     = "<your-wifi-name>";    // your WiFi name
const char PASSWORD[] = "<your-wifi-password"; // your WiFi password
const char * HOST     = "<your-pushgateway>";  // pushGW: ip or dns
const uint16_t PORT   = 9091;                  // pushGW: TCP port

bool DEBUG = false;

// prometheus metadata
String JOB = String("dh22");
String INSTANCE = String("dc");

enum DATA_TYPE {CSV, JSON, PROMETHEUS};

// enable serial outputter: CSV, JSON, PROMETHEUS
DATA_TYPE SERIAL_OUTPUTTER = CSV;
// enable data push to HTTP endpoint CSV, JSON, PROMETHEUS
DATA_TYPE PUSH_DATA = PROMETHEUS;

// prometheus_metric
#include "prometheus_metric.h"
Metric temperature(MetricType::gauge, "temperature_celsius", "DHT22 exported temperature value in celsius", 1);
Metric humidity(MetricType::gauge, "humidity", "DHT22 exported relative humidity value in percent", 1);
Metric absHumidity(MetricType::gauge, "absolute_humidity", "DHT22 computed absolute humidity value", 1);
Metric heatIndex(MetricType::gauge, "heat_index", "DHT22 computed heat index value", 1);
Metric dewPoint(MetricType::gauge, "dew_point", "DHT22 computed dew point value", 1);
Metric tooHot(MetricType::gauge, "too_hot", "DHT22 comfort value too hot", 1);
Metric tooCold(MetricType::gauge, "too_cold", "DHT22 comfort value too cold", 1);
Metric tooHumid(MetricType::gauge, "too_humid", "DHT22 comfort value too humid", 1);
Metric tooDry(MetricType::gauge, "too_dry", "DHT22 comfort value distance too dry", 1);
Metric distanceTooHot(MetricType::gauge, "distance_too_hot", "DHT22 comfort value distance to tooHot", 1);
Metric distanceTooCold(MetricType::gauge, "distance_too_cold", "DHT22 comfort value distance to tooCold", 1);
Metric distanceTooHumid(MetricType::gauge, "distance_too_humid", "DHT22 comfort value distance to tooHumid", 1);
Metric distanceTooDry(MetricType::gauge, "distance_too_dry", "DHT22 comfort value distance to tooDry", 1, std::unordered_map<std::string, std::string>() );
Metric comfortStatus(MetricType::gauge, "comfort_status", "DHT22 comfort status value", 1, {
      {"state", ""},
});


// value object
struct MetricData {
  float temperature;
  float humidity; // relative

  // https://en.wikipedia.org/wiki/Humidity#Absolute_humidity
  float absHumidity;
  // https://en.wikipedia.org/wiki/Heat_index
  float heatIndex;
  // https://en.wikipedia.org/wiki/Dew_point
  float dewPoint;

  // comfort status data
  float distanceTooHot;
  float distanceTooCold;
  float distanceTooHumid;
  float distanceTooDry;
  bool tooHot;
  bool tooCold;
  bool tooHumid;
  bool tooDry;
  String comfortStatus;
  String perceptionStatus;
};

// Metrics implements interaction with dht22 to create MetricData and exports to JSON/CSV/Prometheus
class Metrics {

private:
  MetricData md;

public:

  MetricData getMetricsData() { return md; }

  void createMetricsData() {
    TempAndHumidity tah = dht.getTempAndHumidity();
    md.temperature  = tah.temperature;
    md.humidity     = tah.humidity;
    md.absHumidity  = dht.computeAbsoluteHumidity(tah.temperature, tah.humidity, false); // in "g/m³"
    md.heatIndex    = dht.computeHeatIndex(tah.temperature, tah.humidity, false);
    md.dewPoint     = dht.computeDewPoint(tah.temperature, tah.humidity, false);

    ComfortState cf;
    // if we want to support setting custom comfort ratio
    //float comfortRatio = dht.getComfortRatio(&cf, tah.temperature, tah.humidity, false);
    md.comfortStatus = ComfortStatus(cf);

    ComfortProfile cp = dht.getComfortProfile();
    md.tooHot   = cp.isTooHot(tah.temperature, tah.humidity);
    md.tooCold  = cp.isTooCold(tah.temperature, tah.humidity);
    md.tooHumid = cp.isTooHumid(tah.temperature, tah.humidity);
    md.tooDry   = cp.isTooDry(tah.temperature, tah.humidity);
    md.distanceTooHot   = cp.distanceTooHot(tah.temperature, tah.humidity);
    md.distanceTooCold  = cp.distanceTooCold(tah.temperature, tah.humidity);
    md.distanceTooHumid = cp.distanceTooHumid(tah.temperature, tah.humidity);
    md.distanceTooDry   = cp.distanceTooDry(tah.temperature, tah.humidity);

    PerceptionState ps = (PerceptionState) dht.computePerception(tah.temperature, tah.humidity, false);
    md.perceptionStatus = PerceptionStatusString(ps);
  }

  String ComfortStatus(ComfortState cf) {
    String comfortStatus;
    switch(cf) {
    case Comfort_OK:
      comfortStatus = "Comfort_OK";
      break;
    case Comfort_TooHot:
      comfortStatus = "Comfort_TooHot";
      break;
    case Comfort_TooCold:
      comfortStatus = "Comfort_TooCold";
      break;
    case Comfort_TooDry:
      comfortStatus = "Comfort_TooDry";
      break;
    case Comfort_TooHumid:
      comfortStatus = "Comfort_TooHumid";
      break;
    case Comfort_HotAndHumid:
      comfortStatus = "Comfort_HotAndHumid";
      break;
    case Comfort_HotAndDry:
      comfortStatus = "Comfort_HotAndDry";
      break;
    case Comfort_ColdAndHumid:
      comfortStatus = "Comfort_ColdAndHumid";
      break;
    case Comfort_ColdAndDry:
      comfortStatus = "Comfort_ColdAndDry";
      break;
    default:
      comfortStatus = "Unknown:";
      break;
    };

    return comfortStatus;
  }

  String PerceptionStatusString(PerceptionState ps) {
    switch(ps) {
    case Perception_Dry: return "Perception_DRY";
    case Perception_VeryComfy: return "Perception_VeryComfy";
    case Perception_Comfy: return "Perception_Comfy";
    case Perception_Ok: return "Perception_Ok";
    case Perception_UnComfy: return "Perception_UnComfy";
    case Perception_QuiteUnComfy: return "Perception_QuiteUnComfy";
    case Perception_VeryUnComfy: return "Perception_VeryUnComfy";
    case Perception_SevereUncomfy: return "Perception_SevereUncomfy";
    default: return "Perception_Unkown";
    }
  }

  String CSV() {
    return "temperature;humidity;absHumidity;heatIndex;dewPoint;ComfortStatus;tooHot;tooCold;tooHumid;tooDry;distTooHot;distTooCold;distTooHumid;distTooDry;perceptionState\n"
      + String(md.temperature) + ";"
      + String(md.humidity) + ";"
      + String(md.absHumidity) + ";"
      + String(md.heatIndex) + ";"
      + String(md.dewPoint) + ";"
      + md.comfortStatus + ";"
      + String(md.tooHot) + ";"
      + String(md.tooCold) + ";"
      + String(md.tooHumid) + ";"
      + String(md.tooDry) + ";"
      + String(md.distanceTooHot) + ";"
      + String(md.distanceTooCold) + ";"
      + String(md.distanceTooHumid) + ";"
      + String(md.distanceTooDry) + ";"
      + md.perceptionStatus;
  }

  String JSON() {
    return "{\n \"temperature\": " + String(md.temperature) +
      ",\n \"humidity\": " + String(md.humidity) +
      ",\n \"absHumidity\": " + String(md.absHumidity) +
      ",\n \"heatIndex\": " + String(md.heatIndex) +
      ",\n \"dewPoint\": " + String(md.dewPoint) +
      ",\n \"ComfortStatus\": \"" + md.comfortStatus + "\"" +
      ",\n \"tooHot\": " + String(md.tooHot) +
      ",\n \"tooCold\": " + String(md.tooCold) +
      ",\n \"tooHumid\": " + String(md.tooHumid) +
      ",\n \"tooDry\": " + String(md.tooDry) +
      ",\n \"distTooHot\": " + String(md.distanceTooHot) +
      ",\n \"distTooCold\": " + String(md.distanceTooCold) +
      ",\n \"distTooHumid\": " + String(md.distanceTooHumid) +
      ",\n \"distTooDry\": " + String(md.distanceTooDry) +
      ",\n \"perceptionState\": \"" + md.perceptionStatus +
      "\"\n}";
  }


  String Prometheus() {
    temperature.setValue(md.temperature);
    humidity.setValue(md.humidity);
    absHumidity.setValue(md.absHumidity);
    heatIndex.setValue(md.heatIndex);
    dewPoint.setValue(md.dewPoint);
    tooHot.setValue(md.tooHot);
    tooCold.setValue(md.tooCold);
    tooDry.setValue(md.tooDry);
    tooHumid.setValue(md.tooHumid);
    distanceTooHot.setValue(md.distanceTooHot);
    distanceTooCold.setValue(md.distanceTooCold);
    distanceTooHumid.setValue(md.distanceTooHumid);
    distanceTooDry.setValue(md.distanceTooDry);

    comfortStatus.setValue(1, {
        {"state", md.comfortStatus.c_str()},
    });

    return temperature.getString()
      + humidity.getString()
      + absHumidity.getString()
      + heatIndex.getString()
      + dewPoint.getString()
      + tooHot.getString()
      + tooCold.getString()
      + tooHumid.getString()
      + tooDry.getString()
      + distanceTooHot.getString()
      + distanceTooCold.getString()
      + distanceTooHumid.getString()
      + distanceTooDry.getString()
      + comfortStatus.getString();
  }
};

void setup() {
  Serial.begin(115200);

  WiFiMulti.addAP(SSID, PASSWORD);

  if (DEBUG) {
    Serial.println();
    Serial.println();
    Serial.print("Waiting for WiFi... ");
  }

  while(WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  if (DEBUG) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

  // DH22
  dht.setup(17, DHTesp::AM2302);
  if (DEBUG) {
    Serial.print("dht status: ");
    Serial.println(dht.getStatusString());
  }
  delay(500);
}

void loop()
{
  if (DEBUG) {
    Serial.print("Connecting to ");
    Serial.println(HOST);
  }

  // Use WiFiClient class to create TCP connections
  WiFiClient client;

  if (!client.connect(HOST, PORT)) {
    Serial.println("Connection failed.");
    Serial.println("Waiting 5 seconds before retrying...");
    delay(5000);
    return;
  }

  Metrics m = Metrics();
  m.createMetricsData();
  MetricData md = m.getMetricsData();

  String jsonData = m.JSON();
  String csvData = m.CSV();
  String prometheusData = m.Prometheus();

  switch (SERIAL_OUTPUTTER) {
  case JSON:
    Serial.println(jsonData);
    break;
  case CSV:
    Serial.println(csvData);
    break;
  case PROMETHEUS:
    Serial.println(prometheusData);
    break;
  }

  switch (PUSH_DATA) {
  case JSON:
    client.print("POST /temperature HTTP/1.1\r\n");
    client.print("Content-Type: application/json\r\n");
    client.print("Content-Length: ");
    client.print(jsonData.length());
    client.print("Host: ");
    client.print(HOST);
    client.print("\r\n\r\n");
    client.print(jsonData);
    break;
  case CSV:
    client.print("POST /temperature HTTP/1.1\r\n");
    client.print("Content-Type: text/plain\r\n");
    client.print("Content-Length: ");
    client.print(csvData.length());
    client.print("Host: ");
    client.print(HOST);
    client.print("\r\n\r\n");
    client.print(csvData);
    break;
  case PROMETHEUS:
    client.print("POST /metrics/job/");
    client.print(JOB);
    client.print("/instance/");
    client.print(INSTANCE);
    client.print(" HTTP/1.1\r\n");
    client.print("Content-Type: application/x-www-form-urlencoded\r\n");
    client.print("Content-Length: ");
    client.print(prometheusData.length());
    client.print("\r\n");
    client.print("Host: ");
    client.print(HOST);
    client.print("\r\n\r\n");
    client.print(prometheusData);
    break;
  }

  int maxloops = 0;
  // wait for the server's reply to become available
  while (!client.available() && maxloops < 1000) {
    maxloops++;
    delay(1); //delay 1 msec
  }
  if (client.available() > 0) {
    while (client.available() > 0) {
      //read back one line from the server
      String line = client.readStringUntil('\r\n');
      if (DEBUG) {
        Serial.print("> ");
        Serial.println(line);
      }
    }
  } else {
    Serial.println("HTTP Response timeout from server");
  }

  if (DEBUG) { Serial.println("Closing connection."); }
  client.stop();

  if (DEBUG) { Serial.println("Waiting 5 seconds before restarting..."); }
  delay(5000);
}
