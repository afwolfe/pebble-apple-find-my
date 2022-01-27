import os 
import json

from flask import Flask, jsonify, request 
from pyicloud import PyiCloudService
app = Flask(__name__)


apple_username = os.environ.get("APPLE_USERNAME")
apple_password = os.environ.get("APPLE_PASSWORD")

if apple_username and not apple_password:
    icloud = PyiCloudService(apple_username)
elif apple_username and apple_password:
    icloud = PyiCloudService(apple_username, apple_password)
else:
    raise Exception("Neither APPLE_USERNAME nor APPLE_PASSWORD were set")


@app.route('/find', methods=['POST'])
def find():
    data = json.loads(request.data)
    if 'deviceId' in data:
        deviceId = data['deviceId']
    else:
        return '', 400
        
    print(f"/find {deviceId}")
    if (deviceId in icloud.devices.keys()):
        icloud.devices[deviceId].play_sound()
        return '', 200
    else:
        return '', 404

@app.route('/devices', methods=['GET'])
def list_devices():
    resp = []
    for device in icloud.devices:
        dev_dict = {}
        for key in device.keys():
            dev_dict[key] = device[key]
        resp.append(dev_dict)
    return jsonify(resp)

if __name__ == "__main__":
    app.run(host='0.0.0.0',port=8000)