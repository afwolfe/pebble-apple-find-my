from datetime import timedelta
import json
import logging
import os

from flask import Flask, jsonify, request
from flask_cors import CORS
from flask_jwt_extended import JWTManager, jwt_required, create_access_token, create_refresh_token, get_jwt_identity
from jwt import decode
from pyicloud import PyiCloudService
from werkzeug.security import generate_password_hash, check_password_hash


def create_app():
    app = Flask(__name__)
    CORS(app, resources='/findmy/api/*')
    JWTManager(app)

    app.config['JWT_SECRET_KEY'] = os.environ.get('JWT_SECRET_KEY')
    app.config['JWT_ACCESS_TOKEN_EXPIRES'] = timedelta(hours=1)
    app.config['JWT_REFRESH_TOKEN_EXPIRES'] = timedelta(days=30)

    if not app.config['JWT_SECRET_KEY']:
        raise Exception('Missing JWT_SECRET_KEY')

    api_user = os.environ.get('API_USERNAME')
    api_password = os.environ.get('API_PASSWORD')

    if not api_user and not api_password:
        raise Exception('Missing API credentials')

    # pyicloud initialization
    apple_username = os.environ.get('APPLE_USERNAME')
    apple_password = os.environ.get('APPLE_PASSWORD')

    if apple_username and not apple_password:
        icloud = PyiCloudService(apple_username)
    elif apple_username and apple_password:
        icloud = PyiCloudService(apple_username, apple_password)
    else:
        raise Exception('Neither APPLE_USERNAME nor APPLE_PASSWORD were set')

    users = {
        api_user: generate_password_hash(api_password)
    }


    @app.route('/findmy/api/v1/login', methods=['POST'])
    def login():
        if request.is_json:
            username = request.json['username']
            password = request.json['password']
        else:
            username = request.form['username']
            password = request.form['password']
        if username in users and check_password_hash(users.get(username), password):
            logging.info(f'{username} authenticated successfully.')
            access_token = create_access_token(identity=username)
            # decode in order to return expiration time
            access_expiration = decode(
                access_token,  app.config['JWT_SECRET_KEY'], app.config['JWT_ALGORITHM'])['exp']
            refresh_token = create_refresh_token(identity=username)
            refresh_expiration = decode(
                refresh_token,  app.config['JWT_SECRET_KEY'], app.config['JWT_ALGORITHM'])['exp']

            body = {
                'access': {
                    'token': access_token,
                    'expiration': access_expiration
                },
                'refresh': {
                    'token': refresh_token,
                    'expiration': refresh_expiration
                }
            }
            return json.dumps(body), 200
        else:
            return jsonify(status=401, message='Bad Email or Password'), 401


    @app.route('/findmy/api/v1/refresh', methods=['POST'])
    @jwt_required(refresh=True)
    def refresh():
        identity = get_jwt_identity()
        access_token = create_access_token(identity=identity)
        expiration = decode(
            access_token,  app.config['JWT_SECRET_KEY'], app.config['JWT_ALGORITHM'])['exp']
        return jsonify(token=access_token, expiration=expiration)


    @app.route('/findmy/api/v1/find', methods=['POST'])
    @jwt_required()
    def find():
        data = json.loads(request.data)
        if 'deviceId' in data:
            deviceId = data['deviceId']
        else:
            return json.dumps({'status': 400, 'message': 'deviceId missing from request'}), 400

        logging.info('/findmy/api/v1/find ' + deviceId)
        if (deviceId in icloud.devices.keys()):
            icloud.devices[deviceId].play_sound()
            return json.dumps({'status': 200, 'message': 'Successfully sent find request'}), 200
        else:
            return json.dumps({'status': 404, 'message': 'Device not found'}), 404


    @app.route('/findmy/api/v1/devices', methods=['GET'])
    @jwt_required()
    def list_devices():
        resp = []
        required_keys = [
            'batteryLevel',
            'batteryStatus',
            'deviceClass',
            'deviceStatus',
            'fmlyShare',
            'id',
            'name'
        ]

        # List comprehension to create a list of dictionaries containing the required keys
        # for each device in icloud.devices
        resp = [{k: device[k] for k in required_keys if k in device.keys()}
                for device in icloud.devices]
        return jsonify(resp)
    
    return app
