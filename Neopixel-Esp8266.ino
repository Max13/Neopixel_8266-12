#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>

// #define DEBUG

#define PIN D1
#define PIX 150
#define RGB(r, g, b)    Adafruit_NeoPixel::Color(r, g, b)

#define METHOD  0
#define DEVICE  METHOD + 1
#define RANGE   DEVICE + 1
#define ACTION  RANGE  + 1
#define PARAM   ACTION + 1
#define P_NUM   PARAM  + 1 // Unused, only for length
#define P_LEN   15

#define P_RG1   P_LEN - 2
#define P_RG2   P_LEN - 1

void pixelFlashColor(uint32_t, uint8_t, uint16_t = 150);

const byte      aliases[][6] = {
    "all",
    "adnan",
    "alex"
};

const byte      aliases_coord[][6] = {
    "0-149",
    "1-2",
    "4-5"
};

#define N_ALIAS 3

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
// NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
// NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
// NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
// NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
// NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel neopixel = Adafruit_NeoPixel(PIX, PIN, NEO_GRB + NEO_KHZ800);

const char  *ssid = "Wifi SSID";
const char  *password = "Wifi Password";
WiFiServer  server(80);
WiFiClient  client;

byte        parts[P_NUM][P_LEN];
byte        colorTxt[7];
uint32_t    lastColor = RGB(127, 127, 127);
String      tmp;

void setup()
{
    Serial.begin(115200);
    delay(10);

    neopixel.begin();
    neopixel.setBrightness(10);
    neopixel.show();

    Serial.println();
    Serial.println();
    Serial.print(F("Connecting to "));
    Serial.println(ssid);

    // Loading
    pixelAlternateColors(RGB(0, 0, 255), RGB(0, 255, 0));

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(F("."));
    }
    Serial.println();
    Serial.println(F("WiFi connected"));

    // Start the server and print the IP
    server.begin();
    Serial.print(F("Server started on "));
    Serial.println(WiFi.localIP());

    pixelFlashColor(RGB(0, 255, 0), 2);

    tmp.reserve(200);
}

void loop()
{
    // Clears parts[][]
    // TODO: memset(parts, 0, 2 * P_NUM * P_LEN * sizeof(parts[0][0]));
    for (uint8_t i=0; i<P_NUM; ++i) {
        parts[i][0] = 0;
    }

    // Check if a client has connected
    while (!(client = server.available())) {
        delay(1);
    }
    Serial.println(F("New client"));

    // Wait until the client sends some data
    while (!client.available()) {
        delay(1);
    }

    // Read the first line of the request
    tmp = client.readStringUntil('\r');
    Serial.println(tmp);
    client.flush();

    parseRequest(tmp.c_str());

    #ifdef DEBUG
    Serial.println(F("=== REQ ==="));
    for (uint8_t i=0; i<P_NUM; ++i) {
        Serial.print(i);
        Serial.print(F(": "));
        Serial.println((char*)parts[i]);

        if (i == RANGE) {
            Serial.print(F("   "));
            Serial.print(parts[RANGE][P_RG1], DEC);
            Serial.print('-');
            Serial.println(parts[RANGE][P_RG2], DEC);
        }

        Serial.print(F("   "));
        for (uint8_t j=0; j<P_LEN; ++j) {
            Serial.print(parts[i][j], DEC);
            Serial.print(F(" "));
        }
        Serial.println();
    }
    Serial.println(F("=== /REQ ==="));
    #endif

    if (strcmp((char*)parts[METHOD], "GET") == 0) {
        if (strcmp((char*)parts[DEVICE], "pixels") == 0) {
            if (strcmp((char*)parts[ACTION], "status") == 0) {
                if (parts[PARAM][0] == '0') {                               // Set Status = 0
                    pixelSetColor(parts[RANGE][P_RG1], parts[RANGE][P_RG2], RGB(0, 0, 0));
                    printToClient("0");
                } else if (parts[PARAM][0]) {                               // Set Status = 1
                    pixelSetColor(parts[RANGE][P_RG1], parts[RANGE][P_RG2], lastColor);
                    printToClient("1");
                } else if (neopixel.getPixelColor(parts[RANGE][P_RG1]) == 0) {  // Get status = 0
                    printToClient("0");
                } else {                                                    // Get status = 1
                    printToClient("1");
                }
            } else if (strcmp((char*)parts[ACTION], "color") == 0) {
                if (parts[PARAM][0]) {                                      // Set Color = Hex RGB
                    lastColor = strtoul((char*)parts[PARAM], 0, 16);
                    pixelSetColor(parts[RANGE][P_RG1], parts[RANGE][P_RG2], lastColor);
                    printToClient((char*)parts[PARAM]);
                } else {                                                    // Get Color = Hex RGB
                    sprintf((char*)colorTxt, "%X", lastColor);
                    printToClient((char*)colorTxt);
                }
            } else {
                Serial.println(F("Unrecognized action"));
            }
        } else {
            Serial.println(F("Unrecognized device"));
        }
    } else {
        Serial.println(F("Unrecognized method"));
    }

    client.stop();
    Serial.println(F("Client disonnected"));

    // The client will actually be disconnected
    // when the function returns and 'client' object is detroyed
}

