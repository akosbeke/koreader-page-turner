#include <Arduino.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <esp_wifi.h>
#include <Preferences.h>
#include <HTTPClient.h>

Preferences preferences;

// Connection settings
const char *apSsid = "KoreaderPageTurnerAP";
const char *password = NULL; // no password

#define MAX_CLIENTS 1
#define WIFI_CHANNEL 6 // 2.4ghz channel 6 https://en.wikipedia.org/wiki/List_of_WLAN_channels#2.4_GHz_(802.11b/g/n/ax)

const IPAddress localIP(4, 3, 2, 1);   // the IP address the web server, Samsung requires the IP to be in public space
const IPAddress gatewayIP(4, 3, 2, 1); // IP address of the network should be the same as the local IP for captive portals
const IPAddress subnetMask(255, 255, 255, 0);

const String localIPURL = "http://4.3.2.1"; // a string version of the local IP with http, used for redirecting clients to config page

DNSServer dnsServer;
AsyncWebServer server(80);

// Hardware settings
#define MULTIACTION_PIN 4
#define MULTIACTION_MODE INPUT_PULLUP

void setUpDNSServer(DNSServer &dnsServer, const IPAddress &localIP)
{
// Define the DNS interval in milliseconds between processing DNS requests
#define DNS_INTERVAL 30

  // Set the TTL for DNS response and start the DNS server
  dnsServer.setTTL(3600);
  dnsServer.start(53, "*", localIP);
}

void startSoftAccessPoint(const char *ssid, const char *password, const IPAddress &localIP, const IPAddress &gatewayIP)
{
#define MAX_CLIENTS 1
#define WIFI_CHANNEL 6

  Serial.println("Could not connect to WiFi, starting Soft Access Point...");

  // Set the WiFi mode to access point and station
  WiFi.mode(WIFI_MODE_AP);

  const IPAddress subnetMask(255, 255, 255, 0);

  WiFi.softAPConfig(localIP, gatewayIP, subnetMask);
  WiFi.softAP(ssid, password, WIFI_CHANNEL, 0, MAX_CLIENTS);
  // Set TX power (needed for Super Mini)
  WiFi.setTxPower(WIFI_POWER_8_5dBm);

  // Disable AMPDU RX on the ESP32 WiFi to fix a bug on Android
  esp_wifi_stop();
  esp_wifi_deinit();
  wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();
  my_config.ampdu_rx_enable = false;
  esp_wifi_init(&my_config);
  esp_wifi_start();
  vTaskDelay(100 / portTICK_PERIOD_MS); // Add a small delay

  Serial.println("Soft Access Point started, connect to:");
  Serial.println(ssid);
}

void startWifiOrAp()
{
  const char *apName = "KoreaderPageTurnerAP";
  const char *hostname = "KoreaderPageTurner";

  WiFi.hostname(hostname);
  WiFi.begin(preferences.getString("wifi_ssid", "").c_str(),
             preferences.getString("wifi_password", "").c_str());

  // Set TX power (needed for Super Mini)
  WiFi.setTxPower(WIFI_POWER_8_5dBm);

  // Wait for connection
  Serial.println("Connecting to WiFi..");
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 15000)
  {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nWiFi connected to:");
    Serial.println(preferences.getString("wifi_ssid", ""));
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    startSoftAccessPoint(apSsid, password, localIP, gatewayIP);
  }
}

String getConfigPage()
{
  String wifi_ssid = preferences.getString("wifi_ssid", "");
  String wifi_password = preferences.getString("wifi_password", "");
  String ip = preferences.getString("ip", "");
  String port = preferences.getString("port", "8080");

  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <meta charset="UTF-8">
      <title>Koreader Page Turner Settings</title>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
    </head>
    <body>
      <h2>Koreader Page Turner</h2>
      <form method="POST" action="/save">
        <label for="wifi_ssid">WiFi SSID:</label><br>
        <input type="text" id="wifi_ssid" name="wifi_ssid" value=")rawliteral" +
                wifi_ssid + R"rawliteral("><br><br>
        <label for="wifi_password">WiFi Password:</label><br>
        <input type="password" id="wifi_password" name="wifi_password" value=")rawliteral" +
                wifi_password + R"rawliteral("><br><br>
        <label for="ip">IP Address:</label><br>
        <input type="text" id="ip" name="ip" value=")rawliteral" +
                ip + R"rawliteral("><br><br>
        <label for="port">Port:</label><br>
        <input type="text" id="port" name="port" value=")rawliteral" +
                port + R"rawliteral("><br><br>
        <input type="submit" value="Save">
      </form>
    </body>
    </html>
  )rawliteral";

  return html;
}

