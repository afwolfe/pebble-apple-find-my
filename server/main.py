#!/usr/bin/env python3
import os
from gevent.pywsgi import WSGIServer

import find_my

server_port = int(os.environ.get('PORT')) if os.environ.get('PORT') else 8000
ssl_cert_file = os.environ.get('SSL_CERT_FILE')
ssl_key_file = os.environ.get('SSL_KEY_FILE')

if __name__ == '__main__':
    app = find_my.create_app()
    if ssl_cert_file and ssl_key_file:
        print('Starting in HTTPS mode.')
        server = WSGIServer(('', server_port), app,
                            certfile=ssl_cert_file, keyfile=ssl_key_file)
    else:
        print('Starting in HTTP mode.')
        server = WSGIServer(('', server_port), app)
    server.serve_forever()
