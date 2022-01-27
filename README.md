# pebble-apple-find-my

An Apple "Find My" client for the Pebble smartwatch.

## Server

The `server/` folder implements a REST API in Python for initiating "Find My" requests using [picklepete/pyicloud](https://github.com/picklepete/pyicloud).

**BE WARNED**: The server does not currently have any authentication and should not be used outside of your home network.

## Client

The `watchapp/` folder contains the source for the Pebble app. The PebbleKit JS component communicates with the server and sends the device information to the Pebble.

## Installation

### Server

1. Use pipenv to install the dependencies
    ```sh
    $ cd server
    $ pip install pipenv
    $ pipenv install
    ```
2. Open a pipenv shell and configure your iCloud account for 2FA, see [the pyicloud documentation](https://github.com/picklepete/pyicloud#authentication) for more details.
    * It is recommended that you save your password/2FA in the keyring using the `icloud` command.
    * This will make server startup seamless without the need for interaction
    ```sh
    $ pipenv shell
    $ icloud --username=jappleseed@apple.com
    ICloud Password for jappleseed@apple.com:
    Save password in keyring? (y/N)
    ```
3. Create a .env file in the server folder with at minimum the APPLE_USERNAME variable.
    * If you don't specify the password, pyicloud will attempt to use the keyring
    .env
    ```
    APPLE_USERNAME=jappleseed@apple.com
    ```
4. Start the server from the pipenv shell with `python main.py` OR explicitly run it through pipenv with:
    ```sh
    $ pipenv run python main.py
    ```

### Client

1. Modify the SERVER_URL at the top of `src/pkjs/index.js` to where your server is hosted.
2. Build and install the watchapp using the Pebble SDK
3. Make sure your server is running
4. Launch the watchapp and select a device to trigger a Find My alert on it.

## Roadmap

- [x] List devices on watchapp
- [x] Trigger "Find My" alert from watchapp
- [ ] Additional information on watchapp
  - [ ] Icons for device class
  - [ ] Battery level
  - [ ] Location name using reverse geocoding API
- [ ] Additional endpoint/option to set "Lost Mode"
- [ ] Server authentication
- [ ] Gunicorn server deployment
- [ ] [Clay](https://github.com/pebble/clay) configuration

## Acknowledgements

* Inspiration/Resources
  * [limbo/smartthings-remote](https://github.com/limbo/smartthings-remote) - For inspiration on handling the devices.
  * [pebble-examples](https://github.com/pebble-examples) - For examples of basic functionality and Pebble design patterns.
  * [Rebble](https://rebble.io/) - for keeping the Pebble alive in 2022.
* Libraries
  * [mode80/crc8js](https://github.com/mode80/crc8js/) - JS CRC implementation to create a map of device IDs to avoid sending large device IDs to the Pebble
  * [smallstoneapps/js-message-queue](https://github.com/smallstoneapps/js-message-queue/) - PebbleKit JS AppMessage queue library
  * [matthewtole/pebble-assist.h](https://gist.github.com/matthewtole/7699013) - Utility macros for Pebble development 
  * [picklepete/pyicloud](https://github.com/picklepete/pyicloud) - Python iCloud wrapper.