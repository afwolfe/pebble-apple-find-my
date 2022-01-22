const express = require('express');
var icloud = require("find-my-iphone").findmyphone;
require('dotenv').config();

var app = express();
// register JSON parser
app.use(express.json());

app.use((err, req, res, next) => {
    if (err) {
      return res.sendStatus(500);
    }
    next();
  });

// iCloud-API configuration
var APPLE_USERNAME = process.env.APPLE_USERNAME;
var APPLE_PASSWORD = process.env.APPLE_PASSWORD;

if (!APPLE_USERNAME || !APPLE_PASSWORD) {
    throw new Error("Missing Apple username and or password.");
}

icloud.apple_id = APPLE_USERNAME;
icloud.password = APPLE_PASSWORD;
icloud.init(() => {});

app.post("/find", function (req, res) {
    
    let deviceId = req.body.deviceId;
    console.log(`/find ${deviceId}`);
    if (deviceId == null) {
        res.status = 400;
        res.end();
    }
    icloud.alertDevice(deviceId, function(err) {
        if (err) {
            res.status = 500;
            res.end();
        }
        else {
            res.end();
        }
    });
    
});

app.get("/")

app.get("/devices", function (req, res) {
    // var devices;
    icloud.getDevices(function(error, devices) {
        res.end(JSON.stringify(devices));
    })

});

var server = app.listen(8081, function () {
    var host = server.address().address;
    var port = server.address().port;
    console.log("Listening at http://%s:%s", host, port);
});

 