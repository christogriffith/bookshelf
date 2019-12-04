from flask import Flask
from flask import render_template
app = Flask(__name__)

import power

@app.route('/')
def all():
    return render_template('index.html')

