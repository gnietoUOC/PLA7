#ifndef HOMIE_H
#define HOMIE_H

#define MAX_DEVICES     1
#define MAX_CHILDREN    3
#define MAX_PROPERTIES  6
#define MAX_ATTRIBUTES  4
#define ENV_PROPS       3
#define NA              -1000

#define SECURED
//#define EC2
//#define ZT

#ifdef EC2
  #ifdef ZT
    #define MQHOST        "172.24.71.188" //EC2 ZT  
    #ifdef SECURED
      #define MQPORT          8883           
    #else 
      #define MQPORT          1883           
    #endif
  #else
    #define MQHOST          "54.194.149.95" //EC2 
      #define MQPORT          1883           
  #endif
  #define MQUSERNAME      "soc-iot"
  #define MQPWD           "unic0s19"
#else 
  #define MQHOST          "192.168.0.159"
  #define MQUSERNAME      "genaro"
  #define MQPWD           "passw0rd"
  #ifdef SECURED
    #define MQPORT          8883           
  #else 
    #define MQPORT          1883           
  #endif
#endif

#define RASPBERRY
#ifdef RASPBERRY
  #define PISSID          "Genaro0712"
  #define PIPWD           "passw0rd"
#else
  #define PISSID          "MiFibra-290A"
  #define PIPWD           "g071268b260966"
#endif

#define MQRETAIN        true
#define WILLTOPIC       "MKR1000"
#define WILLMESSAGE     "ATPC"
#define MQCLIENT          "mkr1000"

#define MQPERIOD          1000 // 1sg

#define TIMEZONE        1

//#define DEBUG   //If you comment this line, the DPRINT & DPRINTLN lines are defined as blank.
#ifdef DEBUG    //Macros are usually in all capital letters.
  #define DPRINT(...)    Serial.print(__VA_ARGS__)     //DPRINT is a macro, debug print
  #define DPRINTLN(...)  Serial.println(__VA_ARGS__)   //DPRINTLN is a macro, debug print with new line
#else
  #define DPRINT(...)     //now defines a blank line
  #define DPRINTLN(...)   //now defines a blank line
#endif

#include <Arduino.h>
#include <WiFi101.h>
#include <PubSubClient.h>
#include <Arduino_MKRENV.h>
#include <SD.h>
#include <RTCZero.h>

class Homie;
class Device;
class Node;
class Property;
class Pin;
class Attribute;

class Attribute {

  public:
    Attribute(char *name, char *cvalue);
    Attribute(char *name, int ivalue);
    char *getName();
    char *getCValue();
    int getIValue();

  private:
    char *name;
    char *cvalue;
    int ivalue;
};

class Base {
  public:
//    Base (PubSubClient *client, char* name);
//    Base (PubSubClient *client);
    Base (PubSubClient *client, Base *parent, char* name);
    Base (PubSubClient *client, Base *parent, char* name, int index);
//    Base (PubSubClient *client, Base *parent, char* name, char *units, bool settable);

    char* getName();
    void setName(char *name);
    Base *getParent();
    void setParent(Base *parent);
    void setClient(PubSubClient *client);
    PubSubClient *getClient();
    virtual void update(){};
    virtual void dump(){};
    Attribute **getAttributes();
    int getNumAttributes();
    Attribute *getAttribute(char *name);
    void addAttribute(Attribute *attrib);

    void getPath(char *path);
    void process(char *topic,char *payload);
    void pub(char *tag, char *value);

    static RTCZero *getRTC();

  private:
    char* name;
 //   float* value;
    Base *parent;
    PubSubClient *client;
    int n;
    Attribute **attributes;
    void logger(char *value);
    static RTCZero *rtc;
    
};

class Node : public Base {

  public:
    Node(PubSubClient *client, Device *parent, char* name);
    Property** getProperties();
    int getNumProperties();
    void addProperty(Property *p);
    Property *getProperty(int i);
    Property *getProperty(char *name);
    virtual void process(char *topic, char* value);
    
    void update();
    void dump();
    void clear();

  private: 
    int n;
    Property** properties;

  
};

class Device : public Node {

  public:
    Device(PubSubClient *client, Device *parent, char *name);
    Node** getChildren();
    int getNumChildren();
    void addChild(Node *n);
    Node *getChild(int i);
    Node *getChild(char *name);
    void process(char *topic, char* value);

    void update();
    void dump();
    void clear();   

  private: 
    int n;
//    Node** nodes;
    Node** children;
  
};

class Homie : public Device {

  public:
    void update();
    void dump();
    void reconnect();
//    static Homie *getInstance(PubSubClient *client);
//    static Homie *getInstance();
    Homie(PubSubClient *client);
    void callback(char* topic, uint8_t* payload, unsigned int length);

  private:
//    static void callback(char* topic, byte* payload, unsigned int length);
//    void callback(char* topic, uint8_t* payload, unsigned int length);
//    Homie(PubSubClient *client);
//    Homie(PubSubClient *client);
//    static Homie *homie;
};

//Homie *Homie::homie = 0;

class Property : public Base {

  public:
    Property(PubSubClient *client, Node *parent, char *name, char *units, char *type, bool settable);
    Property(PubSubClient *client, Node *parent, char *name, int index, char *units, char *type, bool settable);
    float getValue();
    void setValue(float value);
    int getIValue();
    void setIValue(int ivalue);
    bool getBValue();
    void setBValue(bool bvalue);
    virtual void set(char *value);
    
    void dump();
    void update();
    void clear();

  private:
    float value;
    int ivalue;
    int bvalue;

};

class Temperature : public Property {

  public:
    Temperature(PubSubClient *client, Node *parent);
    void update();
};

class Humidity : public Property {
  public:
    Humidity(PubSubClient *client, Node *parent);
    void update();
};

class Pressure : public Property {

  public:
    Pressure(PubSubClient *client, Node *parent);
    void update();
};

class Illuminance : public Property {

  public:
    Illuminance(PubSubClient *client, Node *parent);
    void update();
};

class UVA : public Property {

  public:
    UVA(PubSubClient *client, Node *parent);
    void update();
};

class UVB : public Property {

  public:
    UVB(PubSubClient *client, Node *parent);
    void update();
};

class UVIndex : public Property {

  public:
    UVIndex(PubSubClient *client, Node *parent);
    void update();
};

class Memory : public Property {

  public:
    Memory(PubSubClient *client, Node *parent);
    void update();
};

class Pin : public Property {

  public:
    Pin(PubSubClient *client, Node *parent,char *name,int pin);
    void update();
    void set(bool status);
    void set(char *value);

   private:
    int pin;
   
};

class LED : public Pin {

  public:
    LED(PubSubClient *client, Node *parent,int pin);
   
};

class Relay : public Pin {

  public:
    Relay(PubSubClient *client, Node *parent,int pin);
   
};

#endif
