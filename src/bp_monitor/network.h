//WINC1500 Setup
#define WINC_CS   8
#define WINC_IRQ  7
#define WINC_RST  4
#define WINC_EN   2
Adafruit_WINC1500 WiFi(WINC_CS, WINC_IRQ, WINC_RST);
char ssid[] = "InPhiSaITPS2";      // your network SSID (name)
char pass[] = "colidox3045";   // your network password
int keyIndex = 0;                 // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;
Adafruit_WINC1500Client client;
IPAddress server(192, 168, 2, 24); //the server ipaddress?

#define getPage "/"  // path to test page

void httpRequestPOST(String id_num, String syst, String diast, String add) {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  String data, post;
  client.stop();

  String jsonString = "{\"id\":\"";
  jsonString += id_num;
  jsonString += "\",\"diastolic\":\"";
  jsonString += diast;
  jsonString += "\",\"systolic\":\"";
  jsonString += syst;
  jsonString += "\"}";

  data = jsonString;
  post = "POST /" + add + " HTTP/1.1\r\n";

  // if there's a successful connection:
  if (client.connect(server, 3000)) {
    Serial.println("\n\nConnected!");
    Serial.println(data);
    // Make a HTTP request:
    client.print(post);
    client.print("Host: ");
    client.print(server);
    client.print("\r\nContent-Type: application/json\r\n");
    client.print("Content-Length: ");
    client.print(data.length());
    client.print("\r\nConnection: Close\r\n\r\n");
    client.print(data);
    client.print("\r\n");
    Serial.println("Data Sent!");

  } else {
    Serial.println("\nPOST request failed");
  }
  //  if (client.connected()) {
  //    Serial.println("Disconnected");
  //    client.stop();  // DISCONNECT FROM THE SERVER
  //  }
}

void httpRequestGET(String page, int port) {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  //  Serial.println("Preparing to STOP for GET request...");
  client.stop();
  //  Serial.println("Client STOPPED for GET request...");

  // if there's a successful connection:
  if (client.connect(server, port)) {
    Serial.println("connecting to send GET request...");
    // Make a HTTP request:
    client.print("GET ");
    client.print(page);
    client.println(" HTTP/1.1");
    client.print("Host: "); client.println(server);
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
  }
  else {
    // if you couldn't make a connection:
    Serial.println("GET request failed");
  }
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void sendRequest(String post, String data) {
  // if there's a successful connection:
  if (client.connect(server, 3000)) {
    Serial.println("\n\nConnected!");
    Serial.println(data);
    // Make a HTTP request:
    client.print(post);
    client.print("Host: ");
    client.print(server);
    client.print("\r\nContent-Type: application/json\r\n");
    client.print("Content-Length: ");
    client.print(data.length());
    client.print("\r\nConnection: Close\r\n\r\n");
    client.print(data);
    client.print("\r\n");
    Serial.println("\nData Sent!");

  } else {
    Serial.println("\nPOST request failed");
  }
  if (client.connected()) {
    Serial.println("Disconnecting from the server after POST");
    client.stop();  // DISCONNECT FROM THE SERVER
  }
}