// Impossible to pulse while doing something
// void pixelPulse(uint32_t c)
// {
//     lastColor = c;
//     uint8_t r = (uint8_t) (c >> 16),
//             g = (uint8_t) (c >>  8),
//             b = (uint8_t) c;

//     for (ui = 0; ui < PIX; ++ui) {
//         if (ui == 0)
//         for (uj = 0; uj < PIX; ++uj) {
//             neopixel.setPixelColor(uj, c);
//         }
//     }

//     neopixel.show();
// }

bool parseRequest(const char *in)
{
    char    *chr = 0;
    uint8_t i = 0;
    uint8_t len = strlen(in);
    uint8_t ofs = METHOD;
    uint8_t subofs = 0;

    // Finds METHOD
    for (; i < len; ++i) {
        if (in[i] == ' ') {
            strncpy((char*)parts[ofs], in, i);
            parts[ofs][i] = 0;
            break;
        }
    }

    if (i == len) {
        Serial.println(F("Wrong request"));
        return false;
    }

    ++ofs;  // ofs should now be METHOD + 1

    // Finds other parts
    for (++i; i < len && ofs < P_NUM; ++i) {
        if (in[i] == '/') {
            while (i++ < len && in[i] && in[i] != '/' && in[i] != ' ') {
                parts[ofs][subofs++] = in[i];
            }
            parts[ofs][subofs] = 0;
            ++ofs;
            subofs = 0;
        } else if (in[i] == ' ') {
            break;
        }
        --i;
    }

    // Handles range
    if (isalpha(parts[RANGE][0])) {
        if ((i = rangeByKey((char*)parts[RANGE])) == -1) {
            #ifdef DEBUG
            Serial.println(F("Invalid alphanumeric range"));
            #endif
            return false;
        } else {
            strcpy((char*)parts[RANGE], (char*)aliases_coord[i]);
        }
    }

    if ((chr = strchr((char*)parts[RANGE], '-')) != 0) {
        i = (int)(chr - (char*)parts[RANGE]);


        parts[RANGE][P_RG1] = strtoul((char*)parts[RANGE], 0, 10);
        parts[RANGE][P_RG2] = strtoul((char*)&parts[RANGE][i + 1], 0, 10);
    // } else if ((chr = strchr((char*)parts[RANGE], ',')) != 0) {
    //     // Not yet handled
    //     return false;
    } else {
        parts[RANGE][P_RG1] = strtoul((char*)parts[RANGE], 0, 10);
        parts[RANGE][P_RG2] = parts[RANGE][P_RG1];
    }

    return true;
}

int8_t  rangeByKey(const char *key)
{
    for (uint8_t i = 0; i < N_ALIAS; ++i) {
        if (strcmp((char*)aliases[i], key) == 0) {
            return i;
        }
    }

    return -1;
}

void pixelAlternateColors(uint32_t rgb1, uint32_t rgb2)
{
    for (uint8_t i = 0; i < PIX; ++i) {
        neopixel.setPixelColor(i, i % 2 ? rgb1 : rgb2);
    }
    neopixel.show();
}

void pixelSetColor(uint8_t from, uint8_t to, uint32_t rgb)
{
    for (uint8_t i = from; i <= to; ++i) {
        neopixel.setPixelColor(i, rgb);
    }
    neopixel.show();
}

void pixelSetColor(uint32_t rgb)
{
    pixelSetColor(0, PIX - 1, rgb);
}

void pixelFlashColor(uint32_t rgb, uint8_t num, uint16_t wait)
{
    for (uint8_t i = 0; i < num; ++i) {
        pixelSetColor(RGB(0,255,0));
        delay(wait);
        pixelSetColor(RGB(0,0,0));
        if (i != num - 1) {
            delay(wait);
        }
    }
}

void printToClient(const char *s)
{
    client.flush();
    client.print(F("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n"));
    client.print(s);
    client.println();
    delay(1);
}

void printToClient(const String &s)
{
    printToClient(s.c_str());
}

void colorWipe(uint32_t c, uint8_t wait)
{
    for (uint8_t i = 0; i < PIX; ++i) {
        pixelSetColor(i, i, c);
        delay(wait);
    }
}
