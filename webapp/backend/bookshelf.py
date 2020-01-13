#!/usr/bin/env python3
from flask import Flask, request
from flask_restful import Resource, Api
from flask_cors import CORS

from power import Bookshelf, Shelves, Shelf

app = Flask(__name__)
api = Api(app)
CORS(app)

api.add_resource(Bookshelf, '/')
api.add_resource(Shelves, '/shelves')
api.add_resource(Shelf, '/shelves/<shelfId>')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5002)

