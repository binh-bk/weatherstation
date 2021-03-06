from flask import Flask, render_template
import json
import plotly
import sqlite3
from plotly import tools
import plotly.graph_objs as go

app = Flask(__name__)

class SqlReader:

    def getData(self):
        # connection = sqlite3.connect('/mnt/mqtt2/ws.db')
        connection = sqlite3.connect('weatherstation.db')
        cursor = connection.cursor()

        sql_command = """
        SELECT * from  weatherdata
        ORDER BY thetime DESC
        LIMIT 1000;"""

        cursor.execute(sql_command)
        result = cursor.fetchall()

        cursor.close()
        return result

@app.route('/')
def index():
    sql = SqlReader()
    result = sql.getData()

    numOfPoints = len(result)

    ldr = [yValues[2] for yValues in result]
    lux = [yValues[3] for yValues in result]
    ds18b20 = [yValues[4] for yValues in result]
    temperature = [yValues[5] for yValues in result]
    humidity = [yValues[6] for yValues in result]
    xValues = [xValues[1] for xValues in result]

    graphs = [
        dict(
            data=[
                dict(
                    x=xValues,
                    y=ldr,
                    type='scatter'
                ),
            ],
            layout=dict(
                title='LDR vs Time'
            )
        ),
        dict(
            data=[
                dict(
                    x=xValues,
                    y=lux,
                    type='scatter'
                ),
            ],
            layout=dict(
                title='Lux vs Time'
            )
        ),
        dict(
            data=[
                dict(
                    x=xValues,
                    y=ds18b20,
                    type='scatter'
                ),
            ],
            layout=dict(
                title='DS18B20 vs Time'
            )
        ),
        dict(
            data=[
                dict(
                    x=xValues,
                    y=temperature,
                    type='scatter'
                ),
            ],
            layout=dict(
                title='Temperature vs Time'
            )
        ),

        dict(
            data=[
                dict(
                    x=xValues,
                    y=humidity,
                    type='scatter'
                ),
            ],
            layout=dict(
                title='Humidity vs Time'
            )
        )
    ]

    # Add "ids" to each of the graphs to pass up to the client
    # for templating
    ids = ['Graph {}'.format(i) for i, _ in enumerate(graphs)]

    # Convert the figures to JSON
    # PlotlyJSONEncoder appropriately converts pandas, datetime, etc
    # objects to their JSON equivalents
    graphJSON = json.dumps(graphs, cls=plotly.utils.PlotlyJSONEncoder)

    return render_template('layouts/index.html',
                           ids=ids,
                           graphJSON=graphJSON)

if __name__ == '__main__':
    #app.run(host='localhost', port=9999, debug=True)
    app.run(host='0.0.0.0', port=9999)
