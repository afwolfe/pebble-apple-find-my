const Clay = require('pebble-clay');
const MessageQueue= require('message-queue-pebble');
const messageKeys = require('message_keys');

const clayConfig = require('./config.json');
const clay = new Clay(clayConfig);

// Command Enums
const COMMAND_KEY_LIST = 0;
const COMMAND_KEY_FIND = 1;


// DeviceClass Enums
const DEVICE_CLASS_PHONE = 0;
const DEVICE_CLASS_LAPTOP = 1;
const DEVICE_CLASS_TABLET = 2;
const DEVICE_CLASS_DESKTOP = 3;
const DEVICE_CLASS_WATCH = 4;
const DEVICE_CLASS_UNKNOWN = 5;

var serverUrl;
var username;
var password;


var deviceSummaries = [];
var deviceIdMappings = {};


Pebble.addEventListener('webviewclosed', function(e) {
  if (e && !e.response) {
    return;
  }

  // Get the keys and values from each config item
  var settingsDict = clay.getSettings(e.response);

  serverUrl = settingsDict[messageKeys['Server']];
  localStorage.setItem("serverUrl", serverUrl);
  username = settingsDict[messageKeys['Username']];
  localStorage.setItem("username", username);
  password = settingsDict[messageKeys['Password']];
  localStorage.setItem("password", password);

  if (serverUrl) {
    updateDeviceList(sendDevicesToPebble);
  }
});


Pebble.addEventListener("ready", function (e) {

  serverUrl = localStorage.getItem("serverUrl");
  username = localStorage.getItem("username");
  password = localStorage.getItem("password");
  if (serverUrl) {
    updateDeviceList(sendDevicesToPebble);
  }
  else {
    // TODO: Send error message to Pebble to prompt config.
  }

});

// Listen for when an AppMessage is received
Pebble.addEventListener("appmessage", function (event) {
  var msg = event.payload;
  console.log(msg);
  console.log("Message received from watch:", event.data);
  if (msg.hasOwnProperty("Command")) {
    if (msg["Command"] == "FIND" && msg["DeviceId"])
    console.log(msg["DeviceId"]);
    if (deviceIdMappings.hasOwnProperty(msg["DeviceId"])) {
      var deviceId = deviceIdMappings[msg["DeviceId"]];
      var body = {
        "deviceId": deviceId
      };
      findMyXhr("POST", "find", body, function (data) {
        MessageQueue.sendAppMessage({ Command: COMMAND_KEY_FIND, Status: 200 });
      });
    }
    else {
      console.error("Device mapping not found.")
    }
  
  }
});

function sendDevicesToPebble(devices) {
  console.log("Sending devices to Pebble...")
  devices.forEach(function(deviceMsg) {
    console.debug("Sending device: " + deviceMsg);
    deviceMsg.Command = COMMAND_KEY_LIST;
    MessageQueue.sendAppMessage(deviceMsg);
  });
}

function deviceClassToEnum(classString) {
  classString = classString.toLowerCase();
  if (classString.indexOf("iphone") >= 0) { return DEVICE_CLASS_PHONE; }
  else if (classString.indexOf("macbook") >= 0) { return DEVICE_CLASS_LAPTOP; }
  else if (classString.indexOf("ipad") >= 0) { return DEVICE_CLASS_TABLET; }
  else if (classString.indexOf("imac") >= 0) { return DEVICE_CLASS_DESKTOP; }
  else if (classString.indexOf("watch") >= 0) { return DEVICE_CLASS_WATCH; }

  return DEVICE_CLASS_UNKNOWN;
}

function deviceIdToCrc(deviceId) {
  var crc8 = new CRC8();
  var deviceIdBytes = [];
  for (var i = 0; i < deviceId.length; i++){  
    deviceIdBytes.push(deviceId.charCodeAt(i));
  }
  return crc8.checksum(deviceIdBytes);
}

function updateDeviceList(callback) {
  var newDeviceSummaries = [];
  findMyXhr("GET", "devices", null, function (devices) {
    for (i in devices) {
      var device = devices[i];
      console.debug("Found device with ID: " + device.id);
      var deviceCrc = deviceIdToCrc(device.id)
      deviceIdMappings[deviceCrc] = device.id;

      newDeviceSummaries.push({
        "DeviceName": device.name,
        "DeviceId": deviceCrc,
        "DeviceClass": deviceClassToEnum(device.deviceClass)
      });
    }

    // TODO: Use localStorage for caching.
    // localStorage.setItem("deviceSummaries", JSON.stringify(newDeviceSummaries));
    callback(newDeviceSummaries);
  });
}

function findMyXhr(method, endpoint, body, callback) {
  var url = serverUrl + "/" + endpoint;

  var xhr = new XMLHttpRequest();
  xhr.addEventListener("readystatechange", function () {
    if (xhr.readyState === 4) {
      console.debug(method, url, xhr.status);
      if (xhr.status >= 200 && xhr.status < 300) {
        var data;
        if (xhr.responseText) {
          data = JSON.parse(xhr.responseText);
        } else { // If there's no response body, make one:
          data = {"status": xhr.status};
        }
        callback(data);
      } else {
        console.error(xhr.status);
      }
    }
  });
  xhr.open(method, url);
  xhr.setRequestHeader("Accept", "application/json");
  // if (BEARER_TOKEN) {
  //   xhr.setRequestHeader("Authorization", BEARER_TOKEN);
  // }
  if (username && password) {
    xhr.setRequestHeader("Authorization", "Basic " + btoa(username + ":" + password));
  }
  if (body) {
    xhr.setRequestHeader("Content-Type", "application/json");
    xhr.send(JSON.stringify(body));
  } else {
    xhr.send();
  }
}


// https://github.com/mode80/crc8js/
// "Class" for calculating CRC8 checksums...
function CRC8(polynomial, initial_value) { // constructor takes an optional polynomial type from CRC8.POLY
    if (polynomial == null) polynomial = 0xd5
    this.table = CRC8.generateTable(polynomial);
    this.initial_value = initial_value;
  }
  
  // Returns the 8-bit checksum given an array of byte-sized numbers
  CRC8.prototype.checksum = function(byte_array) {
    var c = this.initial_value;
  
    for (var i = 0; i < byte_array.length; i++ ) 
      c = this.table[(c ^ byte_array[i]) % 256] 
  
    return c;
  } 
  
  // returns a lookup table byte array given one of the values from CRC8.POLY 
  CRC8.generateTable =function(polynomial)
  {
    var csTable = [] // 256 max len byte array
    
    for ( var i = 0; i < 256; ++i ) {
      var curr = i
      for ( var j = 0; j < 8; ++j ) {
        if ((curr & 0x80) !== 0) {
          curr = ((curr << 1) ^ polynomial) % 256
        } else {
          curr = (curr << 1) % 256
        }
      }
      csTable[i] = curr 
    }
      
    return csTable
  }