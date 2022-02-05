# pebble-apple-find-my

An Apple "Find My" client for the Pebble smartwatch and accompanying server to interface with iCloud through pyicloud. 

## Status

The watchapp can login to the API server and get an access token in order to list your devices and send a request to find a particular device.

## Server

The `server/` folder implements a REST API in Python for initiating "Find My" requests using [picklepete/pyicloud](https://github.com/picklepete/pyicloud).


## Client

The `watchapp/` folder contains the source for the Pebble app. The PebbleKit JS component communicates with the server and sends the device information to the Pebble.


## Installation

### Server

1. Use pipenv to install the dependencies
    ```sh
    $ cd server
    $ pip install pipenv
    $ pipenv install --skip-lock
    ```
    * The `--skip-lock` is necessary due to a strict version pinning of `click` in pyicloud see [pyicloud issue #289](https://github.com/picklepete/pyicloud/issues/289)
2. Open a pipenv shell and configure your iCloud account for 2FA, see [the pyicloud documentation](https://github.com/picklepete/pyicloud#authentication) for more details.
    * It is recommended that you save your password/2FA in the keyring using the `icloud` command.
    * This will make server startup seamless without the need for interaction
    ```sh
    $ pipenv shell
    $ icloud --username=jappleseed@apple.com
    ICloud Password for jappleseed@apple.com:
    Save password in keyring? (y/N)
    ```
3. Create a .env file in the server folder and set the variables. See `server/.env.example` for values
    * If you don't specify APPLE_PASSWORD, pyicloud will attempt to use the keyring
4. Start the server from the pipenv shell with `python main.py` OR explicitly run it through pipenv with:
    ```sh
    $ pipenv run python main.py
    ```


### Client

1. Build and install the watchapp using the Pebble SDK
2. Make sure your server is running
3. Configure your server URL, and username/password to match the API user and password you set for the server.
4. Launch the watchapp and select a device to trigger a Find My alert on it.


## Roadmap

- [x] List devices on watchapp
- [x] Trigger "Find My" alert from watchapp
- [ ] Additional information on watchapp
  - [x] Show request success/failure on Pebble
  - [ ] Icons for device class
  - [ ] Battery level
  - [ ] Location name using reverse geocoding API
- [ ] Add endpoints to login/save iCloud credentials without pyicloud's `icloud` CLI.
- [ ] Additional endpoint to set "Lost Mode"
- [x] Server authentication
- [x] Gevent WSGI server
- [x] [Clay](https://github.com/pebble/clay) configuration


## Acknowledgements

* Inspiration/Resources
  * [pebble-examples](https://github.com/pebble-examples) - For examples of basic functionality and Pebble design patterns.
  * [Rebble](https://rebble.io/) - for keeping the Pebble alive in 2022.
  * [limbo/smartthings-remote](https://github.com/limbo/smartthings-remote) - For inspiration on handling the devices.
* Libraries
  * [pebble/clay](https://github.com/pebble/clay) - Pebble Config Framework
  * [mode80/crc8js](https://github.com/mode80/crc8js/) - JS CRC implementation to create a map of device IDs to avoid sending large device IDs to the Pebble
  * [vimalloc/flask-jwt-extended](https://github.com/vimalloc/flask-jwt-extended) - JWT support for Flask.
  * [smallstoneapps/js-message-queue](https://github.com/smallstoneapps/js-message-queue/) - PebbleKit JS AppMessage queue library
  * [matthewtole/pebble-assist.h](https://gist.github.com/matthewtole/7699013) - Utility macros for Pebble development 
  * [picklepete/pyicloud](https://github.com/picklepete/pyicloud) - Python iCloud wrapper.
