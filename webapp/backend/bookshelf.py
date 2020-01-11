#!/usr/bin/env python3
from flask import Flask, request
from flask_restful import Resource, Api

from power import Bookshelf

app = Flask(__name__)
api = Api(app)

api.add_resource(Bookshelf, '/', '/<int:halfid>')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5002)