void setUpWebserver(AsyncWebServer &server, const IPAddress &localIP)
{
  // Required
  server.on("/connecttest.txt", [](AsyncWebServerRequest *request)
            { request->redirect("http://logout.net"); }); // windows 11 captive portal workaround
  server.on("/wpad.dat", [](AsyncWebServerRequest *request)
            { request->send(404); }); // win 10

  // Background responses: Probably not all are Required, but some are. Others might speed things up?
  // A Tier (commonly used by modern systems)
  server.on("/generate_204", [](AsyncWebServerRequest *request)
            { request->redirect(localIPURL); }); // android captive portal redirect
  server.on("/redirect", [](AsyncWebServerRequest *request)
            { request->redirect(localIPURL); }); // microsoft redirect
  server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request)
            { request->redirect(localIPURL); }); // apple call home
  server.on("/canonical.html", [](AsyncWebServerRequest *request)
            { request->redirect(localIPURL); }); // firefox captive portal call home
  server.on("/success.txt", [](AsyncWebServerRequest *request)
            { request->send(200); }); // firefox captive portal call home
  server.on("/ncsi.txt", [](AsyncWebServerRequest *request)
            { request->redirect(localIPURL); }); // windows call home

  // return 404 to webpage icon
  server.on("/favicon.ico", [](AsyncWebServerRequest *request)
            { request->send(404); }); // webpage icon

  // Serve Basic HTML Page
  server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request)
            {
		AsyncWebServerResponse *response = request->beginResponse(200, "text/html", getConfigPage());
		response->addHeader("Cache-Control", "public,max-age=31536000");  
		request->send(response);
		Serial.println("Config page served"); });

  // Handle form submission
  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    String wifi_ssid;
    String wifi_password;
    String ip;
    String port;
    // If any of the parameters are missing, redirect to the config page
    if (!request->hasParam("wifi_ssid", true) || !request->hasParam("wifi_password", true) ||
        !request->hasParam("ip", true) || !request->hasParam("port", true))
    {
      Serial.println("Missing parameters, redirecting to config page");
      request->redirect(localIPURL);
      return;
    }
    // Get the parameters
    wifi_ssid = request->getParam("wifi_ssid", true)->value();
    wifi_password = request->getParam("wifi_password", true)->value();
    ip = request->getParam("ip", true)->value();
    port = request->getParam("port", true)->value();

    // Save the parameters to preferences
    preferences.putString("wifi_ssid", wifi_ssid);
    preferences.putString("wifi_password", wifi_password);
    preferences.putString("ip", ip);
    preferences.putString("port", port);

    request->send(200, "text/plain", "Settings saved. Restarting...");

    delay(1000);
    ESP.restart(); });

  server.onNotFound([](AsyncWebServerRequest *request)
                    {
		request->redirect(localIPURL);
		Serial.print("onnotfound ");
		Serial.print(request->host());	
		Serial.print(" ");
		Serial.print(request->url());
		Serial.print(" sent redirect to " + localIPURL + "\n"); });
}

void resetPreferences()
{
  preferences.clear();
  preferences.end();
  Serial.println("Preferences cleared, restarting...");

  delay(1000);
  ESP.restart();
}

void turnPage(int direction)
{
  HTTPClient http;

  // Sanitize the IP: remove "http://", "https://", and trailing "/"
  String ip = preferences.getString("ip", "");

  // Remove "http://" or "https://" if present
  if (ip.startsWith("http://") || ip.startsWith("https://"))
  {
    ip = ip.substring(ip.indexOf("//") + 2); // Skip past "http://" or "https://"
  }

  // Remove trailing "/" if present
  if (ip.endsWith("/"))
  {
    ip = ip.substring(0, ip.length() - 1);
  }

  String url = "http://" + ip + ":" + preferences.getString("port", "8080") + "/koreader/event/GotoViewRel/" + String(direction);

  Serial.println("Turning page: " + url);

  // Send the HTTP request
  http.begin(url);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0)
  {
    String response = http.getString();
    Serial.println("Response: " + response);
  }
  else
  {
    Serial.println("Error on HTTP request");
  }

  http.end();
}

void handleMultiaction()
{
  // click: on for at most 300ms
  // long click: on for at least 300ms but less than 2000ms
  // super long click: on for at least 10000ms

  // Handle the button press
  if (digitalRead(MULTIACTION_PIN) == LOW)
  {
    unsigned long startTime = millis();
    while (digitalRead(MULTIACTION_PIN) == LOW)
    {
      delay(10);
    }
    unsigned long duration = millis() - startTime;

    if (duration < 300)
    {
      // Next page on click
      turnPage(1);
    }
    else if (duration < 2000)
    {
      // Previous page on long click
      turnPage(-1);
    }
    else
    {
      Serial.println("Super long click detected");
      resetPreferences();
    }
  }
}

void startPreferences()
{
  preferences.begin("koreader", false);

  if (!preferences.isKey("wifi_ssid"))
  {
    preferences.putString("wifi_ssid", ""); // Set default ssid
  }

  if (!preferences.isKey("wifi_password"))
  {
    preferences.putString("wifi_password", ""); // Set default password
  }

  if (!preferences.isKey("ip"))
  {
    preferences.putString("ip", ""); // Set default IP
  }

  if (!preferences.isKey("port"))
  {
    preferences.putString("port", "8080"); // Set default port
  }
}

void setup()
{
  Serial.setTxBufferSize(1024);
  Serial.begin(115200);

  delay(1000); // Wait for the Serial to initialize

  pinMode(MULTIACTION_PIN, MULTIACTION_MODE);

  startPreferences();

  // Print a welcome message to the Serial port.
  Serial.println("\n\nStarting up...");
  Serial.printf("%s-%d\n\r", ESP.getChipModel(), ESP.getChipRevision());

  startWifiOrAp();

  setUpDNSServer(dnsServer, localIP);

  setUpWebserver(server, localIP);
  server.begin();

  Serial.print("\n");
  Serial.print("Startup Time:");
  Serial.println(millis());
  Serial.print("\n");
}

void loop()
{
  dnsServer.processNextRequest();
  delay(DNS_INTERVAL);
  handleMultiaction();
}