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
var access = {};
var refresh = {};

// TODO: Cache devices locally
// var deviceSummaries;
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

  // Attempt to load refresh token.
  refresh = localStorage.getItem("refreshToken");
  refresh = refresh ? JSON.parse(refresh) : {};

  if (serverUrl) {
    updateDeviceList(sendDevicesToPebble);
  }
  else {

  }

});

// Listen for when an AppMessage is received
Pebble.addEventListener("appmessage", function (event) {
  var msg = event.payload;
  if (msg.hasOwnProperty("Command")) {
    if (msg["Command"] == COMMAND_KEY_FIND && msg.hasOwnProperty("DeviceId")) {
      if (deviceIdMappings.hasOwnProperty(msg["DeviceId"])) {
        var deviceId = deviceIdMappings[msg["DeviceId"]];
        findDevice(deviceId);
      }
      else {
        console.error("Device mapping not found.")
      }
    }
  }
});

function sendConfigError() {
  MessageQueue.sendAppMessage({ Command: COMMAND_KEY_FIND, Status: 1 });
}

function sendDevicesToPebble(devices) {
  console.log("Sending devices to Pebble...")
  devices.forEach(function(deviceMsg) {
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
  for (var i = 0; i < deviceId.length; i++) {  
    deviceIdBytes.push(deviceId.charCodeAt(i));
  }
  return crc8.checksum(deviceIdBytes);
}

function loginOrRefreshToken(callback, forceNewToken) {
  if (forceNewToken || !access.hasOwnProperty('token') || access['expiration'] > Date.now())  { // If forceRefresh or access token is bad
    if (refresh.hasOwnProperty('token') && refresh['expiration'] < Date.now()) { // If refresh is good
      findMyXhr("POST", "refresh", null, function(data) {
        if (data.hasOwnProperty('token')) {
          access = data;
          callback();
        } else { // failed to refresh.
          refresh = {};
          loginOrRefreshToken(callback, true);
        }
      });
    } else { // Login
      var body = {'username': username, 'password': password};
      findMyXhr("POST", "login", body, function(data) {
        if (data.hasOwnProperty("access") && data.hasOwnProperty("refresh")) {
          access = data.access;
          refresh = data.refresh;
          localStorage.setItem('refreshToken', JSON.stringify(refresh));
        } else {
          console.log("Unable to login to server.");
          sendConfigError();
        }
        callback();
      });
    }
  }
  else { // else means the tokens are good.
    callback();
  }


}

function findDevice(deviceId, callback) {
  loginOrRefreshToken(function() {
    var body = {
      "deviceId": deviceId
    };
    findMyXhr("POST", "find", body, function (data) {
      MessageQueue.sendAppMessage({ Command: COMMAND_KEY_FIND, Status: data.status });
      callback(data);
    });
  })
}

function updateDeviceList(callback) {
  loginOrRefreshToken(function() {
    var newDeviceSummaries = [];
    findMyXhr("GET", "devices", null, function (devices) {
      for (i in devices) {
        var device = devices[i];
        var deviceCrc = deviceIdToCrc(device.id);
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
  });
}

function findMyXhr(method, endpoint, body, callback) {
  var url = serverUrl + "/findmy/api/v1/" + endpoint;

  var xhr = new XMLHttpRequest();
  xhr.addEventListener("readystatechange", function () {
    if (xhr.readyState === 4) {
      console.debug(method, url, xhr.status);
      var data;
      if (xhr.status >= 200 && xhr.status < 300) {
        if (xhr.responseText) {
          data = JSON.parse(xhr.responseText);
        } else { // If there's no response body, make one:
          data = {"status": xhr.status};
        }
        callback(data);
      } else {
        console.error(xhr.status);
        data = {"status": xhr.status};
        callback(data);
      }
    }
  });
  xhr.addEventListener("timeout", function () {
    data = {"status": 408};
    callback(data);
  });

  xhr.open(method, url);
  xhr.timeout = 5000;
  xhr.setRequestHeader("Accept", "application/json");
  if (endpoint !== "login" && access && access.hasOwnProperty("token")) {
    xhr.setRequestHeader("Authorization", "Bearer " + access["token"]);
  } else if (endpoint === "refresh" && refresh && refresh.hasOwnProperty("token")) {
    xhr.setRequestHeader("Authorization", "Bearer " + refresh["token"]);
  }
  else {
    console.log("No token used for authentication.");
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
  CRC8.generateTable = function(polynomial)
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