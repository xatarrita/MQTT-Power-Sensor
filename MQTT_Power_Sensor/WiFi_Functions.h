
/*
 * WiFi, MQTT and Serial setup and operations
 */

String WiFi_SSID = "None";                   // Default SSID string when unconnected
String My_MAC = "";                          // MAC address, to be read from ESP8266

// Initialize the serial port
void Init_Serial() {
  Serial.begin(115200);
  Serial.println(""); Serial.println(""); Serial.println("* Starting up *");
}

// Try to connect to the given WiFi
void WiFi_Startup(const char* ssid, const char* password) {

  WiFi_SSID = ssid; // Set Access Point SSID, used by Build_Payload
  Serial.print("Connecting to " + String(ssid));

  // Static IP Option, set the IP and router details in Custom_Settings.h
#ifdef STATIC_IP
  // Static IP Setup
  WiFi.config(ip, gateway, subnet);
  Serial.print(" using Static IP " + String(ip));
#else
  Serial.print(" using DHCP");
#endif

  // Connect to wifi and turn off access point mode
  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);

  int TimeOut = 0;  // Reset timeout counter

  // Loop while not connected and timeout timer still running
  while ((WiFi.status() != WL_CONNECTED) && (TimeOut < 30)) {

    // Print connection progress ticker
    Status_LED_Off;
    delay(250);
    Status_LED_On;
    delay(250);
    Serial.print(".");
    TimeOut++;
  }

  // Timed out or connected?
  if (WiFi.status() == WL_CONNECTED) {

    // Get MAC address of ESP8266, 6 bytes in an array
    byte mac[6]; WiFi.macAddress(mac);

    My_MAC = "";
    // Build a string of the MAC with "0" padding for each byte, and upper case
    for (int i = 0; i <= 5; i++) {
      String B = String(mac[i], HEX);
      B.toUpperCase();
      if (B.length() < 2) {
        // Pad with leading zero if needed
        B = "0" + B;
      }
      My_MAC += B;
    } // End of for

    // Use the MAC address string to build an array to use as Client ID
    char MAC_as_client_id[My_MAC.length()];
    My_MAC.toCharArray(MAC_as_client_id, My_MAC.length());

    // Print connection report
    Serial.print(" WiFi connected");
#ifdef STATIC_IP
    Serial.print(", DHCP IP Address = " + WiFi.localIP());
#endif
    Serial.println(", " + String(WiFi.RSSI()) + "dB");

  } else {

    Serial.println(" Unable to connect to WiFi!");

  } // End of else

} // End of WiFi_Startup

// Try to connect to any of the WiFi networks configured in Custom_Settings.h
void Connect_To_Any_Known_WiFi() {

  //While not connected to wifi
  while (WiFi.status() != WL_CONNECTED) {

    if ((WiFi.status() != WL_CONNECTED) && (ssid_1 != NULL) && (password_1 != NULL)) {
      Serial.print("Trying slot 1 - "); WiFi_Startup(ssid_1, password_1);      // WiFi start
    }

    if ((WiFi.status() != WL_CONNECTED) && (ssid_2 != NULL) && (password_2 != NULL)) {
      Serial.print("Trying slot 2 - "); WiFi_Startup(ssid_2, password_2);      // WiFi start
    }

    if ((WiFi.status() != WL_CONNECTED) && (ssid_3 != NULL) && (password_3 != NULL)) {
      Serial.print("Trying slot 3 - "); WiFi_Startup(ssid_3, password_3);      // WiFi start
    }

    if ((WiFi.status() != WL_CONNECTED) && (ssid_4 != NULL) && (password_4 != NULL)) {
      Serial.print("Trying slot 4 - "); WiFi_Startup(ssid_4, password_4);      // WiFi start
    }

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Unable to connect to any known WiFi. Retrying...");
    }

  } // End of while loop

} // End of Connect_To_Any_Known_WiFi


void reconnect() {
  // Loop until we're reconnected
  // (or until the watchdog bites)
  while (!psClient.connected()) {

    if (WiFi.status() != WL_CONNECTED) {
      Connect_To_Any_Known_WiFi();
    }

    Serial.print("Attempting MQTT Broker connection...");
    // Attempt to connect

    // Make MAC address array used for Client ID
    char MAC_as_client_id[My_MAC.length()];
    My_MAC.toCharArray(MAC_as_client_id, My_MAC.length());

    const char* lwTopic = mqtt_topic; // use the default topic for the last will message too
    int lwQoS = 1; // send last will at least once
    int lwRetain = 0;
    const char* lwPayload = "{\"lastwill\":\"true\"}";
    bool cleanSession = true;

    // Connect client and use MAC address array as the Client ID
    if (psClient.connect(MAC_as_client_id, mqtt_username, mqtt_password, 
                         lwTopic, lwQoS, lwRetain, lwPayload, cleanSession)) {

      Serial.println(" connected using client ID: " + String(My_MAC));

      // Once connected, publish an announcement...
      birthMsgRequested = true;   // Request a birth report after connection

    } else {
      Serial.println(" failed, rc=" + psClient.state());
      Serial.println("Trying again in 5 seconds...");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
} // End of reconnect


// Compose a payload to return as required
// The payload and headers cannot exceed 128 bytes!
String Build_Payload(bool sendAlsoBirthDetails) {

  // WiFi signal strength in dB
  long rssi = WiFi.RSSI();
  String payload = String("") + "{" +
      "\"A\":\"" + rmsCurrent + "\"," +
      "\"W\":\"" + rmsPower + "\"," +
      "\"KWh\":\"" + kiloWattHours + "\"," +
    //"\"MAC\":\"" + My_MAC + "\"," +
    //"\"SSID\":\"" + WiFi_SSID + "\"," +
      "\"dB\":\"" + rssi + "\",";
  if (sendAlsoBirthDetails) {
    payload = payload + "\"birth\":\"true\",";
  }
  payload = payload + "\"count\":\"" + message_count + "\"}";

  return payload;

} // End of Build_Payload
