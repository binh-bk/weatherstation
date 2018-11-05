# Binh Nguyen November 2018
# A simple FlaskApp to display data of a home weather station
from flask import Flask, render_template
import json
import plotly
import sqlite3
import pandas as pd

from plotly import tools

import logging
from datetime import datetime, timedelta

app = Flask(__name__)
# use full path first if you have problem with not found db
# dbFile = '/mnt/mqtt2/weather.db'
dbFile = '../capture_data/weather.db'


title_format = dict(
    family='Arial, sans-serif',
    size=20,
    color='navy'
    )

xy_format = dict(
    family='Arial, sans-serif',
    size=18,
    color='black'
    )

tickfont=dict(
    family='Arial, sans-serif',
    size=16,
    color='black')


class SqlReader:

    def getData(self, hours=24):
        hours = int(hours)
        conn = sqlite3.connect(dbFile)

        cursor = conn.cursor()
        # sql_command = """
        # SELECT * from  node3_post
        # ORDER BY time DESC        
        # LIMIT {};""".format(hours)
        
        sql_command = """SELECT * FROM node1_post 
        WHERE datetime(time, 'utc')  >= datetime('now','-0{} hours');""".format(hours) 
        df = pd.read_sql_query(sql_command, conn)
        '''make a band of +- 2 STD for a 95% confident level'''
        df['time'] = pd.to_datetime(df['time'])
        df['LDR_AVG'] = df['LDR_AVG']*100/1024
        df['LDR_STD'] = 2*df['LDR_STD']*100/1024
        df['LDR_AVG'] = df['LDR_AVG'].round(1)
        df['LDR_STD'] = df['LDR_STD'].round(1)

        df['TEM_STD'] = 2*df['TEM_STD']
        df['HUM_STD'] = 2*df['HUM_STD']
        cursor.close()
        return df

@app.route('/')
def test():
    title = 'Weather Station: Welcome to the Home WeatherStation'
    return render_template('chart.html', title=title)

@app.route('/light')
def light():
    sql = SqlReader()
    df = sql.getData()
    graphs = [
        dict(
            data=[
                dict(
                    x=df['time'],
                    y=df['LDR_AVG'],
                    type='scatter',
                    name='average',
                    line=dict(color='rgb(128,0,0)')
                ),
                dict(
                    name='+2&sigma;',
                    mode='lines',
                    x=df['time'],
                    y=df['LDR_AVG']+df['LDR_STD'],
                     marker=dict(color='444'),
                    line=dict(width=0),
                ),
                dict(
                    name='-2&sigma;',
                    x=df['time'],
                    y=df['LDR_AVG']-df['LDR_STD'],
                    mode='lines',
                    marker=dict(color='444'),
                    line=dict(width=0),
                    fillcolor='rgba(68, 68, 68, 0.25)',
                    fill='tonexty',
                ),
            ],
            layout=dict(
                title='Light Intensity (Analog Sensor)',
                height=600,
                titlefont=title_format,
                xaxis=dict(
                    title='Time',
                    titlefont=xy_format,
                    tickfont=tickfont,
                    ),
                yaxis=dict(
                    title='Percentage to the Maxium Measurable Value',
                    titlefont=xy_format,
                    tickfont=tickfont,
                )
            )
        ),
    ]
    ids = ['Graph {}'.format(i) for i, _ in enumerate(graphs)]
    graphJSON = json.dumps(graphs, cls=plotly.utils.PlotlyJSONEncoder)
    return render_template('chart.html',ids=ids,
                           graphJSON=graphJSON, title='Light')


@app.route('/temperature')
def temperature():
    sql = SqlReader()
    df = sql.getData()
    graphs = [
        dict(
            data=[
                dict(
                    x=df['time'],
                    y=df['TEM_AVG'],
                    type='scatter',
                    name='Average',
                    line=dict(color='rgb(0,0,0)'),
                    yaxis='y1',
                ),
                dict(
                    name='+2&sigma;',
                    x=df['time'],
                    y=df['TEM_AVG']+df['TEM_STD'],
                    mode='lines',
                    marker=dict(color='444'),
                    line=dict(width=0),
                    # fillcolor='rgba(68, 68, 68, 0.25)',
                    # fill='tonexty'
                ),
                dict(
                    name='-2&sigma;',
                    x=df['time'],
                    y=df['TEM_AVG']-df['TEM_STD'],
                    mode='lines',
                    marker=dict(color='444'),
                    line=dict(width=0),
                    fillcolor='rgba(68, 68, 68, 0.25)',
                    fill='tonexty',
                ),
            ],
            layout=dict(
                title='Outdoor Temperature',
                height=600,
                titlefont=title_format,
                xaxis=dict(
                    title='Time',
                    titlefont=xy_format,
                    tickfont=tickfont,
                    domain=[0.0, 1.00],
                    ),
                yaxis=dict(
                    title='Temperature (&#x2103;)',
                    titlefont=xy_format,
                    tickfont=tickfont,
                    # range=[min_temp, max_temp],
                    autorange=True,
                )
            )
        ),
    ]
    ids = ['Graph {}'.format(i) for i, _ in enumerate(graphs)]
    graphJSON = json.dumps(graphs, cls=plotly.utils.PlotlyJSONEncoder)
    return render_template('chart.html',ids=ids,
                           graphJSON=graphJSON, title='Temperature')


@app.route('/humidity')
def humidity():

    sql = SqlReader()
    df = sql.getData()
    graphs = [
        dict(
            data=[

                dict(
                    name='+2&sigma;',
                    x=df['time'],
                    y=df['HUM_AVG']+df['HUM_STD'],
                    mode='lines',
                    marker=dict(color='444'),
                    line=dict(width=0),
                    # fillcolor='rgba(68, 68, 68, 0.25)',
                    # fill='tonexty'
                ),
                dict(
                    name='-2&sigma;',
                    x=df['time'],
                    y=df['HUM_AVG']-df['HUM_STD'],
                    mode='lines',
                    line=dict(width=0),
                    marker=dict(color='444'),
                    fillcolor='rgba(68, 68, 68, 0.25)',
                    fill='tonexty',
                ),
                dict(
                    x=df['time'],
                    y=df['HUM_AVG'],
                    type='scatter',
                    name='Average',
                    line=dict(color='rgb(0,128,0)')
                ),
            ],
            layout=dict(
                title='Relative Humidity',
                height=600,
                titlefont=title_format,
                xaxis=dict(
                    title='Time',
                    titlefont=xy_format,
                    tickfont=tickfont,
                    domain=[0.0, 1.00],
                    ),
                yaxis=dict(
                    title='Relative Humidity (%)',
                    autorange=True,
                    titlefont=xy_format,
                    tickfont=tickfont,
                )
            )
        ),
    ]

    ids = ['Graph {}'.format(i) for i, _ in enumerate(graphs)]

    graphJSON = json.dumps(graphs, cls=plotly.utils.PlotlyJSONEncoder)
    return render_template('chart.html',ids=ids,
                           graphJSON=graphJSON, title='Humidity')


if __name__ == '__main__':
    app.run(debug=True, port=5000)