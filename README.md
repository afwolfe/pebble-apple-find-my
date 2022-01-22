# pebble-find-my

A barebones Apple "Find My" client for the Pebble smartwatch.

## Server

The `server/` folder implements a basic REST API in Python for initiating "Find My" requests using [picklepete/pyicloud](https://github.com/picklepete/pyicloud)

## Client

The `find-my/` folder contains the source for the Pebble app. Currently it just sends a POST request to a hardcoded server variable using the device's ID.